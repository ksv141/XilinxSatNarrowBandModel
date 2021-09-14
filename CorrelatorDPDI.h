#ifndef	CORRELATORDPDI_H
#define CORRELATORDPDI_H

#include <deque>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "xip_utils.h"
#include "debug.h"

using namespace std;

/**
 * @brief ���������� ��� ������� ������������� Fs = 2*Fd
 * DPDI detector (Theory of Code and Frame Synchronization
 * with Applications to Advanced Wireless Communication Systems, p. 125)
*/
class CorrelatorDPDI
{
public:
	CorrelatorDPDI(uint16_t data_length, int8_t* preamble_data, uint16_t M, uint16_t L, uint16_t F, xip_real burst_est);

private:
	deque<xip_complex> m_correlationReg;     // ������� ��� ���������� ����������

    uint16_t m_dataLength;               // ������ ������ � �����, �� ������� ���������
    uint16_t m_correlatorM;              // ������ ���������� �����������
    uint16_t m_correlatorL;              // ���������� ��������� ������������
    uint16_t m_correlatorF;              // ���������� ������������ ML ��� ���������� ��������������� �������
    xip_real m_burstEstML;               // ��������� �������� ��� ��������������� ������� (�������� ������������� �������������)
};

#endif // CORRELATORDPDI_H