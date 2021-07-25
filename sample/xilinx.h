#pragma once

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "xfft_v9_1_bitacc_cmodel.h"

#include "cordic.h"

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <complex>
#include <cmath>

#include <math.h>
#include <fstream> // for debug only

#define _USE_MATH_DEFINES

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class xilinx_fft
	{
	private:
		const unsigned int C_NFFT_MAX;
		unsigned int n_fft;

		xilinx_ip_xfft_v9_1_state* state;
		xilinx_ip_xfft_v9_1_generics generics;
		xilinx_ip_xfft_v9_1_inputs inputs;

	public:
		xilinx_fft(xilinx_ip_xfft_v9_1_generics gen);
		~xilinx_fft();
		void process(double in_re[], double in_im[],
			double*& out_re, double*& out_im, uint64_t size_);
	};

	class xilinx_ifft
	{
	private:
		const unsigned int C_NFFT_MAX;
		unsigned int n_fft;

		xilinx_ip_xfft_v9_1_state* state;
		xilinx_ip_xfft_v9_1_generics generics;
		xilinx_ip_xfft_v9_1_inputs inputs;

	public:
		xilinx_ifft(xilinx_ip_xfft_v9_1_generics gen);
		~xilinx_ifft();

		void process(double in_re[], double in_im[],
			double*& out_re, double*& out_im, uint64_t size_);
	};


	class complex_multiplier
	{
	private:
		xip_status status;
		xip_cmpy_v6_0* cmpy_std;

		xip_array_complex* reqa;
		xip_array_complex* reqb;
		xip_array_uint* reqctrl;

		unsigned int data_size;

		//static void msg_print(void* dummy, int error, const char* msg);

	public:
		struct cmpl_multiplier_params
		{
			unsigned int rows_size;
			unsigned int colums_size;

			unsigned int data_size;
			unsigned char A_port_width;
			unsigned char B_port_width;
			unsigned char out_port_width;
			int round_mode;

			double coeff_double_to_int_A_port;
			double coeff_double_to_int_B_port;
		};

		cmpl_multiplier_params params;

		xip_array_complex* response;

		complex_multiplier(cmpl_multiplier_params param);

		~complex_multiplier();

		void set_data(double*& a_re, double*& a_im,
			double*& b_re, double*& b_im);

		void process(double*& a_re, double*& a_im,
			double*& b_re, double*& b_im);
	};