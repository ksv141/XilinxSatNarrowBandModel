#ifndef LAGRANGEINTERP_H
#define LAGRANGEINTERP_H

#include <bitset>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"

using namespace std;

// Интерполятор Лагранжа 7-й степени. Обеспечивает сдвиг тактов и дробную передискретизацию
// интервал между двумя смежными отсчетами делится на 1024 интервалов, 
// для каждого из которых используется свой заранее рассчитанный набор из 8 коэффициентов
class LagrangeInterp {
public:
	LagrangeInterp();

	~LagrangeInterp();


private:
	int init_lagrange_interp();
	int lagrange_load_coeff();
	int process_sample_lagrange_interp(xip_complex* in, xip_complex* out, uint32_t pos);
	int destroy_lagrange_interp();

	const uint32_t lagrange_n_intervals = 1024;		// количество интервалов
	const uint32_t lagrange_n_coeff = 8;			// количество коэффициентов
	double* lagrange_coeff;					// наборы коэффициентов фильтра, следуют по порядку

	xip_fir_v7_2* lagrange_interp_fir;		// фильтр на передаче
	xip_fir_v7_2_config lagrange_interp_fir_cnfg;

	xip_array_uint* lagrange_interp_fir_fsel;
	xip_fir_v7_2_cnfg_packet lagrange_interp_fir_cnfg_packet;

	xip_array_real* lagrange_interp_in;		// 3-D массив, содержащий текущий отсчет для обработки
	xip_array_real* lagrange_interp_out;	// 3-D массив, содержащий результат обработки
};

#endif // LAGRANGEINTERP_H