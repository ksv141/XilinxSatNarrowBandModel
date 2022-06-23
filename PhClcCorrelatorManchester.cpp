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
    m_phaseEstMode = true;              // переход в режим оценки фазы, если коррелятор в нем не находился
    process(in);

    xip_real mag_1;
    xip_real arg_1;
    xip_cordic_rect_to_polar(m_sumCorr_1, mag_1, arg_1);  // получаем ампилутуду и фазу
    xip_real mag_2;
    xip_real arg_2;
    xip_cordic_rect_to_polar(m_sumCorr_2, mag_2, arg_2);  // получаем ампилутуду и фазу

    xip_real est = mag_1;
    xip_real est_ph = arg_1;
    //if (mag_1 >= mag_2) {           // Выбор наибольшего значения из сдвинутых на такт 2B
    //    est = mag_1;
    //    est_ph = arg_1;
    //}
    //else {
    //    est = mag_2;
    //    est_ph = arg_2;
    //}

    phase_est = est;

    if (est > m_burstEstML) {
        phase = (int16_t)est_ph;
        m_phaseEstMode = false;         // переключение в режим оценки тактов
        m_symbolTimingProcCounter = 2;
        return true;
    }

    return false;
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
    xip_real mag;
    xip_real arg;
    xip_cordic_rect_to_polar(m_sumCorr_1, mag, arg);
    corr = m_sumCorr_1;
    est = mag;
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

    // размер регистра равен размеру преамбулы *4 с учетом удвоенной бодовой скорости и манчестера
    m_correlationReg.resize(m_preamble.size() * 4, xip_complex{ 0, 0 });
    m_corrValuesReg.resize(5, xip_complex{ 0, 0 });
    m_timingSyncReg.resize(8, 0);
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

        reg_it += 4;
    }

    m_corrValuesReg.pop_back();
    m_corrValuesReg.push_front(rx_ph);

    m_sumCorr_1.re = m_corrValuesReg[2].re - m_corrValuesReg[0].re;
    m_sumCorr_1.im = m_corrValuesReg[2].im - m_corrValuesReg[0].im;
    m_sumCorr_2.re = m_corrValuesReg[4].re - m_corrValuesReg[2].re;
    m_sumCorr_2.im = m_corrValuesReg[4].im - m_corrValuesReg[2].im;

    // Продвигаем суммарный отклик в регистр оценки сдвига тактов
    xip_real rxx = std::max(fabs(m_sumCorr_1.re), fabs(m_sumCorr_1.im));
    m_timingSyncReg.pop_back();
    m_timingSyncReg.push_front(rxx);
}
