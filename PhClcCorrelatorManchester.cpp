#include "PhClcCorrelatorManchester.h"

PhClcCorrelatorManchester::PhClcCorrelatorManchester(int8_t* preamble_data, uint16_t preamble_length, uint32_t burst_est):
	m_preambleLength(preamble_length),
	m_burstEstML(burst_est)
{
	init(preamble_data, preamble_length);
	m_phaseEstMode = true;
}

bool PhClcCorrelatorManchester::phaseEstimate(xip_complex in, int16_t& phase, xip_real& phase_est)
{
    m_phaseEstMode = true;              // переход в режим оценки фазы, если коррел€тор в нем не находилс€
    process(in);

    if (m_corrMnchReg.back() > m_burstEstML) {      // при превышении порога ищем максимальный из 4-х отсчетов, чтобы исключить ложный максимум
        deque<xip_real>::iterator max_it;
        max_it = std::max_element(m_corrMnchReg.begin(), m_corrMnchReg.end());
        phase_est = *max_it;
        m_maxCorrPos = std::distance(m_corrMnchReg.begin(), max_it);
        xip_real mag;
        xip_real arg;
        xip_complex max_sample = m_corrSumValuesReg[m_maxCorrPos];
        xip_cordic_rect_to_polar(max_sample, mag, arg);  // получаем фазу
        phase = (int16_t)arg;
        std::fill(m_corrMnchReg.begin(), m_corrMnchReg.end(), 0);
        std::fill(m_corrSumValuesReg.begin(), m_corrSumValuesReg.end(), xip_complex{0,0});

        m_symbolTimingProcCounter = 2;
        m_phaseEstMode = false;         // переключение в режим оценки тактов

        return true;
    }

    return false;
}

bool PhClcCorrelatorManchester::symbolTimingEstimate(xip_complex in, int16_t& time_shift, xip_real& time_est)
{
    process(in);
    time_est = m_timingSyncReg[0];
    m_symbolTimingProcCounter--;

    // два средних отсчета в регистре должны быть наибольшими
    //if ((m_timingSyncReg[0] > m_timingSyncReg[1]) || (m_timingSyncReg[0] > m_timingSyncReg[2]) ||
    //    (m_timingSyncReg[3] > m_timingSyncReg[1]) || (m_timingSyncReg[3] > m_timingSyncReg[2]))
    //    return false;

    xip_complex rt{ m_timingSyncReg[2] - m_timingSyncReg[0], m_timingSyncReg[3] - m_timingSyncReg[1] };

    xip_real mag;
    xip_real arg;
    xip_cordic_rect_to_polar(rt, mag, arg);     //mu_est = 2.0f / static_cast<float>(M_PI_2) * std::arg(rt);
    xip_real_shift(arg, -2); // !!!!!! подобрано дл€ DDS_PHASE_MODULUS = 16384 и LAGRANGE_INTERVALS = 1024 !!!!
    time_shift = (int16_t)arg;

    return true;
}

bool PhClcCorrelatorManchester::isPhaseEstMode()
{
    return m_phaseEstMode;
}

uint8_t PhClcCorrelatorManchester::getSymbolTimingProcCounter()
{
    return m_symbolTimingProcCounter;
}

void PhClcCorrelatorManchester::test_corr(xip_complex in, xip_complex& corr, xip_real& est)
{
    process(in);
    est = m_corrMnchReg[0];
    corr = m_corrValuesReg[0];
}

int PhClcCorrelatorManchester::getMaxCorrPos()
{
    return m_maxCorrPos;
}

void PhClcCorrelatorManchester::init(int8_t* preamble_data, uint16_t preamble_length)
{
    m_preamble.resize(preamble_length);

    // заполн€ем регистр коррел€тора комплексно-сопр€женной преамбулой
    for (int i = 0; i < preamble_length; i++) {
        xip_complex p = get_cur_constell_sample(preamble_data[i]);
        p.im = -p.im;
        m_preamble[i] = p;
    }

    m_correlationReg.resize(m_preamble.size() * 4, xip_complex{ 0, 0 });    // размер регистра равен размеру преамбулы *4 
                                                                            // с учетом удвоенной бодовой скорости и манчестера
    m_corrValuesReg.resize(4, xip_complex{ 0, 0 });     // размер 4 дл€ вычислени€ сумм коррел€ции полусимволов манчестерского кода
    m_corrSumValuesReg.resize(6, xip_complex{ 0, 0 });     // размер 4 дл€ вычислени€ сумм коррел€ции полусимволов манчестерского кода
    m_corrMnchReg.resize(6, 0);                         // размер 4 дл€ поиска максимального отклика с учетом возможных ложных максимумов
    m_timingSyncReg.resize(8, 0);                       // размер 8 дл€ тактовой синхронизации по 4 точкам с учетом манчестера
}

void PhClcCorrelatorManchester::process(xip_complex in)
{
    // помещаем отсчет в FIFO
    m_correlationReg.pop_back();
    m_correlationReg.push_front(in);

    xip_complex rx_ph{ 0, 0 };
    // FIFO-регистр обходим с конца
    deque<xip_complex>::reverse_iterator reg_it = m_correlationReg.rbegin();
    for (const xip_complex& p : m_preamble) {
        xip_complex res;
        xip_multiply_complex(p, *reg_it, res);  // единичный отклик rm*cm [-2^28, +2^28]
        xip_complex_shift(res, -16);            // сдвигаем до [-2^12, +2^12]
        rx_ph.re += res.re;
        rx_ph.im += res.im;

        reg_it += 4;    // суммарный отклик [-2^16, +2^16]
    }

    m_corrValuesReg.pop_back();
    m_corrValuesReg.push_front(rx_ph);

    xip_complex sum_corr;
    sum_corr.re = m_corrValuesReg[2].re - m_corrValuesReg[0].re;
    sum_corr.im = m_corrValuesReg[2].im - m_corrValuesReg[0].im;
    m_corrSumValuesReg.pop_back();
    m_corrSumValuesReg.push_front(sum_corr);

    // вычисл€ем энергию отклика и продвигаем в регистр дл€ поиска максимума
    xip_complex sum_corr_conj{ sum_corr.re, -sum_corr.im };
    xip_complex sum_corr_pwr;
    xip_multiply_complex(sum_corr, sum_corr_conj, sum_corr_pwr);
    xip_real_shift(sum_corr_pwr.re, -16);            // сдвигаем до [-2^16, +2^16]
    m_corrMnchReg.pop_back();
    m_corrMnchReg.push_front(sum_corr_pwr.re);

    // ѕродвигаем суммарный отклик в регистр оценки сдвига тактов
    xip_real rxx = std::max(fabs(sum_corr.re), fabs(sum_corr.im));
    m_timingSyncReg.pop_back();
    m_timingSyncReg.push_front(rxx);
}
