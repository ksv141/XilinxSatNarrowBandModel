#ifndef POLYPHASEDECIMATOR_H
#define POLYPHASEDECIMATOR_H

#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"

using namespace std;

/**
 * @brief ��������� �� ������ ����������� �������
*/
class PolyphaseDecimator
{
public:
	/**
	 * @brief �����������
	 * @param decim_factor ����������� ���������
	 * @param coeff_file ���� � �������������� �������
	 * @param num_coeff ���������� ������������� �������
	*/
	PolyphaseDecimator(unsigned decim_factor, const string& coeff_file, unsigned num_coeff);

	~PolyphaseDecimator();

	/**
	 * @brief ���������� ��������� ������
	 * @param in ������� ������
	 * @return true - ��������� ��� ������, false - ������ �� ���������
	 * ����� next �������� ������ ����� process ������ false
	*/
	bool process(const xip_complex& in);

	/**
	 * @brief �������� �������������� ������
	 * @param out 
	 * ����� next �������� ������ ����� process ������ false
	*/
	int next(xip_complex& out);

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

	unsigned m_decimFactor;				// ����������� ���������
	unsigned m_numCoeff;				// ���������� ������������� �������
	double* m_firCoeff = nullptr;		// ����� ������������� �������
	unsigned m_decimCounter = 0;		// ��������� ������� ������� �������� [0, m_decimFactor-1]

	// ���������� ��� ������ � xip fir
	xip_fir_v7_2* xip_fir;				// ���������� ������
	xip_fir_v7_2_config xip_fir_cnfg;	// ������ �������
	xip_array_real* xip_fir_in;			// 3-D ������, ���������� ������� ������ ��� ���������
	xip_array_real* xip_fir_out;		// 3-D ������, ���������� ��������� ���������
};

#endif // POLYPHASEDECIMATOR_H