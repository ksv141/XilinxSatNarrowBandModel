#ifndef DDS_H
#define DDS_H

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "dds_compiler_v6_0_bitacc_cmodel.h"
#include "debug.h"

/**
 * @brief ����� ������� ��������� �������
*/
class DDS
{
public:
	DDS();

	~DDS();

private:
	int init_dds_lib();
	int destroy_dds_lib();

	xip_dds_v6_0_config dds_cnfg;
	xip_dds_v6_0* dds_model;

	xip_array_real* dds_in;		// 3-D ������, ���������� ������� ������ ��� ���������
	xip_array_real* dds_out;	// 3-D ������, ���������� ��������� ���������
};

#endif // DDS_H