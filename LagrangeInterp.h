#ifndef LAGRANGEINTERP_H
#define LAGRANGEINTERP_H

#include <bitset>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <vector>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"
#include "xip_utils.h"

using namespace std;

extern const unsigned int LAGRANGE_INTERVALS;
extern const unsigned int LAGRANGE_ORDER;

// »нтерпол€тор Ћагранжа 7-й степени. ќбеспечивает сдвиг тактов и дробную передискретизацию
// интервал между двум€ смежными отсчетами делитс€ на 1024 интервалов, 
// дл€ каждого из которых используетс€ свой заранее рассчитанный набор из 8 коэффициентов
class LagrangeInterp {
public:
	// frac - отношение частоты дискретизации входного сигнала к выходному --> [0.001, 1000]
	// если передискретизаци€ не требуетс€, то frac = 1 
	LagrangeInterp(xip_real frac = 1);

	LagrangeInterp(xip_real from_sampling_freq, xip_real to_sampling_freq);

	~LagrangeInterp();

	/**
	 * @brief обработка очередного отсчета. в буфер отсчетов добавл€етс€ очередной отсчет (FIFO)
	 * @param in 
	*/
	void process(const xip_complex& in);

	/**
	 * @brief получить следующий отсчет. true - есть отсчет, false - нет отсчета (требуетс€ подать на вход следующий)
	 * @param out 
	 * @return 
	*/
	bool next(xip_complex& out);
	
	/**
	 * @brief установить смещение тактов относительно выходных отсчетов
	 * @param value -> [-1.0, 1.0] относительно выходных отсчетов, производитс€ приведение value в пределы [-1.0, 1.0]
	*/
	void shift(double value);

private:
	/**
	 * @brief инициализаци€ библиотеки xip fir и загрузка наборов коэффициентов
	 * @return 
	*/
	int init_lagrange_interp();

	/**
	 * @brief загрузка наборов коэффициентов фильтра в пам€ть
	 * @return 
	*/
	int lagrange_load_coeff();

	/**
	 * @brief интерпол€ци€ очередного отсчета
	 * @param values массив интерполируемых значений
	 * @param out результат интерпол€ции
	 * @param pos позици€ интерпол€ции --> [0, LAGRANGE_INTERVALS-1]
	 * @return 
	*/
	int interpolate(xip_complex* values, xip_complex& out, uint32_t pos);

	/**
	 * @brief инициализаци€ библиотеки xip fir и освобождение пам€ти
	 * @return 
	*/
	int destroy_lagrange_interp();

	size_t samples_count(double inv_factor);
	int to_dx_value(double inv_factor);
	void init(double dx_value);

	const uint32_t lagrange_n_intervals = LAGRANGE_INTERVALS;		// количество интервалов
	const uint32_t lagrange_n_coeff = LAGRANGE_ORDER;				// количество коэффициентов (пор€док фильтра)
	double* lagrange_coeff;											// наборы коэффициентов фильтра, следуют по пор€дку
	const uint32_t FixPointPosition = 20;

	vector<xip_complex> samples;	// FIFO-буфер отсчетов

	// переменные состо€ни€ интерпол€тора
	int32_t dx;
	int32_t fx;		// дробна€ часть
	uint32_t block_size;
	uint32_t block_offset;
	xip_complex* x_ptr;
	xip_complex* pos_ptr;
	xip_complex* end_ptr;

	// паременные дл€ работы с xip fir
	xip_fir_v7_2* lagrange_interp_fir;		// фильтр на передаче
	xip_fir_v7_2_config lagrange_interp_fir_cnfg;

	xip_array_uint* lagrange_interp_fir_fsel;
	xip_fir_v7_2_cnfg_packet lagrange_interp_fir_cnfg_packet;

	xip_array_real* lagrange_interp_in;		// 3-D массив, содержащий текущий отсчет дл€ обработки
	xip_array_real* lagrange_interp_out;	// 3-D массив, содержащий результат обработки
};

#endif // LAGRANGEINTERP_H