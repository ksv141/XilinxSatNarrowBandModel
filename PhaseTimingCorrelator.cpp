#include "PhaseTimingCorrelator.h"

PhaseTimingCorrelator::PhaseTimingCorrelator(int8_t* preamble_data, uint16_t preamble_length, uint32_t burst_est):
	m_preambleLength(preamble_length),
	m_burstEstML(burst_est)
{
	init(preamble_data, preamble_length);
    m_timingSyncReg.resize(4, 0);
}

bool PhaseTimingCorrelator::process(xip_complex in, int16_t& phase, xip_real& time_shift, xip_real& phase_est)
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

    xip_real re_sqr = 0;
    xip_multiply_real(rx_ph.re, rx_ph.re, re_sqr);
    xip_real im_sqr = 0;
    xip_multiply_real(rx_ph.im, rx_ph.im, im_sqr);
    xip_real est = re_sqr + im_sqr;                   // јбсолютное значение коррел€ционного отклика

    phase_est = est;

    xip_real rxx = std::max(rx_ph.re, rx_ph.im);

    m_timingSyncReg.pop_back();
    m_timingSyncReg.push_front(rxx);

    if (est > m_burstEstML) {
        xip_complex rt{ m_timingSyncReg[2] - m_timingSyncReg[0], m_timingSyncReg[3] - m_timingSyncReg[1] };

        xip_real mag;
        xip_real arg;
        xip_cordic_rect_to_polar(rt, mag, arg);
        time_shift = arg;    //mu_est = 2.0f / static_cast<float>(M_PI_2) * std::arg(rt);

        xip_cordic_rect_to_polar(rx_ph, mag, arg);
        phase = (int16_t)arg;
        return true;
    }
    else {
        return false;
    }
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
