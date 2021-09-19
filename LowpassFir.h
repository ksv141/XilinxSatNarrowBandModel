#ifndef	LOWPASSFIR_H
#define LOWPASSFIR_H

#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"

using namespace std;

/**
 * @brief ��� ���
*/
class LowpassFir
{
public:
	/**
	 * @brief �����������
	 * @param coeff_file ���� � �������������� �������
	 * @param num_coeff ���������� ������������� �������
	 * @param is_halfband ������ ������������ (0 - ���, 1 - ��)
	 * @param num_datapath ���������� ������������ �������
	 * @param num_coeff ���������� ������������� �������
	*/
	LowpassFir(const string& coeff_file, unsigned num_coeff, unsigned is_halfband = 0, unsigned num_datapath = 1);

	~LowpassFir();

	/**
	 * @brief ��������� ���������� �������
	 * @param in 
	 * @param out 
	 * @return 
	*/
	int process(const xip_complex& in, xip_complex& out);

private:
	/**
	 * @brief ������������� ���������� xip fir � �������� �������������
	 * @return
	*/
	int init_xip_fir(const string& coeff_file, unsigned num_coeff);

	/**
	 * @brief �������� ������������� ������� � ������
	 * @param coeff_file ���� � �������������� �������
	 * @param num_coeff ���������� ������������� �������
	 * @return
	*/
	int load_coeff(const string& coeff_file, unsigned num_coeff);

	/**
	 * @brief ��������������� ���������� xip fir � ������������ ������
	 * @return
	*/
	int destroy_xip_fir();

	unsigned m_numCoeff;				// ���������� ������������� �������
	double* m_firCoeff = nullptr;		// ����� ������������� �������
	unsigned m_decimCounter = 0;		// ��������� ������� ������� �������� [0, m_decimFactor-1]
	unsigned m_isHalfBand = 0;			// ������ ������������ (0 - ���, 1 - ��)
	unsigned m_numDataPath = 1;			// ���������� ������������ �������

	// ���������� ��� ������ � xip fir
	xip_fir_v7_2* xip_fir;				// ���������� ������
	xip_fir_v7_2_config xip_fir_cnfg;	// ������ �������
	xip_array_real* xip_fir_in;			// 3-D ������, ���������� ������� ������ ��� ���������
	xip_array_real* xip_fir_out;		// 3-D ������, ���������� ��������� ���������
};

#endif // LOWPASSFIR_H