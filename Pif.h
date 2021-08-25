#ifndef PIF_H
#define PIF_H

#include <cmath>
#include <stdexcept>
#include <algorithm>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"

// ��� (Nezami - Loop Filter topology III)
// ���������� ��� ������������ ���-������� 2-�� ������� � �����������
class Pif
{
public:
	Pif(double g1, double g2);

	Pif(double specific_locking_band = 0.001); // specific_locking_band = lock_band / sampling_freq
		                                       // �������: 0.0005 ��� 4.0 / 8000.0
	~Pif();

	int process(const xip_real& in, xip_real& out);

private:
	void calculate_g1_g2(double slb);

	int init_xip_fir();

	int destroy_xip_fir();

	double g[2];	// g[0] - ���������������� ������������, g[1] - ������������ ������������
	xip_real reg;		// ������� ��� �����������

	xip_fir_v7_2* xip_fir;				// ���-������
	xip_fir_v7_2_config xip_fir_cnfg;	// ������ �������
	xip_array_real* xip_fir_in;			// 3-D ������, ���������� ������� ������ ��� ���������
	xip_array_real* xip_fir_out;		// 3-D ������, ���������� ��������� ���������
};

#endif // PIF_H