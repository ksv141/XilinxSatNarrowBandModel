#include "xilinx_classes.h"

xilinx_fft::xilinx_fft(xilinx_ip_xfft_v9_1_generics gen) : C_NFFT_MAX(gen.C_NFFT_MAX)
	{
		n_fft = pow(2, C_NFFT_MAX);

		generics = gen;

		// Create FFT state
		state = xilinx_ip_xfft_v9_1_create_state(generics);
		if (state == NULL) {
			std::cerr << "ERROR: could not create FFT state object" << std::endl;
		}

		inputs.nfft = C_NFFT_MAX;

		// Set scaling schedule to 1/N : 2 in each stage for radix-4 / streaming, 1 in each stage for radix-2 [Lite]
		const int stages = (generics.C_ARCH == 1 || generics.C_ARCH == 3) ? (C_NFFT_MAX + 1) / 2 : C_NFFT_MAX;
		const int scaling = (generics.C_ARCH == 1 || generics.C_ARCH == 3) ? 2 : 1;

		int* scaling_sch = new int[stages];

		for (int i = 0; i < stages; i++) {
			if (i == stages - 1 && (generics.C_ARCH == 1 || generics.C_ARCH == 3) && inputs.nfft % 2 == 1) {
				// Scaling must be 1 or 0 in the final stage when log2(point size) is odd
				// for Radix-4 or Pipelined Streaming architectures
				scaling_sch[i] = 1;
			}
			else {
				scaling_sch[i] = scaling;
			}
		}

		inputs.scaling_sch = scaling_sch;
		inputs.scaling_sch_size = stages;

		// Set direction to FFT
		inputs.direction = 1;
	}

	xilinx_fft::~xilinx_fft()
	{
		xilinx_ip_xfft_v9_1_destroy_state(state);
	}

	void xilinx_fft::process(double in_re[], double in_im[],
		double*& out_re, double*& out_im, uint64_t size_)
	{
		inputs.xn_re = in_re;
		inputs.xn_re_size = size_;
		inputs.xn_im = in_im;
		inputs.xn_im_size = size_;

		// Создание структуры для выходов БПФ и массивов выходных данных
		struct xilinx_ip_xfft_v9_1_outputs outputs;
		const int out_smpl_size = size_;
		std::vector<double> xk_re(n_fft);
		std::vector<double> xk_im(n_fft);
		outputs.xk_re = xk_re.data();
		outputs.xk_re_size = n_fft;
		outputs.xk_im = xk_im.data();
		outputs.xk_im_size = n_fft;

		if (xilinx_ip_xfft_v9_1_bitacc_simulate(state, inputs, &outputs) != 0) {
			std::cerr << "ERROR: simulation did not complete successfully" << std::endl;
			// Destroy the FFT state to free up memory
			xilinx_ip_xfft_v9_1_destroy_state(state);
			return;
		}

		/*if (out_re) delete[]out_re;
		if (out_im) delete[]out_im;

		out_re = new double[size_];
		out_im = new double[size_];*/

		/*out_re = (double*)realloc(out_re, size_ * sizeof(double));
		out_im = (double*)realloc(out_im, size_ * sizeof(double));*/

		memcpy(out_re, outputs.xk_re, size_ * sizeof(double));
		memcpy(out_im, outputs.xk_im, size_ * sizeof(double));
	}


	xilinx_ifft::xilinx_ifft(xilinx_ip_xfft_v9_1_generics gen) : C_NFFT_MAX(gen.C_NFFT_MAX)
	{
		n_fft = pow(2, C_NFFT_MAX);

		generics = gen;

		// Create FFT state
		state = xilinx_ip_xfft_v9_1_create_state(generics);
		if (state == NULL) {
			std::cerr << "ERROR: could not create FFT state object" << std::endl;
		}

		inputs.nfft = C_NFFT_MAX;

		// Set scaling schedule to 1/N : 2 in each stage for radix-4 / streaming, 1 in each stage for radix-2 [Lite]
		const int stages = (generics.C_ARCH == 1 || generics.C_ARCH == 3) ? (C_NFFT_MAX + 1) / 2 : C_NFFT_MAX;
		const int scaling = (generics.C_ARCH == 1 || generics.C_ARCH == 3) ? 2 : 1;

		int* scaling_sch = new int[stages];

		for (int i = 0; i < stages; i++) {
			if (i == stages - 1 && (generics.C_ARCH == 1 || generics.C_ARCH == 3) && inputs.nfft % 2 == 1) {
				// Scaling must be 1 or 0 in the final stage when log2(point size) is odd
				// for Radix-4 or Pipelined Streaming architectures
				scaling_sch[i] = 1;
			}
			else {
				scaling_sch[i] = scaling;
			}
		}

		inputs.scaling_sch = scaling_sch;
		inputs.scaling_sch_size = stages;

		// Set direction to FFT
		inputs.direction = 0;
	}

	xilinx_ifft::~xilinx_ifft()
	{
		xilinx_ip_xfft_v9_1_destroy_state(state);
	}

	void xilinx_ifft::process(double in_re[], double in_im[],
		double*& out_re, double*& out_im, uint64_t size_)
	{
		inputs.xn_re = in_re;
		inputs.xn_re_size = size_;
		inputs.xn_im = in_im;
		inputs.xn_im_size = size_;

		// Создание структуры для выходов БПФ и массивов выходных данных
		struct xilinx_ip_xfft_v9_1_outputs outputs;
		const int out_smpl_size = size_;
		std::vector<double> xk_re(n_fft);
		std::vector<double> xk_im(n_fft);
		outputs.xk_re = xk_re.data();
		outputs.xk_re_size = n_fft;
		outputs.xk_im = xk_im.data();
		outputs.xk_im_size = n_fft;

		if (xilinx_ip_xfft_v9_1_bitacc_simulate(state, inputs, &outputs) != 0) {
			std::cerr << "ERROR: simulation did not complete successfully" << std::endl;
			// Destroy the FFT state to free up memory
			xilinx_ip_xfft_v9_1_destroy_state(state);
			return;
		}

		if (out_re) delete[]out_re;
		if (out_im) delete[]out_im;

		out_re = new double[size_];
		out_im = new double[size_];

		/*out_re = (double*)realloc(out_re, size_ * sizeof(double));
		out_im = (double*)realloc(out_im, size_ * sizeof(double));*/

		memcpy(out_re, outputs.xk_re, size_ * sizeof(double));
		memcpy(out_im, outputs.xk_im, size_ * sizeof(double));
	}


	complex_multiplier::complex_multiplier(cmpl_multiplier_params param)
	{
		params = param;

		data_size = params.data_size;

		// Create a configuration structure
		xip_cmpy_v6_0_config config, config_ret;
		status = xip_cmpy_v6_0_default_config(&config);

		if (status != XIP_STATUS_OK) {
			std::cerr << "ERROR: Could not get C model default configuration " << "err: XIP_STATUS_ERROR" << std::endl;
		}

		//Firstly, create and exercise a simple configuration.
		config.APortWidth = params.A_port_width;
		config.BPortWidth = params.B_port_width;
		config.OutputWidth = params.out_port_width;
		config.RoundMode = params.round_mode; //Note that the check later in this file assumes full width

		cmpy_std = xip_cmpy_v6_0_create(&config, NULL, 0);

		// Можем ли мы правильно прочитать обновленную конфигурацию?
		if (xip_cmpy_v6_0_get_config(cmpy_std, &config_ret) != XIP_STATUS_OK) {
			std::cerr << "ERROR: Could not retrieve C model configuration" << std::endl;
		}

		int number_of_samples = data_size;
		// Объявите любые массивы в структуре запроса и запишите указатели на них в структуру запроса

		// Создание структур запросов и ответов
		// Создание пакета входных данных для операнда A
		reqa = xip_array_complex_create(); //Выделяет память для самой структуры, а не для элементов массива внутри нее.
		xip_array_complex_reserve_dim(reqa, 1); //Выделяет небольшой массив, который должен содержать размер каждого измерения массива данных. Например, 100 выборок х 4  канала х 3 поля.
		reqa->dim_size = 1;
		reqa->dim[0] = number_of_samples;
		reqa->data_size = reqa->dim[0];
		if (xip_array_complex_reserve_data(reqa, reqa->data_size) == XIP_STATUS_OK)    //(Re) выделяет достаточно памяти для максимального размера. Ошибка возвращается, если data_capacity структуры больше выделенного пространства.
		{
			std::cout << "INFO: Reserved memory for request as [" << number_of_samples << "] array " << std::endl;
		}
		else {
			std::cout << "ERROR: Unable to reserve memory for input data packet!" << std::endl;
			exit(2);
		}

		// Создание пакета входных данных для операнда B
		reqb = xip_array_complex_create();
		xip_array_complex_reserve_dim(reqb, 1); //dimensions are (Number of samples)
		reqb->dim_size = 1;
		reqb->dim[0] = number_of_samples;
		reqb->data_size = reqb->dim[0];
		if (xip_array_complex_reserve_data(reqb, reqb->data_size) == XIP_STATUS_OK) {
			std::cout << "INFO: Reserved memory for request as [" << number_of_samples << "] array " << std::endl;
		}
		else {
			std::cout << "ERROR: Unable to reserve memory for input data packet!" << std::endl;
			exit(2);
		}

		// Создать пакет входных данных для ввода ctrl (круглый бит)
		reqctrl = xip_array_uint_create();
		xip_array_uint_reserve_dim(reqctrl, 1); //dimensions are (Number of samples)
		reqctrl->dim_size = 1;
		reqctrl->dim[0] = number_of_samples;
		reqctrl->data_size = reqctrl->dim[0];
		if (xip_array_uint_reserve_data(reqctrl, reqctrl->data_size) == XIP_STATUS_OK) {
			std::cout << "INFO: Reserved memory for request as [" << number_of_samples << "] array " << std::endl;
		}
		else {
			std::cout << "ERROR: Unable to reserve memory for input data packet!" << std::endl;
			exit(2);
		}

		// Request memory for output data
		response = xip_array_complex_create();
		xip_array_complex_reserve_dim(response, 1); //dimensions are (Number of samples)
		response->dim_size = 1;
		response->dim[0] = number_of_samples;
		response->data_size = response->dim[0];
		if (xip_array_complex_reserve_data(response, response->data_size) == XIP_STATUS_OK) {
			std::cout << "INFO: Reserved memory for response as [" << number_of_samples << "] array " << std::endl;
		}
		else {
			std::cout << "ERROR: Unable to reserve memory for output data packet!" << std::endl;
			exit(3);
		}

		/*std::complex<double>** out_mtx = new std::complex<double>*[params.rows_size];

		for (int i = 0; i < params.rows_size; i++)
		{
			out_mtx[i] = new std::complex<double>[params.colums_size];
		}*/
	}

	complex_multiplier::~complex_multiplier()
	{
		//delete[] out_mtx;

		xip_array_complex_destroy(reqa);
		xip_array_complex_destroy(reqb);
		xip_array_uint_destroy(reqctrl);
		xip_array_complex_destroy(response);
		xip_cmpy_v6_0_destroy(cmpy_std);
	}

	void complex_multiplier::set_data(double*& a_re, double*& a_im,
		double*& b_re, double*& b_im)
	{
		xip_uint roundbit;
		xip_complex value;

		xip_complex a, b;

		for (int i = 0; i < data_size; i++)
		{
			roundbit = i % 2;
			a.re = (xip_real)a_re[i];
			a.im = (xip_real)a_im[i];
			b.re = (xip_real)b_re[i];
			b.im = (xip_real)b_im[i];

			if (xip_cmpy_v6_0_xip_array_complex_set_data(reqa, a, i) != XIP_STATUS_OK)
				std::cerr << "Error in xip_array_complex_set_data" << std::endl;
			if (xip_cmpy_v6_0_xip_array_complex_set_data(reqb, b, i) != XIP_STATUS_OK)
				std::cerr << "Error in xip_array_complex_set_data" << std::endl;
			if (xip_cmpy_v6_0_xip_array_uint_set_data(reqctrl, roundbit, i) != XIP_STATUS_OK)
				std::cerr << "Error in xip_array_uint_set_data" << std::endl;

			/*std::cout << "Sample " << i << " a = " << a.re << " + j" << a.im << std::endl;
			std::cout << "Sample " << i << " b = " << b.re << " + j" << b.im << std::endl;*/

	#if(DEBUG)
			//Check that data values can be read back from the input data structures
			if (xip_cmpy_v6_0_xip_array_complex_get_data(reqb, &value, ii) != XIP_STATUS_OK)
				std::cerr << "Error in xip_array_complex_get_data" << std::endl;
			std::cout << "input array b sample " << ii << ", real = " << value.re << " imag = " << value.im << std::endl;
	#endif
		}
	}

	void complex_multiplier::process(double*& a_re, double*& a_im,
		double*& b_re, double*& b_im)
	{
		set_data(a_re, a_im, b_re, b_im);

		if (xip_cmpy_v6_0_data_do(cmpy_std, reqa, reqb, reqctrl, response) != XIP_STATUS_OK) {
			std::cerr << "ERROR: C model did not complete successfully" << " err: " << XIP_STATUS_ERROR << std::endl;
			xip_array_complex_destroy(reqa);
			xip_array_complex_destroy(reqb);
			xip_array_uint_destroy(reqctrl);
			xip_array_complex_destroy(response);
			xip_cmpy_v6_0_destroy(cmpy_std);
			return;
		}
		else {
			std::cout << "C model completed successfully" << std::endl;
		}
	}