#ifndef LAGRANGEINTERP_H
#define LAGRANGEINTERP_H

#include <bitset>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"
#include "xip_utils.h"

using namespace std;

extern const int LAGRANGE_INTERVALS;

// ������������ �������� 7-� �������. ������������ ����� ������ � ������� �����������������
// �������� ����� ����� �������� ��������� ������� �� 1024 ����������, 
// ��� ������� �� ������� ������������ ���� ������� ������������ ����� �� 8 �������������
class LagrangeInterp {
public:
	// frac - ��������� ������� ������������� ��������� ������� �� ��������
	// ���� ����������������� �� ���������, �� frac = 1
	LagrangeInterp(xip_real frac = 1);

	~LagrangeInterp();

	// ��������� �������� ������ [-1, +1]
	void setShift(xip_real shift);

	// ���������� ��������� ������
	void process(const xip_complex& in);

	// �������� ��������� ������. true - ���� ������, false - ��� ������� (��������� ������ �� ���� ���������)
	bool next(xip_complex& out);

	// ���������� ������� �������� ��� ������� ������������ �� ���������
	void process(xip_real shift);

	/**
	 * @brief ��� ������������ �������������
	 * @param in ����
	 * @param out �����
	 * @param time_shift ����� � ��������� [-1023, 1023]
	*/
	void process(const xip_complex& in, xip_complex& out, int time_shift);

	uint32_t getPos();
	/**
	 * @brief ���������� �������� ������ �� [-inf, +inf] � ���� [0, 1023]
	 * @param cur_shift ������� �����
	 * @return 
	*/
	uint32_t countPos(int cur_shift);

private:
	int init_lagrange_interp();
	int lagrange_load_coeff();
	int process_sample_lagrange_interp(const xip_complex& in, xip_complex& out, uint32_t pos);
	int destroy_lagrange_interp();

	const uint32_t lagrange_n_intervals = LAGRANGE_INTERVALS;		// ���������� ����������
	const uint32_t lagrange_n_coeff = 8;							// ���������� �������������
	double* lagrange_coeff;											// ������ ������������� �������, ������� �� �������

	xip_real m_fraction;          // ��������� ������� ������������� ��������� ������� �� ��������
	xip_real m_dk;                // ������� ������� ������������ [0; 1]
	int m_decim;				  // ������� ������������ ��������
	uint32_t m_pos;			  // ������� �����
	xip_real m_prevShift;         // ���������� �����
	xip_complex m_currentSample;  // ������� ������� ������

	xip_fir_v7_2* lagrange_interp_fir;		// ������ �� ��������
	xip_fir_v7_2_config lagrange_interp_fir_cnfg;

	xip_array_uint* lagrange_interp_fir_fsel;
	xip_fir_v7_2_cnfg_packet lagrange_interp_fir_cnfg_packet;

	xip_array_real* lagrange_interp_in;		// 3-D ������, ���������� ������� ������ ��� ���������
	xip_array_real* lagrange_interp_out;	// 3-D ������, ���������� ��������� ���������
};

#endif // LAGRANGEINTERP_H