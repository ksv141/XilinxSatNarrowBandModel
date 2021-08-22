#include "LagrangeInterp.h"

using namespace std;

// ������������ �������� 7-� ������� ��� �����������������, ������������ ��� ���
// �������� ����� ����� �������� ��������� ������� �� 1024 ����������, 
// ��� ������� �� ������� ������������ ���� ������� ������������ ����� �� 8 �������������

uint32_t lagrange_n_intervals = 1024;	// ���������� ����������
uint32_t lagrange_n_coeff = 8;			// ���������� �������������
double* lagrange_coeff = nullptr;		// ������ ������������� �������, ������� �� �������

xip_fir_v7_2* lagrange_interp_fir;		// ������ �� ��������
xip_fir_v7_2_config lagrange_interp_fir_cnfg;

xip_array_real* lagrange_interp_in;		// 3-D ������, ���������� ������� ������ ��� ���������
xip_array_real* lagrange_interp_out;	// 3-D ������, ���������� ��������� ���������

int lagrange_load_coeff();

// ������������� �������-������������� ��������
// ����������� 1024 ������ �� 8 �������������, ������ ����� ������������� � ����������� �� �������� �������������
// �������� ���������� � ��������� 1/1024 �� ��������� ����� ���������
int init_lagrange_interp()
{
	if (lagrange_load_coeff()) {
		printf("Enable to load Lagarnge coeffs\n");
		return -1;
	}

	xip_fir_v7_2_default_config(&lagrange_interp_fir_cnfg);
	lagrange_interp_fir_cnfg.name = "lagrange_interp_fir";
	lagrange_interp_fir_cnfg.filter_type = XIP_FIR_SINGLE_RATE;
	lagrange_interp_fir_cnfg.coeff = lagrange_coeff;
	lagrange_interp_fir_cnfg.num_coeffs = lagrange_n_coeff;
	lagrange_interp_fir_cnfg.coeff_sets = lagrange_n_intervals;

	// 2 ������ ��� ������������ � ������ �����. 
	// ��� �� ���� ����� ��������� ������������ ��������� ���� ������� � ������ XIP_FIR_ADVANCED_CHAN_SEQ
	lagrange_interp_fir_cnfg.num_channels = 2;

	// Create filter instance
	lagrange_interp_fir = xip_fir_v7_2_create(&lagrange_interp_fir_cnfg, &msg_print, 0);
	if (!lagrange_interp_fir) {
		printf("Error creating instance %s\n", lagrange_interp_fir_cnfg.name);
		return -1;
	}
	else {
		printf("Created instance %s\n", lagrange_interp_fir_cnfg.name);
	}

	// ����������� ������ ��� �������� �������
	lagrange_interp_in = xip_array_real_create();
	if (!lagrange_interp_in) {
		printf("Unable to create array!\n");
		return -1;
	}
	xip_array_real_reserve_dim(lagrange_interp_in, 3);
	lagrange_interp_in->dim_size = 3; // 3D array
	lagrange_interp_in->dim[0] = lagrange_interp_fir_cnfg.num_paths;
	lagrange_interp_in->dim[1] = lagrange_interp_fir_cnfg.num_channels;
	lagrange_interp_in->dim[2] = 1; // vectors in a single packet
	lagrange_interp_in->data_size = lagrange_interp_in->dim[0] * lagrange_interp_in->dim[1] * lagrange_interp_in->dim[2];
	if (xip_array_real_reserve_data(lagrange_interp_in, lagrange_interp_in->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// ����������� ������ ��� ��������� �������
	lagrange_interp_out = xip_array_real_create();
	xip_array_real_reserve_dim(lagrange_interp_out, 3);
	lagrange_interp_out->dim_size = 3; // 3D array
	if (xip_fir_v7_2_calc_size(lagrange_interp_fir, lagrange_interp_in, lagrange_interp_out, 0) != XIP_STATUS_OK) {
		printf("Unable to calculate output date size\n");
		return -1;
	}
	if (xip_array_real_reserve_data(lagrange_interp_out, lagrange_interp_out->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	return 0;
}

int process_sample_lagrange_interp(xip_complex* in, xip_complex* out, uint32_t pos)
{
	return 0;
}

// �������� 1024 ������ �� 8 �������������
int lagrange_load_coeff()
{
	lagrange_coeff = new double[(size_t)lagrange_n_coeff * (size_t)lagrange_n_intervals];

	for (size_t i = 0; i < lagrange_n_coeff; i++) {
		string fname;
		stringstream fname_ss;
		fname_ss << "lagrange_coeff\\LagrangeFixed" << i << ".coe";
		fname_ss >> fname;

		ifstream in(fname);
		if (!in.is_open())
			return -1;

		for (size_t j = 0; j < lagrange_n_intervals; j++) {
			string val_str;
			in >> val_str;
			bitset<16> val_bs(val_str);
			int16_t val_int = (int16_t)val_bs.to_ulong();
			lagrange_coeff[i + j * lagrange_n_coeff] = val_int;
		}
	}

	//ofstream out("coeff_out.txt");
	//for (int i = 0; i < lagrange_n_coeff * lagrange_n_intervals; i++)
	//{
	//	if (i % 8 == 0)
	//		out << "*********" << endl;
	//	out << lagrange_coeff[i] << endl;
	//}
	//out.close();

	return 0;
}

int destroy_lagrange_interp()
{
	if (xip_fir_v7_2_destroy(lagrange_interp_fir) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(lagrange_interp_in) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(lagrange_interp_out) != XIP_STATUS_OK) {
		return -1;
	}

	if (lagrange_coeff)
		delete[] lagrange_coeff;

	printf("Deleted instance of Lagrange interp and free memory\n");

	return 0;
}