#include "CorrelatorDPDIManchester.h"

CorrelatorDPDIManchester::CorrelatorDPDIManchester(int8_t* preamble_data, uint16_t preamble_length, 
	uint16_t M, uint16_t L, uint32_t burst_est):
	m_preambleLength(preamble_length),
	m_correlatorM(M),
	m_correlatorL(L),
	m_burstEstML(burst_est)
{
	m_argShift = static_cast<uint16_t>(log2(4 * M));     // ������� ��������� ��� ������ M
	init(preamble_data, preamble_length);
}

bool CorrelatorDPDIManchester::process(xip_complex in, int16_t& dph, xip_real& cur_est)
{
    // �������� ������ � FIFO
    m_correlationReg.pop_back();
    m_correlationReg.push_front(in);

    xip_complex rx_1{ 0, 0 };                           // ������� �������� ���������� �����������
    xip_complex reg_1{ 0, 0 };                          // ���������� �������� ���������� �����������
    xip_complex SumL_1{ 0, 0 };                         // ����� �� L ���������������� ���������� (z)

    // FIFO-������� ������� � �����
    deque<xip_complex>::reverse_iterator reg_it = m_correlationReg.rbegin();

	vector<xip_complex>::iterator preamb_it = m_preamble.begin();
	for (int i = 0; i < m_correlatorL; i++) {       // ���������� ��������������� ������� z �� L ������������
		rx_1 = xip_complex{ 0, 0 };
		for (int j = 0; j < m_correlatorM; j++) {   // ���������� �������� xk ��� ���������� �����������
			xip_complex res;
			xip_multiply_complex(*preamb_it, *reg_it, res); // ��������� ������ rm*cm [-2^28, +2^28]
			xip_complex_shift(res, -16);            // �������� �� [-2^12, +2^12]
			rx_1.re += res.re;
			rx_1.im += res.im;

			reg_it += 4;
			preamb_it++;
		}
		xip_complex res;
		xip_complex reg_1_conj{ reg_1.re, -reg_1.im };
		xip_multiply_complex(rx_1, reg_1_conj, res);  // ���������� z
		// ����� �������� [-2^27, +2^27]
		xip_complex_shift(res, -11);                // �������� �� [-2^16, +2^16]
		SumL_1.re += res.re;
		SumL_1.im += res.im;

		reg_1 = rx_1;                               // ���������� ���������� �������� ���������� �����������
	}

    m_corr_4 = m_corr_3;                            // ������ 4 ��������� �������� �������
    m_corr_3 = m_corr_2;
    m_corr_2 = m_corr_1;
    m_corr_1 = SumL_1;

    xip_complex sum_1{ m_corr_1.re + m_corr_3.re,  m_corr_1.im + m_corr_3.im };     // ��������� ������ ��� 2-� ������� ������������� �������
    xip_complex sum_2{ m_corr_2.re + m_corr_4.re,  m_corr_2.im + m_corr_4.im };     // ��������� ������ ��� 2-� ������� ������������� ������� �� ������� �� ���� �� 2B

    xip_real mag_1;
    xip_real arg_1;
    xip_cordic_rect_to_polar(sum_1, mag_1, arg_1);  // �������� ���������� � ����
    xip_real mag_2;
    xip_real arg_2;
    xip_cordic_rect_to_polar(sum_2, mag_2, arg_2);  // �������� ���������� � ����

    xip_real est;
    xip_real est_dph;
    if (mag_1 >= mag_2) {           // ����� ����������� �������� �� ��������� �� ���� 2B
        est = mag_1;
        est_dph = arg_1;
    }
    else {
        est = mag_2;
        est_dph = arg_2;
    }

    cur_est = est;

    if (est > m_burstEstML) {       // ����� � ������������ � ��������� ������������� �������������
        // ������ �������� �� ������� (v = Arg{x}/4M), � ���/����
        dph = (int)est_dph >> m_argShift;
        return true;
    }
    else {
        return false;
    }
}

void CorrelatorDPDIManchester::test_corr(xip_complex in, xip_complex* corr, xip_real* est, xip_real* dph)
{
    // �������� ������ � FIFO
    m_correlationReg.pop_back();
    m_correlationReg.push_front(in);

    xip_complex rx_1{ 0, 0 };                           // ������� �������� ���������� �����������
    xip_complex reg_1{ 0, 0 };                          // ���������� �������� ���������� �����������
    xip_complex SumL_1{ 0, 0 };                         // ����� �� L ���������������� ���������� (z)

    // FIFO-������� ������� � �����
    deque<xip_complex>::reverse_iterator reg_it = m_correlationReg.rbegin();

    vector<xip_complex>::iterator preamb_it = m_preamble.begin();
    for (int i = 0; i < m_correlatorL; i++) {       // ���������� ��������������� ������� z �� L ������������
        rx_1 = xip_complex{ 0, 0 };
        for (int j = 0; j < m_correlatorM; j++) {   // ���������� �������� xk ��� ���������� �����������
            xip_complex res;
            xip_multiply_complex(*preamb_it, *reg_it, res); // ��������� ������ rm*cm [-2^28, +2^28]
            xip_complex_shift(res, -16);            // �������� �� [-2^12, +2^12]
			//xip_complex_shift(res, -12);            // �������� �� [-2^12, +2^12]
			rx_1.re += res.re;
            rx_1.im += res.im;

            reg_it += 4;
            preamb_it++;
        }
        xip_complex res;
        xip_complex reg_1_conj{ reg_1.re, -reg_1.im };
        xip_multiply_complex(rx_1, reg_1_conj, res);  // ���������� z
        // ����� �������� [-2^27, +2^27]
        xip_complex_shift(res, -11);                // �������� �� [-2^16, +2^16]
        SumL_1.re += res.re;
        SumL_1.im += res.im;

        reg_1 = rx_1;                               // ���������� ���������� �������� ���������� �����������
    }

    m_corr_4 = m_corr_3;
    m_corr_3 = m_corr_2;
    m_corr_2 = m_corr_1;
    m_corr_1 = SumL_1;

    xip_complex sum_1{ m_corr_1.re + m_corr_3.re,  m_corr_1.im + m_corr_3.im };
    xip_complex sum_2{ m_corr_2.re + m_corr_4.re,  m_corr_2.im + m_corr_4.im };

    corr[0] = m_corr_1;
    corr[1] = m_corr_2;
    corr[2] = m_corr_3;
    corr[3] = m_corr_4;
    corr[4] = sum_1;
    corr[5] = sum_2;

    xip_real mag;
    xip_real arg;
    //xip_cordic_rect_to_polar(m_corr_1, mag, arg);
    //est[0] = mag;
    //dph[0] = (int)arg >> m_argShift;
    //xip_cordic_rect_to_polar(m_corr_2, mag, arg);
    //est[1] = mag;
    //dph[1] = (int)arg >> m_argShift;
    //xip_cordic_rect_to_polar(m_corr_3, mag, arg);
    //est[2] = mag;
    //dph[2] = (int)arg >> m_argShift;
    //xip_cordic_rect_to_polar(m_corr_4, mag, arg);
    //est[3] = mag;
    //dph[3] = (int)arg >> m_argShift;
    xip_cordic_rect_to_polar(sum_1, mag, arg);
    est[4] = mag;
    dph[4] = (int)arg >> m_argShift;
    //xip_cordic_rect_to_polar(sum_2, mag, arg);
    //est[5] = mag;
    //dph[5] = (int)arg >> m_argShift;

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

    // ������ �������� ����� ������� ��������� *4 � ������ ��������� ������� �������� � ����������
    m_correlationReg.resize(m_preamble.size() * 4, xip_complex{ 0, 0 });
}
