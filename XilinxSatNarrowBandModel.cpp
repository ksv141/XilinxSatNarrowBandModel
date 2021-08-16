// Smoke test program for bit accurate C model

#include <iostream>
#include <vector>
#include <complex>
#include <cstdlib>

#define _USE_MATH_DEFINES

#include <math.h>
#include <fstream> // for debug only

#include "cmpy_v6_0_bitacc_cmodel.h"
//#include "gmp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

// Debug functions
#define DEBUG 1

static void msg_print(void* dummy, int error, const char* msg)
{
	std::cout << msg << std::endl;
}
// End of debug functions

#define DATA_SIZE 10

int test_cmpy_v6_0_bitacc_cmodel()
{
	size_t ii; //loop variable for data samples
	xip_uint roundbit;
	xip_complex value;

	cout << "C model version = " << xip_cmpy_v6_0_get_version() << endl;

	// Create a configuration structure
	xip_cmpy_v6_0_config config, config_ret;
	xip_status status = xip_cmpy_v6_0_default_config(&config);

	if (status != XIP_STATUS_OK) {
		cerr << "ERROR: Could not get C model default configuration" << endl;
		return XIP_STATUS_ERROR;
	}

	//Firstly, create and exercise a simple configuration.
	config.APortWidth = 16;
	config.BPortWidth = 16;
	config.OutputWidth = 33;
	config.RoundMode = XIP_CMPY_V6_0_ROUND; //Note that the check later in this file assumes full width

	// Create model object
	xip_cmpy_v6_0* cmpy_std;
	cmpy_std = xip_cmpy_v6_0_create(&config, &msg_print, 0);

	if (status != XIP_STATUS_OK) {
		cerr << "ERROR: Could not create C model state object" << endl;
		return XIP_STATUS_ERROR;
	}

	// Can we read back the updated configuration correctly?
	if (xip_cmpy_v6_0_get_config(cmpy_std, &config_ret) != XIP_STATUS_OK) {
		cerr << "ERROR: Could not retrieve C model configuration" << endl;
	}

	int number_of_samples = DATA_SIZE;
	// Declare any arrays in the request structure and write pointers to them into the request structure

	// Create request and response structures
	// Create input data packet for operand A
	xip_array_complex* reqa = xip_array_complex_create();
	xip_array_complex_reserve_dim(reqa, 1); //dimensions are (Number of samples)
	reqa->dim_size = 1;
	reqa->dim[0] = number_of_samples;
	reqa->data_size = reqa->dim[0];
	if (xip_array_complex_reserve_data(reqa, reqa->data_size) == XIP_STATUS_OK) {
		cout << "INFO: Reserved memory for request as [" << number_of_samples << "] array " << endl;
	}
	else {
		cout << "ERROR: Unable to reserve memory for input data packet!" << endl;
		exit(2);
	}

	// Create input data packet for operand B
	xip_array_complex* reqb = xip_array_complex_create();
	xip_array_complex_reserve_dim(reqb, 1); //dimensions are (Number of samples)
	reqb->dim_size = 1;
	reqb->dim[0] = number_of_samples;
	reqb->data_size = reqb->dim[0];
	if (xip_array_complex_reserve_data(reqb, reqb->data_size) == XIP_STATUS_OK) {
		cout << "INFO: Reserved memory for request as [" << number_of_samples << "] array " << endl;
	}
	else {
		cout << "ERROR: Unable to reserve memory for input data packet!" << endl;
		exit(2);
	}

	// Create input data packet for ctrl input (Round bit)
	xip_array_uint* reqctrl = xip_array_uint_create();
	xip_array_uint_reserve_dim(reqctrl, 1); //dimensions are (Number of samples)
	reqctrl->dim_size = 1;
	reqctrl->dim[0] = number_of_samples;
	reqctrl->data_size = reqctrl->dim[0];
	if (xip_array_uint_reserve_data(reqctrl, reqctrl->data_size) == XIP_STATUS_OK) {
		cout << "INFO: Reserved memory for request as [" << number_of_samples << "] array " << endl;
	}
	else {
		cout << "ERROR: Unable to reserve memory for input data packet!" << endl;
		exit(2);
	}

	//create input data
	xip_complex a, b;
	for (ii = 0; ii < DATA_SIZE; ii++)
	{
		//roundbit = ii % 2;
		roundbit = 0;
		a.re = (xip_real)(ii+0.1);
		a.im = (xip_real)ii;
		b.re = (xip_real)(16 - ii);
		b.im = (xip_real)ii;
		if (xip_cmpy_v6_0_xip_array_complex_set_data(reqa, a, ii) != XIP_STATUS_OK)
			cerr << "Error in xip_array_complex_set_data" << endl;
		if (xip_cmpy_v6_0_xip_array_complex_set_data(reqb, b, ii) != XIP_STATUS_OK)
			cerr << "Error in xip_array_complex_set_data" << endl;
		if (xip_cmpy_v6_0_xip_array_uint_set_data(reqctrl, roundbit, ii) != XIP_STATUS_OK)
			cerr << "Error in xip_array_uint_set_data" << endl;
		cout << "Sample " << ii << " a = " << a.re << " + j" << a.im << endl;
		cout << "Sample " << ii << " b = " << b.re << " + j" << b.im << endl;

#if(DEBUG)
		//Check that data values can be read back from the input data structures
		if (xip_cmpy_v6_0_xip_array_complex_get_data(reqb, &value, ii) != XIP_STATUS_OK)
			cerr << "Error in xip_array_complex_get_data" << endl;
		cout << "input array b sample " << ii << ", real = " << value.re << " imag = " << value.im << endl;
#endif
	}

	// Request memory for output data
	xip_array_complex* response = xip_array_complex_create();
	xip_array_complex_reserve_dim(response, 1); //dimensions are (Number of samples)
	response->dim_size = 1;
	response->dim[0] = number_of_samples;
	response->data_size = response->dim[0];
	if (xip_array_complex_reserve_data(response, response->data_size) == XIP_STATUS_OK) {
		cout << "INFO: Reserved memory for response as [" << number_of_samples << "] array " << endl;
	}
	else {
		cout << "ERROR: Unable to reserve memory for output data packet!" << endl;
		exit(3);
	}

	// Run the model
	cout << "Running the C model..." << endl;

	if (xip_cmpy_v6_0_data_do(cmpy_std, reqa, reqb, reqctrl, response) != XIP_STATUS_OK) {
		cerr << "ERROR: C model did not complete successfully" << endl;
		xip_array_complex_destroy(reqa);
		xip_array_complex_destroy(reqb);
		xip_array_uint_destroy(reqctrl);
		xip_array_complex_destroy(response);
		xip_cmpy_v6_0_destroy(cmpy_std);
		return XIP_STATUS_ERROR;
	}
	else {
		cout << "C model completed successfully" << endl;
	}

#if(DEBUG)
	// When enabled, this will print the result data to stdout
	for (int sample = 0; sample < number_of_samples; sample++) {
		xip_cmpy_v6_0_xip_array_complex_get_data(response, &value, sample);
		cout << "Sample " << sample << ":  out real = " << value.re << " imag = " << value.im << endl;
	}
#endif

	// Check response is correct
	for (ii = 0; ii < DATA_SIZE; ii++)
	{
		//This example has natural width, so simple calculation
		xip_complex expected, got, x, y;
		xip_cmpy_v6_0_xip_array_complex_get_data(reqa, &x, ii);
		xip_cmpy_v6_0_xip_array_complex_get_data(reqb, &y, ii);
		xip_cmpy_v6_0_xip_array_complex_get_data(response, &got, ii);

		//Note that the following equations assume that the output width is the full
		//width of the calculation, i.e. neither truncation nor rounding occurs
		expected.re = x.re*y.re - x.im*y.im;
		expected.im = x.re*y.im + x.im*y.re;
		if (expected.re != got.re || expected.im != got.im) {
			cerr << "ERROR: C model data output is incorrect for sample" << ii << "Expected real = " << expected.re << " imag = " << expected.im << " Got real = " << got.re << " imag = " << got.im << endl;

			xip_array_complex_destroy(reqa);
			xip_array_complex_destroy(reqb);
			xip_array_uint_destroy(reqctrl);
			xip_array_complex_destroy(response);
			xip_cmpy_v6_0_destroy(cmpy_std);
			return XIP_STATUS_ERROR;
		}
		else {
			cout << "Sample " << ii << " was as expected" << endl;
		}
	}
	cout << "C model data output is correct" << endl;

	// Clean up
	xip_array_complex_destroy(reqa);
	xip_array_complex_destroy(reqb);
	xip_array_uint_destroy(reqctrl);
	xip_array_complex_destroy(response);
	cout << "C model input and output data freed" << endl;

	xip_cmpy_v6_0_destroy(cmpy_std);
	cout << "C model destroyed" << endl;

	//End of test of simple configuration

	// We will already have returned if there was an error
	return XIP_STATUS_OK;
}