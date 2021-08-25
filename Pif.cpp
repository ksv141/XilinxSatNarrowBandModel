#include "Pif.h"


Pif::Pif(double g1, double g2)
{
	g[0] = g1;
	g[1] = g2;
	reg = 0;
	init_xip_fir();
}

Pif::Pif(double specific_locking_band)
{
	calculate_g1_g2(specific_locking_band);
	reg = 0;
	init_xip_fir();
}


Pif::~Pif()
{
	destroy_xip_fir();
}

int Pif::process(const xip_real& in, xip_real& out)
{
	xip_fir_v7_2_xip_array_real_set_chan(xip_fir_in, in, 0, 0, 0, P_BASIC);

	// Send input data and filter
	if (xip_fir_v7_2_data_send(xip_fir, xip_fir_in) != XIP_STATUS_OK) {
		printf("Error sending data\n");
		return -1;
	}

	// Retrieve filtered data
	if (xip_fir_v7_2_data_get(xip_fir, xip_fir_out, 0) != XIP_STATUS_OK) {
		printf("Error getting data\n");
		return -1;
	}

	xip_real out_fir;
	xip_fir_v7_2_xip_array_real_get_chan(xip_fir_out, &out_fir, 0, 0, 0, P_BASIC);
	out_fir += reg;
	reg = out_fir;
	out = out_fir;

	return 0;
}

void Pif::calculate_g1_g2(double slb)
{
	using namespace std;

	if (slb <= 0.0 || 1.0 < slb)
		throw std::invalid_argument("invalid 'locking_band' value");

	double kappa = 0.707; /* 0.4 - 0.8 */
	double w_T = 2.0 * slb / (kappa + 1 / (4.0 * kappa));

	double g_c = exp(-2 * kappa * w_T);
	g[0] = 1 - g_c;
	g[1] = 1 + g_c - 2 * exp(-kappa * w_T) * cos(w_T * sqrt(1 - kappa * kappa));

	g[0] = (max)(g[0], 1e-20);
	g[1] = (max)(g[1], 1e-20);
}

int Pif::init_xip_fir()
{
	xip_fir_v7_2_default_config(&xip_fir_cnfg);
	xip_fir_cnfg.name = "pif_fir";
	xip_fir_cnfg.filter_type = XIP_FIR_SINGLE_RATE;
	xip_fir_cnfg.coeff = g;
	xip_fir_cnfg.num_coeffs = 2;
	xip_fir_cnfg.coeff_width = 24;
	xip_fir_cnfg.coeff_fract_width = 24;
	xip_fir_cnfg.quantization = XIP_FIR_QUANTIZED_ONLY;
	xip_fir_cnfg.output_rounding_mode = XIP_FIR_FULL_PRECISION;
	xip_fir_cnfg.data_width = 26;
	xip_fir_cnfg.data_fract_width = 10;

	// Create filter instance
	xip_fir = xip_fir_v7_2_create(&xip_fir_cnfg, &msg_print, 0);
	if (!xip_fir) {
		printf("Error creating instance %s\n", xip_fir_cnfg.name);
		return -1;
	}

	// Резервируем память для входного отсчета
	xip_fir_in = xip_array_real_create();
	if (!xip_fir_in) {
		printf("Unable to create array!\n");
		return -1;
	}
	xip_array_real_reserve_dim(xip_fir_in, 3);
	xip_fir_in->dim_size = 3; // 3D array
	xip_fir_in->dim[0] = xip_fir_cnfg.num_paths;
	xip_fir_in->dim[1] = xip_fir_cnfg.num_channels;
	xip_fir_in->dim[2] = 1; // vectors in a single packet
	xip_fir_in->data_size = xip_fir_in->dim[0] * xip_fir_in->dim[1] * xip_fir_in->dim[2];
	if (xip_array_real_reserve_data(xip_fir_in, xip_fir_in->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// Резервируем память для выходного отсчета
	xip_fir_out = xip_array_real_create();
	xip_array_real_reserve_dim(xip_fir_out, 3);
	xip_fir_out->dim_size = 3; // 3D array
	if (xip_fir_v7_2_calc_size(xip_fir, xip_fir_in, xip_fir_out, 0) != XIP_STATUS_OK) {
		printf("Unable to calculate output date size\n");
		return -1;
	}
	if (xip_array_real_reserve_data(xip_fir_out, xip_fir_out->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	return 0;
}

int Pif::destroy_xip_fir()
{
	if (xip_array_real_destroy(xip_fir_in) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(xip_fir_out) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_fir_v7_2_destroy(xip_fir) != XIP_STATUS_OK) {
		return -1;
	}

	printf("Deleted instance of PIF FIR and free memory\n");
	return 0;
}
