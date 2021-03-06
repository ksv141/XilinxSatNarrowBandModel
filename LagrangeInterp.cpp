#include "LagrangeInterp.h"

const unsigned int LAGRANGE_INTERVALS = 1024;			// ?????????? ?????????? ????????? ?????? ????????? ???????????? (?????? ?????)
const unsigned int LAGRANGE_INTERVALS_LOG2 = 10;		// log2(1024)
const unsigned int LAGRANGE_ORDER = 8;					// ??????? ?????????????
const unsigned int LAGRANGE_FIXED_POINT_POSITION = 10;	// ???????? ???????? ?????? --> [-1.0, 1.0] (? ?????)

LagrangeInterp::LagrangeInterp(xip_real frac, unsigned num_datapath):
	samples(num_datapath, vector<xip_complex>(samples_count(frac), xip_complex{ 0, 0 })),
	m_numDataPath(num_datapath)
{
	int dx_value = to_dx_value(1.0 / frac);
	init(dx_value);
	init_lagrange_interp();
}

LagrangeInterp::LagrangeInterp(xip_real from_sampling_freq, xip_real to_sampling_freq, unsigned num_datapath):
	samples(num_datapath, vector<xip_complex>(samples_count(from_sampling_freq / to_sampling_freq), xip_complex{ 0, 0 })),
	m_numDataPath(num_datapath)
{
	int dx_value = to_dx_value(from_sampling_freq / to_sampling_freq);
	init(dx_value);
	init_lagrange_interp();
}

LagrangeInterp::~LagrangeInterp()
{
	destroy_lagrange_interp();
}

void LagrangeInterp::shift(double value)
{
	value <= -1.0 ? -1.0 : (value >= 1.0 ? 1.0 : value);	// ??????????? value ?????????? [-1.0, 1.0]
	int32_t x = fx + static_cast<int32_t>(dx * value);
	fx = x & (FixPointPosMaxVal - 1);
	x_ptr += x >> FixPointPosition;
}

void LagrangeInterp::shift(int32_t value)
{
	// ??????????? value ?????????? [-2^FixPointPosition, 2^FixPointPosition]
	if (value < -(int32_t)FixPointPosMaxVal)
		value = -(int32_t)FixPointPosMaxVal;
	else if (value > (int32_t)FixPointPosMaxVal)
		value = (int32_t)FixPointPosMaxVal;

	int32_t x = fx + value;
	fx = x & (FixPointPosMaxVal - 1);
	x_ptr += x >> FixPointPosition;
}

void LagrangeInterp::process(const xip_complex& in)
{
	// ? ????? ???????? ??????????? ????????? ?????? (FIFO)
	if (pos_ptr == end_ptr)
	{
		xip_complex* s = &samples[0][0];
		memcpy(s, s + block_offset, block_size * sizeof(xip_complex));
		pos_ptr -= block_offset;
		x_ptr -= block_offset;
	}

	*pos_ptr = in;
	++pos_ptr;
}

void LagrangeInterp::process(const xip_complex* in)
{
	// ? ????? ???????? ??????????? ????????? ?????? (FIFO)
	if (pos_ptr == end_ptr)
	{
		for (int i = 0; i < m_numDataPath; i++) {
			xip_complex* s = &samples[i][0];
			memcpy(s, s + block_offset, block_size * sizeof(xip_complex));
		}

		//xip_complex* s = &samples[0][0];
		//memcpy(s, s + block_offset, block_size * sizeof(xip_complex));
		pos_ptr -= block_offset;
		x_ptr -= block_offset;
	}

	for (int i = 0; i < m_numDataPath; i++) {
		int pos_ind = pos_ptr - &samples[0][0];
		samples[i][pos_ind] = in[i];
	}

	//*pos_ptr = in;
	++pos_ptr;
}

bool LagrangeInterp::next(xip_complex& out)
{
	if (x_ptr > pos_ptr)
		return false;

	const uint32_t coeffs_shift = FixPointPosition - LAGRANGE_INTERVALS_LOG2;
	interpolate(x_ptr - LAGRANGE_ORDER, out, static_cast<unsigned>(fx >> coeffs_shift));

	int32_t x = fx + dx;
	fx = x & (FixPointPosMaxVal - 1);
	x_ptr += x >> FixPointPosition;

	return true;
}

bool LagrangeInterp::next(xip_complex* out)
{
	if (x_ptr > pos_ptr)
		return false;

	const uint32_t coeffs_shift = FixPointPosition - LAGRANGE_INTERVALS_LOG2;
	interpolate((x_ptr - &samples[0][0]) - LAGRANGE_ORDER, out, static_cast<unsigned>(fx >> coeffs_shift));

	int32_t x = fx + dx;
	fx = x & (FixPointPosMaxVal - 1);
	x_ptr += x >> FixPointPosition;

	return true;
}

// ????????????? ???????-????????????? ????????
// ??????????? 1024 ?????? ?? 8 ?????????????, ?????? ????? ????????????? ? ??????????? ?? ???????? ?????????????
// ???????? ?????????? ? ????????? 1/1024 ?? ????????? ????? ?????????
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

	// 2 ?????? ??? ???????????? ? ?????? ?????. 
	// ??? ?? ???? ????? ????????? ???????????? ????????? ???? ??????? ? ?????? XIP_FIR_ADVANCED_CHAN_SEQ
	lagrange_interp_fir_cnfg.num_channels = 2 * m_numDataPath;

	// Create filter instance
	lagrange_interp_fir = xip_fir_v7_2_create(&lagrange_interp_fir_cnfg, &msg_print, 0);
	if (!lagrange_interp_fir) {
		printf("Error creating instance %s\n", lagrange_interp_fir_cnfg.name);
		return -1;
	}

	// ??????????? ?????? ??? ???????? ???????
	lagrange_interp_in = xip_array_real_create();
	if (!lagrange_interp_in) {
		printf("Unable to create array!\n");
		return -1;
	}
	xip_array_real_reserve_dim(lagrange_interp_in, 3);
	lagrange_interp_in->dim_size = 3; // 3D array
	lagrange_interp_in->dim[0] = lagrange_interp_fir_cnfg.num_paths;
	lagrange_interp_in->dim[1] = lagrange_interp_fir_cnfg.num_channels;
	lagrange_interp_in->dim[2] = LAGRANGE_ORDER; // vectors in a single packet
	lagrange_interp_in->data_size = lagrange_interp_in->dim[0] * lagrange_interp_in->dim[1] * lagrange_interp_in->dim[2];
	if (xip_array_real_reserve_data(lagrange_interp_in, lagrange_interp_in->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// ??????????? ?????? ??? ????????? ???????
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

	// ??????????? ?????? ??? config-??????
	lagrange_interp_fir_fsel = xip_array_uint_create();
	xip_array_uint_reserve_dim(lagrange_interp_fir_fsel, 1);
	lagrange_interp_fir_fsel->dim_size = 1; // 1-D array
	lagrange_interp_fir_fsel->dim[0] = 1;   // ???? ???????????? ??? ???? ??????? (XIP_FIR_CONFIG_SINGLE)
	lagrange_interp_fir_fsel->data_size = lagrange_interp_fir_fsel->dim[0];
	if (xip_array_uint_reserve_data(lagrange_interp_fir_fsel, lagrange_interp_fir_fsel->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}
	lagrange_interp_fir_cnfg_packet.fsel = lagrange_interp_fir_fsel;

	return 0;
}

// ????????? ?????? ???????
// pos - ???????? ????????????? [0, 1023]
int LagrangeInterp::interpolate(xip_complex* values, xip_complex& out, uint32_t pos)
{
	if (pos >= LAGRANGE_INTERVALS)
		throw std::range_error("pos must be less then LAGRANGE_INTERVALS");

	// ????????? ?????? ????????????? ???????, ???????????????? ???????? pos
	lagrange_interp_fir_cnfg_packet.fsel->data[0] = pos;
	// Send config data
	if (xip_fir_v7_2_config_send(lagrange_interp_fir, &lagrange_interp_fir_cnfg_packet) != XIP_STATUS_OK) {
		printf("Error sending config packet\n");
		return -1;
	}

	// ????????????? ??????? ??????
	// ??????????????? ?????? ????????? ? ?????? ? ???????? ???????
	for (int i = 0; i < LAGRANGE_ORDER; i++) {
		xip_fir_v7_2_xip_array_real_set_chan(lagrange_interp_in, values[LAGRANGE_ORDER-i-1].re, 0, 0, i, P_BASIC);	// re
		xip_fir_v7_2_xip_array_real_set_chan(lagrange_interp_in, values[LAGRANGE_ORDER-i-1].im, 0, 1, i, P_BASIC);	// im
	}

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

	xip_fir_v7_2_xip_array_real_get_chan(lagrange_interp_out, &out.re, 0, 0, LAGRANGE_ORDER-1, P_BASIC);	// re
	xip_fir_v7_2_xip_array_real_get_chan(lagrange_interp_out, &out.im, 0, 1, LAGRANGE_ORDER-1, P_BASIC);	// im

	// ???????????? ???????
	//xip_complex_shift(out, -(int)(lagrange_interp_fir_cnfg.data_width - lagrange_interp_fir_cnfg.data_fract_width - 1));
	xip_complex_shift(out, -15);

	return 0;
}

int LagrangeInterp::interpolate(int samples_pos, xip_complex* out, uint32_t pos)
{
	if (pos >= LAGRANGE_INTERVALS)
		throw std::range_error("pos must be less then LAGRANGE_INTERVALS");

	// ????????? ?????? ????????????? ???????, ???????????????? ???????? pos
	lagrange_interp_fir_cnfg_packet.fsel->data[0] = pos;
	// Send config data
	if (xip_fir_v7_2_config_send(lagrange_interp_fir, &lagrange_interp_fir_cnfg_packet) != XIP_STATUS_OK) {
		printf("Error sending config packet\n");
		return -1;
	}

	// ????????????? ??????? ??????
	// ??????????????? ?????? ????????? ? ?????? ? ???????? ???????
	for (int k = 0; k < m_numDataPath; k++) {
		for (int i = 0; i < LAGRANGE_ORDER; i++) {
			xip_fir_v7_2_xip_array_real_set_chan(lagrange_interp_in, samples[k][samples_pos + LAGRANGE_ORDER - i - 1].re, 0, k*2, i, P_BASIC);		// re
			xip_fir_v7_2_xip_array_real_set_chan(lagrange_interp_in, samples[k][samples_pos + LAGRANGE_ORDER - i - 1].im, 0, k*2+1, i, P_BASIC);	// im
		}
	}


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

	for (unsigned k = 0; k < m_numDataPath; k++) {
		xip_fir_v7_2_xip_array_real_get_chan(lagrange_interp_out, &out[k].re, 0, k*2, LAGRANGE_ORDER - 1, P_BASIC);	// re
		xip_fir_v7_2_xip_array_real_get_chan(lagrange_interp_out, &out[k].im, 0, k*2+1, LAGRANGE_ORDER - 1, P_BASIC);	// im
		// ???????????? ???????
		xip_complex_shift(out[k], -15);
	}

	return 0;
}

// ???????? 1024 ?????? ?? 8 ?????????????
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

// ???????????????
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

size_t LagrangeInterp::samples_count(double inv_factor)
{
	return LAGRANGE_ORDER * 32 + 2 * static_cast<int>(ceil(inv_factor));
}

int LagrangeInterp::to_dx_value(double inv_factor)
{
	if (inv_factor <= 0.0 || inv_factor < 2e-6 || 1000.0 < inv_factor)
		throw invalid_argument("invalid 'interpolation_factor' value");

	return static_cast<int>(inv_factor * FixPointPosMaxVal);
}

void LagrangeInterp::init(double dx_value)
{
	dx = dx_value;
	fx = 0;
	block_size = LAGRANGE_ORDER + (dx >> FixPointPosition) + 1;
	block_offset = static_cast<uint32_t>(samples[0].size() - block_size);
	x_ptr = &samples[0][0] + block_size;
	end_ptr = &samples[0][0] + samples[0].size();
	pos_ptr = x_ptr - LAGRANGE_ORDER / 2 - 1;
}

double LagrangeInterp::get_coefficient(unsigned set_no, unsigned index)
{
	return lagrange_coeff[set_no * lagrange_n_coeff + index];
}
