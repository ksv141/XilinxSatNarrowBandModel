#include "CorrelatorDPDI.h"

CorrelatorDPDI::CorrelatorDPDI(uint16_t data_length, int8_t* preamble_data, uint16_t preamble_length, 
								uint16_t M, uint16_t L, uint16_t F, xip_real burst_est):
	m_dataLength(data_length),
	m_preambleLength(preamble_length),
	m_correlatorM(M),
	m_correlatorL(L),
	m_correlatorF(F),
	m_burstEstML(burst_est)
{
	init(preamble_data, preamble_length);
}

bool CorrelatorDPDI::process(xip_complex in, xip_real& dph, xip_real& cur_est)
{
    // �������� ������ � FIFO
    m_correlationReg.pop_back();
    m_correlationReg.push_front(in);

    xip_complex rx_1{ 0, 0 };                           // ������� �������� ���������� �����������
    xip_complex reg_1{ 0, 0 };                          // ���������� �������� ���������� �����������
    xip_complex SumL_1{ 0, 0 };                         // ����� �� L ���������������� ���������� (z)
    xip_complex SumL_2{ 0, 0 };                         // ����� �� L ���������������� ����������, ��������� �� 1 ���� (z)

    // FIFO-������� ������� � �����
    deque<xip_complex>::reverse_iterator reg_it = m_correlationReg.rbegin();

    for (int k = 0; k < m_correlatorF; k++) {           // ���������� ��������������� ������� ��� F ��������
        vector<xip_complex>::iterator preamb_it = m_preamble.begin();
        for (int i = 0; i < m_correlatorL; i++) {       // ���������� ��������������� ������� z ��� ����� ���������
            rx_1 = xip_complex{ 0, 0 };
            for (int j = 0; j < m_correlatorM; j++) {   // ���������� �������� xk ��� ���������� �����������
                xip_complex res;
                if (xip_multiply_complex(*preamb_it, *reg_it, res) == -1) // rm*cm ��� ������ ��������� ��������
                    return false;
                rx_1.re += res.re;
                rx_1.im += res.im;

                reg_it += 2;
                preamb_it++;
            }
            xip_complex res;
            xip_complex reg_1_conj{ reg_1.re, -reg_1.im };
            if (xip_multiply_complex(rx_1, reg_1_conj, res) == -1)  // ���������� z ��� ������
                return false;
            SumL_1.re += res.re;
            SumL_1.im += res.im;

            reg_1 = rx_1;                               // �������� �� ���� ��� ������
        }
        reg_it += 2 * m_dataLength;                     // �������� � ���������� ������ � ��������
    }

    SumL_2 = m_prev_sum_1;

    xip_real re_sqr = 0;
    xip_multiply_real(SumL_1.re, SumL_1.re, re_sqr);
    xip_real im_sqr = 0;
    xip_multiply_real(SumL_1.im, SumL_1.im, im_sqr);
    xip_real est_1 = re_sqr + im_sqr;                   // ���������� �������� ��������������� ������� ��� ������

    xip_multiply_real(SumL_2.re, SumL_2.re, re_sqr);
    xip_multiply_real(SumL_2.im, SumL_2.im, im_sqr);
    xip_real est_2 = re_sqr + im_sqr;                   // ���������� �������� ��������������� ������� ��� ��������

    xip_real est;
    xip_complex maxSum;
    if (est_1 >= est_2) {           // ����� ����������� �������� z �� ������� � ���������
        est = est_1;
        maxSum = SumL_1;
    }
    else {
        est = est_2;
        maxSum = SumL_2;
    }

    m_prev_sum_1 = SumL_1;          // ���������� ����� ���������� �� ������� �����
    cur_est = est;

    if (est > m_burstEstML) {       // ����� � ������������ � ��������� ������������� �������������
        //dph = std::arg(maxSum) / (2 * m_correlatorM);   // ������ �������� �� ������� (v = Arg{x}/2M), � ���/����
        return true;
    }
    else {
        return false;
    }
}

void CorrelatorDPDI::init(int8_t* preamble_data, uint16_t preamble_length)
{
    if (preamble_length < m_correlatorM * m_correlatorL)
        throw runtime_error("Preamble length incorrect");

    m_preamble.clear();

    // ��������� ������� ����������� ����������-����������� ����������
    for (int i = 0; i < preamble_length; i++) {
        xip_complex p = get_cur_constell_preamble_sample(preamble_data[i]);
        p.im = -p.im;
        m_preamble.push_back(p);
    }

    // ������ �������� ����� ������� ������ *2 � ������ ��������� ������� �������� � ���������� ������������
    m_correlationReg.resize((m_dataLength + m_preamble.size()) * m_correlatorF * 2, xip_complex{ 0, 0 });
}
