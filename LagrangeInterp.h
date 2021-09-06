#ifndef LAGRANGEINTERP_H
#define LAGRANGEINTERP_H

#include <bitset>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <vector>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"
#include "xip_utils.h"

using namespace std;

extern const unsigned int LAGRANGE_INTERVALS;
extern const unsigned int LAGRANGE_ORDER;
extern const unsigned int LAGRANGE_FIXED_POINT_POSITION;

// ������������ �������� 7-� �������. ������������ ����� ������ � ������� �����������������
// �������� ����� ����� �������� ��������� ������� �� 1024 ����������, 
// ��� ������� �� ������� ������������ ���� ������� ������������ ����� �� 8 �������������
class LagrangeInterp {
public:
	// frac - ��������� ������� ������������� �������� ������� � ��������� --> [0.001, 1000]
	// ���� ����������������� �� ���������, �� frac = 1 
	LagrangeInterp(xip_real frac = 1);

	LagrangeInterp(xip_real from_sampling_freq, xip_real to_sampling_freq);

	~LagrangeInterp();

	/**
	 * @brief ��������� ���������� �������. � ����� �������� ����������� ��������� ������ (FIFO)
	 * @param in 
	*/
	void process(const xip_complex& in);

	/**
	 * @brief �������� ��������� ������. true - ���� ������, false - ��� ������� (��������� ������ �� ���� ���������)
	 * @param out 
	 * @return 
	*/
	bool next(xip_complex& out);
	
	/**
	 * @brief ���������� �������� ������ ������������ �������� ��������. ������������ ������
	 * @param value -> [-1.0, 1.0] ������������ �������� ��������, ������������ ���������� value � ������� [-1.0, 1.0]
	*/
	void shift(double value);

	/**
	 * @brief ���������� �������� ������ ������������ �������� ��������. ������������� ������
	 * @param value -> [-2^FixPointPosition, 2^FixPointPosition] ������������ �������� ��������, 
	 * ������������ ���������� value � ��������� ��������
	*/
	void shift(int32_t value);

private:
	/**
	 * @brief ������������� ���������� xip fir � �������� ������� �������������
	 * @return 
	*/
	int init_lagrange_interp();

	/**
	 * @brief �������� ������� ������������� ������� � ������
	 * @return 
	*/
	int lagrange_load_coeff();

	/**
	 * @brief ������������ ���������� �������
	 * @param values ������ ��������������� ��������
	 * @param out ��������� ������������
	 * @param pos ������� ������������ --> [0, LAGRANGE_INTERVALS-1]
	 * @return 
	*/
	int interpolate(xip_complex* values, xip_complex& out, uint32_t pos);

	/**
	 * @brief ������������� ���������� xip fir � ������������ ������
	 * @return 
	*/
	int destroy_lagrange_interp();

	size_t samples_count(double inv_factor);
	int to_dx_value(double inv_factor);
	void init(double dx_value);

	const uint32_t lagrange_n_intervals = LAGRANGE_INTERVALS;		// ���������� ����������
	const uint32_t lagrange_n_coeff = LAGRANGE_ORDER;				// ���������� ������������� (������� �������)
	double* lagrange_coeff;											// ������ ������������� �������, ������� �� �������
	const uint32_t FixPointPosition = LAGRANGE_FIXED_POINT_POSITION;
	const uint32_t FixPointPosMaxVal = 1 << FixPointPosition;

	vector<xip_complex> samples;	// FIFO-����� ��������

	// ���������� ��������� �������������
	int32_t dx;
	int32_t fx;		// ������� �����
	uint32_t block_size;
	uint32_t block_offset;
	xip_complex* x_ptr;
	xip_complex* pos_ptr;
	xip_complex* end_ptr;

	// ���������� ��� ������ � xip fir
	xip_fir_v7_2* lagrange_interp_fir;		// ������ �� ��������
	xip_fir_v7_2_config lagrange_interp_fir_cnfg;

	xip_array_uint* lagrange_interp_fir_fsel;
	xip_fir_v7_2_cnfg_packet lagrange_interp_fir_cnfg_packet;

	xip_array_real* lagrange_interp_in;		// 3-D ������, ���������� ������� ������ ��� ���������
	xip_array_real* lagrange_interp_out;	// 3-D ������, ���������� ��������� ���������
};

#endif // LAGRANGEINTERP_H