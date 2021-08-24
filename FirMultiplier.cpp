#include "FirMultiplier.h"

// ���������� ������������ ����� �� ������ Fir

xip_fir_v7_2* fir_real_multiplier;			// ������
xip_fir_v7_2_config fir_real_multiplier_cnfg;
double fir_real_multiplier_coeff = 0;

xip_array_real* fir_real_multiplier_in;		// 3-D ������, ���������� ������� ������ ��� ���������
xip_array_real* fir_real_multiplier_out;	// 3-D ������, ���������� ��������� ���������

int init_fir_real_multiplier()
{
	// ������������ ���������� ��� ����� ��������
	xip_fir_v7_2_default_config(&fir_real_multiplier_cnfg);
	fir_real_multiplier_cnfg.name = "fir_real_multiplier";
	fir_real_multiplier_cnfg.filter_type = XIP_FIR_SINGLE_RATE;
	fir_real_multiplier_cnfg.coeff = &fir_real_multiplier_coeff;
	fir_real_multiplier_cnfg.num_coeffs = 1;
	fir_real_multiplier_cnfg.coeff_width = 32;
	fir_real_multiplier_cnfg.coeff_fract_width = 16;
	fir_real_multiplier_cnfg.quantization = XIP_FIR_QUANTIZED_ONLY; // XIP_FIR_MAXIMIZE_DYNAMIC_RANGE;
	fir_real_multiplier_cnfg.output_rounding_mode = XIP_FIR_FULL_PRECISION;
	fir_real_multiplier_cnfg.data_width = 32;
	fir_real_multiplier_cnfg.data_fract_width = 16;

	// Create filter instances
	fir_real_multiplier = xip_fir_v7_2_create(&fir_real_multiplier_cnfg, &msg_print, 0);
	if (!fir_real_multiplier) {
		printf("Error creating instance %s\n", fir_real_multiplier_cnfg.name);
		return -1;
	}

	// ����������� ������ ��� �������� �������
	fir_real_multiplier_in = xip_array_real_create();
	if (!fir_real_multiplier_in) {
		printf("Unable to create array!\n");
		return -1;
	}
	xip_array_real_reserve_dim(fir_real_multiplier_in, 3);
	fir_real_multiplier_in->dim_size = 3; // 3D array
	fir_real_multiplier_in->dim[0] = fir_real_multiplier_cnfg.num_paths;
	fir_real_multiplier_in->dim[1] = fir_real_multiplier_cnfg.num_channels;
	fir_real_multiplier_in->dim[2] = 1; // vectors in a single packet
	fir_real_multiplier_in->data_size = fir_real_multiplier_in->dim[0] * fir_real_multiplier_in->dim[1] * fir_real_multiplier_in->dim[2];
	if (xip_array_real_reserve_data(fir_real_multiplier_in, fir_real_multiplier_in->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// ����������� ������ ��� ��������� �������
	fir_real_multiplier_out = xip_array_real_create();
	xip_array_real_reserve_dim(fir_real_multiplier_out, 3);
	fir_real_multiplier_out->dim_size = 3; // 3D array
	if (xip_fir_v7_2_calc_size(fir_real_multiplier, fir_real_multiplier_in, fir_real_multiplier_out, 0) != XIP_STATUS_OK) {
		printf("Unable to calculate output date size\n");
		return -1;
	}
	if (xip_array_real_reserve_data(fir_real_multiplier_out, fir_real_multiplier_out->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	return 0;
}

int destroy_fir_real_multiplier()
{
	if (xip_array_real_destroy(fir_real_multiplier_in) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(fir_real_multiplier_out) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_fir_v7_2_destroy(fir_real_multiplier) != XIP_STATUS_OK) {
		return -1;
	}

	printf("Deleted instance of Fir multiplier and free memory\n");
	return 0;
}

int process_multiply_real(const xip_real& a, const xip_real& b, xip_real& out)
{
	return 0;
}
