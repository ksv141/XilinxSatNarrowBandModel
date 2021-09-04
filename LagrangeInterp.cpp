#include "LagrangeInterp.h"

const int LAGRANGE_INTERVALS = 1024;	// количество интервалов разбиения одного интервала интерполяции (одного такта)

LagrangeInterp::LagrangeInterp(xip_real frac):
	m_fraction(frac),
	m_dk(1),
	m_decim(0),
	m_prevShift(0)
{
	init_lagrange_interp();
}

LagrangeInterp::~LagrangeInterp()
{
	destroy_lagrange_interp();
}

void LagrangeInterp::setShift(xip_real shift)
{
	m_shift = shift;
	m_shift <= -1.0 ? -1.0 : (m_shift >= 1.0 ? 1.0 : m_shift);
}

void LagrangeInterp::process(const xip_complex& in)
{
	m_currentSample = in;
}

bool LagrangeInterp::next(xip_complex& out)
{
	m_dk = 1 - m_shift;
	if (m_dk > 1)
		m_dk = 2 - m_dk;
	process(m_currentSample, out, m_dk);
	return true;
}

void LagrangeInterp::process(xip_real shift)
{
	while (true) {
		if (m_dk <= 0) {
			m_decim = static_cast<int>(fabs(ceil(m_dk)));
			if (m_decim > 0) {
				cout << "m_decim = " << m_decim;
				--m_decim;
				cout << " reset to " << m_decim << endl;
				//m_inputSamples.pop_back();
				//m_inputSamples.push_front(in);
			}
			cout << "m_dk = " << m_dk;
			m_dk += 1;
			cout << " reset to " << m_dk << endl;
			break;
		}
		//cplx_fl s = process(in, m_dk);
		//out.push_back(s);
		cout << "process m_dk = " << m_dk << endl;
		m_dk = m_dk - m_prevShift + shift - m_fraction;
		m_prevShift = shift;

		cout << "m_dk = " << m_dk << "\tm_decim = " << m_decim << endl;
	}
}

void LagrangeInterp::process(const xip_complex& in, xip_complex& out, int time_shift)
{
	//uint32_t pos = shift * lagrange_n_intervals;
	//if (pos >= lagrange_n_intervals)
	//	pos -= lagrange_n_intervals;
	process_sample_lagrange_interp(in, out, time_shift);
}

// инициализация фильтра-интерполятора Лагранжа
// загружаются 1024 набора по 8 коэффициентов, каждый набор задействуется в зависимости от смещения интерполятора
// смещение дискретное с величиной 1/1024 от интервала между отсчетами
int LagrangeInterp::init_lagrange_interp()
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
	lagrange_interp_fir_cnfg.output_rounding_mode = XIP_FIR_FULL_PRECISION;
	lagrange_interp_fir_cnfg.data_width = 32;
	lagrange_interp_fir_cnfg.data_fract_width = 16;

	// 2 канала для вещественной и мнимой части. 
	// Тут по идее нужно настроить параллельную обработку двух каналов в режиме XIP_FIR_ADVANCED_CHAN_SEQ
	lagrange_interp_fir_cnfg.num_channels = 2;

	// Create filter instance
	lagrange_interp_fir = xip_fir_v7_2_create(&lagrange_interp_fir_cnfg, &msg_print, 0);
	if (!lagrange_interp_fir) {
		printf("Error creating instance %s\n", lagrange_interp_fir_cnfg.name);
		return -1;
	}

	// Резервируем память для входного отсчета
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

	// Резервируем память для выходного отсчета
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

	// Резервируем память для config-пакета
	lagrange_interp_fir_fsel = xip_array_uint_create();
	xip_array_uint_reserve_dim(lagrange_interp_fir_fsel, 1);
	lagrange_interp_fir_fsel->dim_size = 1; // 1-D array
	lagrange_interp_fir_fsel->dim[0] = 1;   // одна конфигурация для двух каналов (XIP_FIR_CONFIG_SINGLE)
	lagrange_interp_fir_fsel->data_size = lagrange_interp_fir_fsel->dim[0];
	if (xip_array_uint_reserve_data(lagrange_interp_fir_fsel, lagrange_interp_fir_fsel->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}
	lagrange_interp_fir_cnfg_packet.fsel = lagrange_interp_fir_fsel;

	return 0;
}

// обработка одного отсчета
// pos - смещение интерполятора [0, 1023]
int LagrangeInterp::process_sample_lagrange_interp(const xip_complex& in, xip_complex& out, uint32_t pos)
{
	// Установка набора коэффициентов фильтра, соответствующего смещению pos
	lagrange_interp_fir_cnfg_packet.fsel->data[0] = pos;
	// Send config data
	if (xip_fir_v7_2_config_send(lagrange_interp_fir, &lagrange_interp_fir_cnfg_packet) != XIP_STATUS_OK) {
		printf("Error sending config packet\n");
		return -1;
	}

	// инициализация входных данных
	xip_fir_v7_2_xip_array_real_set_chan(lagrange_interp_in, in.re, 0, 0, 0, P_BASIC);	// re
	xip_fir_v7_2_xip_array_real_set_chan(lagrange_interp_in, in.im, 0, 1, 0, P_BASIC);	// im

	// Send input data and filter
	if (xip_fir_v7_2_data_send(lagrange_interp_fir, lagrange_interp_in) != XIP_STATUS_OK) {
		printf("Error sending data\n");
		return -1;
	}

	// Retrieve filtered data
	if (xip_fir_v7_2_data_get(lagrange_interp_fir, lagrange_interp_out, 0) != XIP_STATUS_OK) {
		printf("Error getting data\n");
		return -1;
	}

	xip_fir_v7_2_xip_array_real_get_chan(lagrange_interp_out, &out.re, 0, 0, 0, P_BASIC);	// re
	xip_fir_v7_2_xip_array_real_get_chan(lagrange_interp_out, &out.im, 0, 1, 0, P_BASIC);	// im

	return 0;
}

// загрузка 1024 набора из 8 коэффициентов
int LagrangeInterp::lagrange_load_coeff()
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

	return 0;
}

// деинициализация
int LagrangeInterp::destroy_lagrange_interp()
{
	if (xip_array_real_destroy(lagrange_interp_in) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(lagrange_interp_out) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_uint_destroy(lagrange_interp_fir_fsel) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_fir_v7_2_destroy(lagrange_interp_fir) != XIP_STATUS_OK) {
		return -1;
	}

	if (lagrange_coeff)
		delete[] lagrange_coeff;

	printf("Deleted instance of Lagrange interp and free memory\n");

	return 0;
}