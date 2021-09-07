#include "test_utils.h"

static const double PI = 3.141592653589793238463;
static const double _2_PI = PI * 2;

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
		xip_complex_shift(res, -(int)(dds.getOutputWidth()-1));	// ��������� ������������ �������� ���������� (��������� ������� �����)
		
		if (min_val > res.im)	// ��� ������� ������������ ���������������
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

void generate_sin_signal(const string& out, double freq, double sample_freq, size_t count, int bits)
{
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	//ofstream dbg_out("dbg_out.txt");

	double dph = _2_PI * freq / sample_freq;	// ����� ���� �� ����
	double ph = 0;						// ������� ����
	complex<double> sample;
	complex<double> _im_j{ 0, 1 };		// ������ �������
	for (size_t i = 0; i < count; i++) {
		ph = fmod(ph + dph, _2_PI);
		sample = exp(_im_j * (ph));		// ������ � ���������� 1
		xip_complex res{ sample.real(), sample.imag() };
		xip_complex_shift(res, bits-1);
		tC::write_real<int16_t>(out_file, res.re);
		tC::write_real<int16_t>(out_file, res.im);
		//dbg_out << res << endl;
	}
	//dbg_out.close();
	fclose(out_file);
}
