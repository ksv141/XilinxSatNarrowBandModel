#ifndef	PHCLCCORRELATORMANCHESTER_H
#define PHCLCCORRELATORMANCHESTER_H

#include <cstdint>
#include <deque>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "xip_utils.h"
#include "debug.h"
#include "constellation.h"

using namespace std;

extern const unsigned int LAGRANGE_INTERVALS;

/**
 * @brief ���������� ��� ������� � ���������� ������������� �� ��������� ��� �������������� ���� �� 4B
 * ������ �������� ������ �� 4 ������
 * (A Fast Synchronizer for Burst Modems with Simultaneous Symbol Timing and Carrier Phase Estimations Dengwei Fu and Alan N. Willson, Jr.)
*/
class PhClcCorrelatorManchester
{
public:
	/**
	 * @brief ������������� �����������
	 * @param preamble_data ���������
	 * @param preamble_length ������ ���������
	 * @param burst_est ��������� �������� ��� ��������������� ������� (�������� ������������� �������������)
	*/
	PhClcCorrelatorManchester(int8_t* preamble_data, uint16_t preamble_length, uint32_t burst_est);

	/**
	 * @brief ������ �������� ������
	 * @param in ������� ������
	 * @param phase ������ ������ ���� --> [-8192, 8192]
	 * @param time_shift ������ ������ ������
	 * @param phase_est ������� �������������� ������ (������������ ��� �������)
	 * @return ���� (true) ��� ��� (false) ������������ ������ �����������
	*/
	bool phaseEstimate(xip_complex in, int16_t& phase, int16_t& time_shift, xip_real& phase_est);

	/**
	 * @brief ��� ������������
	 * @param in 
	 * @param corr 
	 * @param est 
	*/
	void test_corr(xip_complex in, xip_complex& corr, xip_real& est);

	/**
	 * @brief ��� ������������
	 * @return 
	*/
	int getMaxCorrPos();

	/**
	 * @brief ���������� ����� ����������� �������� (��� ����������� �����������)
	 * @return 
	*/
	deque<xip_complex>& getCorrelationReg();

private:
	/**
	 * @brief ������������� ��������� �����������
	*/
	void init(int8_t* preamble_data, uint16_t preamble_length);

	/**
	 * @brief ���������� ����������
	 * @param in ������� ������
	*/
	void process(xip_complex in);

	const unsigned m_corrRegSize = 4;		 // ������� ��������� ��� ���������� ���� � ��������� ������

	deque<xip_complex> m_correlationReg;     // FIFO-������� ��� ���������� ����������
	deque<xip_complex> m_corrValuesReg;		 // FIFO-������� ��� �������� �������������� �������� (��� ������������ ������������)
	deque<xip_complex> m_corrSumValuesReg;		 // FIFO-������� ��� �������� ��������� �������������� �������� (��� ������ ����)
	deque<xip_real> m_timingSyncReg;		 // FIFO-������� ��� �������� �������������
	deque<xip_real> m_corrMnchReg;			 // FIFO-������� ��� ���� �������������� �������� ������������ �������������� ����
	vector<xip_complex> m_preamble;          // ������� � ���������� (� ���� ����������-����������� �����)
	int m_maxCorrPos;						 // ������� ������������� ������� � �������� m_corrValuesReg

	uint16_t m_preambleLength;           // ������ ���������
	xip_real m_burstEstML;               // ��������� �������� ��� ��������������� ������� (�������� ������������� �������������)
};

#endif // PHCLCCORRELATORMANCHESTER_H