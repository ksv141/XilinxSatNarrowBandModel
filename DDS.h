#ifndef DDS_H
#define DDS_H

#include <stdexcept>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "dds_compiler_v6_0_bitacc_cmodel.h"
#include "debug.h"

/**
 * @brief ����� ������� ��������� �������
 * 2 ������: 
 *	- �������������
 *	- ������������ ������������� (���� � ����� � ���������� ������� ����) 
*/
class DDS
{
public:
	/**
	 * @brief �������������
	 * @param phase_modulus �������� ��������� ���� [0, phase_modulus]
	 * @param two_symmmetric_channels ������������ ������������� ����� (���� � ����� � ���������� ������� ����)
	*/
	DDS(int phase_modulus, bool two_symmmetric_channels = false);

	~DDS();

	/**
	 * @brief ������������� ���������
	 * @param dph ����� ���� --> [0, phase_modulus]
	*/
	int process(double dph, xip_complex& out);

	/**
	 * @brief ������������� ��������� (����� � ����)
	 * @param dph ����� ���� ����� --> [0, phase_modulus], ���� ������� ����������� ������������ phase_modulus
	*/
	int process(double dph, xip_complex& out_up, xip_complex& out_down);

	/**
	 * @brief ���������� ������ ������������ sin/cos � �����
	 * @return 
	*/
	unsigned int getOutputWidth();

private:
	int init_dds_lib(unsigned channels);
	int destroy_dds_lib();

	xip_dds_v6_0_config dds_cnfg;
	xip_dds_v6_0* dds_model;

	xip_array_real* dds_in;		// 3-D ������, ���������� ������� ������ ��� ���������
	xip_array_real* dds_out;	// 3-D ������, ���������� ��������� ���������

	xip_uint no_of_input_fields;	// ����� ����� �� ������� ���������
	xip_uint no_of_output_fields;	// ����� ����� � �������� ���������

	int m_phaseModulus;				// �������� ��������� ���� [0, 16383] --> [0, 2pi]
};

#endif // DDS_H