#include "autoganecontrol.h"

AutoGaneControl::AutoGaneControl(unsigned window_size_log2, double norm_power):
    m_normPower(norm_power), 
    m_windowSizeLog2(window_size_log2),
    m_currentPower(0),
    m_counter(0)
{
	m_windowSize = 1 << m_windowSizeLog2;
	m_pwrReg.resize(m_windowSize, 0);

	m_normPowerSqrt = sqrt(norm_power);
	init_xip_fir(window_size_log2);
}

AutoGaneControl::~AutoGaneControl()
{
	destroy_xip_fir();
}

bool AutoGaneControl::process(const xip_complex& in, xip_complex& out)
{
	xip_real re_sqr = 0;
	xip_multiply_real(in.re, in.re, re_sqr);
	xip_real im_sqr = 0;
	xip_multiply_real(in.im, in.im, im_sqr);

	xip_real sum_pwr = 0;
	xip_fir_process(re_sqr, im_sqr, sum_pwr);

	// регулировка выполняется после заполнения буфера АРУ
	if (m_counter < m_windowSize) {
		m_counter++;
		out = in;
		return false;
	}

	xip_real_shift(sum_pwr, -(int)m_windowSizeLog2);		// усреднение за окно

	xip_sqrt_real(sum_pwr, sum_pwr);

	xip_real in_mul;
	xip_multiply_real(in.re, m_normPowerSqrt, in_mul);
	int32_division(in_mul, sum_pwr, out.re);
	xip_multiply_real(in.im, m_normPowerSqrt, in_mul);
	int32_division(in_mul, sum_pwr, out.im);

	return true;
}

void AutoGaneControl::reset()
{
	xip_fir_v7_2_reset(xip_fir);
}

int AutoGaneControl::init_xip_fir(int window_size)
{
	int num_coeff = 1 << (window_size + 1);		// для каждого отсчета требуется 2 значения (re^2 + im^2)
	double* g = new double[num_coeff];
	for (int i = 0; i < num_coeff; i++)
		g[i] = 1.0;

	xip_fir_v7_2_default_config(&xip_fir_cnfg);
	xip_fir_cnfg.name = "agc_fir";
	xip_fir_cnfg.filter_type = XIP_FIR_SINGLE_RATE;
	xip_fir_cnfg.coeff = g;
	xip_fir_cnfg.num_coeffs = num_coeff;
	xip_fir_cnfg.coeff_width = 2;
	xip_fir_cnfg.coeff_fract_width = 0;
	xip_fir_cnfg.quantization = XIP_FIR_QUANTIZED_ONLY;
	xip_fir_cnfg.output_rounding_mode = XIP_FIR_FULL_PRECISION;
	xip_fir_cnfg.data_width = 26;	// подбирается исходя из динамического диапазона входного сигнала и окна усреднения
	xip_fir_cnfg.data_fract_width = 10;

	// Create filter instance
	xip_fir = xip_fir_v7_2_create(&xip_fir_cnfg, &msg_print, 0);
	if (!xip_fir) {
		printf("Error creating instance %s\n", xip_fir_cnfg.name);
		return -1;
	}

	delete[] g;

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
	xip_fir_in->dim[2] = 2; // vectors in a single packet (re, im)
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

int AutoGaneControl::destroy_xip_fir()
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

int AutoGaneControl::xip_fir_process(const xip_real& in_re, const xip_real& in_im, xip_real& out)
{
	xip_fir_v7_2_xip_array_real_set_chan(xip_fir_in, in_re, 0, 0, 0, P_BASIC);
	xip_fir_v7_2_xip_array_real_set_chan(xip_fir_in, in_im, 0, 0, 1, P_BASIC);

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

	if (xip_fir_v7_2_xip_array_real_get_chan(xip_fir_out, &out, 0, 0, 1, P_BASIC) != XIP_STATUS_OK) {
		printf("Error getting data\n");
		return -1;
	}

	return 0;
}
