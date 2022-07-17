#include "test_utils.h"

using namespace std;

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

xip_real getLineValue(xip_real ph)
{
	xip_real val = 0;
	if ((ph > PI_2) && (ph < PI_34)) {
		val = -(ph - PI_2) / PI_2 + 1;
	}
	else if (ph <= PI_2) {
		val = ph / PI_2;
	}
	else if (ph >= PI_34) {
		val = (ph - PI_34) / PI_2 - 1;
	}
	return val;
}

void signal_freq_shift_dopl(const string& in, const string& out, double fs, 
							double freq_ampl, double freq_peiod, int mode)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	double dph_dop = _2_PI / (freq_peiod * fs);		// набег фазы изменения частоты Доплера за такт, рад/отс [0, 2pi]
	double ampl_dop = freq_ampl / fs;				// максимальное изменение частоты Доплера за такт, Гц/отс
	double ph_dop = 0;								// текущее значение фазы частоты Доплера

	DDS dds(DDS_PHASE_MODULUS);
	double dph = 0;		// текущее значение набега фазы за такт
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {

		ph_dop = std::fmod(ph_dop + dph_dop, _2_PI);
		if (mode == 0)
			dph = (int16_t)(ampl_dop * std::sin(ph_dop) * DDS_PHASE_MODULUS);    // синусоидальная модель изменения набега фазы
		else if (mode == 1)
			dph = (int16_t)(ampl_dop * getLineValue(ph_dop) * DDS_PHASE_MODULUS);    // линейная модель изменения набега фазы
		else
			throw out_of_range("invalid dopler mode");

		if ((dph > DDS_PHASE_MODULUS / 2) || (dph < -DDS_PHASE_MODULUS / 2))
			throw out_of_range("frequency shift is out of range");

		if (dph < 0)					// набег фазы за такт в диапазоне[0, 16383] -- > [0, 2pi]
			dph += DDS_PHASE_MODULUS;

		xip_complex sample{ re, im };
		xip_complex mod_sample{ 0, 0 };
		dds.process(dph, mod_sample);
		xip_complex res;
		xip_multiply_complex(sample, mod_sample, res);
		xip_complex_shift(res, -(int)(dds.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)

		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);
	}

	fclose(in_file);
	fclose(out_file);
}

void signal_freq_phase_shift(const string& in, const string& out, int16_t freq_shift_mod, int16_t phase)
{
	if ((freq_shift_mod > DDS_PHASE_MODULUS / 2) || (freq_shift_mod < -DDS_PHASE_MODULUS / 2))
		throw out_of_range("frequency shift is out of range");

	FILE * in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE * out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	double dph = freq_shift_mod;	// набег фазы за такт в диапазоне[0, 16383] -- > [0, 2pi]
	double poff = phase;			// фазовое смещение в диапазоне[0, 16383] -- > [0, 2pi]

	//ofstream dbg_out("dbg_out.txt");
	if (dph < 0)
		dph += DDS_PHASE_MODULUS;
	if (poff < 0)
		poff += DDS_PHASE_MODULUS;

	DDS dds(DDS_PHASE_MODULUS);
	dds.setPhaseOffset(poff);
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

	ofstream dbg_out("dbg_out.txt");

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
		dbg_out << res << endl;
	}
	dbg_out.close();
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
	else if (decim_factor == 16) {
		coeff_file = "pph_decimator_x16.fcf";
		coeff_count = 384;
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
	else if (interp_factor == 16) {
		coeff_file = "pph_interpolator_x16.fcf";
		coeff_count = 384;
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

		//dbg_out << sample << endl;
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

bool signal_freq_est_stage(const string& in, uint16_t M, uint16_t L, uint32_t burst_est, int16_t& freq_est)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return false;

	ofstream dbg_out("dbg_out.txt");

	LowpassFir fir_1("rc_root_x4_25_41.fcf", 41, 0, 2);
	LowpassFir fir_2("rc_root_x4_25_41.fcf", 41, 1, 2);

	CorrelatorDPDIManchester corr_1((int8_t*)PREAMBLE_DATA, PREAMBLE_LENGTH, M, L, burst_est, 4);
	CorrelatorDPDIManchester corr_2((int8_t*)PREAMBLE_DATA, PREAMBLE_LENGTH, M, L, burst_est, 4);
	int16_t re;
	int16_t im;
	bool res = false;
	int i = 0;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		int16_t dph = 0;

		xip_real corr_est = 0;
		//if (corr_stage.process(sample, dph, corr_est)) {
		//	freq_est = dph;
		//	res = true;
		//	break;
		//}

		xip_complex sample_1;
		xip_complex sample_2;
		fir_1.process(sample, sample_1);
		fir_2.process(sample, sample_2);

		xip_real est_1;
		xip_real freq_1;
		corr_1.test_corr(sample_1, est_1, freq_1);
		xip_real est_2;
		xip_real freq_2;
		corr_2.test_corr(sample_2, est_2, freq_2);

		dbg_out << est_1 << '\t' << freq_1 << '\t' << est_2 << '\t' << freq_2 << endl;
		i++;
	}

	dbg_out.close();
	fclose(in_file);
	return res;
}

bool signal_phase_time_est_stage(const string& in, uint32_t burst_est, int16_t& phase, xip_real& time_shift, int& t_count)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return false;

	ofstream dbg_out("dbg_out.txt");

	PhClcCorrelatorManchester corr_stage((int8_t*)PREAMBLE_DATA, PREAMBLE_LENGTH,	burst_est);
	int16_t re;
	int16_t im;
	bool res = false;
	int counter = 0;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex_shift(sample, -1);            // уменьшаем динамический диапазон для больших уровней шума

		xip_complex corr{ 0,0 };
		xip_real est = 0;
		int16_t ph = 0;
		int16_t ts = 0;

		counter++;
		if (corr_stage.phaseEstimate(sample, ph, ts, est))
		{
			int pos = corr_stage.getMaxCorrPos();
			dbg_out << counter-pos << '\t' << est << '\t' << pos << '\t' << ph << '\t' << ts << endl;
		}

		//corr_stage.test_corr(sample, corr, est);
		//dbg_out << counter << '\t' << corr << '\t' << est << endl;

		//if (corr_stage.isPhaseEstMode()) {
		//	int16_t ph = 0;
		//	xip_real phase_est = 0;
		//	if (corr_stage.phaseEstimate(sample, ph, phase_est)) {
		//		phase = ph;
		//	}
		//}
		//else {
		//	int16_t t_shift = 0;
		//	xip_real time_est = 0;
		//	if (corr_stage.getSymbolTimingProcCounter()) {
		//		if (corr_stage.symbolTimingEstimate(sample, t_shift, time_est)) {
		//			time_shift = t_shift;
		//			t_count = corr_stage.getSymbolTimingProcCounter();
		//			res = true;
		//			break;
		//		}
		//	}
		//	dbg_out << time_est << endl;
		//}
	}

	dbg_out.close();
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

void signal_ddc_estimate(const string& in, unsigned& corr_num, int16_t& freq_est_stage_1, int16_t& freq_est_stage_2,
						int16_t& phase_est, int16_t& symbol_timing_est, int16_t& total_freq_est)
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
		phase_est = ddc_tree.getPhaseEst();
		symbol_timing_est = ddc_tree.getSymbolTimingEst();
		total_freq_est = ddc_tree.countTotalFreqShift();
		break;
		//dbg_out << res << endl;
	}
	//dbg_out.close();
	fclose(in_file);
}

void signal_estimate_demodulate(const string& in, const string& dem_out)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(dem_out.c_str(), "wb");
	if (!out_file)
		return;

	//ofstream dbg_out("dbg_out.txt");
	HalfBandDDCTree ddc_tree;
	DDS freq_shift_dds(DDS_PHASE_MODULUS);		// генератор для коррекции частотного сдвига в полосе обнаружителя
	DDS estimator_dds(DDS_PHASE_MODULUS);		// генератор для переноса сигнала в полосу обнаружителя
	LowpassFir lowpass_fir_0("lowpass_200kHz.fcf", 51);		// ФНЧ для 0-й ступени децимации (1600 кГц --> 400 кГц)
	LowpassFir lowpass_fir_1("lowpass_400_50kHz.fcf", 42);	// ФНЧ для 1-й ступени децимации (400 кГц --> 100 кГц)
	LowpassFir lowpass_fir_2("lowpass_100_9143Hz.fcf", 57);	// ФНЧ для 2-й ступени децимации (100 кГц --> 25 кГц)
	PolyphaseDecimator decim_0(4, "pph_decimator_x4.fcf", 96);	// дециматор 0-й ступени (1600 кГц --> 400 кГц)
	PolyphaseDecimator decim_1(4, "pph_decimator_x4.fcf", 96);	// дециматор 1-й ступени (400 кГц --> 100 кГц)
	PolyphaseDecimator decim_2(4, "pph_decimator_x4.fcf", 96);	// дециматор 2-й ступени (100 кГц --> 25 кГц)
	LagrangeInterp dmd_interp(25000, 18286, 1);	// интерполятор 25 кГц --> 2B
	AutoGaneControl agc(AGC_WND_SIZE_LOG2, get_cur_constell_pwr());	// АРУ
	DDS pll_dds(DDS_PHASE_MODULUS);				// генератор ФАПЧ
	Pif pif_pll(PIF_PLL_Kp, PIF_PLL_Ki);		// ПИФ ФАПЧ
	StsEstimate m_stsEst;						// блок оценки ошибки тактовой синхры
	Pif pif_sts(PIF_STS_Kp, PIF_STS_Ki);		// ПИФ СТС
	DoplerEstimate doplEst;						// блок оценки смещения Доплера
	Pif pif_dopl(PIF_DOPL_Kp, PIF_DOPL_Ki);		// ПИФ Доплера
	DDS dopl_dds(DDS_PHASE_MODULUS);			// генератор компенсации смещения Доплера

	double est_freq_shift = -200000;			// смещение частоты первого обнаружителя (200 кГц)
	int16_t est_freq_shift_mod = (int16_t)((est_freq_shift / HIGH_SAMPLE_RATE) * DDS_PHASE_MODULUS);
	if (est_freq_shift_mod < 0)
		est_freq_shift_mod += DDS_PHASE_MODULUS;

	int16_t re;
	int16_t im;
	xip_complex sample;
	int counter = 0;
	int16_t total_freq_est = 0;
	bool freq_est_done = false;
	int i = 0;				// счетчик для 2B --> B
	xip_real err_pll = 0;	// оценка ошибки ФАПЧ
	xip_real doplFreqEst = 0;		// оценка смещения Доплера
	xip_real doplFreqEstShift = 0;	// оценка изменения частоты за такт
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		if (++counter == 10000) {
			counter = 0;
			cout << ". ";
		}
		sample.re = re;
		sample.im = im;
		xip_complex mod_sample{ 0, 0 };

		//******* Петля компенсации смещения Доплера ******************
		doplFreqEst -= doplFreqEstShift;
		if (doplFreqEst < 0)								// переведем в диапазон работы DDS --> [0, 16384]
			doplFreqEst += DDS_PHASE_MODULUS;
		doplFreqEst = fmod(doplFreqEst, DDS_PHASE_MODULUS);
		dopl_dds.process(doplFreqEst, mod_sample);
		xip_multiply_complex(sample, mod_sample, sample);
		xip_complex_shift(sample, -(int)(dopl_dds.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)
		//*************************************************************

		//******* Приведение сигнала к полосе обнаружителя ************
		// Перенос сигнала в полосу обнаружителя (+/- 200 кГц)
		estimator_dds.process(est_freq_shift_mod, mod_sample);
		xip_multiply_complex(sample, mod_sample, sample);
		xip_complex_shift(sample, -(int)(estimator_dds.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)
		// ФНЧ для отфильтровки сигнала в полосе обнаружителя
		lowpass_fir_0.process(sample, sample);
		// Децимация до полосы обнаружителя (1600 кГц --> 400 кГц)
		if (decim_0.process(sample))
			continue;
		decim_0.next(sample);
		//*************************************************************

		if (!freq_est_done) {	// если сигнал еще не обнаружен, то продолжать его обнаружение
			freq_est_done = ddc_tree.process(sample);
			if (freq_est_done) {	// при обнаружении и оценке сигнала вычислить частотное смещение
				// !!! оптимизировать вычисление total_freq_est
				total_freq_est = ddc_tree.countTotalFreqShift();
				total_freq_est -= DDS_PHASE_MODULUS >> 1;	// полоса смещена вниз к 0
				total_freq_est = -total_freq_est;
				if ((total_freq_est > DDS_PHASE_MODULUS / 2) || (total_freq_est < -DDS_PHASE_MODULUS / 2))
					throw out_of_range("frequency shift is out of range");
				if (total_freq_est < 0)
					total_freq_est += DDS_PHASE_MODULUS; // набег фазы за такт в диапазоне[0, 16383] -- > [0, 2pi]
			}
		}
		else {	// если сигнал обнаружен коррелятором, корректируем частоту и демодулируем
			xip_complex mod_sample{ 0, 0 };
			// коррекция частоты
			freq_shift_dds.process(total_freq_est, mod_sample);
			xip_multiply_complex(sample, mod_sample, sample);
			xip_complex_shift(sample, -(int)(freq_shift_dds.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)
			// ФНЧ 1
			lowpass_fir_1.process(sample, sample);
			// децимация (400 кГц --> 100 кГц)
			if (decim_1.process(sample))
				continue;
			decim_1.next(sample);
			// ФНЧ 2
			lowpass_fir_2.process(sample, sample);
			// децимация (100 кГц --> 25 кГц)
			if (decim_2.process(sample))
				continue;
			decim_2.next(sample);

			//********** демодуляция
			// интерполятор 25 кГц --> 2B
			dmd_interp.process(sample);
			while (dmd_interp.next(sample)) {
				// согласованный фильтр на 2B
				process_sample_channel_matched_receive(&sample, &sample);
				// 2B --> B
				if (i == 1) {
					i = 0;
					continue;
				}
				i = 1;

				// АРУ для точной оценки ошибки тактовой синхры
				// уровень сигнала нормируется относительно уровня сигнального созвездия
				if (!agc.process(sample, sample))
					continue;
				// для сигнального созвездия +/-4096 сигнал с выхода АРУ будет в диапазоне [-2^14, 2^14]

				xip_complex dopl_sample = sample;

				//******** петля ФАПЧ, компенсация частотного смещения ****************
				xip_complex pll_corr{ 0, 0 };
				pll_dds.process(err_pll, pll_corr);					// сигнал ГУН
				xip_multiply_complex(sample, pll_corr, sample);		// компенсация
				xip_complex_shift(sample, -(int)(pll_dds.getOutputWidth() - 1));
				//*********************************************************************
				xip_complex est = nearest_point_psk4(sample);		// жесткое решение

				//******** оценка частотного сдвига ***********************************
				xip_complex err_pll_sample{ sample.re, -sample.im };  // комплексно-сопряженное от текущего отсчета
				xip_multiply_complex(err_pll_sample, est, err_pll_sample);
				err_pll = err_pll_sample.im;	// ошибка ФАПЧ

				// примем максимальный диапазон отклонения фазы [-0.5, 0.5] рад
				// для сигнального созвездия +/-4096 сдвиг будет в диапазоне [-0.5*2^25, 0.5*2^25] --> рад << 25
				// приведем к диапазону [-2^15, 2^15] для работы ПИФ
				xip_real_shift(err_pll, -10);
				pif_pll.process(err_pll, err_pll);	// сглаживание и интеграция сигнала ошибки в ПИФ

				// переведем в диапазон работы DDS --> [0, 16384] --> [0, 2pi]
				xip_multiply_real(err_pll, DDS_RAD_CONST, err_pll);
				xip_real_shift(err_pll, -18);
				if (err_pll < 0)
					err_pll += DDS_PHASE_MODULUS;
				//*********************************************************************

				//************ оценка ошибки тактовой синхры **************************
				xip_real sts_err = m_stsEst.getErr(sample, est);
				// для сигнального созвездия +/-4096 ошибка будет в диапазоне [-2^26, 2^26]

				// уменьшаем динамический диапазон до диапазона ПИФ --> [-2^15, 2^15] 
				// величину сдвига нужно подобирать исходя из ресурсов. 
				// для максимальной точности можно без сдвига
				// для минимальной точности и экономии ресурса можно сдвинуть сразу до [-2^10, 2^10]
				// меньше [-2^10, 2^10] ученьшать нецелесообразно, т.к. это диапазон интерполятора
				xip_real_shift(sts_err, -11);

				pif_sts.process(sts_err, sts_err);	// сглаживание и интеграция сигнала ошибки в ПИФ

				// уменьшаем динамический диапазон до диапазона интерполятора --> [-2^10, 2^10]
				xip_real_shift(sts_err, -3);

				// коррекция смещения интерполятора
				dmd_interp.shift((int32_t)sts_err);
				//*********************************************************************

				//************* Оценка смещения Доплера *******************************
				xip_real dopl_err = doplEst.getErr(dopl_sample, est);	// оценка для 1B
				xip_real dopl_err_B;
				pif_dopl.process(dopl_err, dopl_err_B);			// сглаживание и интеграция сигнала ошибки в ПИФ
					//dbg_out << dopl_err << '\t' << doplFreqEst << endl;

				doplFreqEstShift = dopl_err_B / 8192;
				//*********************************************************************

				tC::write_real<int16_t>(out_file, sample.re);
				tC::write_real<int16_t>(out_file, sample.im);
			}
		}
	}
	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void signal_estimate_demodulate_mnch_test(const string& in, const string& dem_out)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(dem_out.c_str(), "wb");
	if (!out_file)
		return;

	LagrangeInterp dmd_interp(16000, 16000, 1);	// интерполятор на 2B (манчестерских)
	AutoGaneControl agc(AGC_WND_SIZE_LOG2, get_cur_constell_pwr());	// АРУ
	DDS pll_dds(DDS_PHASE_MODULUS);				// генератор ФАПЧ
	Pif pif_pll(PIF_PLL_Kp, PIF_PLL_Ki);		// ПИФ ФАПЧ
	xip_real err_pll = 0;						// ошибка фазы (ФАПЧ)
	StsEstimate m_stsEst;						// блок оценки ошибки тактовой синхры
	Pif pif_sts(PIF_STS_Kp, PIF_STS_Ki);		// ПИФ СТС
	int i = 0;									// счетчик для 2B --> B
	xip_complex prev_sample{ 0,0 };				// предыдущий отсчет на 1B для декодирования манчестера
	int mnch_i = 0;								// счетчик для сопряженных манчестерских символов
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };

		dmd_interp.process(sample);
		while (dmd_interp.next(sample)) {
			// 2B --> B
			if (i == 1) {
				i = 0;
				continue;
			}
			i = 1;

			// АРУ для точной оценки ошибки тактовой синхры
			// уровень сигнала нормируется относительно уровня сигнального созвездия
			// для сигнального созвездия +/-4096 сигнал с выхода АРУ будет в диапазоне [-2^14, 2^14]
			agc.process(sample);
			while (agc.next(sample)) {

				//******** петля ФАПЧ, компенсация частотного смещения ****************
				xip_complex pll_corr{ 0, 0 };
				pll_dds.process(err_pll, pll_corr);					// сигнал ГУН
				xip_multiply_complex(sample, pll_corr, sample);		// компенсация
				xip_complex_shift(sample, -(int)(pll_dds.getOutputWidth() - 1));

				//*********************************************************************
				xip_complex est = nearest_point_psk2(sample);		// жесткое решение (для СТС и ФАПЧ)

					//******** оценка частотного сдвига ***********************************
				xip_complex err_pll_sample{ sample.re, -sample.im };  // комплексно-сопряженное от текущего отсчета
				xip_multiply_complex(err_pll_sample, est, err_pll_sample);
				err_pll = err_pll_sample.im;	// ошибка ФАПЧ

				// примем максимальный диапазон отклонения фазы [-0.5, 0.5] рад
				// для сигнального созвездия +/-4096 сдвиг будет в диапазоне [-0.5*2^25, 0.5*2^25] --> рад << 25
				// приведем к диапазону [-2^15, 2^15] для работы ПИФ
				xip_real_shift(err_pll, -10);
				pif_pll.process(err_pll, err_pll);	// сглаживание и интеграция сигнала ошибки в ПИФ

				// переведем в диапазон работы DDS --> [0, 16384] --> [0, 2pi]
				xip_multiply_real(err_pll, DDS_RAD_CONST, err_pll);
				xip_real_shift(err_pll, -18);
				if (err_pll < 0)
					err_pll += DDS_PHASE_MODULUS;
				//*********************************************************************

				//************ оценка ошибки тактовой синхры **************************
				xip_real sts_err = m_stsEst.getErr(sample, est);
				// для сигнального созвездия +/-4096 ошибка будет в диапазоне [-2^26, 2^26]

				// уменьшаем динамический диапазон до диапазона ПИФ --> [-2^15, 2^15] 
				// величину сдвига нужно подобирать исходя из ресурсов. 
				// для максимальной точности можно без сдвига
				// для минимальной точности и экономии ресурса можно сдвинуть сразу до [-2^10, 2^10]
				// меньше [-2^10, 2^10] ученьшать нецелесообразно, т.к. это диапазон интерполятора
				xip_real_shift(sts_err, -11);

				pif_sts.process(sts_err, sts_err);	// сглаживание и интеграция сигнала ошибки в ПИФ

				// уменьшаем динамический диапазон до диапазона интерполятора --> [-2^10, 2^10]
				xip_real_shift(sts_err, -3);

				// коррекция смещения интерполятора
				dmd_interp.shift((int32_t)sts_err);
				//*********************************************************************

				//*********** декодирование манчестерского кода ***********************
				if (mnch_i == 1) {
					xip_complex mnch_sum{ prev_sample.re - sample.re, prev_sample.im - sample.im };
					int symb = nearest_index_psk2(mnch_sum);
				}
				mnch_i = (mnch_i + 1) % 2;
				prev_sample = sample;
				//*********************************************************************
				tC::write_real<int16_t>(out_file, sample.re);
				tC::write_real<int16_t>(out_file, sample.im);
			}
		}
	}

	fclose(in_file);
	fclose(out_file);
}

void signal_estimate_demodulate_dopl_test(const string& in, const string& dem_out)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(dem_out.c_str(), "wb");
	if (!out_file)
		return;
	ofstream dbg_out("dbg_out.txt");
	//freopen("out.txt", "w", stdout);


	LagrangeInterp dmd_interp(1, 1, 1);
	//LagrangeInterp dmd_interp(25000, 18286, 1);
	//AutoGaneControl agc(7, get_cur_constell_pwr());	// АРУ
	AutoGaneControl agc(AGC_WND_SIZE_LOG2, get_cur_constell_pwr());	// АРУ
	LowpassFir lowpass_fir_0("lowpass_200kHz.fcf", 51);		// ФНЧ для 0-й ступени децимации (1600 кГц --> 400 кГц)
	LowpassFir lowpass_fir_1("lowpass_400_50kHz.fcf", 42);	// ФНЧ для 1-й ступени децимации (400 кГц --> 100 кГц)
	LowpassFir lowpass_fir_2("lowpass_100_9143Hz.fcf", 57);	// ФНЧ для 2-й ступени децимации (100 кГц --> 25 кГц)
	PolyphaseDecimator decim_0(4, "pph_decimator_x4.fcf", 96);	// дециматор 0-й ступени (1600 кГц --> 400 кГц)
	PolyphaseDecimator decim_1(4, "pph_decimator_x4.fcf", 96);	// дециматор 1-й ступени (400 кГц --> 100 кГц)
	PolyphaseDecimator decim_2(4, "pph_decimator_x4.fcf", 96);	// дециматор 2-й ступени (100 кГц --> 25 кГц)

	DDS pll_dds(DDS_PHASE_MODULUS);				// генератор ФАПЧ
	Pif pif_pll(PIF_PLL_Kp, PIF_PLL_Ki);		// ПИФ ФАПЧ
	StsEstimate m_stsEst;						// блок оценки ошибки тактовой синхры
	Pif pif_sts(PIF_STS_Kp, PIF_STS_Ki);		// ПИФ СТС
	DoplerEstimate doplEst;						// блок оценки смещения Доплера
	//Pif pif_dopl(PIF_DOPL_Kp, PIF_DOPL_Ki);		// ПИФ Доплера
	Pif pif_dopl(0.01);		// ПИФ Доплера
	DDS dopl_dds(DDS_PHASE_MODULUS);			// генератор компенсации смещения Доплера
	xip_real err_pll = 0;	// оценка ошибки ФАПЧ
	xip_real doplFreqEst = 0;		// оценка смещения Доплера
	xip_real doplFreqEstShift = 0;	// оценка смещения Доплера
	int i = 0;				// счетчик для 2B --> B
	int16_t re;
	int16_t im;
	xip_complex sample;
	int counter = 0;
	int counter_all = 0;
	double sum_dopl_err = 0;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		if (++counter == 10000) {
			counter = 0;
			cout << ". ";
		}
		counter_all++;

		sample.re = re;
		sample.im = im;

		xip_complex mod_sample{ 0, 0 };
		//******* Петля компенсации смещения Доплера ******************
		doplFreqEst -= doplFreqEstShift;
		if (doplFreqEst < 0)								// переведем в диапазон работы DDS --> [0, 16384]
			doplFreqEst += DDS_PHASE_MODULUS;
		doplFreqEst = fmod(doplFreqEst, DDS_PHASE_MODULUS);
		//dbg_out << doplFreqEst << endl;
		dopl_dds.process(doplFreqEst, mod_sample);
		xip_multiply_complex(sample, mod_sample, sample);
		xip_complex_shift(sample, -(int)(dopl_dds.getOutputWidth() -1 ));	// уменьшаем динамический диапазон результата (подобрано опытным путем)
		//*************************************************************

		// ФНЧ для отфильтровки сигнала в полосе обнаружителя
		lowpass_fir_0.process(sample, sample);
		// Децимация до полосы обнаружителя (1600 кГц --> 400 кГц)
		if (decim_0.process(sample))
			continue;
		decim_0.next(sample);
		// ФНЧ 1
		lowpass_fir_1.process(sample, sample);
		// децимация (400 кГц --> 100 кГц)
		if (decim_1.process(sample))
			continue;
		decim_1.next(sample);
		// ФНЧ 2
		lowpass_fir_2.process(sample, sample);
		// децимация (100 кГц --> 25 кГц)
		if (decim_2.process(sample))
			continue;
		decim_2.next(sample);

		//********** демодуляция
		// интерполятор 25 кГц --> 2B
		dmd_interp.process(sample);
		while (dmd_interp.next(sample)) {
			// согласованный фильтр на 2B
			process_sample_channel_matched_receive(&sample, &sample);
			// 2B --> B
			if (i == 1) {
				i = 0;
				continue;
			}
			i = 1;

			// АРУ для точной оценки ошибки тактовой синхры
			// уровень сигнала нормируется относительно уровня сигнального созвездия
			if (!agc.process(sample, sample))
				continue;
			// для сигнального созвездия +/-4096 сигнал с выхода АРУ будет в диапазоне [-2^14, 2^14]

			xip_complex dopl_sample = sample;

			//******** петля ФАПЧ, компенсация частотного смещения ****************
			xip_complex pll_corr{ 0, 0 };
			pll_dds.process(err_pll, pll_corr);					// сигнал ГУН
			xip_multiply_complex(sample, pll_corr, sample);		// компенсация
			xip_complex_shift(sample, -(int)(pll_dds.getOutputWidth() - 1));
			//*********************************************************************
			xip_complex est = nearest_point_psk4(sample);		// жесткое решение

			//******** оценка частотного сдвига ***********************************
			xip_complex err_pll_sample{ sample.re, -sample.im };  // комплексно-сопряженное от текущего отсчета
			xip_multiply_complex(err_pll_sample, est, err_pll_sample);
			err_pll = err_pll_sample.im;	// ошибка ФАПЧ

			// примем максимальный диапазон отклонения фазы [-0.5, 0.5] рад
			// для сигнального созвездия +/-4096 сдвиг будет в диапазоне [-0.5*2^25, 0.5*2^25] --> рад << 25
			// приведем к диапазону [-2^15, 2^15] для работы ПИФ
			xip_real_shift(err_pll, -10);
			pif_pll.process(err_pll, err_pll);	// сглаживание и интеграция сигнала ошибки в ПИФ

			// переведем в диапазон работы DDS --> [0, 16384] --> [0, 2pi]
			xip_multiply_real(err_pll, DDS_RAD_CONST, err_pll);
			xip_real_shift(err_pll, -18);
			if (err_pll < 0)
				err_pll += DDS_PHASE_MODULUS;
			//*********************************************************************

			//************ оценка ошибки тактовой синхры **************************
			xip_real sts_err = m_stsEst.getErr(sample, est);
			// для сигнального созвездия +/-4096 ошибка будет в диапазоне [-2^26, 2^26]

			// уменьшаем динамический диапазон до диапазона ПИФ --> [-2^15, 2^15] 
			// величину сдвига нужно подобирать исходя из ресурсов. 
			// для максимальной точности можно без сдвига
			// для минимальной точности и экономии ресурса можно сдвинуть сразу до [-2^10, 2^10]
			// меньше [-2^10, 2^10] ученьшать нецелесообразно, т.к. это диапазон интерполятора
			xip_real_shift(sts_err, -11);

			pif_sts.process(sts_err, sts_err);	// сглаживание и интеграция сигнала ошибки в ПИФ

			// уменьшаем динамический диапазон до диапазона интерполятора --> [-2^10, 2^10]
			xip_real_shift(sts_err, -3);

			// коррекция смещения интерполятора
			dmd_interp.shift((int32_t)sts_err);
			//*********************************************************************

			//************* Оценка смещения Доплера *******************************
			xip_real dopl_err = doplEst.getErr(dopl_sample, est);	// оценка для 1B
			xip_real dopl_err_B;
			pif_dopl.process(dopl_err, dopl_err_B);			// сглаживание и интеграция сигнала ошибки в ПИФ
			dbg_out << dopl_err_B << endl;

			doplFreqEstShift = dopl_err_B / 8192;
			//doplFreqEst -= dopl_err_B/128;// *9143.0 / 100000.0;
			//if (doplFreqEst < 0)								// переведем в диапазон работы DDS --> [0, 16384]
			//	doplFreqEst += DDS_PHASE_MODULUS;
			//doplFreqEst = fmod(doplFreqEst, DDS_PHASE_MODULUS);
			//dbg_out << doplFreqEst << endl;

			//dbg_out << setw(15) << dopl_err << setw(15) << dopl_err_B << setw(15) << doplFreqEst << endl;
			//*********************************************************************

			tC::write_real<int16_t>(out_file, sample.re);
			tC::write_real<int16_t>(out_file, sample.im);
		}
	}

	dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void signal_frame_test(const string& in, bool is_binary, bool has_preamble, bool has_postamble)
{
	ofstream dbg_out("dbg_out.txt");

	SignalSource sig_test(in, is_binary, has_preamble, has_postamble);
	xip_complex sample;
	while (true)
	{
		if (!sig_test.nextSampleFromFile(sample))
			break;
		dbg_out << sample << endl;
	}

	dbg_out.close();
}

void signal_pwr_measure(const string& in, unsigned wnd_size)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;

	ofstream dbg_out("dbg_out.txt");

	deque<xip_real> pwrReg(wnd_size, 0);
	xip_real current_pwr = 0;
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex sum_corr_conj{ sample.re, -sample.im };
		xip_complex sum_corr_pwr;
		xip_multiply_complex(sample, sum_corr_conj, sum_corr_pwr);
		//xip_real_shift(sum_corr_pwr.re, -16);            // сдвигаем до [-2^16, +2^16]
		current_pwr += sum_corr_pwr.re;
		current_pwr -= pwrReg.back();
		pwrReg.pop_back();
		pwrReg.push_front(sum_corr_pwr.re);
		xip_real awg_pwr = current_pwr / wnd_size;
		xip_real awg_pwr_db = 10 * log10(awg_pwr);

		dbg_out << awg_pwr << '\t' << awg_pwr_db << endl;
	}

	dbg_out.close();
	fclose(in_file);
}

void signal_awgn(const string& in, const string& out, xip_real sig_pwr_db, xip_real snr_db, unsigned ns)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	//ofstream dbg_out("dbg_out.txt");
	xip_real noise_power = sig_pwr_db - snr_db;
	noise_power = std::pow(10, (sig_pwr_db - snr_db) / 10);
	xip_real noise_level = std::sqrt(noise_power * ns / 2);
	std::random_device rd{};
	std::mt19937 gen{ rd() };
	std::normal_distribution<double> rand_normal{ 0.0, 1.0 };

	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		double _re = rand_normal(gen);
		double _im = rand_normal(gen);
		xip_complex noise{ _re * noise_level, _im * noise_level };
		xip_complex res{ sample.re + noise.re, sample.im + noise.im };
		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);

		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void test_agc()
{
	AutoGaneControl agc(AGC_WND_SIZE_LOG2, get_cur_constell_pwr());	// АРУ
	ofstream dbg_out("dbg_out.txt");
	int N = 1000;
	for (int i = 0; i < N; i++) {
		xip_complex x{ 10000, 10000 };
		agc.process(x);
		xip_complex out{ 0, 0 };
		while (agc.next(out)) {
			dbg_out << out << endl;
		}
	}

	dbg_out.close();
}

