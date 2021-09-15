#include "test_utils.h"

void signal_freq_shift(const string& in, const string& out, double freq_shift, double fs)
{
	if (freq_shift > fs)
		throw out_of_range("frequency shift is greater than sampling frequency");

	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	double dph = (freq_shift/fs)* DDS_PHASE_MODULUS;	// набег фазы за такт в диапазоне[0, 16383] -- > [0, 2pi]

	//ofstream dbg_out("dbg_out.txt");
	if (dph < 0)
		dph += DDS_PHASE_MODULUS;

	DDS dds(DDS_PHASE_MODULUS);
	int16_t re;
	int16_t im;
	double dds_phase, dds_sin, dds_cos;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
			tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		dds.process(dph, dds_phase, dds_sin, dds_cos);
		xip_complex mod_sample{ dds_cos, dds_sin };
		xip_complex res;
		xip_multiply_complex(sample, mod_sample, res);
		xip_complex_shift(res, -(int)(dds.getOutputWidth()-1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)
		
		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);

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

	//ofstream dbg_out("dbg_out.txt");

	LagrangeInterp itrp(from_sampling_freq, to_sampling_freq);
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

	PolyphaseDecimator decim(decim_factor, "pph_decimator_x4.fcf", 96);
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

	PolyphaseInterpolator interp(interp_factor, "pph_interpolator_x64.fcf", 1536);
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

void signal_correlation(const string& in)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;

	ofstream dbg_out("dbg_out.txt");

	CorrelatorDPDI corr_stage_1(FRAME_DATA_SIZE, (int8_t*)SignalSource::preambleData, SignalSource::preambleLength,
								1, 32, 1, DPDI_BURST_ML_SATGE_1);
	int16_t re;
	int16_t im;
	int counter = 0;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_real dph = 0;
		xip_real corr_est = 0;
		corr_stage_1.process(sample, dph, corr_est);

		dbg_out << corr_est << endl;
		counter++;
	}

	dbg_out.close();
	fclose(in_file);
}