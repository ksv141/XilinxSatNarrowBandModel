#ifndef	PHASETIMINGCORRELATOR_H
#define PHASETIMINGCORRELATOR_H

#include <cstdint>
#include <deque>
#include <vector>
#include <stdexcept>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "xip_utils.h"
#include "debug.h"
#include "constellation.h"

using namespace std;

/**
 * @brief ���������� ��� ������� � ���������� ������������� �� ���������
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
	 * @brief ������ �������� � ��������� ������
	 * @param in ������� ������
	 * @param phase ������ ������ ���� --> [-8192, 8192] 
	 * @param time_shift ������ ������ ������
	 * @param phase_est ������� �������������� ������ (������������ ��� �������)
	 * @return ���� (true) ��� ��� (false) ������������ ������ �����������
	*/
	bool process(xip_complex in, int16_t& phase, xip_real& time_shift, xip_real& phase_est);

private:
	/**
	 * @brief ������������� ��������� �����������
	*/
	void init(int8_t* preamble_data, uint16_t preamble_length);

	deque<xip_complex> m_correlationReg;     // FIFO-������� ��� ���������� ����������
	deque<xip_real> m_timingSyncReg;		 // FIFO-������� ��� �������� �������������
	vector<xip_complex> m_preamble;          // ������� � ���������� (� ���� ����������-����������� �����)

	uint16_t m_preambleLength;           // ������ ���������
	xip_real m_burstEstML;               // ��������� �������� ��� ��������������� ������� (�������� ������������� �������������)
};

#endif // PHASETIMINGCORRELATOR_H