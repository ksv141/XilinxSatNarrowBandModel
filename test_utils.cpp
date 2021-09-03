#include "test_utils.h"

void signal_freq_shift(const string& in, const string& out, double dph)
{
	FILE* in_file = fopen(in.c_str(), "rb");
	if (!in_file)
		return;
	FILE* out_file = fopen(out.c_str(), "wb");
	if (!out_file)
		return;

	ofstream dbg_out("dbg_out.txt");

	fx_cmpl_point in_sample;
	int16_t re;
	int16_t im;
	while (tC::read_real<int16_t, int16_t>(in_file, re) &&
			tC::read_real<int16_t, int16_t>(in_file, im)) {
		dbg_out << re << "\t" << im << endl;
	}

	dbg_out.close();
	fclose(in_file);
	fclose(out_file);
}
