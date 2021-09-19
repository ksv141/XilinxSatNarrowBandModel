#include "xip_utils.h"

xip_cordic_v6_0_config xip_sqrt_cnfg;		// конфиг корня
xip_cordic_v6_0* xip_sqrt;					// вычислитель корня
xip_array_real* xip_sqrt_arg;				// аргумент корня
xip_array_real* xip_sqrt_result;			// результат корня

xip_cordic_v6_0_config xip_rect_to_polar_cnfg;		// конфиг комплексного конвертора
xip_cordic_v6_0* xip_rect_to_polar;					// комплексный конвертор
xip_array_complex* xip_rect_to_polar_in;			// входное комплексное число
xip_array_real* xip_rect_to_polar_mag;				// модуль
xip_array_real* xip_rect_to_polar_arg;				// аргумент

xip_cmpy_v6_0_config xip_multiplier_cnfg;	// конфиг умножителя
xip_cmpy_v6_0* xip_multiplier;				// умножитель
xip_array_complex* xip_multiplier_reqa;		// первый аргумент умножителя
xip_array_complex* xip_multiplier_reqb;		// второй аргумент умножителя
xip_array_uint* xip_multiplier_reqctrl;		// бит округления умножителя
xip_array_complex* xip_multiplier_response;	// результат умножителя

int a_max;
int b_max;

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
	a_max = (1 << (xip_multiplier_cnfg.APortWidth - 1)) - 1;
	b_max = (1 << (xip_multiplier_cnfg.BPortWidth - 1)) - 1;

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

int init_xip_cordic_sqrt()
{
	if (xip_cordic_v6_0_default_config(&xip_sqrt_cnfg) != XIP_STATUS_OK) {
		printf("ERROR: Could not get C model default configuration\n");
		return -1;
	}

	//Firstly, create and exercise a simple configuration.
	xip_sqrt_cnfg.CordicFunction = XIP_CORDIC_V6_0_F_SQRT;
	xip_sqrt_cnfg.CoarseRotate = 0;
	xip_sqrt_cnfg.DataFormat = XIP_CORDIC_V6_0_FORMAT_USIG_INT;
	xip_sqrt_cnfg.InputWidth = 32;
	xip_sqrt_cnfg.OutputWidth = 17;
	xip_sqrt_cnfg.Precision = 0;
	xip_sqrt_cnfg.RoundMode = XIP_CORDIC_V6_0_ROUND_TRUNCATE;
	xip_sqrt_cnfg.ScaleComp = XIP_CORDIC_V6_0_SCALE_NONE;

	xip_sqrt = xip_cordic_v6_0_create(&xip_sqrt_cnfg, &msg_print, 0);

	if (!xip_sqrt) {
		printf("ERROR: Could not create C model state object\n");
		return -1;
	}

	// Create input data packet for operand
	xip_sqrt_arg = xip_array_real_create();
	xip_array_real_reserve_dim(xip_sqrt_arg, 1);
	xip_sqrt_arg->dim_size = 1;
	xip_sqrt_arg->dim[0] = 1;
	xip_sqrt_arg->data_size = xip_sqrt_arg->dim[0];
	if (xip_array_real_reserve_data(xip_sqrt_arg, xip_sqrt_arg->data_size) != XIP_STATUS_OK) {
		printf("ERROR: Unable to reserve memory for input data packet!\n");
		return -1;
	}

	// Create output data packet for output data
	xip_sqrt_result = xip_array_real_create();
	xip_array_real_reserve_dim(xip_sqrt_result, 1);
	xip_sqrt_result->dim_size = 1;
	xip_sqrt_result->dim[0] = 1;
	xip_sqrt_result->data_size = xip_sqrt_result->dim[0];
	if (xip_array_real_reserve_data(xip_sqrt_result, xip_sqrt_result->data_size) != XIP_STATUS_OK) {
		printf("ERROR: Unable to reserve memory for output data packet!\n");
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

int destroy_xip_cordic_sqrt()
{
	if (xip_array_real_destroy(xip_sqrt_arg) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(xip_sqrt_result) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_cordic_v6_0_destroy(xip_sqrt) != XIP_STATUS_OK) {
		return -1;
	}

	printf("Deleted instance of xip cordic sqrt and free memory\n");
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
	xip_real a_arg = a;
	xip_real b_arg = b;
	if (a_arg > a_max)
		a_arg = a_max;
	if (a_arg < -a_max)
		a_arg = -a_max;
	if (b_arg > b_max)
		b_arg = b_max;
	if (b_arg < -b_max)
		b_arg = -b_max;

	xip_complex a_cplx{ a_arg, 0 };
	xip_complex b_cplx{ b_arg, 0 };
	xip_complex out_cplx;

	if (xip_multiply_complex(a_cplx, b_cplx, out_cplx) != 0)
		return -1;

	out = out_cplx.re;

	return 0;
}

int xip_sqrt_real(const xip_real& arg, xip_real& out)
{
	if (xip_cordic_v6_0_xip_array_real_set_data(xip_sqrt_arg, arg, 0) != XIP_STATUS_OK) {
		printf("Error in xip_cordic_v6_0_xip_array_real_set_data\n");
		return -1;
	}

	// !!! при многократном выполнении функции xip_cordic_v6_0_sqrt возникает утечка памяти
	// за 1 вызов утекает ~128 байт
	if (xip_cordic_v6_0_sqrt(xip_sqrt, xip_sqrt_arg, xip_sqrt_result, xip_sqrt_arg->data_size) != XIP_STATUS_OK) {
		printf("ERROR: C model did not complete successfully");
		return -1;
	}

	if (xip_cordic_v6_0_xip_array_real_get_data(xip_sqrt_result, &out, 0) != XIP_STATUS_OK) {
		printf("Error in xip_cordic_v6_0_xip_array_real_get_data");
		return -1;
	}
	return 0;
}

int init_xip_cordic_rect_to_polar()
{
	if (xip_cordic_v6_0_default_config(&xip_rect_to_polar_cnfg) != XIP_STATUS_OK) {
		printf("ERROR: Could not get C model default configuration\n");
		return -1;
	}

	//Firstly, create and exercise a simple configuration.
	xip_rect_to_polar_cnfg.CordicFunction = XIP_CORDIC_V6_0_F_TRANSLATE;
	xip_rect_to_polar_cnfg.CoarseRotate = XIP_CORDIC_V6_0_TRUE;
	xip_rect_to_polar_cnfg.DataFormat = XIP_CORDIC_V6_0_FORMAT_SIG_FRAC;
	xip_rect_to_polar_cnfg.PhaseFormat = XIP_CORDIC_V6_0_FORMAT_SCA;
	xip_rect_to_polar_cnfg.InputWidth = 20;
	xip_rect_to_polar_cnfg.OutputWidth = 16;	// фаза будет в диапазоне [-8192, 8192]
	xip_rect_to_polar_cnfg.Precision = 0;
	xip_rect_to_polar_cnfg.RoundMode = XIP_CORDIC_V6_0_ROUND_TRUNCATE;
	xip_rect_to_polar_cnfg.ScaleComp = XIP_CORDIC_V6_0_SCALE_NONE;

	xip_rect_to_polar = xip_cordic_v6_0_create(&xip_rect_to_polar_cnfg, &msg_print, 0);

	if (!xip_rect_to_polar) {
		printf("ERROR: Could not create C model state object\n");
		return -1;
	}

	// Create input data packet for operand
	xip_rect_to_polar_in = xip_array_complex_create();
	xip_array_complex_reserve_dim(xip_rect_to_polar_in, 1);
	xip_rect_to_polar_in->dim_size = 1;
	xip_rect_to_polar_in->dim[0] = 1;
	xip_rect_to_polar_in->data_size = xip_rect_to_polar_in->dim[0];
	if (xip_array_complex_reserve_data(xip_rect_to_polar_in, xip_rect_to_polar_in->data_size) != XIP_STATUS_OK) {
		printf("ERROR: Unable to reserve memory for input data packet!");
		return -1;
	}

	// Create output data packet for output data
	xip_rect_to_polar_mag = xip_array_real_create();
	xip_array_real_reserve_dim(xip_rect_to_polar_mag, 1);
	xip_rect_to_polar_mag->dim_size = 1;
	xip_rect_to_polar_mag->dim[0] = 1;
	xip_rect_to_polar_mag->data_size = xip_rect_to_polar_mag->dim[0];
	if (xip_array_real_reserve_data(xip_rect_to_polar_mag, xip_rect_to_polar_mag->data_size) != XIP_STATUS_OK) {
		printf("ERROR: Unable to reserve memory for output data packet!\n");
		return -1;
	}

	xip_rect_to_polar_arg = xip_array_real_create();
	xip_array_real_reserve_dim(xip_rect_to_polar_arg, 1);
	xip_rect_to_polar_arg->dim_size = 1;
	xip_rect_to_polar_arg->dim[0] = 1;
	xip_rect_to_polar_arg->data_size = xip_rect_to_polar_arg->dim[0];
	if (xip_array_real_reserve_data(xip_rect_to_polar_arg, xip_rect_to_polar_arg->data_size) != XIP_STATUS_OK) {
		printf("ERROR: Unable to reserve memory for output data packet!\n");
		return -1;
	}

	return 0;
}

int destroy_xip_cordic_rect_to_polar()
{
	if (xip_array_complex_destroy(xip_rect_to_polar_in) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(xip_rect_to_polar_mag) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(xip_rect_to_polar_arg) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_cordic_v6_0_destroy(xip_rect_to_polar) != XIP_STATUS_OK) {
		return -1;
	}

	printf("Deleted instance of xip cordic rect to polar and free memory\n");
	return 0;
}

int xip_cordic_rect_to_polar(const xip_complex& in, xip_real& mag, xip_real& arg)
{
	if (xip_cmpy_v6_0_xip_array_complex_set_data(xip_rect_to_polar_in, in, 0) != XIP_STATUS_OK) {
		printf("Error in xip_cordic_v6_0_xip_array_real_set_data\n");
		return -1;
	}

	if (xip_cordic_v6_0_translate(xip_rect_to_polar, 
								xip_rect_to_polar_in, 
								xip_rect_to_polar_mag, 
								xip_rect_to_polar_arg, 
								xip_rect_to_polar_in->data_size) != XIP_STATUS_OK) {
		printf("ERROR: C model did not complete successfully");
		return -1;
	}

	if (xip_cordic_v6_0_xip_array_real_get_data(xip_rect_to_polar_mag, &mag, 0) != XIP_STATUS_OK) {
		printf("Error in xip_cordic_v6_0_xip_array_real_get_data");
		return -1;
	}
	if (xip_cordic_v6_0_xip_array_real_get_data(xip_rect_to_polar_arg, &arg, 0) != XIP_STATUS_OK) {
		printf("Error in xip_cordic_v6_0_xip_array_real_get_data");
		return -1;
	}

	return 0;
}

int int32_division(double a, double b, double& res)
{
	if ((b < 0) || (a < INT32_MIN) || (a > INT32_MAX)) {
		printf("int32_division error: invalid argument\n");
		return -1;
	}
	if (fabs(a) < b) {
		res = 0;
		return 0;
	}

	// вещественные аргументы переводятся в int32 максимального диапазона
	while (a >= INT32_MIN && a <= INT32_MAX) {
		xip_real_shift(a, 1);
		xip_real_shift(b, 1);
	}
	xip_real_shift(a, -1);
	xip_real_shift(b, -1);

	int32_t ia = (int32_t)a;
	uint32_t ub = (uint32_t)b;
	if (ub == 0) {
		printf("int32_division error: invalid argument\n");
		return -1;
	}
	// частные случаи
	if (ia == ub) {
		res = 1;
		return 0;
	}
	if (ub == 1) {
		res = ia;
		return 0;
	}

	// деление с остатком столбиком
	uint32_t reminder = a < 0 ? -a : a;
	uint32_t ures = 0;
	while (!(reminder < ub)) {
		uint32_t tmp_dev = ub;
		uint32_t tmp_res = 1;
		while (tmp_dev <= reminder) {
			tmp_dev <<= 1;
			tmp_res <<= 1;
		}
		tmp_dev >>= 1;
		tmp_res >>= 1;
		ures += tmp_res;
		reminder -= tmp_dev;
	}

	res = (a < 0 ? -(double)ures : ures);
	return 0;
}

void xip_real_shift(xip_real& in_out, int pos)
{
	uint64_t coeff = 1 << abs(pos);
	if (pos > 0)
		in_out *= coeff;
	else if (pos < 0)
		in_out /= coeff;
}

void xip_complex_shift(xip_complex& in_out, int pos)
{
	xip_real_shift(in_out.re, pos);
	xip_real_shift(in_out.im, pos);
}

ostream& operator<<(ostream& out, const xip_complex& data)
{
	out << data.re;
	if (data.im >= 0)
		out << '+';
	out << data.im << 'i';
	return out;
}
