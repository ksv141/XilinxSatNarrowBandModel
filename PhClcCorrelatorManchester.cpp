#include "PhClcCorrelatorManchester.h"

PhClcCorrelatorManchester::PhClcCorrelatorManchester(int8_t* preamble_data, uint16_t preamble_length, uint32_t burst_est):
	m_preambleLength(preamble_length),
	m_burstEstML(burst_est)
{
	init(preamble_data, preamble_length);
}

bool PhClcCorrelatorManchester::phaseEstimate(xip_complex in, int16_t& phase, int16_t& time_shift, xip_real& phase_est)
{
    process(in);

    if (m_corrMnchReg.back() > m_burstEstML) {      // при превышении порога ищем максимальный из 4-х отсчетов, чтобы исключить ложный максимум
        deque<xip_real>::iterator max_it;
        max_it = std::max_element(m_corrMnchReg.begin(), m_corrMnchReg.end());
        phase_est = *max_it;
        m_maxCorrPos = std::distance(m_corrMnchReg.begin(), max_it);    // позиция максимального отклика

        // оценка фазы максимального отклика
        xip_real mag;
        xip_real arg;
        xip_complex max_sample = m_corrSumValuesReg[m_maxCorrPos];
        xip_cordic_rect_to_polar(max_sample, mag, arg);  // получаем фазу
        phase = (int16_t)arg;

        // оценка тактового сдвига по 4 точкам относительно максимального отклика с учетом манчестера
        //int rx0_pos = m_maxCorrPos;
        //int rx_1_pos = rx0_pos + 1;
        //int rx1_pos = rx0_pos - 1;
        //int rx2_pos = rx1_pos - 1;
        //xip_complex rt{ m_timingSyncReg[rx0_pos] - m_timingSyncReg[rx2_pos], m_timingSyncReg[rx_1_pos] - m_timingSyncReg[rx1_pos] };
        //xip_cordic_rect_to_polar(rt, mag, arg);     //mu_est = 2.0f / static_cast<float>(M_PI_2) * std::arg(rt);
        //xip_real_shift(arg, -2); // !!!!!! подобрано для DDS_PHASE_MODULUS = 16384 и LAGRANGE_INTERVALS = 1024 !!!!
        //time_shift = (int16_t)arg;

        // обнуляем регистры, если нужно повторить поиск корреляции
        std::fill(m_corrMnchReg.begin(), m_corrMnchReg.end(), 0);
        std::fill(m_corrSumValuesReg.begin(), m_corrSumValuesReg.end(), xip_complex{0,0});

        return true;
    }

    return false;
}

void PhClcCorrelatorManchester::test_corr(xip_complex in, xip_complex& corr, xip_real& est)
{
    process(in);
    est = m_timingSyncReg[0];
    corr = m_corrValuesReg[0];
}

int PhClcCorrelatorManchester::getMaxCorrPos()
{
    return m_maxCorrPos;
}

deque<xip_complex>& PhClcCorrelatorManchester::getCorrelationReg()
{
    return m_correlationReg;
}

void PhClcCorrelatorManchester::init(int8_t* preamble_data, uint16_t preamble_length)
{
    m_preamble.resize(preamble_length);

    // заполняем регистр коррелятора комплексно-сопряженной преамбулой
    for (int i = 0; i < preamble_length; i++) {
        xip_complex p = get_cur_constell_sample(preamble_data[i]);
        p.im = -p.im;
        m_preamble[i] = p;
    }

    m_correlationReg.resize(m_preamble.size() * 4 + m_corrRegSize, xip_complex{ 0, 0 });    // размер регистра равен размеру преамбулы *4 
                                                                            // с учетом удвоенной бодовой скорости и манчестера
    m_corrValuesReg.resize(3, xip_complex{ 0, 0 });     // размер подобран для вычисления сумм корреляции полусимволов манчестерского кода
    m_corrSumValuesReg.resize(m_corrRegSize, xip_complex{ 0, 0 });
    m_corrMnchReg.resize(m_corrRegSize, 0);                         // размер подобран для поиска максимального отклика с учетом возможных ложных максимумов
    m_timingSyncReg.resize(m_corrRegSize, 0);                       // размер подобран для тактовой синхронизации по 4 точкам с учетом манчестера
}

void PhClcCorrelatorManchester::process(xip_complex in)
{
    // помещаем отсчет в FIFO
    m_correlationReg.pop_back();
    m_correlationReg.push_front(in);

    xip_complex rx_ph{ 0, 0 };
    // FIFO-регистр обходим с конца
    deque<xip_complex>::reverse_iterator reg_it = m_correlationReg.rbegin() + m_corrRegSize;
    for (const xip_complex& p : m_preamble) {
        xip_complex res;
        xip_multiply_complex(p, *reg_it, res);  // единичный отклик rm*cm [-2^28, +2^28]
        xip_complex_shift(res, -16);            // сдвигаем до [-2^12, +2^12]
        rx_ph.re += res.re;                     // суммарный отклик [-2^16, +2^16]
        rx_ph.im += res.im;

        reg_it += 4;
    }

    m_corrValuesReg.pop_back();
    m_corrValuesReg.push_front(rx_ph);

    xip_complex sum_corr;
    sum_corr.re = m_corrValuesReg[2].re - m_corrValuesReg[0].re;
    sum_corr.im = m_corrValuesReg[2].im - m_corrValuesReg[0].im;
    m_corrSumValuesReg.pop_back();
    m_corrSumValuesReg.push_front(sum_corr);

    // вычисляем энергию отклика и продвигаем в регистр для поиска максимума
    xip_complex sum_corr_conj{ sum_corr.re, -sum_corr.im };
    xip_complex sum_corr_pwr;
    xip_multiply_complex(sum_corr, sum_corr_conj, sum_corr_pwr);
    xip_real_shift(sum_corr_pwr.re, -16);            // сдвигаем до [-2^16, +2^16]
    m_corrMnchReg.pop_back();
    m_corrMnchReg.push_front(sum_corr_pwr.re);

    // Продвигаем суммарный отклик в регистр оценки сдвига тактов
    //xip_real rxx = std::max(fabs(sum_corr.re), fabs(sum_corr.im));
    xip_real rxx = std::max(fabs(m_corrValuesReg[2].re), fabs(m_corrValuesReg[2].im));
    m_timingSyncReg.pop_back();
    m_timingSyncReg.push_front(rxx);
}
