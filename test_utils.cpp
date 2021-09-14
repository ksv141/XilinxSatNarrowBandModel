#include "test_utils.h"

void signal_freq_shift(const string& in, const string& out, double dph)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	//ofstream dbg_out("dbg_out.txt");

	DDS dds(DDS_PHASE_MODULUS);
	int16_t re;
	int16_t im;
	double dds_phase, dds_sin, dds_cos;
	int counter = 0;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
			tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		if (counter % 10 == 0)
			dds.process(dph, dds_phase, dds_sin, dds_cos);
		xip_complex mod_sample{ dds_cos, dds_sin };
		xip_complex res;
		xip_multiply_complex(sample, mod_sample, res);
		xip_complex_shift(res, -(int)(dds.getOutputWidth()-1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)
		
		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);

		counter++;

		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void signal_time_shift(const string& in, const string& out, int32_t time_shift)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	//ofstream dbg_out("dbg_out.txt");

	LagrangeInterp itrp;
	itrp.shift(time_shift);
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex res{0,0};
		itrp.process(sample);
		if (!itrp.next(res))
			continue;
		//itrp.process(sample, res, time_shift);

		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);

		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void signal_time_shift_dyn(const string& in, const string& out, int shift_step)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	//ofstream dbg_out("dbg_out.txt");

	LagrangeInterp itrp;
	int16_t re;
	int16_t im;
	int dir = 1;
	int counter = 0;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex res{ 0,0 };
		itrp.process(sample);
		if (!itrp.next(res))
			continue;

		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);

		if (counter > shift_step*1000 || counter < 0) {
			dir *= -1;
		}
		if (counter % shift_step == 0)
			itrp.shift(dir);

		counter += dir;
		//dbg_out << shift << "\t" << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void signal_resample(const string& in, const string& out, double from_sampling_freq, double to_sampling_freq)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	ofstream dbg_out("dbg_out.txt");

	LagrangeInterp itrp(from_sampling_freq, to_sampling_freq);
	int16_t re;
	int16_t im;
	int counter = 0;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex res{ 0,0 };
		itrp.process(sample);
		while (itrp.next(res)) {
			tC::write_real<int16_t>(out_file, res.re);
			tC::write_real<int16_t>(out_file, res.im);

			dbg_out << res << endl;
		}
		counter++;
	}

	dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void generate_sin_signal(const string& out, double freq, double sample_freq, size_t count, int bits)
{
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	//ofstream dbg_out("dbg_out.txt");

	double dph = _2_PI * freq / sample_freq;	// набег фазы за такт
	double ph = 0;						// текущая фаза
	complex<double> sample;
	complex<double> _im_j{ 0, 1 };		// мнимая единица
	for (size_t i = 0; i < count; i++) {
		ph = fmod(ph + dph, _2_PI);
		sample = exp(_im_j * (ph));		// сигнал с амплитудой 1
		xip_complex res{ sample.real(), sample.imag() };
		xip_complex_shift(res, bits-1);
		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);
		//dbg_out << res << endl;
	}
	//dbg_out.close();
	fclose(out_file);
}

void signal_decimate(const string& in, const string& out, unsigned decim_factor)
{

}

int test_fir_order()
{
	xip_fir_v7_2* fir_real_summator;					// фильтр
	xip_fir_v7_2_config fir_real_summator_cnfg;			// конфигурация фильтра
	double fir_real_summator_coeff[] = { 1, 2, 3, 10, 20, 30 };		// коэффициенты сумматора

	xip_fir_v7_2_cnfg_packet fir_real_summator_cnfg_packet;
	xip_array_real* fir_real_summator_in;				// 3-D массив, содержащий текущий отсчет для обработки
	xip_array_real* fir_real_summator_out;				// 3-D массив, содержащий результат обработки

	xip_fir_v7_2_default_config(&fir_real_summator_cnfg);
	fir_real_summator_cnfg.name = "fir_real_summator";
	fir_real_summator_cnfg.filter_type = XIP_FIR_SINGLE_RATE;
	fir_real_summator_cnfg.coeff = fir_real_summator_coeff;
	fir_real_summator_cnfg.num_coeffs = 3;
	fir_real_summator_cnfg.coeff_sets = 2;
	//fir_real_summator_cnfg.quantization = XIP_FIR_INTEGER_COEFF;
	fir_real_summator_cnfg.output_rounding_mode = XIP_FIR_FULL_PRECISION;
	fir_real_summator_cnfg.data_width = 32;
	fir_real_summator_cnfg.data_fract_width = 16;
	fir_real_summator_cnfg.num_channels = 2;

	// Create filter instances
	fir_real_summator = xip_fir_v7_2_create(&fir_real_summator_cnfg, &msg_print, 0);
	if (!fir_real_summator) {
		printf("Error creating instance %s\n", fir_real_summator_cnfg.name);
		return -1;
	}

	// Резервируем память для входного отсчета
	fir_real_summator_in = xip_array_real_create();
	if (!fir_real_summator_in) {
		printf("Unable to create array!\n");
		return -1;
	}
	xip_array_real_reserve_dim(fir_real_summator_in, 3);
	fir_real_summator_in->dim_size = 3; // 3D array
	fir_real_summator_in->dim[0] = fir_real_summator_cnfg.num_paths;
	fir_real_summator_in->dim[1] = fir_real_summator_cnfg.num_channels;
	fir_real_summator_in->dim[2] = 3; // vectors in a single packet
	fir_real_summator_in->data_size = fir_real_summator_in->dim[0] * fir_real_summator_in->dim[1] * fir_real_summator_in->dim[2];
	if (xip_array_real_reserve_data(fir_real_summator_in, fir_real_summator_in->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// Резервируем память для выходного отсчета
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
	// Резервируем память для config-пакета

	xip_array_uint* fir_real_summator_fir_fsel = xip_array_uint_create();
	xip_array_uint_reserve_dim(fir_real_summator_fir_fsel, 1);
	fir_real_summator_fir_fsel->dim_size = 1; // 1-D array
	fir_real_summator_fir_fsel->dim[0] = 1;   // одна конфигурация для двух каналов (XIP_FIR_CONFIG_SINGLE)
	fir_real_summator_fir_fsel->data_size = fir_real_summator_fir_fsel->dim[0];
	if (xip_array_uint_reserve_data(fir_real_summator_fir_fsel, fir_real_summator_fir_fsel->data_size) != XIP_STATUS_OK) {
		printf("Unable to reserve data!\n");
		return -1;
	}
	fir_real_summator_cnfg_packet.fsel = fir_real_summator_fir_fsel;

	xip_complex a[] = { {1, 2}, {3, 4}, {5, 6} };
	xip_complex out;
	int pos = 1;

	// Установка набора коэффициентов фильтра, соответствующего смещению pos
	fir_real_summator_cnfg_packet.fsel->data[0] = pos;
	// Send config data
	if (xip_fir_v7_2_config_send(fir_real_summator, &fir_real_summator_cnfg_packet) != XIP_STATUS_OK) {
		printf("Error sending config packet\n");
		return -1;
	}

	for (int i = 0; i < 3; i++) {
		xip_fir_v7_2_xip_array_real_set_chan(fir_real_summator_in, a[i].re, 0, 0, i, P_BASIC);
		xip_fir_v7_2_xip_array_real_set_chan(fir_real_summator_in, a[i].im, 0, 1, i, P_BASIC);
	}

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

	// результатом является второй элемент выходного буфера
	xip_fir_v7_2_xip_array_real_get_chan(fir_real_summator_out, &out.re, 0, 0, 2, P_BASIC);
	xip_fir_v7_2_xip_array_real_get_chan(fir_real_summator_out, &out.im, 0, 1, 2, P_BASIC);

	cout << out << endl;

	if (xip_array_real_destroy(fir_real_summator_in) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(fir_real_summator_out) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_uint_destroy(fir_real_summator_fir_fsel) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_fir_v7_2_destroy(fir_real_summator) != XIP_STATUS_OK) {
		return -1;
	}

	printf("Deleted instance of Fir multiplier and free memory\n");

	return 0;
}
