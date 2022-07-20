#include "CorrelatorDPDIManchester.h"

CorrelatorDPDIManchester::CorrelatorDPDIManchester(int8_t* preamble_data, uint16_t preamble_length, 
	uint16_t M, uint16_t L, uint32_t burst_est, uint16_t baud_mul):
	m_preambleLength(preamble_length),
	m_correlatorM(M),
	m_correlatorL(L),
	m_burstEstML(burst_est),
	m_baudMul(baud_mul)
{
	m_argShift = static_cast<uint16_t>(log2(baud_mul * M));
    m_sumMnchStep = baud_mul / 2;
	init(preamble_data, preamble_length);
}

bool CorrelatorDPDIManchester::freqEstimate(const xip_complex& in, xip_complex& corr_val, int& max_corr_pos, xip_real& cur_est)
{
    process(in);

    if (m_corrMnchReg.back() > m_burstEstML) {      // ��� ���������� ������ ���� ������������ �� 4-� ��������, ����� ��������� ������ ��������
        deque<xip_real>::iterator max_it;
        max_it = std::max_element(m_corrMnchReg.begin(), m_corrMnchReg.end());
        cur_est = *max_it;
        int pos = std::distance(m_corrMnchReg.begin(), max_it);    // ������� ������������� �������
        corr_val = m_corrSumValuesReg[pos];
        max_corr_pos = pos;

        // �������� ��������, ���� ����� ��������� ����� ����������
        std::fill(m_corrMnchReg.begin(), m_corrMnchReg.end(), 0);
        std::fill(m_corrSumValuesReg.begin(), m_corrSumValuesReg.end(), xip_complex{ 0,0 });

        return true;
    }

    return false;
}

xip_real CorrelatorDPDIManchester::getMaxCorrVal(xip_complex& corr_val, int& max_corr_pos)
{
    deque<xip_real>::iterator max_it;
    max_it = std::max_element(m_corrMnchReg.begin(), m_corrMnchReg.end());
    xip_real cur_est = *max_it;
    int pos = std::distance(m_corrMnchReg.begin(), max_it);    // ������� ������������� �������
    corr_val = m_corrSumValuesReg[pos];
    max_corr_pos = pos;
    return cur_est;
}

int16_t CorrelatorDPDIManchester::countFreq(const xip_complex& corr_val)
{
    xip_real mag;
    xip_real arg;
    xip_cordic_rect_to_polar(corr_val, mag, arg);
    return static_cast<int16_t>((int)arg >> m_argShift);
}

void CorrelatorDPDIManchester::test_corr(const xip_complex& in, xip_real& est, xip_real& dph)
{
    // �������� ������ � FIFO
    m_correlationReg.pop_back();
    m_correlationReg.push_front(in);

    xip_complex rx{ 0, 0 };                           // ������� �������� ���������� �����������
    xip_complex prev_rx{ 0, 0 };                      // ���������� �������� ���������� �����������
    xip_complex SumL{ 0, 0 };                         // ����� �� L ���������������� ���������� (z)

    // FIFO-������� ������� � �����
    deque<xip_complex>::reverse_iterator reg_it = m_correlationReg.rbegin();

    vector<xip_complex>::iterator preamb_it = m_preamble.begin();
    for (int i = 0; i < m_correlatorL; i++) {       // ���������� ��������������� ������� z �� L ������������
        rx = xip_complex{ 0, 0 };
        for (int j = 0; j < m_correlatorM; j++) {   // ���������� �������� xk ��� ���������� �����������
            xip_complex res;
            xip_multiply_complex(*preamb_it, *reg_it, res); // ��������� ������ rm*cm [-2^28, +2^28]
            //xip_complex_shift(res, -16);            // �������� �� [-2^12, +2^12]
			xip_complex_shift(res, -14);            // �������� �� [-2^12, +2^12]
			rx.re += res.re;
            rx.im += res.im;

            reg_it += m_baudMul;
            preamb_it++;
        }
        xip_complex res;
        xip_complex reg_1_conj{ prev_rx.re, -prev_rx.im };
        xip_multiply_complex(rx, reg_1_conj, res);  // ���������� z
        // ����� �������� [-2^27, +2^27]
        xip_complex_shift(res, -11);                // �������� �� [-2^16, +2^16]
        SumL.re += res.re;
        SumL.im += res.im;

        prev_rx = rx;                               // ���������� ���������� �������� ���������� �����������
    }

	m_corr.push_front(SumL);
	m_corr.pop_back();

    xip_complex sum_mnch{ m_corr[0].re + m_corr[m_sumMnchStep].re,  m_corr[0].im + m_corr[m_sumMnchStep].im };

    // ��������� ������� ������� � ���������� � ������� ��� ������ ���������
    xip_complex sum_corr_conj{ sum_mnch.re, -sum_mnch.im };
    xip_complex sum_corr_pwr;
    xip_multiply_complex(sum_mnch, sum_corr_conj, sum_corr_pwr);
    xip_real_shift(sum_corr_pwr.re, -16);            // �������� �� [-2^16, +2^16]
    m_corrMnchReg.pop_back();
    m_corrMnchReg.push_front(sum_corr_pwr.re);

    est = sum_corr_pwr.re;

    //xip_real mag;
    //xip_real arg;
    //xip_cordic_rect_to_polar(sum_mnch, mag, arg);
    //est = mag;
    //dph = (int)arg >> m_argShift;
}

deque<xip_complex>& CorrelatorDPDIManchester::getBuffer()
{
    return m_correlationReg;
}

void CorrelatorDPDIManchester::init(int8_t* preamble_data, uint16_t preamble_length)
{
    if (preamble_length < m_correlatorM * m_correlatorL)
        throw runtime_error("Preamble length incorrect");

    m_preamble.resize(preamble_length);

    // ��������� ������� ����������� ����������-����������� ����������
    for (int i = 0; i < preamble_length; i++) {
        xip_complex p = get_cur_constell_sample(preamble_data[i]);
        p.im = -p.im;
        m_preamble[i] = p;
    }

    // ������ �������� ����� ������� ��������� � ���������� ������������ ������� ��������
    m_correlationReg.resize(m_preamble.size() * m_baudMul + m_baudMul, xip_complex{ 0, 0 });

	// ������ �������� ����� ��������� ����������� ������������ ������� ��������
	m_corr.resize(m_baudMul, xip_complex{ 0, 0 });

    // ������ �������� ��� ������ ������������� ������� � ������ ������� ������ ���������� (��������������)
    m_corrMnchReg.resize(m_baudMul, 0);
    m_corrSumValuesReg.resize(m_baudMul, xip_complex{ 0, 0 });
}

void CorrelatorDPDIManchester::process(const xip_complex& in)
{
    // �������� ������ � FIFO
    m_correlationReg.pop_back();
    m_correlationReg.push_front(in);

    xip_complex rx{ 0, 0 };                           // ������� �������� ���������� �����������
    xip_complex prev_rx{ 0, 0 };                      // ���������� �������� ���������� �����������
    xip_complex SumL{ 0, 0 };                         // ����� �� L ���������������� ���������� (z)

    // FIFO-������� ������� � �����
    deque<xip_complex>::reverse_iterator reg_it = m_correlationReg.rbegin() + m_baudMul;

    vector<xip_complex>::iterator preamb_it = m_preamble.begin();
    for (int i = 0; i < m_correlatorL; i++) {       // ���������� ��������������� ������� z �� L ������������
        rx = xip_complex{ 0, 0 };
        for (int j = 0; j < m_correlatorM; j++) {   // ���������� �������� xk ��� ���������� �����������
            xip_complex res;
            xip_multiply_complex(*preamb_it, *reg_it, res); // ��������� ������ rm*cm [-2^28, +2^28]
            //xip_complex_shift(res, -16);            // �������� �� [-2^12, +2^12]
            xip_complex_shift(res, -14);            // �������� �� [-2^12, +2^12]
            rx.re += res.re;
            rx.im += res.im;

            reg_it += m_baudMul;
            preamb_it++;
        }
        xip_complex res;
        xip_complex prev_rx_conj{ prev_rx.re, -prev_rx.im };
        xip_multiply_complex(rx, prev_rx_conj, res);  // ���������� z
        // ����� �������� [-2^27, +2^27]
        xip_complex_shift(res, -11);                // �������� �� [-2^16, +2^16]
        SumL.re += res.re;
        SumL.im += res.im;

        prev_rx = rx;                               // ���������� ���������� �������� ���������� �����������
    }

    m_corr.push_front(SumL);                        // ������ 4 ��������� �������� �������
    m_corr.pop_back();

    // ��������� ������ ��� 2-� ������� ������������� �������
    xip_complex sum_mnch{ m_corr[0].re + m_corr[m_sumMnchStep].re,  m_corr[0].im + m_corr[m_sumMnchStep].im };

    // ���������� � ������� ��� ����������� ������ �������
    m_corrSumValuesReg.pop_back();
    m_corrSumValuesReg.push_front(sum_mnch);

    // ��������� ������� ������� � ���������� � ������� ��� ������ ���������
    xip_complex sum_corr_conj{ sum_mnch.re, -sum_mnch.im };
    xip_complex sum_corr_pwr;
    xip_multiply_complex(sum_mnch, sum_corr_conj, sum_corr_pwr);
    xip_real_shift(sum_corr_pwr.re, -16);            // �������� �� [-2^16, +2^16]
    m_corrMnchReg.pop_back();
    m_corrMnchReg.push_front(sum_corr_pwr.re);
}
