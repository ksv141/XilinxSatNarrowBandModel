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
	double min_val = 1000000;
	double max_val = -1000000;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
			tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		dds.process(dph, dds_phase, dds_sin, dds_cos);
		xip_complex mod_sample{ dds_cos, dds_sin };
		xip_complex res;
		xip_multiply_complex(sample, mod_sample, res);
		xip_complex_shift(res, -(int)(dds.getOutputWidth()-1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)
		
		if (min_val > res.im)	// для подбора коэффициента масштабирования
			min_val = res.im;
		if (max_val < res.im)
			max_val = res.im;

		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);

		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}

void signal_time_shift(const string& in, const string& out, int time_shift)
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
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
		tC::read_real<int16_t, int16_t>(in_file, im)) {
		xip_complex sample{ re, im };
		xip_complex res;
		itrp.process(sample, res, time_shift);

		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);

		//dbg_out << res << endl;
	}

	//dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}
