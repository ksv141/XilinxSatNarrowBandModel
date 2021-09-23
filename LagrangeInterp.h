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
extern const unsigned int LAGRANGE_FIXED_POINT_POSITION;

// Интерполятор Лагранжа 7-й степени. Обеспечивает сдвиг тактов и дробную передискретизацию
// интервал между двумя смежными отсчетами делится на 1024 интервалов, 
// для каждого из которых используется свой заранее рассчитанный набор из 8 коэффициентов
class LagrangeInterp {
public:
	// frac - отношение частоты дискретизации входного сигнала к выходному --> [0.001, 1000]
	// если передискретизация не требуется, то frac = 1 
	// num_datapath количество параллельных потоков
	LagrangeInterp(xip_real frac = 1, unsigned num_datapath = 1);

	// num_datapath количество параллельных потоков
	LagrangeInterp(xip_real from_sampling_freq, xip_real to_sampling_freq, unsigned num_datapath = 1);

	~LagrangeInterp();

	/**
	 * @brief обработка очередного отсчета (одноканальный режим). в буфер отсчетов добавляется очередной отсчет (FIFO)
	 * @param in 
	*/
	void process(const xip_complex& in);

	/**
	 * @brief обработка очередного отсчета (многоканальный режим)
	 * @param in массив с многоканальным входным отсчетом
	*/
	void process(const xip_complex* in);

	/**
	 * @brief получить следующий отсчет (одноканальный режим). 
	 * true - есть отсчет, false - нет отсчета (требуется подать на вход следующий)
	 * @param out 
	 * @return 
	*/
	bool next(xip_complex& out);

	/**
	 * @brief получить следующий отсчет (многоканальный режим). 
	 * true - есть отсчет, false - нет отсчета (требуется подать на вход следующий)
	 * @param out
	 * @return
	*/
	bool next(xip_complex* out);

	/**
	 * @brief добавить смещение тактов относительно выходных отсчетов. вещественная версия
	 * @param value -> [-1.0, 1.0] относительно выходных отсчетов, производится приведение value в пределы [-1.0, 1.0]
	*/
	void shift(double value);

	/**
	 * @brief добавить смещение тактов относительно выходных отсчетов. целочисленная версия
	 * @param value -> [-2^FixPointPosition, 2^FixPointPosition] относительно выходных отсчетов, 
	 * производится приведение value в указанный диапазон
	*/
	void shift(int32_t value);

private:
	/**
	 * @brief инициализация библиотеки xip fir и загрузка наборов коэффициентов
	 * @return 
	*/
	int init_lagrange_interp();

	/**
	 * @brief загрузка наборов коэффициентов фильтра в память
	 * @return 
	*/
	int lagrange_load_coeff();

	/**
	 * @brief интерполяция очередного отсчета (одноканальный режим)
	 * @param values массив интерполируемых значений
	 * @param out результат интерполяции
	 * @param pos позиция интерполяции --> [0, LAGRANGE_INTERVALS-1]
	 * @return 
	*/
	int interpolate(xip_complex* values, xip_complex& out, uint32_t pos);

	/**
	 * @brief интерполяция очередного отсчета (многоканальный режим)
	 * @param samples_pos позиция в многоканальном массиве интерполируемых значений
	 * @param out многоканальный результат интерполяции
	 * @param pos позиция интерполяции --> [0, LAGRANGE_INTERVALS-1]
	 * @return
	*/
	int interpolate(int samples_pos, xip_complex* out, uint32_t pos);

	/**
	 * @brief деинициализация библиотеки xip fir и освобождение памяти
	 * @return 
	*/
	int destroy_lagrange_interp();

	size_t samples_count(double inv_factor);
	int to_dx_value(double inv_factor);
	void init(double dx_value);
	double get_coefficient(unsigned set_no, unsigned index);

	const uint32_t lagrange_n_intervals = LAGRANGE_INTERVALS;		// количество интервалов
	const uint32_t lagrange_n_coeff = LAGRANGE_ORDER;				// количество коэффициентов (порядок фильтра)
	double* lagrange_coeff;											// наборы коэффициентов фильтра, следуют по порядку
	const uint32_t FixPointPosition = LAGRANGE_FIXED_POINT_POSITION;
	const uint32_t FixPointPosMaxVal = 1 << FixPointPosition;
	unsigned m_numDataPath = 1;										// количество параллельных потоков


	vector< vector<xip_complex> > samples;							// многоканальный FIFO-буфер отсчетов

	// переменные состояния интерполятора
	int32_t dx;
	int32_t fx;		// дробная часть
	uint32_t block_size;
	uint32_t block_offset;
	xip_complex* x_ptr;
	xip_complex* pos_ptr;
	xip_complex* end_ptr;

	// паременные для работы с xip fir
	xip_fir_v7_2* lagrange_interp_fir;		// интерполирующий фильтр
	xip_fir_v7_2_config lagrange_interp_fir_cnfg;

	xip_array_uint* lagrange_interp_fir_fsel;
	xip_fir_v7_2_cnfg_packet lagrange_interp_fir_cnfg_packet;

	xip_array_real* lagrange_interp_in;		// 3-D массив, содержащий текущий отсчет для обработки
	xip_array_real* lagrange_interp_out;	// 3-D массив, содержащий результат обработки
};

#endif // LAGRANGEINTERP_H