#ifndef DDS_H
#define DDS_H

#include <stdexcept>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "dds_compiler_v6_0_bitacc_cmodel.h"
#include "debug.h"

/**
 * @brief ����� ������� ��������� �������
*/
class DDS
{
public:
	DDS(int phase_modulus);

	~DDS();

	int process(double dph, double& out_phase, double& out_sin, double& out_cos);

	/**
	 * @brief ���������� ������ ������������ sin/cos � �����
	 * @return 
	*/
	unsigned int getOutputWidth();

private:
	int init_dds_lib();
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