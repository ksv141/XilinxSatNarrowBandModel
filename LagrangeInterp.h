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
#include "xip_utils.h"

using namespace std;

extern const int LAGRANGE_INTERVALS;

// »нтерпол€тор Ћагранжа 7-й степени. ќбеспечивает сдвиг тактов и дробную передискретизацию
// интервал между двум€ смежными отсчетами делитс€ на 1024 интервалов, 
// дл€ каждого из которых используетс€ свой заранее рассчитанный набор из 8 коэффициентов
class LagrangeInterp {
public:
	// frac - отношение частоты дискретизации выходного сигнала ко входному
	// если передискретизаци€ не требуетс€, то frac = 1
	LagrangeInterp(xip_real frac = 1);

	~LagrangeInterp();

	// выставить смещение тактов [-1, +1]
	void setShift(xip_real shift);

	// обработать очередной отсчет
	void process(const xip_complex& in);

	// получить следующий отсчет. true - есть отсчет, false - нет отсчета (требуетс€ подать на вход следующий)
	bool next(xip_complex& out);

	// вычисление вектора отсчетов дл€ кратной интерпол€ции со смещением
	void process(xip_real shift);

	/**
	 * @brief дл€ тестировани€ интерпол€тора
	 * @param in вход
	 * @param out выход
	 * @param time_shift сдвиг в диапазоне [-1023, 1023]
	*/
	void process(const xip_complex& in, xip_complex& out, int time_shift);

	uint32_t getPos();
	/**
	 * @brief приведение величины сдвига из [-inf, +inf] к полю [0, 1023]
	 * @param cur_shift текущий сдвиг
	 * @return 
	*/
	uint32_t countPos(int cur_shift);

private:
	int init_lagrange_interp();
	int lagrange_load_coeff();
	int process_sample_lagrange_interp(const xip_complex& in, xip_complex& out, uint32_t pos);
	int destroy_lagrange_interp();

	const uint32_t lagrange_n_intervals = LAGRANGE_INTERVALS;		// количество интервалов
	const uint32_t lagrange_n_coeff = 8;							// количество коэффициентов
	double* lagrange_coeff;											// наборы коэффициентов фильтра, следуют по пор€дку

	xip_real m_fraction;          // отношение частоты дискретизации выходного сигнала ко входному
	xip_real m_dk;                // текуща€ позици€ интерпол€ции [0; 1]
	int m_decim;				  // счетчик децимируемых отсчетов
	uint32_t m_pos;			  // текущий сдвиг
	xip_real m_prevShift;         // предыдущий сдвиг
	xip_complex m_currentSample;  // текущий входной отсчет

	xip_fir_v7_2* lagrange_interp_fir;		// фильтр на передаче
	xip_fir_v7_2_config lagrange_interp_fir_cnfg;

	xip_array_uint* lagrange_interp_fir_fsel;
	xip_fir_v7_2_cnfg_packet lagrange_interp_fir_cnfg_packet;

	xip_array_real* lagrange_interp_in;		// 3-D массив, содержащий текущий отсчет дл€ обработки
	xip_array_real* lagrange_interp_out;	// 3-D массив, содержащий результат обработки
};

#endif // LAGRANGEINTERP_H