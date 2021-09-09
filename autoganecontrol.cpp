#include "autoganecontrol.h"

AutoGaneControl::AutoGaneControl(int window_size, double norm_power):
    m_normPower(norm_power), 
    m_windowSize(window_size), 
    m_currentPower(0),
    m_counter(0)
{
    m_pwrReg.resize(window_size, 0);

	init_xip_fir(window_size);
}

AutoGaneControl::~AutoGaneControl()
{
	destroy_xip_fir();
}

bool AutoGaneControl::process(const xip_complex& in, xip_complex& out)
{
	double pwr_x = (in.re * in.re + in.im * in.im)/m_windowSize;
    m_currentPower += pwr_x;
    m_currentPower -= m_pwrReg.back();
    m_pwrReg.pop_back();
    m_pwrReg.push_front(pwr_x);

    if (m_counter < m_windowSize) {
        m_counter++;
        return false;
    }

	double norm = sqrt(m_currentPower/m_normPower);
	if (norm == 0)
		out = in;
	else
	{
		out.re = in.re / norm;
		out.im = in.im / norm;
	}

    return true;
}

void AutoGaneControl::reset()
{
    std::fill(m_pwrReg.begin(), m_pwrReg.end(), 0);
    m_currentPower = 0;

	xip_fir_v7_2_reset(xip_fir);
}

int AutoGaneControl::init_xip_fir(int window_size)
{
	double* g = new double[window_size];
	for (int i = 0; i < window_size; i++)
		g[i] = 1.0;

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
	xip_fir_in->dim[2] = 2; // vectors in a single packet
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
