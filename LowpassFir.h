#ifndef	LOWPASSFIR_H
#define LOWPASSFIR_H

#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"

using namespace std;

/**
 * @brief ФНЧ КИХ
*/
class LowpassFir
{
public:
	/**
	 * @brief конструктор
	 * @param coeff_file файл с коэффициентами фильтра
	 * @param num_coeff количество коэффициентов фильтра
	 * @param is_halfband фильтр полуполосный (0 - нет, 1 - да)
	 * @param num_datapath количество параллельных потоков
	 * @param num_coeff количество коэффициентов фильтра
	*/
	LowpassFir(const string& coeff_file, unsigned num_coeff, unsigned is_halfband = 0, unsigned num_datapath = 1);

	~LowpassFir();

	/**
	 * @brief обработка очередного отсчета
	 * @param in 
	 * @param out 
	 * @return 
	*/
	int process(const xip_complex& in, xip_complex& out);

private:
	/**
	 * @brief инициализация библиотеки xip fir и загрузка коэффициентов
	 * @return
	*/
	int init_xip_fir(const string& coeff_file, unsigned num_coeff);

	/**
	 * @brief загрузка коэффициентов фильтра в память
	 * @param coeff_file файл с коэффициентами фильтра
	 * @param num_coeff количество коэффициентов фильтра
	 * @return
	*/
	int load_coeff(const string& coeff_file, unsigned num_coeff);

	/**
	 * @brief деинициализация библиотеки xip fir и освобождение памяти
	 * @return
	*/
	int destroy_xip_fir();

	unsigned m_numCoeff;				// количество коэффициентов фильтра
	double* m_firCoeff = nullptr;		// набор коэффициентов фильтра
	unsigned m_decimCounter = 0;		// кольцевой счетчик входных отсчетов [0, m_decimFactor-1]
	unsigned m_isHalfBand = 0;			// фильтр полуполосный (0 - нет, 1 - да)
	unsigned m_numDataPath = 1;			// количество параллельных потоков

	// паременные для работы с xip fir
	xip_fir_v7_2* xip_fir;				// полифазный фильтр
	xip_fir_v7_2_config xip_fir_cnfg;	// конфиг фильтра
	xip_array_real* xip_fir_in;			// 3-D массив, содержащий текущий отсчет для обработки
	xip_array_real* xip_fir_out;		// 3-D массив, содержащий результат обработки
};

#endif // LOWPASSFIR_H