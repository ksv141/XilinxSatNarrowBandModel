#ifndef	CORRELATORDPDIMANCHESTER_H
#define CORRELATORDPDIMANCHESTER_H

#include <deque>
#include <vector>
#include <stdexcept>
#include <algorithm>

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
	 * @param baud_mul ����������� ���������� ������� ��������, �� ������� �������� ���������� (� ����������� - 4B/8B/16B...)
	*/
	CorrelatorDPDIManchester(int8_t* preamble_data, uint16_t preamble_length,
		uint16_t M, uint16_t L, uint32_t burst_est, uint16_t baud_mul = 4);

	/**
	 * @brief ������ �������
	 * @param in ������� ������
	 * @param corr_val ������� �������������� ������ (��������������� ������ ��� ������������ ������)
	 * @param max_corr_pos ������� ������������� ������� � �������� m_corrValuesReg
	 * @param cur_est ������� �������� ��������������� ������� (������������ ��� �������)
	 * @return ���� (true) ��� ��� (false) ������������ ������
	*/
	bool freqEstimate(const xip_complex& in, xip_complex& corr_val, int& max_corr_pos, xip_real& cur_est);

	/**
	 * @brief ����� ������������� �������� ������� ���������� � �������� m_corrMnchReg
	 * @param corr_val �������� ��������������� �������, ��������������� ���������� �������������
	 * @param max_corr_pos ������� ������������� ������� � �������� m_corrValuesReg
	 * @return ������������ ������� ����������
	*/
	xip_real getMaxCorrVal(xip_complex& corr_val, int& max_corr_pos);

	/**
	 * @brief ������ ������� �� ��������������� �������
	 * @param corr_val ������� �������������� ������
	 * @return (����� ���� �� ������) --> [-8192, 8192]
	*/
	int16_t countFreq(const xip_complex& corr_val);

	/**
	 * @brief ��� ������������
	 * @param in 
	 * @param cur_corr 
	 * @param cur_est 
	*/
	void test_corr(const xip_complex& in, xip_real& est, xip_real& dph);

	/**
	 * @brief ���������� ����� �����������
	*/
	deque<xip_complex>& getBuffer();

private:
	/**
	 * @brief ������������� ��������� �����������
	*/
	void init(int8_t* preamble_data, uint16_t preamble_length);

	/**
	 * @brief ���������� ����������
	 * @param in ������� ������
	*/
	void process(const xip_complex& in);

	deque<xip_complex> m_correlationReg;     // FIFO-������� ��� ���������� ����������
	vector<xip_complex> m_preamble;          // ������� � ���������� (� ���� ����������-����������� �����)

	deque<xip_complex> m_corr;				// ������� ���������� �� N ����������� ��� �������� �������� ������������ (������ ����� m_baudMul)
	deque<xip_complex> m_corrSumValuesReg;	// FIFO-������� ��� �������� ��������� �������������� �������� (��� ������ �������)
	deque<xip_real> m_corrMnchReg;			// FIFO-������� ��� ������� ���� �������������� �������� ������������ �������������� ����

	uint16_t m_preambleLength;           // ������ ���������
	uint16_t m_correlatorM;              // ������ ���������� �����������
	uint16_t m_correlatorL;              // ���������� ��������� ������������
	xip_real m_burstEstML;               // ��������� �������� ��� ��������������� ������� (�������� ������������� �������������)
	uint16_t m_baudMul;					 // ����������� ���������� ������� ��������, �� ������� �������� ����������
										 // ��� ���������� - 2B, � ����������� - 4B/8B/16B...
	unsigned m_sumMnchStep;				 // ��� ��������� ���������� ����������� ������������� �������� (2/4/8...)

	uint16_t m_argShift;				 // �������� �������� ������ ��� ���������� v = Arg{x}/(m_baudMul*M) (� ������ 2B � ����������)
};

#endif // CORRELATORDPDIMANCHESTER_H