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

// Интерполятор Лагранжа 7-й степени. Обеспечивает сдвиг тактов и дробную передискретизацию
// интервал между двумя смежными отсчетами делится на 1024 интервалов, 
// для каждого из которых используется свой заранее рассчитанный набор из 8 коэффициентов
class LagrangeInterp {
public:
	// frac - отношение частоты дискретизации входного сигнала к выходному --> [0.001, 1000]
	// если передискретизация не требуется, то frac = 1 
	LagrangeInterp(xip_real frac = 1);

	LagrangeInterp(xip_real from_sampling_freq, xip_real to_sampling_freq);

	~LagrangeInterp();

	// выставить смещение тактов [-1, +1]
	void setShift(xip_real shift);

	// обработать очередной отсчет
	void process(const xip_complex& in);

	// получить следующий отсчет. true - есть отсчет, false - нет отсчета (требуется подать на вход следующий)
	bool next(xip_complex& out);

	// вычисление вектора отсчетов для кратной интерполяции со смещением
	void process(xip_real shift);

	/**
	 * @brief для тестирования интерполятора
	 * @param in вход
	 * @param out выход
	 * @param time_shift сдвиг в диапазоне [-1023, 1023]
	*/
	void process(const xip_complex& in, xip_complex& out, int time_shift);

	/**
	 * @brief приведение величины сдвига из [-inf, +inf] к полю [0, 1023]
	 * @param cur_shift текущий сдвиг
	 * @return 
	*/
	uint32_t countPos(int cur_shift);

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
	 * @brief интерполяция очередного отсчета
	 * @param in вход
	 * @param out результат интерполяции
	 * @param pos позиция интерполяции --> [0, LAGRANGE_INTERVALS-1]
	 * @return 
	*/
	int interpolate(const xip_complex& in, xip_complex& out, uint32_t pos);
	/**
	 * @brief инициализация библиотеки xip fir и освобождение памяти
	 * @return 
	*/
	int destroy_lagrange_interp();

	size_t samples_count(double inv_factor);
	int to_dx_value(double inv_factor);
	void init(double dx_value);

	const uint32_t lagrange_n_intervals = LAGRANGE_INTERVALS;		// количество интервалов
	const uint32_t lagrange_n_coeff = LAGRANGE_ORDER;				// количество коэффициентов (порядок фильтра)
	double* lagrange_coeff;											// наборы коэффициентов фильтра, следуют по порядку
	const uint32_t FixPointPosition = 20;

	vector<xip_complex> samples;	// буфер отсчетов

	// переменные состояния интерполятора
	int32_t dx;
	int32_t fx;		// дробная часть
	uint32_t block_size;
	uint32_t block_offset;
	xip_complex* x_ptr;
	xip_complex* pos_ptr;
	xip_complex* end_ptr;


	//************** to delete **********************
	xip_real m_fraction;          // отношение частоты дискретизации выходного сигнала ко входному
	xip_real m_dk;                // текущая позиция интерполяции [0; 1]
	int m_decim;				  // счетчик децимируемых отсчетов
	uint32_t m_pos;			  // текущий сдвиг
	xip_real m_prevShift;         // предыдущий сдвиг
	xip_complex m_currentSample;  // текущий входной отсчет
	//***********************************************

	// паременные для работы с xip fir
	xip_fir_v7_2* lagrange_interp_fir;		// фильтр на передаче
	xip_fir_v7_2_config lagrange_interp_fir_cnfg;

	xip_array_uint* lagrange_interp_fir_fsel;
	xip_fir_v7_2_cnfg_packet lagrange_interp_fir_cnfg_packet;

	xip_array_real* lagrange_interp_in;		// 3-D массив, содержащий текущий отсчет для обработки
	xip_array_real* lagrange_interp_out;	// 3-D массив, содержащий результат обработки
};

#endif // LAGRANGEINTERP_H