#include "PolyphaseDecimator.h"

PolyphaseDecimator::PolyphaseDecimator(unsigned decim_factor, const string& coeff_file, unsigned num_coeff):
	m_decimFactor(decim_factor),
	m_numCoeff(num_coeff)
{
	init_xip_fir(coeff_file, num_coeff);
}

PolyphaseDecimator::~PolyphaseDecimator()
{
	destroy_xip_fir();
}

bool PolyphaseDecimator::process(const xip_complex& in)
{
	if (m_decimCounter >= m_decimFactor)
		return false;

	if (xip_fir_v7_2_xip_array_real_set_chan(xip_fir_in, in.re, 0, 0, m_decimCounter, P_BASIC) != XIP_STATUS_OK) {
		printf("Unable to set data!\n");
		return false;
	}
	if (xip_fir_v7_2_xip_array_real_set_chan(xip_fir_in, in.im, 0, 1, m_decimCounter, P_BASIC) != XIP_STATUS_OK) {
		printf("Unable to set data!\n");
		return false;
	}

	m_decimCounter++;
	if (m_decimCounter >= m_decimFactor)
		return false;
	return true;
}

int PolyphaseDecimator::next(xip_complex& out)
{
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

	xip_fir_v7_2_xip_array_real_get_chan(xip_fir_out, &out.re, 0, 0, 0, P_BASIC);	// re
	xip_fir_v7_2_xip_array_real_get_chan(xip_fir_out, &out.im, 0, 1, 0, P_BASIC);	// im

	m_decimCounter = 0;

	return 0;
}

int PolyphaseDecimator::init_xip_fir(const string& coeff_file, unsigned num_coeff)
{
	if (load_coeff(coeff_file, num_coeff)) {
		printf("Enable to load fir coeffs\n");
		return -1;
	}

	xip_fir_v7_2_default_config(&xip_fir_cnfg);
	xip_fir_cnfg.name = "polyphase_decimator_fir";
	xip_fir_cnfg.filter_type = XIP_FIR_DECIMATION;
	xip_fir_cnfg.rate_change = XIP_FIR_INTEGER_RATE;
	xip_fir_cnfg.decim_rate = m_decimFactor;
	xip_fir_cnfg.coeff = m_firCoeff;
	xip_fir_cnfg.num_coeffs = m_numCoeff;
	xip_fir_cnfg.coeff_width = 24;
	xip_fir_cnfg.coeff_fract_width = 24;
	xip_fir_cnfg.quantization = XIP_FIR_QUANTIZED_ONLY;
	xip_fir_cnfg.output_rounding_mode = XIP_FIR_FULL_PRECISION;
	xip_fir_cnfg.data_width = 26;
	xip_fir_cnfg.data_fract_width = 10;

	// 2 канала дл€ вещественной и мнимой части. 
	// “ут по идее можно настроить параллельную обработку двух каналов в режиме XIP_FIR_ADVANCED_CHAN_SEQ
	xip_fir_cnfg.num_channels = 2;
	
	// Create filter instance
	xip_fir = xip_fir_v7_2_create(&xip_fir_cnfg, &msg_print, 0);
	if (!xip_fir) {
		printf("Error creating instance %s\n", xip_fir_cnfg.name);
		return -1;
	}

	// –езервируем пам€ть дл€ входного отсчета
	xip_fir_in = xip_array_real_create();
	if (!xip_fir_in) {
		printf("Unable to create array!\n");
		return -1;
	}
	xip_array_real_reserve_dim(xip_fir_in, 3);
	xip_fir_in->dim_size = 3; // 3D array
	xip_fir_in->dim[0] = xip_fir_cnfg.num_paths;
	xip_fir_in->dim[1] = xip_fir_cnfg.num_channels;
	xip_fir_in->dim[2] = m_decimFactor; // vectors in a single packet
	xip_fir_in->data_size = xip_fir_in->dim[0] * xip_fir_in->dim[1] * xip_fir_in->dim[2];
	if (xip_array_real_reserve_data(xip_fir_in, xip_fir_in->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// –езервируем пам€ть дл€ выходного отсчета
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

int PolyphaseDecimator::load_coeff(const string& coeff_file, unsigned num_coeff)
{
	m_firCoeff = new double[num_coeff];

	ifstream in(coeff_file);
	if (!in.is_open())
		return -1;

	for (unsigned i = 0; i < num_coeff; i++) {
		in >> m_firCoeff[i];
	}
	in.close();

	return 0;
}

int PolyphaseDecimator::destroy_xip_fir()
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

	if (m_firCoeff)
		delete[] m_firCoeff;

	printf("Deleted instance of polyphase decimator and free memory\n");
	return 0;
}
