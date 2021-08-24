#include "FirSummator.h"

// �������� ������������ ����� �� ������ Fir 2-� ������� � ���������� ��������������

xip_fir_v7_2* fir_real_summator;						// ������
xip_fir_v7_2_config fir_real_summator_cnfg;			// ������������ �������
xip_fir_v7_2_rld_packet fir_real_summator_rld;		// reload-����� ��� ��������� ������������ a0
xip_fir_v7_2_cnfg_packet fir_real_summator_cnfg_rld;	// config-����� ��� ��������� ������������ a0
xip_array_uint* fir_real_summator_cnfg_fsel;
double fir_real_summator_coeff = 0;					// ��������� �������� ������������ a0

xip_array_real* fir_real_summator_in;		// 3-D ������, ���������� ������� ������ ��� ���������
xip_array_real* fir_real_summator_out;	// 3-D ������, ���������� ��������� ���������

int init_fir_real_summator()
{
	// ������������ ���������� ��� ����� ��������
	xip_fir_v7_2_default_config(&fir_real_summator_cnfg);
	fir_real_summator_cnfg.name = "fir_real_summator";
	fir_real_summator_cnfg.filter_type = XIP_FIR_SINGLE_RATE;
	fir_real_summator_cnfg.coeff = &fir_real_summator_coeff;
	fir_real_summator_cnfg.reloadable = 1;
	fir_real_summator_cnfg.num_coeffs = 1;
	fir_real_summator_cnfg.coeff_width = 26;
	fir_real_summator_cnfg.coeff_fract_width = 10;
	fir_real_summator_cnfg.quantization = XIP_FIR_QUANTIZED_ONLY;
	fir_real_summator_cnfg.output_rounding_mode = XIP_FIR_FULL_PRECISION;
	fir_real_summator_cnfg.data_width = 26;
	fir_real_summator_cnfg.data_fract_width = 10;

	// Create filter instances
	fir_real_summator = xip_fir_v7_2_create(&fir_real_summator_cnfg, &msg_print, 0);
	if (!fir_real_summator) {
		printf("Error creating instance %s\n", fir_real_summator_cnfg.name);
		return -1;
	}

	// ���������������� �����. ��������� ��� ���������� reload-������
	fir_real_summator_cnfg_fsel = xip_array_uint_create();
	xip_array_uint_reserve_dim(fir_real_summator_cnfg_fsel, 1);
	fir_real_summator_cnfg_fsel->dim_size = 1;
	fir_real_summator_cnfg_fsel->dim[0] = fir_real_summator_cnfg.num_channels;
	fir_real_summator_cnfg_fsel->data_size = fir_real_summator_cnfg_fsel->dim[0];
	if (xip_array_uint_reserve_data(fir_real_summator_cnfg_fsel, fir_real_summator_cnfg_fsel->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	fir_real_summator_cnfg_rld.fsel = fir_real_summator_cnfg_fsel;
	fir_real_summator_cnfg_rld.fsel->data[0] = 0;

	// ����������� ������ � reload-������ ��� ������������ a0
	fir_real_summator_rld.fsel = 0;
	fir_real_summator_rld.coeff = xip_array_real_create();
	xip_array_real_reserve_dim(fir_real_summator_rld.coeff, 1);
	fir_real_summator_rld.coeff->dim_size = 1;
	fir_real_summator_rld.coeff->dim[0] = 1;
	fir_real_summator_rld.coeff->data_size = fir_real_summator_rld.coeff->dim[0];
	if (xip_array_real_reserve_data(fir_real_summator_rld.coeff, fir_real_summator_rld.coeff->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve coeff!\n");
		return -1;
	}

	// ����������� ������ ��� �������� �������
	fir_real_summator_in = xip_array_real_create();
	if (!fir_real_summator_in) {
		printf("Unable to create array!\n");
		return -1;
	}
	xip_array_real_reserve_dim(fir_real_summator_in, 3);
	fir_real_summator_in->dim_size = 3; // 3D array
	fir_real_summator_in->dim[0] = fir_real_summator_cnfg.num_paths;
	fir_real_summator_in->dim[1] = fir_real_summator_cnfg.num_channels;
	fir_real_summator_in->dim[2] = 1; // vectors in a single packet
	fir_real_summator_in->data_size = fir_real_summator_in->dim[0] * fir_real_summator_in->dim[1] * fir_real_summator_in->dim[2];
	if (xip_array_real_reserve_data(fir_real_summator_in, fir_real_summator_in->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// ����������� ������ ��� ��������� �������
	fir_real_summator_out = xip_array_real_create();
	xip_array_real_reserve_dim(fir_real_summator_out, 3);
	fir_real_summator_out->dim_size = 3; // 3D array
	if (xip_fir_v7_2_calc_size(fir_real_summator, fir_real_summator_in, fir_real_summator_out, 0) != XIP_STATUS_OK) {
		printf("Unable to calculate output date size\n");
		return -1;
	}
	if (xip_array_real_reserve_data(fir_real_summator_out, fir_real_summator_out->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	return 0;
}

int destroy_fir_real_summator()
{
	if (xip_array_real_destroy(fir_real_summator_rld.coeff) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_uint_destroy(fir_real_summator_cnfg_fsel) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(fir_real_summator_in) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(fir_real_summator_out) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_fir_v7_2_destroy(fir_real_summator) != XIP_STATUS_OK) {
		return -1;
	}

	printf("Deleted instance of Fir multiplier and free memory\n");
	return 0;
}

int process_fir_real_sum(const xip_real& a, const xip_real& b, xip_real& out)
{
	// Send config data
	if (xip_fir_v7_2_config_send(fir_real_summator, &fir_real_summator_cnfg_rld) != XIP_STATUS_OK) {
		printf("Error sending config packet\n");
		return -1;
	}

	// ������ ��������� ������������� ��� ����������� a0 ������� ����� reload-�����
	fir_real_summator_rld.coeff->data[0] = a;
	// Send reload data
	if (xip_fir_v7_2_reload_send(fir_real_summator, &fir_real_summator_rld) != XIP_STATUS_OK) {
		printf("Error sending reload packet\n");
		return -1;
	}

	// ������ ��������� ���������� ��� ������� �����
	xip_fir_v7_2_xip_array_real_set_chan(fir_real_summator_in, b, 0, 0, 0, P_BASIC);

	// Send input data and filter
	if (xip_fir_v7_2_data_send(fir_real_summator, fir_real_summator_in) != XIP_STATUS_OK) {
		printf("Error sending data\n");
		return -1;
	}

	// Retrieve filtered data
	if (xip_fir_v7_2_data_get(fir_real_summator, fir_real_summator_out, 0) != XIP_STATUS_OK) {
		printf("Error getting data\n");
		return -1;
	}

	xip_fir_v7_2_xip_array_real_get_chan(fir_real_summator_out, &out, 0, 0, 0, P_BASIC);

	return 0;
}