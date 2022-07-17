#include "LowpassFir.h"

LowpassFir::LowpassFir(const string& coeff_file, unsigned num_coeff, unsigned coeff_begin, unsigned coeff_step, 
	unsigned is_halfband, unsigned num_datapath):
	m_isHalfBand(is_halfband),
	m_numDataPath(num_datapath)
{
	unsigned N_1 = num_coeff - coeff_begin;
	m_numCoeff = N_1 / coeff_step;
	if (N_1 % coeff_step)
		m_numCoeff++;
	init_xip_fir(coeff_file, num_coeff, coeff_begin, coeff_step);
}

LowpassFir::~LowpassFir()
{
	destroy_xip_fir();
}

int LowpassFir::process(const xip_complex& in, xip_complex& out)
{
	xip_fir_v7_2_xip_array_real_set_chan(xip_fir_in, in.re, 0, 0, 0, P_BASIC);	// re
	xip_fir_v7_2_xip_array_real_set_chan(xip_fir_in, in.im, 0, 1, 0, P_BASIC);	// im

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

	return 0;
}

int LowpassFir::process(const xip_complex* in, xip_complex* out)
{
	for (unsigned i = 0; i < m_numDataPath; i++) {
		xip_fir_v7_2_xip_array_real_set_chan(xip_fir_in, in[i].re, 0, i*2, 0, P_BASIC);		// re
		xip_fir_v7_2_xip_array_real_set_chan(xip_fir_in, in[i].im, 0, i*2+1, 0, P_BASIC);	// im
	}

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

	for (unsigned i = 0; i < m_numDataPath; i++) {
		xip_fir_v7_2_xip_array_real_get_chan(xip_fir_out, &out[i].re, 0, i*2, 0, P_BASIC);		// re
		xip_fir_v7_2_xip_array_real_get_chan(xip_fir_out, &out[i].im, 0, i*2+1, 0, P_BASIC);	// im
	}

	return 0;
}

void LowpassFir::print_coeff()
{
	for (int i = 0; i < m_numCoeff; i++)
		std::cout << i << '\t' << m_firCoeff[i] << std::endl;
}

int LowpassFir::init_xip_fir(const string& coeff_file, unsigned num_coeff, unsigned coeff_begin, unsigned coeff_step)
{
	if (load_coeff(coeff_file, num_coeff, coeff_begin, coeff_step)) {
		printf("Enable to load fir coeffs\n");
		return -1;
	}

	xip_fir_v7_2_default_config(&xip_fir_cnfg);
	xip_fir_cnfg.name = "fir";
	xip_fir_cnfg.filter_type = XIP_FIR_SINGLE_RATE;
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
	xip_fir_cnfg.num_channels = 2 * m_numDataPath;
	xip_fir_cnfg.is_halfband = m_isHalfBand;

	// ѕо идее параллельные потоки нужно помещать в datapath, 
	// но num_path - может быть не более 16, поэтому потоки помещаем в каналы, а не в datapath
	// xip_fir_cnfg.num_paths = m_numDataPath;

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
	xip_fir_in->dim[2] = 1; // vectors in a single packet
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

int LowpassFir::load_coeff(const string& coeff_file, unsigned num_coeff, unsigned coeff_begin, unsigned coeff_step)
{
	m_firCoeff = new double[m_numCoeff];

	ifstream in(coeff_file);
	if (!in.is_open())
		return -1;

	double coef = 0;
	unsigned j = 0;
	for (unsigned i = 0; i < num_coeff; i++) {
		in >> coef;
		if (i >= coeff_begin && ((i - coeff_begin) % coeff_step == 0)) {
			m_firCoeff[j] = coef;
			j++;
		}
	}
	in.close();

	return 0;
}

int LowpassFir::destroy_xip_fir()
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

	printf("Deleted instance of fir and free memory\n");
	return 0;
}
