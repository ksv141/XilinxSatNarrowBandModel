#include "debug.h"

void msg_print(void* dummy, int error, const char* msg)
{
	printf("%s\n", msg);
}

//---------------------------------------------------------------------------------------------------------------------
//Print a xip_array_real
void print_array_real(const xip_array_real* x)
{
	putchar('[');
	if (x && x->data && x->data_size)
	{
		const xip_real* p = x->data;
		const xip_real* q = x->data;
		const xip_real* r = x->data + x->data_size;
		while (q != r)
		{
			if (q != p) putchar(' ');
			printf("%g", *q);
			q++;
		}
	}
	putchar(']');
	putchar('\n');
}
//---------------------------------------------------------------------------------------------------------------------
// Print a xip_array_complex
void print_array_complex(const xip_array_complex * x)
{
	putchar('[');
	if (x && x->data && x->data_size)
	{
		const xip_complex* p = x->data;
		const xip_complex* q = x->data;
		const xip_complex* r = x->data + x->data_size;
		while (q != r)
		{
			if (q != p) putchar(' ');
			if (!q->re) printf("%gi", q->im);  // Bi
			else if (q->im < 0.0) printf("%g%gi", q->re, q->im);  // A-Bi
			else if (q->im > 0.0) printf("%g+%gi", q->re, q->im);  // A+Bi
			else                 printf("%g", q->re);  // A
			q++;
		}
	}
	putchar(']');
	putchar('\n');
}

//----------------------------------------------------------------------------------------------------------------------
// String arrays used by the print_config funtion
const char* filt_desc[5] = { "SINGLE RATE", "INTERPOLATION", "DECIMATION", "HILBERT", "INTERPOLATED" };
const char* seq_desc[2] = { "Basic" , "Advanced" };

//----------------------------------------------------------------------------------------------------------------------
// Print a summary of a filter configuration
int print_config(const xip_fir_v7_2_config * cfg) {
	printf("Configuration of %s:\n", cfg->name);
	printf("\tFilter       : ");
	if (cfg->filter_type == XIP_FIR_SINGLE_RATE || cfg->filter_type == XIP_FIR_HILBERT) {
		printf("%s\n", filt_desc[cfg->filter_type]);
	}
	else if (cfg->filter_type == XIP_FIR_INTERPOLATED) {
		printf("%s by %d\n", filt_desc[cfg->filter_type], cfg->zero_pack_factor);
	}
	else {
		printf("%s up by %d down by %d\n", filt_desc[cfg->filter_type], cfg->interp_rate, cfg->decim_rate);
	}
	printf("\tCoefficients : %d ", cfg->coeff_sets);
	if (cfg->is_halfband) {
		printf("Halfband ");
	}
	if (cfg->reloadable) {
		printf("Reloadable ");
	}
	printf("coefficient set(s) of %d taps\n", cfg->num_coeffs);
	printf("\tData         : %d path(s) of %d %s channel(s)\n", cfg->num_paths, cfg->num_channels, seq_desc[cfg->chan_seq]);

	return 0;
}

//----------------------------------------------------------------------------------------------------------------------
// Fill data array with a scaled impulse. Assumes 3-D array.
int test_create_impulse(xip_array_real* x) {
	for (int path = 0; path < x->dim[0]; path++) {
		for (int chan = 0; chan < x->dim[1]; chan++) {
			for (int i = 1; i < x->dim[2]; i++) {
				xip_fir_v7_2_xip_array_real_set_chan(x, 0, path, chan, i, P_BASIC);
			}
		}
	}
	xip_fir_v7_2_xip_array_real_set_chan(x, 4096, 0, 0, 0, P_BASIC);
	xip_fir_v7_2_xip_array_real_set_chan(x, 4096, 0, 1, 0, P_BASIC);
	return 0;
}
