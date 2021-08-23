#include "xip_utils.h"

xip_cmpy_v6_0_config xip_multiplier_cnfg;
xip_cmpy_v6_0* xip_multiplier;				// умножитель
xip_array_complex* xip_multiplier_reqa;		// первый аргумент
xip_array_complex* xip_multiplier_reqb;		// второй аргумент
xip_array_uint* xip_multiplier_reqctrl;		// бит округления
xip_array_complex* xip_multiplier_response;	// результат

// инициализация комплексного умножителя
// умножает 2 единичных комплексных значения
int init_xip_multiplier() 
{
	if (xip_cmpy_v6_0_default_config(&xip_multiplier_cnfg) != XIP_STATUS_OK) {
		printf("ERROR: Could not get xip_multiplier default configuration");
		return -1;
	}
	xip_multiplier_cnfg.APortWidth = 16;
	xip_multiplier_cnfg.BPortWidth = 16;
	xip_multiplier_cnfg.OutputWidth = 33;
	xip_multiplier_cnfg.RoundMode = XIP_CMPY_V6_0_ROUND;

	xip_multiplier = xip_cmpy_v6_0_create(&xip_multiplier_cnfg, &msg_print, 0);
	if (!xip_multiplier) {
		printf("Error creating xip_multiplier instance\n");
		return -1;
	}

	// Create input data packet for operand A
	xip_multiplier_reqa = xip_array_complex_create();
	xip_array_complex_reserve_dim(xip_multiplier_reqa, 1);
	xip_multiplier_reqa->dim_size = 1;
	xip_multiplier_reqa->dim[0] = 1;
	xip_multiplier_reqa->data_size = xip_multiplier_reqa->dim[0];
	if (xip_array_complex_reserve_data(xip_multiplier_reqa, xip_multiplier_reqa->data_size) != XIP_STATUS_OK) {
		printf("ERROR: Unable to reserve memory for input data packet!");
		return -1;
	}

	// Create input data packet for operand B
	xip_multiplier_reqb = xip_array_complex_create();
	xip_array_complex_reserve_dim(xip_multiplier_reqb, 1);
	xip_multiplier_reqb->dim_size = 1;
	xip_multiplier_reqb->dim[0] = 1;
	xip_multiplier_reqb->data_size = xip_multiplier_reqb->dim[0];
	if (xip_array_complex_reserve_data(xip_multiplier_reqb, xip_multiplier_reqb->data_size) != XIP_STATUS_OK) {
		printf("ERROR: Unable to reserve memory for input data packet!");
		return -1;
	}

	// Create input data packet for ctrl input (Round bit)
	xip_multiplier_reqctrl = xip_array_uint_create();
	xip_array_uint_reserve_dim(xip_multiplier_reqctrl, 1);
	xip_multiplier_reqctrl->dim_size = 1;
	xip_multiplier_reqctrl->dim[0] = 1;
	xip_multiplier_reqctrl->data_size = xip_multiplier_reqctrl->dim[0];
	if (xip_array_uint_reserve_data(xip_multiplier_reqctrl, xip_multiplier_reqctrl->data_size) != XIP_STATUS_OK) {
		printf("ERROR: Unable to reserve memory for ctrl data packet!");
		return -1;
	}

	// Request memory for output data
	xip_multiplier_response = xip_array_complex_create();
	xip_array_complex_reserve_dim(xip_multiplier_response, 1); 
	xip_multiplier_response->dim_size = 1;
	xip_multiplier_response->dim[0] = 1;
	xip_multiplier_response->data_size = xip_multiplier_response->dim[0];
	if (xip_array_complex_reserve_data(xip_multiplier_response, xip_multiplier_response->data_size) != XIP_STATUS_OK) {
		printf("ERROR: Unable to reserve memory for output data packet!");
		return -1;
	}

	return 0;
}

int destroy_xip_multiplier()
{
	if (xip_array_complex_destroy(xip_multiplier_reqa) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_complex_destroy(xip_multiplier_reqb) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_uint_destroy(xip_multiplier_reqctrl) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_complex_destroy(xip_multiplier_response) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_cmpy_v6_0_destroy(xip_multiplier) != XIP_STATUS_OK) {
		return -1;
	}

	printf("Deleted instance of xip multiplier and free memory\n");
	return 0;
}

int xip_multiply_complex(const xip_complex& a, const xip_complex& b, xip_complex& out)
{
	if (xip_cmpy_v6_0_xip_array_complex_set_data(xip_multiplier_reqa, a, 0) != XIP_STATUS_OK) {
		printf("Error in xip_array_complex_set_data");
		return -1;
	}
	if (xip_cmpy_v6_0_xip_array_complex_set_data(xip_multiplier_reqb, b, 0) != XIP_STATUS_OK) {
		printf("Error in xip_array_complex_set_data");
		return -1;
	}
	// бит округления ставим 0 для режима XIP_CMPY_V6_0_ROUND
	if (xip_cmpy_v6_0_xip_array_uint_set_data(xip_multiplier_reqctrl, 0, 0) != XIP_STATUS_OK) {
		printf("Error in xip_array_uint_set_data");
		return -1;
	}
	if (xip_cmpy_v6_0_data_do(xip_multiplier, 
							xip_multiplier_reqa, 
							xip_multiplier_reqb, 
							xip_multiplier_reqctrl,
							xip_multiplier_response) != XIP_STATUS_OK) {
		printf("ERROR: C model did not complete successfully");
		return -1;
	}
	if (xip_cmpy_v6_0_xip_array_complex_get_data(xip_multiplier_response, &out, 0) != XIP_STATUS_OK) {
		printf("Error in xip_array_complex_get_data");
		return -1;
	}

	return 0;
}

// умножение вещественных чисел реализовано как частный случай умножения комплексных
int xip_multiply_real(const xip_real& a, const xip_real& b, xip_real& out)
{
	xip_complex a_cplx{ a, 0 };
	xip_complex b_cplx{ b, 0 };
	xip_complex out_cplx;

	if (xip_multiply_complex(a_cplx, b_cplx, out_cplx) != 0)
		return -1;

	out = out_cplx.re;

	return 0;
}

ostream& operator<<(ostream& out, const xip_complex& data) 
{
	out << data.re;
	if (data.im >= 0)
		out << '+';
	out << data.im << 'i';
	return out;
}
