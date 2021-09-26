#include "PhaseTimingCorrelator.h"

PhaseTimingCorrelator::PhaseTimingCorrelator(int8_t* preamble_data, uint16_t preamble_length, uint32_t burst_est):
	m_preambleLength(preamble_length),
	m_burstEstML(burst_est)
{
	init(preamble_data, preamble_length);
    m_timingSyncReg.resize(4, 0);
    m_phaseEstMode = true;
}

void PhaseTimingCorrelator::process(xip_complex in)
{
    // помещаем отсчет в FIFO
    m_correlationReg.pop_back();
    m_correlationReg.push_front(in);

    xip_complex rx_ph{ 0, 0 };
    // FIFO-регистр обходим с конца
    deque<xip_complex>::reverse_iterator reg_it = m_correlationReg.rbegin();
    for (const xip_complex& p : m_preamble) {
        xip_complex res;
        xip_multiply_complex(p, *reg_it, res);
        // единичный отклик [-2^26, +2^26]
        xip_complex_shift(res, -16);            // сдвигаем до [-2^10, +2^10]
        rx_ph.re += res.re;
        rx_ph.im += res.im;

        reg_it += 2;
    }

    m_currentCorrelation = rx_ph;

    // ѕродвигаем коррел€цию в регистр оценки сдвига тактов
    xip_real rxx = (fabs(rx_ph.re) > fabs(rx_ph.im)) ? rx_ph.re : rx_ph.im;
    m_timingSyncReg.pop_back();
    m_timingSyncReg.push_front(rxx);
}

bool PhaseTimingCorrelator::phaseEstimate(xip_complex in, int16_t& phase, xip_real& phase_est)
{
    m_phaseEstMode = true;              // переход в режим оценки фазы, если коррел€тор в нем не находилс€
    process(in);

    xip_real re_sqr = 0;
    xip_multiply_real(m_currentCorrelation.re, m_currentCorrelation.re, re_sqr);	// --> [-2^20, +2^20]
    xip_real_shift(re_sqr, -14);        // сдвигаем до [-2^10, +2^10]
    xip_real im_sqr = 0;
    xip_multiply_real(m_currentCorrelation.im, m_currentCorrelation.im, im_sqr);
    xip_real_shift(im_sqr, -14);		// сдвигаем до [-2^10, +2^10]
    xip_real est = re_sqr + im_sqr;     // значение коррел€ционного отклика
    phase_est = est;

    if (est > m_burstEstML) {
        xip_real mag;
        xip_real arg;
        xip_cordic_rect_to_polar(m_currentCorrelation, mag, arg);
        phase = (int16_t)arg;
        m_phaseEstMode = false;         // переключение в режим оценки тактов
        m_symbolTimingProcCounter = 2;
        return true;
    }

    return false;
}

bool PhaseTimingCorrelator::symbolTimingEstimate(xip_complex in, xip_real& time_shift, xip_real& time_est)
{
    process(in);
    time_est = m_timingSyncReg[0];
    m_symbolTimingProcCounter--;

    // два средних отсчета в регистре должны быть наибольшими
    if ((m_timingSyncReg[0] > m_timingSyncReg[1]) || (m_timingSyncReg[0] > m_timingSyncReg[2]) ||
        (m_timingSyncReg[3] > m_timingSyncReg[1]) || (m_timingSyncReg[3] > m_timingSyncReg[2]))
        return false;

    xip_complex rt{ m_timingSyncReg[2] - m_timingSyncReg[0], m_timingSyncReg[3] - m_timingSyncReg[1] };

    xip_real mag;
    xip_real arg;
    xip_cordic_rect_to_polar(rt, mag, arg);
    time_shift = arg;    //mu_est = 2.0f / static_cast<float>(M_PI_2) * std::arg(rt);

    return true;
}

bool PhaseTimingCorrelator::isPhaseEstMode()
{
    return m_phaseEstMode;
}

uint8_t PhaseTimingCorrelator::getSymbolTimingProcCounter()
{
    return m_symbolTimingProcCounter;
}

void PhaseTimingCorrelator::init(int8_t* preamble_data, uint16_t preamble_length)
{
    m_preamble.clear();

    // заполн€ем регистр коррел€тора комплексно-сопр€женной преамбулой
    for (int i = 0; i < preamble_length; i++) {
        xip_complex p = get_cur_constell_preamble_sample(preamble_data[i]);
        p.im = -p.im;
        m_preamble.push_back(p);
    }

    // размер регистра равен размеру преамбулы*2 с учетом удвоенной бодовой скорости
    m_correlationReg.resize(m_preamble.size() * 2, xip_complex{ 0, 0 });
}
