#ifndef	CORRELATORDPDIMANCHESTER_H
#define CORRELATORDPDIMANCHESTER_H

#include <deque>
#include <vector>
#include <stdexcept>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "xip_utils.h"
#include "debug.h"
#include "constellation.h"

using namespace std;

/**
 * @brief ���������� ��� �������������� ���� �� 4B
 * DPDI detector (Theory of Code and Frame Synchronization
 * with Applications to Advanced Wireless Communication Systems, p. 125)
*/
class CorrelatorDPDIManchester
{
public:
	/**
	 * @brief ������������� �����������
	 * @param preamble_data ���������
	 * @param preamble_length ������ ���������
	 * @param M ������ ���������� �����������
	 * @param L ���������� ��������� ������������
	 * @param burst_est ��������� �������� ��� ��������������� ������� (�������� ������������� �������������)
	*/
	CorrelatorDPDIManchester(int8_t* preamble_data, uint16_t preamble_length,
		uint16_t M, uint16_t L, uint32_t burst_est, uint16_t baud_mul);

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
	 * @brief ��� ������������
	 * @param in 
	 * @param cur_corr 
	 * @param cur_est 
	*/
	void test_corr(xip_complex in, xip_real* est, xip_real* dph);

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

	xip_complex m_corr_1{ 0, 0 };			// ���������� �� 4-� �����������
	xip_complex m_corr_2{ 0, 0 };
	xip_complex m_corr_3{ 0, 0 };
	xip_complex m_corr_4{ 0, 0 };

	deque<xip_complex> m_corr;				// ������� ���������� �� N ����������� (������ ����� m_baudMul)

	uint16_t m_preambleLength;           // ������ ���������
	uint16_t m_correlatorM;              // ������ ���������� �����������
	uint16_t m_correlatorL;              // ���������� ��������� ������������
	xip_real m_burstEstML;               // ��������� �������� ��� ��������������� ������� (�������� ������������� �������������)
	uint16_t m_baudMul;					 // ����������� ���������� ������� ��������, �� ������� �������� ����������
										 // ��� ���������� - 2B, � ����������� - 4B/8B/16B...

	uint16_t m_argShift;				 // �������� �������� ������ ��� ���������� v = Arg{x}/4M (� ������ 2B � ����������)
};

#endif // CORRELATORDPDIMANCHESTER_H