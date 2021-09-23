#ifndef	CORRELATORDPDI_H
#define CORRELATORDPDI_H

#include <deque>
#include <vector>
#include <stdexcept>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "xip_utils.h"
#include "debug.h"
#include "constellation.h"

using namespace std;

/**
 * @brief ���������� ��� ������� ������������� Fs = 2*Fd
 * DPDI detector (Theory of Code and Frame Synchronization
 * with Applications to Advanced Wireless Communication Systems, p. 125)
*/
class CorrelatorDPDI
{
public:
	/**
	 * @brief ������������� �����������
	 * @param data_length ������ ������ � �����, �� ������� ���������
	 * @param preamble_data ���������
	 * @param preamble_length ������ ���������
	 * @param M ������ ���������� �����������
	 * @param L ���������� ��������� ������������
	 * @param F ���������� ������������ ML ��� ���������� ��������������� �������
	 * @param burst_est ��������� �������� ��� ��������������� ������� (�������� ������������� �������������)
	*/
	CorrelatorDPDI(uint16_t data_length, int8_t* preamble_data, uint16_t preamble_length,
					uint16_t M, uint16_t L, uint16_t F, uint32_t burst_est);

	/**
	 * @brief ������ �������
	 * @param in ������� ������
	 * @param dph ������ ������� (����� ���� �� ������) --> [-8192, 8192] 
	 * ������ �������������� ������ ��� ���������� ������
	 * @param cur_est ������� �������������� ������ (������������ ��� �������),
	 * @return ���� (true) ��� ��� (false) ������������ ������
	*/
	bool process(xip_complex in, int16_t& dph, xip_real& cur_est);

	/**
	 * @brief ������ ������� ������� �� ������� ������ (��������, � ������ �����������)
	 * @param samples ������� ����� ��������
	 * @param dph ������ ������� (����� ���� �� ������) --> [-8192, 8192]
	 * ������ �������������� ������ ��� ���������� ������
	 * @return ���� (true) ��� ��� (false) ������������ ������
	*/
	bool processBuffer(const deque<xip_complex>& samples, int16_t& dph);

	/**
	 * @brief ���������� ����� �����������
	*/
	deque<xip_complex>& getBuffer();

private:
	/**
	 * @brief ������������� ��������� �����������
	*/
	void init(int8_t* preamble_data, uint16_t preamble_length);

	deque<xip_complex> m_correlationReg;     // FIFO-������� ��� ���������� ����������
	vector<xip_complex> m_preamble;          // ������� � ���������� (� ���� ����������-����������� �����)
	xip_complex m_prev_sum_1{ 0, 0 };		 // ����� ���������� �� ���������� �����

    uint16_t m_dataLength;               // ������ ������ � �����, �� ������� ���������
	uint16_t m_preambleLength;           // ������ ���������
	uint16_t m_correlatorM;              // ������ ���������� �����������
    uint16_t m_correlatorL;              // ���������� ��������� ������������
    uint16_t m_correlatorF;              // ���������� ������������ ML ��� ���������� ��������������� �������
    xip_real m_burstEstML;               // ��������� �������� ��� ��������������� ������� (�������� ������������� �������������)

	uint16_t m_argShift;				 // �������� �������� ������ ��� ���������� v = Arg{x}/2M
};

#endif // CORRELATORDPDI_H