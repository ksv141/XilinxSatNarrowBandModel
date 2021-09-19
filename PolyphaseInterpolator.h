#ifndef POLYPHASEINTERPOLATOR_H
#define POLYPHASEINTERPOLATOR_H

#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"

using namespace std;

/**
 * @brief интерпол€тор на основе полифазного фильтра
*/
class PolyphaseInterpolator
{
public:
	/**
	 * @brief конструктор
	 * @param interp_factor коэффициент интерпол€ции
	 * @param coeff_file файл с коэффициентами фильтра
	 * @param num_coeff количество коэффициентов фильтра
	*/
	PolyphaseInterpolator(unsigned interp_factor, const string& coeff_file, unsigned num_coeff);

	~PolyphaseInterpolator();

	/**
	 * @brief обработать очередной отсчет
	 * @param in входной отсчет
	 * очередной вызов process возможен только когда next вернет false
	*/
	int process(const xip_complex& in);

	/**
	 * @brief получить интерполированный отсчет
	 * @param out интерполированный отсчет
	 * @return true - есть еще отсчет, false - отсчетов больше нет
	 * очередной вызов process возможен только когда next вернет false
	*/
	bool next(xip_complex& out);

private:
	/**
	 * @brief инициализаци€ библиотеки xip fir и загрузка коэффициентов
	 * @return
	*/
	int init_xip_fir(const string& coeff_file, unsigned num_coeff);

	/**
	 * @brief загрузка коэффициентов фильтра в пам€ть
	 * @param coeff_file файл с коэффициентами фильтра
	 * @param num_coeff количество коэффициентов фильтра
	 * @return
	*/
	int load_coeff(const string& coeff_file, unsigned num_coeff);

	/**
	 * @brief деинициализаци€ библиотеки xip fir и освобождение пам€ти
	 * @return
	*/
	int destroy_xip_fir();

	unsigned m_interpFactor;			// коэффициент интерпол€ции
	unsigned m_numCoeff;				// количество коэффициентов фильтра
	double* m_firCoeff = nullptr;		// набор коэффициентов фильтра
	unsigned m_interpCounter = 0;		// кольцевой счетчик выходных отсчетов [0, m_interpFactor-1]

	// паременные дл€ работы с xip fir
	xip_fir_v7_2* xip_fir;				// полифазный фильтр
	xip_fir_v7_2_config xip_fir_cnfg;	// конфиг фильтра
	xip_array_real* xip_fir_in;			// 3-D массив, содержащий текущий отсчет дл€ обработки
	xip_array_real* xip_fir_out;		// 3-D массив, содержащий результат обработки
};

#endif // POLYPHASEINTERPOLATOR_H