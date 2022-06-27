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
    m_phaseEstMode = true;              // ������� � ����� ������ ����, ���� ���������� � ��� �� ���������
    process(in);

    if (m_corrMnchReg.back() > m_burstEstML) {      // ��� ���������� ������ ���� ������������ �� 4-� ��������, ����� ��������� ������ ��������
        deque<xip_real>::iterator max_it;
        max_it = std::max_element(m_corrMnchReg.begin(), m_corrMnchReg.end());
        phase_est = *max_it;
        m_maxCorrPos = std::distance(m_corrMnchReg.begin(), max_it);
        xip_real mag;
        xip_real arg;
        xip_complex max_sample = m_corrSumValuesReg[m_maxCorrPos];
        xip_cordic_rect_to_polar(max_sample, mag, arg);  // �������� ����
        phase = (int16_t)arg;
        std::fill(m_corrMnchReg.begin(), m_corrMnchReg.end(), 0);
        std::fill(m_corrSumValuesReg.begin(), m_corrSumValuesReg.end(), xip_complex{0,0});

        m_symbolTimingProcCounter = 2;
        m_phaseEstMode = false;         // ������������ � ����� ������ ������

        return true;
    }

    return false;
}

bool PhClcCorrelatorManchester::symbolTimingEstimate(xip_complex in, int16_t& time_shift, xip_real& time_est)
{
    process(in);
    time_est = m_timingSyncReg[0];
    m_symbolTimingProcCounter--;

    // ��� ������� ������� � �������� ������ ���� �����������
    //if ((m_timingSyncReg[0] > m_timingSyncReg[1]) || (m_timingSyncReg[0] > m_timingSyncReg[2]) ||
    //    (m_timingSyncReg[3] > m_timingSyncReg[1]) || (m_timingSyncReg[3] > m_timingSyncReg[2]))
    //    return false;

    xip_complex rt{ m_timingSyncReg[2] - m_timingSyncReg[0], m_timingSyncReg[3] - m_timingSyncReg[1] };

    xip_real mag;
    xip_real arg;
    xip_cordic_rect_to_polar(rt, mag, arg);     //mu_est = 2.0f / static_cast<float>(M_PI_2) * std::arg(rt);
    xip_real_shift(arg, -2); // !!!!!! ��������� ��� DDS_PHASE_MODULUS = 16384 � LAGRANGE_INTERVALS = 1024 !!!!
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

    // ��������� ������� ����������� ����������-����������� ����������
    for (int i = 0; i < preamble_length; i++) {
        xip_complex p = get_cur_constell_sample(preamble_data[i]);
        p.im = -p.im;
        m_preamble[i] = p;
    }

    m_correlationReg.resize(m_preamble.size() * 4, xip_complex{ 0, 0 });    // ������ �������� ����� ������� ��������� *4 
                                                                            // � ������ ��������� ������� �������� � ����������
    m_corrValuesReg.resize(4, xip_complex{ 0, 0 });     // ������ 4 ��� ���������� ���� ���������� ������������ �������������� ����
    m_corrSumValuesReg.resize(6, xip_complex{ 0, 0 });     // ������ 4 ��� ���������� ���� ���������� ������������ �������������� ����
    m_corrMnchReg.resize(6, 0);                         // ������ 4 ��� ������ ������������� ������� � ������ ��������� ������ ����������
    m_timingSyncReg.resize(8, 0);                       // ������ 8 ��� �������� ������������� �� 4 ������ � ������ ����������
}

void PhClcCorrelatorManchester::process(xip_complex in)
{
    // �������� ������ � FIFO
    m_correlationReg.pop_back();
    m_correlationReg.push_front(in);

    xip_complex rx_ph{ 0, 0 };
    // FIFO-������� ������� � �����
    deque<xip_complex>::reverse_iterator reg_it = m_correlationReg.rbegin();
    for (const xip_complex& p : m_preamble) {
        xip_complex res;
        xip_multiply_complex(p, *reg_it, res);  // ��������� ������ rm*cm [-2^28, +2^28]
        xip_complex_shift(res, -16);            // �������� �� [-2^12, +2^12]
        rx_ph.re += res.re;
        rx_ph.im += res.im;

        reg_it += 4;    // ��������� ������ [-2^16, +2^16]
    }

    m_corrValuesReg.pop_back();
    m_corrValuesReg.push_front(rx_ph);

    xip_complex sum_corr;
    sum_corr.re = m_corrValuesReg[2].re - m_corrValuesReg[0].re;
    sum_corr.im = m_corrValuesReg[2].im - m_corrValuesReg[0].im;
    m_corrSumValuesReg.pop_back();
    m_corrSumValuesReg.push_front(sum_corr);

    // ��������� ������� ������� � ���������� � ������� ��� ������ ���������
    xip_complex sum_corr_conj{ sum_corr.re, -sum_corr.im };
    xip_complex sum_corr_pwr;
    xip_multiply_complex(sum_corr, sum_corr_conj, sum_corr_pwr);
    xip_real_shift(sum_corr_pwr.re, -16);            // �������� �� [-2^16, +2^16]
    m_corrMnchReg.pop_back();
    m_corrMnchReg.push_front(sum_corr_pwr.re);

    // ���������� ��������� ������ � ������� ������ ������ ������
    xip_real rxx = std::max(fabs(sum_corr.re), fabs(sum_corr.im));
    m_timingSyncReg.pop_back();
    m_timingSyncReg.push_front(rxx);
}
