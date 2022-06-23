#ifndef	PHCLCCORRELATORMANCHESTER_H
#define PHCLCCORRELATORMANCHESTER_H

#include <cstdint>
#include <deque>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "xip_utils.h"
#include "debug.h"
#include "constellation.h"

using namespace std;

/**
 * @brief Коррелятор для фазовой и символьной синхронизации по преамбуле для манчестерского кода на 4B
 * Оценка кактовой ошибки по 4 точкам
 * (A Fast Synchronizer for Burst Modems with Simultaneous Symbol Timing and Carrier Phase Estimations Dengwei Fu and Alan N. Willson, Jr.)
*/
class PhClcCorrelatorManchester
{
public:
	/**
	 * @brief инициализация коррелятора
	 * @param preamble_data преамбула
	 * @param preamble_length размер преамбулы
	 * @param burst_est Пороговое значение для корреляционного отклика (критерий максимального правдоподобия)
	*/
	PhClcCorrelatorManchester(int8_t* preamble_data, uint16_t preamble_length, uint32_t burst_est);

	/**
	 * @brief оценка фазового сдвига
	 * @param in входной отсчет
	 * @param phase оценка сдвига фазы --> [-8192, 8192]
	 * @param phase_est текущий корреляционный отклик (используется для отладки)
	 * @return есть (true) или нет (false) срабатывание порога коррелятора
	*/
	bool phaseEstimate(xip_complex in, int16_t& phase, xip_real& phase_est);

	/**
	 * @brief оценка тактового сдвига
	 * @param in входной отсчет
	 * @param time_shift оценка сдвига тактов
	 * @param time_est текущий корреляционный отклик (используется для отладки)
	 * @return есть (true) или нет (false) срабатывание условия для оценки сдвига
	*/
	//bool symbolTimingEstimate(xip_complex in, int16_t& time_shift, xip_real& time_est);

	/**
	 * @brief возвращает текущий режим коррелятора
	 * @return
	*/
	bool isPhaseEstMode();

	/**
	 * @brief возвращает число попыток оценить тактовый сдвиг
	 * @return
	*/
	uint8_t getSymbolTimingProcCounter();

	/**
	 * @brief для тестирования
	 * @param in 
	 * @param corr 
	 * @param est 
	*/
	void test_corr(xip_complex in, xip_complex& corr, xip_real& est);

private:
	/**
	 * @brief инициализация регистров коррелятора
	*/
	void init(int8_t* preamble_data, uint16_t preamble_length);

	/**
	 * @brief вычисление корреляции
	 * @param in входной отсчет
	*/
	void process(xip_complex in);

	deque<xip_complex> m_correlationReg;     // FIFO-регистр для вычисления корреляции
	deque<xip_complex> m_corrValuesReg;		 // FIFO-регистр для хранения корреляционных откликов
	deque<xip_real> m_timingSyncReg;		 // FIFO-регистр для тактовой синхронизации
	xip_complex m_sumCorr_1;					 // суммарный отклик для двух манчестерских символов
	xip_complex m_sumCorr_2;					 // суммарный отклик для двух манчестерских символов
	vector<xip_complex> m_preamble;          // Регистр с преамбулой (в виде комплексно-сопряженных чисел)
	//xip_complex m_currentCorrelation;		 // текущее значение с выхода коррелятора
	bool m_phaseEstMode;					 // режим оценки фазы (используется для переключения в режим оценки тактов)
	uint8_t m_symbolTimingProcCounter;		 // счетчик попыток оценить тактовый сдвиг (условие может не выполниться)

	uint16_t m_preambleLength;           // Размер преамбулы
	xip_real m_burstEstML;               // Пороговое значение для корреляционного отклика (критерий максимального правдоподобия)
};

#endif // PHCLCCORRELATORMANCHESTER_H