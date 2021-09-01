// Smoke test program for bit accurate C model

#include <iostream>
#include <vector>
#include <complex>
#include <cstdlib>

#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>
#include <fstream> // for debug only

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "dds_compiler_v6_0_bitacc_cmodel.h"
#include "debug.h"
#include "XilinxIpTests.h"
//#include "gmp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

// Debug functions
#define DEBUG 1

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
	config.RoundMode = XIP_CMPY_V6_0_TRUNCATE; //Note that the check later in this file assumes full width

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
		roundbit = ii % 2;
		a.re = (xip_real)ii;
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
	xip_array_complex * response = xip_array_complex_create();
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
		expected.re = x.re * y.re - x.im * y.im;
		expected.im = x.re * y.im + x.im * y.re;
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

//----------------------------------------------------------------------------------------------------------------------
// Fill data array with a scaled impulse. Assumes 3-D array.
int create_impulse(xip_array_real* x) {
	int path;
	int chan;
	int i;
	for (path = 0; path < x->dim[0]; path++) {
		for (chan = 0; chan < x->dim[1]; chan++) {
			xip_fir_v7_2_xip_array_real_set_chan(x, (double)((path + 1) * (chan + 1)), path, chan, 0, P_BASIC);
			for (i = 1; i < x->dim[2]; i++) {
				xip_fir_v7_2_xip_array_real_set_chan(x, 0, path, chan, i, P_BASIC);
			}
		}
	}
	return 0;
}

int test_xip_fir_bitacc_cmodel()
{
	const char* ver_str = xip_fir_v7_2_get_version();
	printf("-------------------------------------------------------------------------------\n");
	printf("FIR Compiler C Model version: %s\n", ver_str);

	printf("-------------------------------------------------------------------------------\n");
	printf("Default core....\n");
	printf("-------------------------------------------------------------------------------\n");

	// Define filter configuration
	xip_fir_v7_2_config fir_default_cnfg;
	xip_fir_v7_2_default_config(&fir_default_cnfg);

	fir_default_cnfg.name = "fir_default";
	print_config(&fir_default_cnfg);

	//Create filter instances
	xip_fir_v7_2* fir_default = xip_fir_v7_2_create(&fir_default_cnfg, &msg_print, 0);
	if (!fir_default) {
		printf("Error creating instance %s\n", fir_default_cnfg.name);
		return -1;
	}
	else {
		printf("Created instance %s\n", fir_default_cnfg.name);
	}

	// Create input data packet
	xip_array_real* din = xip_array_real_create();
	xip_array_real_reserve_dim(din, 3);
	din->dim_size = 3; // 3D array
	din->dim[0] = fir_default_cnfg.num_paths;
	din->dim[1] = fir_default_cnfg.num_channels;
	din->dim[2] = fir_default_cnfg.num_coeffs; // vectors in a single packet
	din->data_size = din->dim[0] * din->dim[1] * din->dim[2];
	if (xip_array_real_reserve_data(din, din->data_size) == XIP_STATUS_OK) {
		printf("Reserved data\n");
	}
	else {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// Create output data packet
	//  - Automatically sized using xip_fir_v7_2_calc_size
	xip_array_real* fir_default_out = xip_array_real_create();
	xip_array_real_reserve_dim(fir_default_out, 3);
	fir_default_out->dim_size = 3; // 3D array
	if (xip_fir_v7_2_calc_size(fir_default, din, fir_default_out, 0) == XIP_STATUS_OK) {
		printf("Calculated output data size\n");
		if (xip_array_real_reserve_data(fir_default_out, fir_default_out->data_size) == XIP_STATUS_OK) {
			printf("Reserved data\n");
		}
		else {
			printf("Unable to reserve data!\n");
			return -1;
		}
	}
	else {
		printf("Unable to calculate output date size\n");
		return -1;
	}

	// Populate data in with an impulse
	printf("Create impulse\n");
	create_impulse(din);

	// Send input data and filter
	if (xip_fir_v7_2_data_send(fir_default, din) == XIP_STATUS_OK) {
		printf("Sent data     : ");
		print_array_real(din);
	}
	else {
		printf("Error sending data\n");
		return -1;
	}

	// Retrieve filtered data
	if (xip_fir_v7_2_data_get(fir_default, fir_default_out, 0) == XIP_STATUS_OK) {
		printf("Fetched result: ");
		print_array_real(fir_default_out);
	}
	else {
		printf("Error getting data\n");
		return -1;
	}

	printf("\nTest reset....\n");

	// Send the same data but only fetch first half of the output, reset and then repeat but fetch all the data

	if (xip_fir_v7_2_data_send(fir_default, din) == XIP_STATUS_OK) {
		printf("Sent data     : ");
		print_array_real(din);
	}
	else {
		printf("Error sending data\n");
		return -1;
	}

	// Create a new output data packet
	xip_array_real* fir_default_out2 = xip_array_real_create();
	xip_array_real_reserve_dim(fir_default_out2, 3);
	fir_default_out2->dim_size = 3; // 3D array
	if (xip_fir_v7_2_calc_size(fir_default, din, fir_default_out2, 0) == XIP_STATUS_OK) {
		printf("Calculated output data size\n");
		// Reduce output packet to only read half the generated data
		fir_default_out2->dim[2] = fir_default_out2->dim[2] / 2;
		fir_default_out2->data_size = fir_default_out2->dim[1] * fir_default_out2->dim[1] * fir_default_out2->dim[2];
		if (xip_array_real_reserve_data(fir_default_out2, fir_default_out2->data_size) == XIP_STATUS_OK) {
			printf("Reserved data\n");
		}
		else {
			printf("Unable to reserve data!\n");
			return -1;
		}
	}
	else {
		printf("Unable to calculate output date size\n");
		return -1;
	}

	// Get reduced output data packet
	if (xip_fir_v7_2_data_get(fir_default, fir_default_out2, 0) == XIP_STATUS_OK) {
		printf("Fetched result: ");
		print_array_real(fir_default_out2);
	}
	else {
		printf("Error getting data\n");
		return -1;
	}

	// Apply reset to model
	printf("Apply reset\n");
	if (xip_fir_v7_2_reset(fir_default) != XIP_STATUS_OK) {
		printf("Error applying reset\n");
		return -1;
	}

	// Try fetching some data
	if (xip_fir_v7_2_data_get(fir_default, fir_default_out2, 0) == XIP_STATUS_OK) {
		if (fir_default_out2->dim[2] == 0) {
			printf("Zero data fetched\n");
		}
		else {
			printf("Error, fetched result: ");
			print_array_real(fir_default_out2);
			return -1;
		}
	}
	else {
		printf("Error getting data\n");
		return -1;
	}

	// Send input data again and read output
	if (xip_fir_v7_2_data_send(fir_default, din) == XIP_STATUS_OK) {
		printf("Sent data     : ");
	}
	else {
		printf("Error sending data\n");
		return -1;
	}
	print_array_real(din);

	// Read full output packet size
	fir_default_out2->dim[2] = fir_default_cnfg.num_coeffs;
	fir_default_out2->data_size = fir_default_out2->dim[1] * fir_default_out2->dim[1] * fir_default_out2->dim[2];
	if (xip_array_real_reserve_data(fir_default_out2, fir_default_out2->data_size) == XIP_STATUS_OK) {
		printf("Reserved data\n");
	}
	else {
		printf("Unable to reserve data!\n");
		return -1;
	}

	if (xip_fir_v7_2_data_get(fir_default, fir_default_out2, 0) == XIP_STATUS_OK) {
		printf("Fetched result: ");
	}
	else {
		printf("Error getting data\n");
		return -1;
	}
	print_array_real(fir_default_out2);

	//De-allocate data
	xip_array_real_destroy(din);
	xip_array_real_destroy(fir_default_out);
	xip_array_real_destroy(fir_default_out2);

	//De-allocate fir instances
	xip_fir_v7_2_destroy(fir_default);

	printf("-------------------------------------------------------------------------------\n");
	printf("Config and reload example....\n");
	printf("-------------------------------------------------------------------------------\n");

	// Define filter configurations
	xip_fir_v7_2_config fir1_cnfg, fir2_cnfg;
	xip_fir_v7_2_default_config(&fir1_cnfg);

	fir1_cnfg.name = "fir1";
	fir1_cnfg.num_channels = 3;
	fir1_cnfg.coeff_sets = 2;
	fir1_cnfg.num_coeffs = 10;
	fir1_cnfg.reloadable = 1;
	fir1_cnfg.config_method = XIP_FIR_CONFIG_BY_CHANNEL;

	xip_fir_v7_2_default_config(&fir2_cnfg);
	fir2_cnfg.name = "fir2";
	fir2_cnfg.num_channels = 3;
	const double fir2_coeffs[2] = { 0,2 };
	fir2_cnfg.coeff = &fir2_coeffs[0];
	fir2_cnfg.num_coeffs = 2;

	//Create filter instances
	print_config(&fir1_cnfg);
	xip_fir_v7_2* fir1 = xip_fir_v7_2_create(&fir1_cnfg, &msg_print, 0);
	if (!fir1) {
		printf("Error creating instance\n");
		return -1;
	}
	else {
		printf("Created instance\n");
	}

	print_config(&fir2_cnfg);
	xip_fir_v7_2* fir2 = xip_fir_v7_2_create(&fir2_cnfg, &msg_print, 0);
	if (!fir2) {
		printf("Error creating instance\n");
		return -1;
	}
	else {
		printf("Created instance\n");
	}

	// Create input data packet
	din = xip_array_real_create();
	xip_array_real_reserve_dim(din, 3);
	din->dim_size = 3; // 3D array
	din->dim[0] = fir1_cnfg.num_paths; // Number of paths
	din->dim[1] = fir1_cnfg.num_channels; // Number of channels
	din->dim[2] = fir1_cnfg.num_coeffs; // vectors in a single packet
	din->data_size = din->dim[0] * din->dim[1] * din->dim[2];
	if (xip_array_real_reserve_data(din, din->data_size) == XIP_STATUS_OK) {
		printf("Reserved data\n");
	}
	else {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// Create output data packet
	//  - Automatically sized using xip_fir_v7_2_calc_size
	xip_array_real* fir1_out = xip_array_real_create();
	xip_array_real_reserve_dim(fir1_out, 3);
	fir1_out->dim_size = 3; // 3D array
	if (xip_fir_v7_2_calc_size(fir1, din, fir1_out, 0) == XIP_STATUS_OK) {
		printf("Calculated output data size\n");
		if (xip_array_real_reserve_data(fir1_out, fir1_out->data_size) == XIP_STATUS_OK) {
			printf("Reserved data\n");
		}
		else {
			printf("Unable to reserve data!\n");
			return -1;
		}
	}
	else {
		printf("Unable to calculate output date size\n");
		return -1;
	}

	// Create config packet
	xip_array_uint* fsel = xip_array_uint_create();
	xip_array_uint_reserve_dim(fsel, 1);
	fsel->dim_size = 1;
	fsel->dim[0] = fir1_cnfg.num_channels;
	fsel->data_size = fsel->dim[0];
	if (xip_array_uint_reserve_data(fsel, fsel->data_size) == XIP_STATUS_OK) {
		printf("Reserved data\n");
	}
	else {
		printf("Unable to reserve data!\n");
		return -1;
	}
	xip_fir_v7_2_cnfg_packet cnfg;
	cnfg.fsel = fsel;
	cnfg.fsel->data[0] = 0;
	cnfg.fsel->data[1] = 1; // Set 2nd channel to use 2nd coefficient set
	cnfg.fsel->data[2] = 4; // Set invalid filter set to demonstrate error generated by model

	// Create reload packet
	xip_real new_coeffs[10] = { 1,2,3,4,5,6,7,8,9,10 };

	xip_fir_v7_2_rld_packet rld;
	rld.fsel = 0;
	rld.coeff = xip_array_real_create();
	xip_array_real_reserve_dim(rld.coeff, 1);
	rld.coeff->dim_size = 1;
	rld.coeff->dim[0] = 10;
	rld.coeff->data_size = rld.coeff->dim[0];
	if (xip_array_real_reserve_data(rld.coeff, rld.coeff->data_size) == XIP_STATUS_OK) {
		printf("Reserved coeff\n");
	}
	else {
		printf("Unable to reserve coeff!\n");
		return -1;
	}
	// Copy coefficients into reload packet
	int coeff_i;
	for (coeff_i = 0; coeff_i < 10; coeff_i++) {
		rld.coeff->data[coeff_i] = new_coeffs[coeff_i];
	}

	printf("Test default configuration of %s......\n", fir1_cnfg.name);

	// Define input data
	printf("Create impulse\n");
	create_impulse(din);

	// Send input data and filter
	if (xip_fir_v7_2_data_send(fir1, din) == XIP_STATUS_OK) {
		printf("Sent data     : ");
		print_array_real(din);
	}
	else {
		printf("Error sending data\n");
		return -1;
	}

	// Retrieve filtered data
	if (xip_fir_v7_2_data_get(fir1, fir1_out, 0) == XIP_STATUS_OK) {
		printf("Fetched result: ");
		print_array_real(fir1_out);
	}
	else {
		printf("Error getting data\n");
		return -1;
	}

	// Connect the 1st filter's output to the 2nd filters input
	printf("\nConnect %s and %s and test in a chain......\n", fir1_cnfg.name, fir2_cnfg.name);

	// Register fir1_out array as data sink of fir1
	xip_fir_v7_2_set_data_sink(fir1, fir1_out, 0);

	// Set xip_fir_v7_2_data_send_handler of fir2 as the data handler of fir1's output array
	xip_fir_v7_2_set_data_handler(fir1, &xip_fir_v7_2_data_send_handler, fir2, 0);

	// Create output data structure for 2nd filter and use as a data sink, i.e. filter data does not need to be "pulled" using data_get
	xip_array_real* fir2_out = xip_array_real_create();
	xip_fir_v7_2_set_data_sink(fir2, fir2_out, 0);

	printf("Update configuration of %s\n", fir1_cnfg.name);
	// Send config data
	if (xip_fir_v7_2_config_send(fir1, &cnfg) == XIP_STATUS_OK) {
		printf("Sent config packet\n");
	}
	else {
		printf("Error sending config packet\n");
		return -1;
	}

	printf("Reload new coefficients for set %d of %s\n", rld.fsel, fir1_cnfg.name);
	// Send reload data
	if (xip_fir_v7_2_reload_send(fir1, &rld) == XIP_STATUS_OK) {
		printf("Sent reload packet\n");
	}
	else {
		printf("Error sending reload packet\n");
		return -1;
	}

	// Send input data and filter
	//  - Once fir1 has processed the input data and generated an output fir2 will be called via the registered data
	//    handler and will in turn generate an output
	if (xip_fir_v7_2_data_send(fir1, din) == XIP_STATUS_OK) {
		printf("Sent data: ");
		print_array_real(din);
	}
	else {
		printf("Error sending data\n");
		return -1;
	}

	// Filter output written directly to output data structures
	printf("%s output: ", fir1_cnfg.name);
	print_array_real(fir1_out);
	printf("%s output: ", fir2_cnfg.name);
	print_array_real(fir2_out);

	//De-allocate data
	xip_array_real_destroy(din);
	xip_array_real_destroy(fir1_out);
	xip_array_real_destroy(fir2_out);
	xip_array_uint_destroy(fsel);

	//De-allocate reload coeff array
	xip_array_real_destroy(rld.coeff);

	//De-allocate fir instances
	xip_fir_v7_2_destroy(fir1);
	xip_fir_v7_2_destroy(fir2);

	printf("-------------------------------------------------------------------------------\n");
	printf("Rate change exmaple....\n");
	printf("-------------------------------------------------------------------------------\n");

	xip_fir_v7_2_config fir_up_cnfg, fir_sr_cnfg, fir_down_cnfg;

	xip_fir_v7_2_default_config(&fir_up_cnfg);
	fir_up_cnfg.name = "fir_up";
	fir_up_cnfg.filter_type = XIP_FIR_INTERPOLATION;
	fir_up_cnfg.rate_change = XIP_FIR_INTEGER_RATE;
	fir_up_cnfg.interp_rate = 4;

	xip_fir_v7_2_default_config(&fir_sr_cnfg);
	fir_sr_cnfg.name = "fir_sr";
	const double fir_sr_coeffs[2] = { 0,2 };
	fir_sr_cnfg.coeff = &fir_sr_coeffs[0];
	fir_sr_cnfg.num_coeffs = 2;

	xip_fir_v7_2_default_config(&fir_down_cnfg);
	fir_down_cnfg.name = "fir_down";
	fir_down_cnfg.filter_type = XIP_FIR_DECIMATION;
	fir_down_cnfg.rate_change = XIP_FIR_INTEGER_RATE;
	fir_down_cnfg.decim_rate = 3;

	//Create filter instances
	print_config(&fir_up_cnfg);
	xip_fir_v7_2* fir_up = xip_fir_v7_2_create(&fir_up_cnfg, &msg_print, 0);
	if (!fir_up) {
		printf("Error creating instance\n");
		return -1;
	}
	else {
		printf("Created instance\n");
	}

	print_config(&fir_sr_cnfg);
	xip_fir_v7_2* fir_sr = xip_fir_v7_2_create(&fir_sr_cnfg, &msg_print, 0);
	if (!fir_sr) {
		printf("Error creating instance\n");
		return -1;
	}
	else {
		printf("Created instance\n");
	}

	print_config(&fir_down_cnfg);
	xip_fir_v7_2* fir_down = xip_fir_v7_2_create(&fir_down_cnfg, &msg_print, 0);
	if (!fir_down) {
		printf("Error creating instance\n");
		return -1;
	}
	else {
		printf("Created instance\n");
	}

	// Create input data packet
	din = xip_array_real_create();
	xip_array_real_reserve_dim(din, 3);
	din->dim_size = 3; // 3D array
	din->dim[0] = fir_up_cnfg.num_paths; // Number of paths
	din->dim[1] = fir_up_cnfg.num_channels; // Number of channels
	din->dim[2] = fir_up_cnfg.num_coeffs / fir_up_cnfg.interp_rate + 1; // vectors in a single packet
	din->data_size = din->dim[0] * din->dim[1] * din->dim[2];
	if (xip_array_real_reserve_data(din, din->data_size) == XIP_STATUS_OK) {
		printf("Reserved data\n");
	}
	else {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// Create output arrays and register as data sinks
	xip_array_real* fir_up_out = xip_array_real_create();
	xip_fir_v7_2_set_data_sink(fir_up, fir_up_out, 0);
	xip_array_real* fir_sr_out = xip_array_real_create();
	xip_fir_v7_2_set_data_sink(fir_sr, fir_sr_out, 0);
	xip_array_real* fir_down_out = xip_array_real_create();
	xip_fir_v7_2_set_data_sink(fir_down, fir_down_out, 0);

	// Connect filters in a single chain
	xip_fir_v7_2_set_data_handler(fir_up, &xip_fir_v7_2_data_send_handler, fir_sr, 0);
	xip_fir_v7_2_set_data_handler(fir_sr, &xip_fir_v7_2_data_send_handler, fir_down, 0);

	// Define input data
	printf("Create impulse\n");
	create_impulse(din);

	// Send input data and filter
	if (xip_fir_v7_2_data_send(fir_up, din) == XIP_STATUS_OK) {
		printf("Sent data     : ");
		print_array_real(din);
	}
	else {
		printf("Error sending data\n");
		return -1;
	}

	// Filter outputs written directly to output data structures
	printf("%s output: ", fir_up_cnfg.name);
	print_array_real(fir_up_out);
	printf("%s output: ", fir_sr_cnfg.name);
	print_array_real(fir_sr_out);
	printf("%s output: ", fir_down_cnfg.name);
	print_array_real(fir_down_out);

	//De-allocate data
	xip_array_real_destroy(din);
	xip_array_real_destroy(fir_up_out);
	xip_array_real_destroy(fir_sr_out);
	xip_array_real_destroy(fir_down_out);

	//De-allocate fir instances
	xip_fir_v7_2_destroy(fir_up);
	xip_fir_v7_2_destroy(fir_sr);
	xip_fir_v7_2_destroy(fir_down);

	printf("-------------------------------------------------------------------------------\n");
	printf("Hilbert filter....\n");
	printf("-------------------------------------------------------------------------------\n");

	xip_fir_v7_2_config fir_hilb_cnfg;
	xip_fir_v7_2_default_config(&fir_hilb_cnfg);

	fir_hilb_cnfg.name = "fir_hilb";
	fir_hilb_cnfg.filter_type = XIP_FIR_HILBERT;

	//Create filter instances
	print_config(&fir_hilb_cnfg);
	xip_fir_v7_2* fir_hilb = xip_fir_v7_2_create(&fir_hilb_cnfg, &msg_print, 0);
	if (!fir_hilb) {
		printf("Error creating instance\n");
		return -1;
	}
	else {
		printf("Created instance\n");
	}

	// Create input data packet
	din = xip_array_real_create();
	xip_array_real_reserve_dim(din, 3);
	din->dim_size = 3; // 3D array
	din->dim[0] = fir_hilb_cnfg.num_paths; // Number of paths
	din->dim[1] = fir_hilb_cnfg.num_channels; // Number of channels
	din->dim[2] = fir_hilb_cnfg.num_coeffs; // vectors in a single packet
	din->data_size = din->dim[0] * din->dim[1] * din->dim[2];
	if (xip_array_real_reserve_data(din, din->data_size) == XIP_STATUS_OK) {
		printf("Reserved data\n");
	}
	else {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// Create output data packet structure
	//  - Will be used as a data sink. The model will allocate appropriate memory
	xip_array_complex* fir_hilb_out = xip_array_complex_create();
	xip_fir_v7_2_set_data_sink(fir_hilb, 0, fir_hilb_out);

	// Define input data, scaled impulse on each channel
	printf("Create impulse\n");
	create_impulse(din);

	// Send input data and filter
	if (xip_fir_v7_2_data_send(fir_hilb, din) == XIP_STATUS_OK) {
		printf("Sent data: ");
		print_array_real(din);
	}
	else {
		printf("Error sending data\n");
		return -1;
	}

	// Output data is written directly to registered data sink array
	printf("%s output: ", fir_hilb_cnfg.name);
	print_array_complex(fir_hilb_out);

	//De-allocate data
	xip_array_real_destroy(din);
	xip_array_complex_destroy(fir_hilb_out);

	//De-allocate fir instances
	xip_fir_v7_2_destroy(fir_hilb);

	printf("-------------------------------------------------------------------------------\n");
	printf("Advanced channel configuration....\n");
	printf("-------------------------------------------------------------------------------\n");

	// Define filter configurations
	xip_fir_v7_2_config fir_adv_cnfg;
	xip_fir_v7_2_default_config(&fir_adv_cnfg);

	fir_adv_cnfg.name = "fir_adv";
	fir_adv_cnfg.chan_seq = XIP_FIR_ADVANCED_CHAN_SEQ;
	fir_adv_cnfg.num_channels = 4;
	fir_adv_cnfg.init_pattern = P4_3;

	//Create filter instances
	print_config(&fir_adv_cnfg);
	xip_fir_v7_2* fir_adv = xip_fir_v7_2_create(&fir_adv_cnfg, &msg_print, 0);
	if (!fir_adv) {
		printf("Error creating instance %s\n", fir_adv_cnfg.name);
		return -1;
	}
	else {
		printf("Created instance %s\n", fir_adv_cnfg.name);
	}

	// Create input data packet
	din = xip_array_real_create();
	xip_array_real_reserve_dim(din, 3);
	din->dim_size = 3; // 3D array
	din->dim[0] = fir_adv_cnfg.num_paths; // Number of paths
	din->dim[1] = fir_adv_cnfg.num_channels; // Number of channels
	din->dim[2] = fir_adv_cnfg.num_coeffs; // vectors in a single packet
	din->data_size = din->dim[0] * din->dim[1] * din->dim[2];
	if (xip_array_real_reserve_data(din, din->data_size) == XIP_STATUS_OK) {
		printf("Reserved data\n");
	}
	else {
		printf("Unable to reserve data!\n");
		return -1;
	}

	// Create output data packet
	//  - Automatically sized using xip_fir_v7_2_calc_size
	xip_array_real* fir_adv_out = xip_array_real_create();
	xip_array_real_reserve_dim(fir_adv_out, 3);
	fir_adv_out->dim_size = 3; // 3D array
	if (xip_fir_v7_2_calc_size(fir_adv, din, fir_adv_out, 0) == XIP_STATUS_OK) {
		printf("Calculated output data size\n");
		if (xip_array_real_reserve_data(fir_adv_out, fir_adv_out->data_size) == XIP_STATUS_OK) {
			printf("Reserved data\n");
		}
		else {
			printf("Unable to reserve data!\n");
			return -1;
		}
	}
	else {
		printf("Unable to calculate output date size\n");
		return -1;
	}

	// Populate input data with an impulse
	printf("Create impulse\n");
	// First set all the locations to zero
	int path;
	int chan;
	int i;
	for (path = 0; path < din->dim[0]; path++) {
		for (chan = 0; chan < din->dim[1]; chan++) {
			for (i = 0; i < din->dim[2]; i++) {
				xip_fir_v7_2_xip_array_real_set_chan(din, 0, path, chan, i, P_BASIC);
			}
		}
	}
	// Selected pattern P4_3 for fir_adv initialization pattern which contains 3 channels; 1 x fs/2 and 2 x fs/4
	// Set a scaled impulse for each channel
	xip_fir_v7_2_xip_array_real_set_chan(din, 1.0, 0, 0, 0, P4_3);
	xip_fir_v7_2_xip_array_real_set_chan(din, 2.0, 0, 1, 0, P4_3);
	xip_fir_v7_2_xip_array_real_set_chan(din, 3.0, 0, 2, 0, P4_3);

	// Send input data and filter
	if (xip_fir_v7_2_data_send(fir_adv, din) == XIP_STATUS_OK) {
		printf("Sent data     : ");
		print_array_real(din);
	}
	else {
		printf("Error sending data\n");
		return -1;
	}

	// Retrieve filtered data
	if (xip_fir_v7_2_data_get(fir_adv, fir_adv_out, 0) == XIP_STATUS_OK) {
		printf("Fetched result: ");
		print_array_real(fir_adv_out);
	}
	else {
		printf("Error getting data\n");
		return -1;
	}

	//De-allocate data
	xip_array_real_destroy(din);
	xip_array_real_destroy(fir_adv_out);

	//De-allocate fir instances
	xip_fir_v7_2_destroy(fir_adv);

	printf("...End\n");

	return 0;
}

int test_xip_dds_bitacc_cmodel()
{
	const int number_of_samples = 50;
	int channel;

	// Report the version of the model we are calling
	std::cout << "INFO: C model version = " << xip_dds_v6_0_get_version() << std::endl;

	// Create a configuration structure and set to core defaults.
	// Setting the defaults first means only those parameters for
	// which the desired configuration differs from the default need
	// be set up in the following lines.
	xip_dds_v6_0_config config, config_ret;
	xip_status status = xip_dds_v6_0_default_config(&config);

	// Did we get the default configuration correctly?
	if (status != XIP_STATUS_OK) {
		std::cerr << "ERROR: Could not get C model default configuration" << std::endl;
		return 1;
	}

	//Now modify the configuration as desired for your particular configuration
	config.name = "dds_compiler_v6_0";
	config.PartsPresent = XIP_DDS_PHASE_GEN_AND_SIN_COS_LUT;
	config.DDS_Clock_Rate = 100.0;
	config.Channels = 1;
	config.Mode_of_Operation = XIP_DDS_MOO_RASTERIZED;
	config.Modulus = 3000;
	config.ParameterEntry = XIP_DDS_HARDWARE_PARAMS;
	config.Spurious_Free_Dynamic_Range = 45.0;
	config.Frequency_Resolution = 0.4;
	config.Noise_Shaping = XIP_DDS_NS_NONE;
	config.Phase_Increment = XIP_DDS_PINCPOFF_FIXED;
	config.Resync = XIP_DDS_ABSENT;
	config.Phase_Offset = XIP_DDS_PINCPOFF_NONE;
	config.Output_Selection = XIP_DDS_OUT_SIN_AND_COS;
	config.Negative_Sine = XIP_DDS_ABSENT;
	config.Negative_Cosine = XIP_DDS_ABSENT;
	config.Amplitude_Mode = XIP_DDS_FULL_RANGE;
	config.Memory_Type = XIP_DDS_MEM_AUTO;
	config.Optimization_Goal = XIP_DDS_OPTGOAL_AUTO;
	config.DSP48_Use = XIP_DDS_DSP_MIN;
	config.Has_TREADY = XIP_DDS_ABSENT;
	config.S_CONFIG_Sync_Mode = XIP_DDS_CONFIG_SYNC_VECTOR;
	config.Output_Form = XIP_DDS_OUTPUT_TWOS;
	config.Latency_Configuration = XIP_DDS_LATENCY_AUTO;
	config.Has_ARESETn = XIP_DDS_ABSENT;
	config.PINC[0] = 1; // 1101000110110111
	// config.PINC[1]                  = 0;
	// ...
	config.POFF[0] = 0;
	// config.POFF[1]                  = 0;
	// ...
	config.Latency = 4;
	config.Output_Width = 8;
	config.Phase_Width = 12;

	// Set up some objects to hold state and the configuration data
	// for programmable phase increment and phase offset
	xip_dds_v6_0* pstate = nullptr;
	xip_dds_v6_0_config_pkt* pinc_poff_config = nullptr;

	// Create model object with your particular configuration
	pstate = xip_dds_v6_0_create(&config, &msg_print, 0);

	// Can we read back the updated configuration correctly?
	if (xip_dds_v6_0_get_config(pstate, &config_ret) != XIP_STATUS_OK) {
		std::cerr << "ERROR: Could not retrieve C model configuration" << std::endl;
	}

	// Set up some arrays to hold values for programmable phase increment and phase offset
	xip_dds_v6_0_data pinc[XIP_DDS_CHANNELS_MAX];
	xip_dds_v6_0_data poff[XIP_DDS_CHANNELS_MAX];

	//------------------------------------------------------------------------------------
	// Set up fields and reserve memory for data and config packets, for this configuration
	//------------------------------------------------------------------------------------

	//Calculate the number of input fields
	xip_uint no_of_input_fields = 0;
	if (config_ret.PartsPresent == XIP_DDS_SIN_COS_LUT_ONLY) {
		no_of_input_fields++; //Phase_In
	}
	else {
		if (config_ret.Phase_Increment == XIP_DDS_PINCPOFF_STREAM) {
			no_of_input_fields++; //PINC
			if (config_ret.Resync == XIP_DDS_PRESENT) {
				no_of_input_fields++; //RESYNC
			}
		}
		if (config_ret.Phase_Offset == XIP_DDS_PINCPOFF_STREAM) {
			no_of_input_fields++; //POFF
		}
	}

	//Calculate the number of output fields
	xip_uint no_of_output_fields = 0; //phase output is not optional in the c model.
	if (config_ret.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY) {
		no_of_output_fields = 1; //PHASE_OUT
	}
	if (config_ret.PartsPresent != XIP_DDS_PHASE_GEN_ONLY) {
		if (config_ret.Output_Selection == XIP_DDS_OUT_SIN_ONLY) no_of_output_fields++; //SIN
		if (config_ret.Output_Selection == XIP_DDS_OUT_COS_ONLY) no_of_output_fields++; //COS
		if (config_ret.Output_Selection == XIP_DDS_OUT_SIN_AND_COS) no_of_output_fields += 2; //SIN and COS
	}

	// Create and allocate memory for I/O structures
	// Create request and response structures

	// Create config packet - pass pointer by reference
	if (config_ret.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY && (config_ret.Phase_Increment == XIP_DDS_PINCPOFF_PROG || config_ret.Phase_Offset == XIP_DDS_PINCPOFF_PROG)) {
		if (xip_dds_v6_0_alloc_config_pkt(&pinc_poff_config, config_ret.Channels, config_ret.Channels) == XIP_STATUS_OK) {
			std::cout << "INFO: Reserved memory for config packet" << std::endl;
		}
		else {
			std::cerr << "ERROR: Unable to reserve memory for config packet" << std::endl;
			exit(1);
		}
	}

	// Create input data packet
	xip_array_real* din = xip_array_real_create();
	xip_array_real_reserve_dim(din, 3); //dimensions are (Number of samples, channels, PINC/POFF/Phase)
	din->dim_size = 3;
	din->dim[0] = number_of_samples;
	din->dim[1] = config_ret.Channels;
	din->dim[2] = no_of_input_fields;
	din->data_size = din->dim[0] * din->dim[1] * din->dim[2];
	if (xip_array_real_reserve_data(din, din->data_size) == XIP_STATUS_OK) {
		std::cout << "INFO: Reserved memory for request as [" << number_of_samples << "," << config_ret.Channels << "," << no_of_input_fields << "] array " << std::endl;
	}
	else {
		std::cout << "ERROR: Unable to reserve memory for input data packet!" << std::endl;
		exit(2);
	}

	// Request memory for output data
	xip_array_real* dout = xip_array_real_create();
	xip_array_real_reserve_dim(dout, 3); //dimensions are (Number of samples, channels, PINC/POFF/Phase)
	dout->dim_size = 3;
	dout->dim[0] = number_of_samples;
	dout->dim[1] = config_ret.Channels;
	dout->dim[2] = no_of_output_fields;
	dout->data_size = dout->dim[0] * dout->dim[1] * dout->dim[2];
	if (xip_array_real_reserve_data(dout, dout->data_size) == XIP_STATUS_OK) {
		std::cout << "INFO: Reserved memory for response as [" << number_of_samples << "," << config_ret.Channels << "," << no_of_output_fields << "] array " << std::endl;
	}
	else {
		std::cout << "ERROR: Unable to reserve memory for output data packet!" << std::endl;
		exit(3);
	}


	//---------------------------------------------------
	// Populate the input structure with example data
	//---------------------------------------------------
	size_t sample = 0;
	size_t field = 0;
	xip_dds_v6_0_data value = 0.0;

	// Set up pinc and poff, and call config routine, if either phase increment or phase offset is programmable
	if (config_ret.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY) {
		if (config_ret.Phase_Increment == XIP_DDS_PINCPOFF_PROG || config_ret.Phase_Offset == XIP_DDS_PINCPOFF_PROG) {

			for (channel = 0; channel < config_ret.Channels; channel++) {
				if (config_ret.Phase_Increment == XIP_DDS_PINCPOFF_PROG) {
					//field is PINC
					if (config_ret.Mode_of_Operation == XIP_DDS_MOO_RASTERIZED) {
						pinc[channel] = rand() % (config_ret.Modulus); // Allow zero too
					}
					else {
						pinc[channel] = rand() % (1ULL << (config_ret.resPhase_Width)); // Allow zero too
					}
				}
				if (config_ret.Phase_Offset == XIP_DDS_PINCPOFF_PROG) {
					//field is POFF
					if (config_ret.Mode_of_Operation == XIP_DDS_MOO_RASTERIZED) {
						poff[channel] = (channel + 1) % (config_ret.Modulus);
					}
					else {
						poff[channel] = (channel + 1) % (1ULL << (config_ret.resPhase_Width)); // Allow zero too
					}
				}
			}

			// Copy our local pinc/poff data into the memory we allocated in the config structure
			// If not present, leave the initial values
			if (config_ret.Phase_Increment == XIP_DDS_PINCPOFF_PROG) {
				memcpy(pinc_poff_config->din_pinc, pinc, config_ret.Channels * sizeof(xip_dds_v6_0_data));
			}
			if (config_ret.Phase_Offset == XIP_DDS_PINCPOFF_PROG) {
				memcpy(pinc_poff_config->din_poff, poff, config_ret.Channels * sizeof(xip_dds_v6_0_data));
			}

			// Run the config routine
			if (xip_dds_v6_0_config_do(pstate, pinc_poff_config) == XIP_STATUS_OK) {
				std::cout << "INFO: config_do was successful" << std::endl;
			}
			else {
				std::cerr << "ERROR: config_packet failed" << std::endl;
				exit(4);
			}

		}
	}

	int resync_sample = rand() % (number_of_samples - 2) + 1; // Do a resync randomly in the frame between 2nd and 2nd-last sample
	for (sample = 0; sample < number_of_samples; sample++) {
		for (channel = 0; channel < config_ret.Channels; channel++) {

			field = 0;

			// Phase_In input, for Sin/Cos LUT configuration only
			if (config_ret.PartsPresent == XIP_DDS_SIN_COS_LUT_ONLY) {
				//field is PHASE_IN
				if (config_ret.Mode_of_Operation == XIP_DDS_MOO_RASTERIZED) {
					value = rand() % (config_ret.Modulus); // Allow zero too
				}
				else {
					value = rand() % (1ULL << (config_ret.resPhase_Width)); // Allow zero too
				}
				xip_dds_v6_0_xip_array_real_set_data(din, value, sample, channel, field);
				field++;
			}

			// Streaming phase increment
			if (config_ret.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY) {
				if (config_ret.Phase_Increment == XIP_DDS_PINCPOFF_STREAM) {
					//field is PINC
					if (config_ret.Mode_of_Operation == XIP_DDS_MOO_RASTERIZED) {
						value = rand() % (config_ret.Modulus); // Allow zero too
					}
					else {
						value = rand() % (1ULL << (config_ret.resPhase_Width)); // Allow zero too
					}
					xip_dds_v6_0_xip_array_real_set_data(din, value, sample, channel, field);
					field++;
				}
			}

			// Streaming phase offset
			if (config_ret.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY) {
				if (config_ret.Phase_Offset == XIP_DDS_PINCPOFF_STREAM) {
					//field is POFF
					if (config_ret.Mode_of_Operation == XIP_DDS_MOO_RASTERIZED) {
						value = (channel + 1 + sample) % (config_ret.Modulus);
					}
					else {
						value = (channel + 1 + sample) % (1ULL << (config_ret.resPhase_Width));
					}
					xip_dds_v6_0_xip_array_real_set_data(din, value, sample, channel, field);
					field++;
				}
			}

			// Finally do resync, if required
			if (config_ret.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY) {
				if ((config_ret.Phase_Increment == XIP_DDS_PINCPOFF_STREAM) && (config_ret.Resync == XIP_DDS_PRESENT)) {
					//field is Resync
					if (sample == resync_sample) {
						value = 1;
					}
					else {
						value = 0;
					}
					xip_dds_v6_0_xip_array_real_set_data(din, value, sample, channel, field);
					field++;
				}
			}

		}
	}

	//------------------
	// Simulate the core
	//------------------
	std::cout << "INFO: Running the C model..." << std::endl;

	if (xip_dds_v6_0_data_do(pstate,   //pointer to c model instance
		din, //pointer to input data structure
		dout, //pointer to output structure
		number_of_samples, //first dimension of either data structure
		config_ret.Channels, //2nd dimension of either data structure
		no_of_input_fields, //3rd dimension of input
		no_of_output_fields //3rd dimension of output
	) != XIP_STATUS_OK) {
		std::cerr << "ERROR: C model did not complete successfully" << std::endl;
		xip_array_real_destroy(din);
		xip_array_real_destroy(dout);
		xip_dds_v6_0_destroy(pstate);
		if (config_ret.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY && (config_ret.Phase_Increment == XIP_DDS_PINCPOFF_PROG || config_ret.Phase_Offset == XIP_DDS_PINCPOFF_PROG)) {
			xip_dds_v6_0_free_config_pkt(&pinc_poff_config);
		}
		exit(5);
	}
	else {
		std::cout << "INFO: C model transaction completed successfully" << std::endl;
	}

#if(DEBUG)
	// When enabled, this will print the result data to stdout
	for (int sample = 0; sample < number_of_samples; sample++) {
		std::cout << std::endl << "Sample " << sample;
		for (int chan = 0; chan < config_ret.Channels; chan++) {
			std::cout << std::endl << "Channel " << sample;
			field = 0;
			xip_dds_v6_0_xip_array_real_get_data(dout, &value, sample, chan, field);
			std::cout << ":  out phase = " << value;
			field++;
			if (config_ret.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY) {
				if (config_ret.Output_Selection != XIP_DDS_OUT_COS_ONLY) {
					xip_dds_v6_0_xip_array_real_get_data(dout, &value, sample, chan, field);
					std::cout << " out sin = " << value;
					field++;
				}
				if (config_ret.Output_Selection != XIP_DDS_OUT_SIN_ONLY) {
					xip_dds_v6_0_xip_array_real_get_data(dout, &value, sample, chan, field);
					std::cout << " out cos = " << value;
				}
			}
			std::cout << std::endl;
		}
	}
#endif

	//-----------------
	// Reset the core
	// This will clear the phase accumulator state, and any resync input,
	// but leave any programmed phase increment/phase offset values
	// unchanged.
	//-----------------
	if (xip_dds_v6_0_reset(pstate) == XIP_STATUS_OK) {
		std::cout << "INFO: C model reset successfully" << std::endl;
	}
	else {
		std::cout << "ERROR: C model reset did not complete successfully" << std::endl;
		exit(6);
	}

	//------------------------
	// Simulate the core again
	//------------------------
	std::cout << "INFO: Running the C model again after reset..." << std::endl;

	if (xip_dds_v6_0_data_do(pstate,   //pointer to c model instance
		din, //pointer to input data structure
		dout, //pointer to output structure
		number_of_samples, //first dimension of either data structure
		config_ret.Channels, //2nd dimension of either data structure
		no_of_input_fields, //3rd dimension of input
		no_of_output_fields //3rd dimension of output
	) != XIP_STATUS_OK) {
		std::cerr << "ERROR: C model did not complete successfully" << std::endl;
		xip_array_real_destroy(din);
		xip_array_real_destroy(dout);
		xip_dds_v6_0_destroy(pstate);
		if (config_ret.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY && (config_ret.Phase_Increment == XIP_DDS_PINCPOFF_PROG || config_ret.Phase_Offset == XIP_DDS_PINCPOFF_PROG)) {
			xip_dds_v6_0_free_config_pkt(&pinc_poff_config);
		}
		exit(7);
	}
	else {
		std::cout << "INFO: C model transaction completed successfully" << std::endl;
	}

#if(DEBUG)
	// When enabled, this will print the result data to stdout
	const int SCALE_FACTOR = sizeof(int) * CHAR_BIT - config_ret.Output_Width;
	for (int sample = 0; sample < number_of_samples; sample++) {
		std::cout << std::endl << "Sample " << sample;
		for (int chan = 0; chan < config_ret.Channels; chan++) {
			std::cout << std::endl << "Channel " << sample;
			field = 0;
			xip_dds_v6_0_xip_array_real_get_data(dout, &value, sample, chan, field);
			std::cout << ":  out phase = " << value;
			field++;
			if (config_ret.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY) {
				if (config_ret.Output_Selection != XIP_DDS_OUT_COS_ONLY) {
					xip_dds_v6_0_xip_array_real_get_data(dout, &value, sample, chan, field);
					std::cout << " out sin = " << (((int)value << SCALE_FACTOR) >> SCALE_FACTOR);
					field++;
				}
				if (config_ret.Output_Selection != XIP_DDS_OUT_SIN_ONLY) {
					xip_dds_v6_0_xip_array_real_get_data(dout, &value, sample, chan, field);
					std::cout << " out cos = " << (((int)value << SCALE_FACTOR) >> SCALE_FACTOR);
				}
			}
			std::cout << std::endl;
		}
	}
#endif

	//------------------------------
	// Call destructors, free memory
	//------------------------------
	xip_array_real_destroy(din);
	xip_array_real_destroy(dout);
	std::cout << "INFO: C model input and output data freed" << std::endl;
	if (config_ret.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY && (config_ret.Phase_Increment == XIP_DDS_PINCPOFF_PROG || config_ret.Phase_Offset == XIP_DDS_PINCPOFF_PROG)) {
		xip_dds_v6_0_free_config_pkt(&pinc_poff_config);
		std::cout << "INFO: C model config packet data freed" << std::endl;
	}
	xip_dds_v6_0_destroy(pstate);
	std::cout << "INFO: C model destroyed" << std::endl;

	return 0;
}
