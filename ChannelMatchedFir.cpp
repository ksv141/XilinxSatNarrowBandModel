#include "ChannelMatchedFir.h"

// +---------------+-------------+-------+--------+-------+--------------+
// |     Type      |   MagSpec   | Fc/Fs | Rollof | Order |    Window    |
// +---------------+-------------+-------+--------+-------+--------------+
// | raised-cosine | square root |  1/4  |  .25   |  18   | Kaiser, B=.5 |
// +---------------+-------------+-------+--------+-------+--------------+
const double rc_root_x2_25_19[19] =
{
	-0.0013789727382389502,
	 0.010108258970470938,
	-0.0088158332983470158,
	-0.018254667807044128,
	 0.032040090881415004,
	 0.026209138464556331,
	-0.08457623076097491,
	-0.032022493980703122,
	 0.31066605580209167,
	 0.53415494309189537,
	 0.31066605580209167,
	-0.032022493980703122,
	-0.08457623076097491,
	 0.026209138464556331,
	 0.032040090881415004,
	-0.018254667807044128,
	-0.0088158332983470158,
	 0.010108258970470938,
	-0.0013789727382389502
};

xip_fir_v7_2* fir_channel_matched;
xip_fir_v7_2_config fir_channel_matched_cnfg;

// инициализация канального/согласованного фильтра (SRRC)
int init_channel_matched_fir()
{
	xip_fir_v7_2_default_config(&fir_channel_matched_cnfg);
	fir_channel_matched_cnfg.name = "srrc_fir";
	fir_channel_matched_cnfg.filter_type = XIP_FIR_SINGLE_RATE;
	fir_channel_matched_cnfg.coeff = rc_root_x2_25_19;
	fir_channel_matched_cnfg.num_coeffs = 19;
	fir_channel_matched_cnfg.coeff_fract_width = 24;
	fir_channel_matched_cnfg.quantization = XIP_FIR_QUANTIZED_ONLY; // XIP_FIR_MAXIMIZE_DYNAMIC_RANGE;
	fir_channel_matched_cnfg.output_rounding_mode = XIP_FIR_FULL_PRECISION;
	// 2 канала для вещественной и мнимой части. 
	// Тут по идее нужно настроить параллельную обработку двух каналов в режиме XIP_FIR_ADVANCED_CHAN_SEQ
	fir_channel_matched_cnfg.num_channels = 2;	

	//Create filter instances
	fir_channel_matched = xip_fir_v7_2_create(&fir_channel_matched_cnfg, &msg_print, 0);
	if (!fir_channel_matched) {
		printf("Error creating instance %s\n", fir_channel_matched_cnfg.name);
		return -1;
	}
	else {
		printf("Created instance %s\n", fir_channel_matched_cnfg.name);
	}

	return 0;
}

int destroy_channel_matched_fir()
{
	if (xip_fir_v7_2_destroy(fir_channel_matched) != XIP_STATUS_OK) {
		return -1;
	}
	printf("Deleted instance of SRRC\n");
	return 0;
}

int process_data_channel_matched_fir()
{
	// Create input data packet
	xip_array_real* din = xip_array_real_create();
	xip_array_real_reserve_dim(din, 3);
	din->dim_size = 3; // 3D array
	din->dim[0] = fir_channel_matched_cnfg.num_paths;
	din->dim[1] = fir_channel_matched_cnfg.num_channels;
	din->dim[2] = fir_channel_matched_cnfg.num_coeffs; // vectors in a single packet
	din->data_size = din->dim[0] * din->dim[1] * din->dim[2];
	if (xip_array_real_reserve_data(din, din->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// Create output data packet
	//  - Automatically sized using xip_fir_v7_2_calc_size
	xip_array_real* fir_default_out = xip_array_real_create();
	xip_array_real_reserve_dim(fir_default_out, 3);
	fir_default_out->dim_size = 3; // 3D array
	if (xip_fir_v7_2_calc_size(fir_channel_matched, din, fir_default_out, 0) != XIP_STATUS_OK) {
		printf("Unable to calculate output date size\n");
		return -1;
	}
	if (xip_array_real_reserve_data(fir_default_out, fir_default_out->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// единичный импульс
	test_create_impulse(din);
	print_array_real(din);

	// Send input data and filter
	if (xip_fir_v7_2_data_send(fir_channel_matched, din) != XIP_STATUS_OK) {
		printf("Error sending data\n");
		return -1;
	}

	// Retrieve filtered data
	if (xip_fir_v7_2_data_get(fir_channel_matched, fir_default_out, 0) != XIP_STATUS_OK) {
		printf("Error getting data\n");
		return -1;
	}

	printf("Fetched result: ");
	print_array_real(fir_default_out);

	xip_array_real_destroy(din);
	xip_array_real_destroy(fir_default_out);
	return 0;
}
