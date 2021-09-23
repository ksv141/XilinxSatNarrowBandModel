#include "test_utils.h"

void signal_freq_shift(const string& in, const string& out, double freq_shift, double fs)
{
	if (freq_shift > fs)
		throw out_of_range("frequency shift is greater than sampling frequency");

	int16_t freq_shift_mod = (int16_t)((freq_shift / fs) * DDS_PHASE_MODULUS);
	signal_freq_shift(in, out, freq_shift_mod);
}

void signal_freq_shift_symmetric(const string& in, const string& out_up, const string& out_down, double freq_shift, double fs)
{
	if (freq_shift > fs)
		throw out_of_range("frequency shift is greater than sampling frequency");

	int16_t freq_shift_mod = (int16_t)((freq_shift / fs) * DDS_PHASE_MODULUS);
	signal_freq_shift_symmetric(in, out_up, out_down, freq_shift_mod);
}

void signal_freq_shift(const string& in, const string& out, int16_t freq_shift_mod)
{
	if ((freq_shift_mod > DDS_PHASE_MODULUS/2) || (freq_shift_mod < -DDS_PHASE_MODULUS / 2))
		throw out_of_range("frequency shift is out of range");

	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	double dph = freq_shift_mod;	// набег фазы за такт в диапазоне[0, 16383] -- > [0, 2pi]

	//ofstream dbg_out("dbg_out.txt");
	if (dph < 0)
		dph += DDS_PHASE_MODULUS;

	DDS dds(DDS_PHASE_MODULUS);
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex mod_sample{ 0, 0 };
		dds.process(dph, mod_sample);
		xip_complex res;
		xip_multiply_complex(sample, mod_sample, res);
		xip_complex_shift(res, -(int)(dds.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)

		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);

		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void signal_freq_shift_symmetric(const string& in, const string& out_up, const string& out_down, int16_t freq_shift_mod)
{
	if ((freq_shift_mod > DDS_PHASE_MODULUS / 2) || (freq_shift_mod < -DDS_PHASE_MODULUS / 2))
		throw out_of_range("frequency shift is out of range");

	FILE * in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE * out_file_up = fopen(out_up.c_str(), "wb");
	if (!out_file_up)
		return;
	FILE* out_file_down = fopen(out_down.c_str(), "wb");
	if (!out_file_down)
		return;

	double dph = freq_shift_mod;	// набег фазы за такт в диапазоне[0, 16383] -- > [0, 2pi]

	//ofstream dbg_out("dbg_out.txt");
	if (dph < 0)
		dph += DDS_PHASE_MODULUS;

	DDS dds(DDS_PHASE_MODULUS, true);
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex mod_sample_up{ 0, 0 };
		xip_complex mod_sample_down{ 0, 0 };
		dds.process(dph, mod_sample_up, mod_sample_down);
		xip_complex res;
		xip_multiply_complex(sample, mod_sample_up, res);
		xip_complex_shift(res, -(int)(dds.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)

		tC::write_real<int16_t>(out_file_up, res.re);
		tC::write_real<int16_t>(out_file_up, res.im);

		xip_multiply_complex(sample, mod_sample_down, res);
		xip_complex_shift(res, -(int)(dds.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)

		tC::write_real<int16_t>(out_file_down, res.re);
		tC::write_real<int16_t>(out_file_down, res.im);
		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file_up);
	fclose(out_file_down);
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

	//ofstream dbg_out("dbg_out.txt");

	LagrangeInterp itrp(from_sampling_freq, to_sampling_freq, 1);
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex res{ 0,0 };
		itrp.process(sample);
		while (itrp.next(res)) {
			tC::write_real<int16_t>(out_file, res.re);
			tC::write_real<int16_t>(out_file, res.im);

			//dbg_out << res << endl;
		}
	}

	//dbg_out.close();
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
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	//ofstream dbg_out("dbg_out.txt");
	string coeff_file;
	unsigned coeff_count = 0;
	if (decim_factor == 4) {
		coeff_file = "pph_decimator_x4.fcf";
		coeff_count = 96;
	}
	else
	{
		cerr << "invalid decimation factor" << endl;
		return;
	}

	PolyphaseDecimator decim(decim_factor, coeff_file, coeff_count);
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex res{ 0,0 };
		if (decim.process(sample))
			continue;
		decim.next(res);
		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);

		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void signal_interpolate(const string& in, const string& out, unsigned interp_factor)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	//ofstream dbg_out("dbg_out.txt");
	string coeff_file;
	unsigned coeff_count = 0;
	if (interp_factor == 64) {
		coeff_file = "pph_interpolator_x64.fcf";
		coeff_count = 1536;
	}
	else if (interp_factor == 4) {
		coeff_file = "pph_interpolator_x4.fcf";
		coeff_count = 96;
	}
	else
	{
		cerr << "invalid interpolation factor" << endl;
		return;
	}

	PolyphaseInterpolator interp(interp_factor, coeff_file, coeff_count);
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex res{ 0,0 };
		interp.process(sample);
		while (interp.next(res)) {
			tC::write_real<int16_t>(out_file, res.re);
			tC::write_real<int16_t>(out_file, res.im);
		}
		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void signal_lowpass(const string& in, const string& out, const string& coeff_file, unsigned num_coeff)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	//ofstream dbg_out("dbg_out.txt");

	LowpassFir fir(coeff_file, num_coeff);
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex res{ 0,0 };
		fir.process(sample, res);
		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);

		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void signal_agc(const string& in, const string& out, unsigned window_size_log2, double norm_power)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	//ofstream dbg_out("dbg_out.txt");
	AutoGaneControl agc(window_size_log2, norm_power);
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex res{ 0,0 };
		if (!agc.process(sample, res))
			continue;
		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);

		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

bool signal_freq_est_stage(const string& in, uint16_t M, uint16_t L, uint16_t F, uint32_t burst_est, int16_t& freq_est)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return false;

	//ofstream dbg_out("dbg_out.txt");

	CorrelatorDPDI corr_stage(FRAME_DATA_SIZE, (int8_t*)SignalSource::preambleData, SignalSource::preambleLength,
								M, L, F, burst_est);
	int16_t re;
	int16_t im;
	bool res = false;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		int16_t dph = 0;
		xip_real corr_est = 0;
		if (corr_stage.process(sample, dph, corr_est)) {
			freq_est = dph;
			res = true;
			break;
		}
		//dbg_out << corr_est << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	return res;
}

void signal_halfband_ddc(const string& in, const string& out_up, const string& out_down)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file_up = fopen(out_up.c_str(), "wb");
	if (!out_file_up)
		return;
	FILE* out_file_down = fopen(out_down.c_str(), "wb");
	if (!out_file_down)
		return;

	//ofstream dbg_out("dbg_out.txt");
	HalfBandDDCTree ddc_tree;
	int16_t re;
	int16_t im;
	xip_complex sample;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		sample.re = re;
		sample.im = im;
		xip_complex res{ 0,0 };
		if (!ddc_tree.process(sample))
			continue;

		xip_complex* out_sample = ddc_tree.getData();
		tC::write_real<int16_t>(out_file_up, out_sample[14].re);
		tC::write_real<int16_t>(out_file_up, out_sample[14].im);
		tC::write_real<int16_t>(out_file_down, out_sample[15].re);
		tC::write_real<int16_t>(out_file_down, out_sample[15].im);

		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file_up);
	fclose(out_file_down);
}

void signal_ddc_estimate(const string& in, unsigned& corr_num, int16_t& freq_est_stage_1, int16_t& freq_est_stage_2)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;

	//ofstream dbg_out("dbg_out.txt");
	HalfBandDDCTree ddc_tree;
	int16_t re;
	int16_t im;
	xip_complex sample;
	int counter = 0;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		if (++counter == 1000) {
			counter = 0;
			cout << ". ";
		}
		sample.re = re;
		sample.im = im;
		xip_complex res{ 0,0 };
		if (!ddc_tree.process(sample))
			continue;
		corr_num = ddc_tree.getFreqEstCorrNum();
		freq_est_stage_1 = ddc_tree.getfreqEstStage_1();
		freq_est_stage_2 = ddc_tree.getfreqEstStage_2();
		break;
		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
}
