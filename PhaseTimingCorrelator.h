#ifndef	PHASETIMINGCORRELATOR_H
#define PHASETIMINGCORRELATOR_H

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

/**
 * @brief ���������� ��� ������� � ���������� ������������� �� ���������
 * (A Fast Synchronizer for Burst Modems with Simultaneous Symbol Timing and Carrier Phase Estimations Dengwei Fu and Alan N. Willson, Jr.)
*/
class PhaseTimingCorrelator
{
public:
	/**
	 * @brief ������������� �����������
	 * @param preamble_data ���������
	 * @param preamble_length ������ ���������
	 * @param burst_est ��������� �������� ��� ��������������� ������� (�������� ������������� �������������)
	*/
	PhaseTimingCorrelator(int8_t* preamble_data, uint16_t preamble_length, uint32_t burst_est);

	/**
	 * @brief ������ �������� ������
	 * @param in ������� ������
	 * @param phase ������ ������ ���� --> [-8192, 8192]
	 * @param phase_est ������� �������������� ������ (������������ ��� �������)
	 * @return ���� (true) ��� ��� (false) ������������ ������ �����������
	*/
	bool phaseEstimate(xip_complex in, int16_t& phase, xip_real& phase_est);

	/**
	 * @brief ������ ��������� ������
	 * @param in ������� ������
	 * @param time_shift ������ ������ ������
	 * @param time_est ������� �������������� ������ (������������ ��� �������)
	 * @return ���� (true) ��� ��� (false) ������������ ������� ��� ������ ������
	*/
	bool symbolTimingEstimate(xip_complex in, int16_t& time_shift, xip_real& time_est);

	/**
	 * @brief ���������� ������� ����� �����������
	 * @return 
	*/
	bool isPhaseEstMode();

	/**
	 * @brief ���������� ����� ������� ������� �������� �����
	 * @return 
	*/
	uint8_t getSymbolTimingProcCounter();

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

	deque<xip_complex> m_correlationReg;     // FIFO-������� ��� ���������� ����������
	deque<xip_real> m_timingSyncReg;		 // FIFO-������� ��� �������� �������������
	vector<xip_complex> m_preamble;          // ������� � ���������� (� ���� ����������-����������� �����)
	xip_complex m_currentCorrelation;		 // ������� �������� � ������ �����������
	bool m_phaseEstMode;					 // ����� ������ �������� ������ (������������ ��� ������������ � ����� ������ ������)
	uint8_t m_symbolTimingProcCounter;		 // ������� ������� ������� �������� ����� (������� ����� �� �����������)

	uint16_t m_preambleLength;           // ������ ���������
	xip_real m_burstEstML;               // ��������� �������� ��� ��������������� ������� (�������� ������������� �������������)
};

#endif // PHASETIMINGCORRELATOR_H