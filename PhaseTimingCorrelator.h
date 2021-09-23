#ifndef	PHASETIMINGCORRELATOR_H
#define PHASETIMINGCORRELATOR_H

#include <cstdint>
#include <deque>
#include <vector>
#include <stdexcept>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "xip_utils.h"
#include "debug.h"
#include "constellation.h"

using namespace std;

/**
 * @brief Коррелятор для фазовой и символьной синхронизации по преамбуле
*/
class PhaseTimingCorrelator
{
public:
	/**
	 * @brief инициализация коррелятора
	 * @param preamble_data преамбула
	 * @param preamble_length размер преамбулы
	 * @param burst_est Пороговое значение для корреляционного отклика (критерий максимального правдоподобия)
	*/
	PhaseTimingCorrelator(int8_t* preamble_data, uint16_t preamble_length, uint32_t burst_est);

	/**
	 * @brief оценка фазового и тактового сдвига
	 * @param in входной отсчет
	 * @param phase оценка сдвига фазы --> [-8192, 8192] 
	 * @param time_shift оценка сдвига тактов
	 * @param phase_est текущий корреляционный отклик (используется для отладки)
	 * @return есть (true) или нет (false) срабатывание порога коррелятора
	*/
	bool process(xip_complex in, int16_t& phase, xip_real& time_shift, xip_real& phase_est);

private:
	/**
	 * @brief инициализация регистров коррелятора
	*/
	void init(int8_t* preamble_data, uint16_t preamble_length);

	deque<xip_complex> m_correlationReg;     // FIFO-регистр для вычисления корреляции
	deque<xip_real> m_timingSyncReg;		 // FIFO-регистр для тактовой синхронизации
	vector<xip_complex> m_preamble;          // Регистр с преамбулой (в виде комплексно-сопряженных чисел)

	uint16_t m_preambleLength;           // Размер преамбулы
	xip_real m_burstEstML;               // Пороговое значение для корреляционного отклика (критерий максимального правдоподобия)
};

#endif // PHASETIMINGCORRELATOR_H