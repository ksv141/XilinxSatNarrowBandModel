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
 * @brief  оррел€тор дл€ фазовой и символьной синхронизации по преамбуле дл€ манчестерского кода на 4B
 * ќценка кактовой ошибки по 4 точкам
 * (A Fast Synchronizer for Burst Modems with Simultaneous Symbol Timing and Carrier Phase Estimations Dengwei Fu and Alan N. Willson, Jr.)
*/
class PhClcCorrelatorManchester
{
public:
	/**
	 * @brief инициализаци€ коррел€тора
	 * @param preamble_data преамбула
	 * @param preamble_length размер преамбулы
	 * @param burst_est ѕороговое значение дл€ коррел€ционного отклика (критерий максимального правдоподоби€)
	*/
	PhClcCorrelatorManchester(int8_t* preamble_data, uint16_t preamble_length, uint32_t burst_est);

	/**
	 * @brief оценка фазового сдвига
	 * @param in входной отсчет
	 * @param phase оценка сдвига фазы --> [-8192, 8192]
	 * @param phase_est текущий коррел€ционный отклик (используетс€ дл€ отладки)
	 * @return есть (true) или нет (false) срабатывание порога коррел€тора
	*/
	bool phaseEstimate(xip_complex in, int16_t& phase, xip_real& phase_est);

	/**
	 * @brief оценка тактового сдвига
	 * @param in входной отсчет
	 * @param time_shift оценка сдвига тактов
	 * @param time_est текущий коррел€ционный отклик (используетс€ дл€ отладки)
	 * @return есть (true) или нет (false) срабатывание услови€ дл€ оценки сдвига
	*/
	//bool symbolTimingEstimate(xip_complex in, int16_t& time_shift, xip_real& time_est);

	/**
	 * @brief возвращает текущий режим коррел€тора
	 * @return
	*/
	bool isPhaseEstMode();

	/**
	 * @brief возвращает число попыток оценить тактовый сдвиг
	 * @return
	*/
	uint8_t getSymbolTimingProcCounter();

	/**
	 * @brief дл€ тестировани€
	 * @param in 
	 * @param corr 
	 * @param est 
	*/
	void test_corr(xip_complex in, xip_complex& corr, xip_real& est);

	/**
	 * @brief дл€ тестировани€
	 * @return 
	*/
	int getMaxCorrPos();

private:
	/**
	 * @brief инициализаци€ регистров коррел€тора
	*/
	void init(int8_t* preamble_data, uint16_t preamble_length);

	/**
	 * @brief вычисление коррел€ции
	 * @param in входной отсчет
	*/
	void process(xip_complex in);

	deque<xip_complex> m_correlationReg;     // FIFO-регистр дл€ вычислени€ коррел€ции
	deque<xip_complex> m_corrValuesReg;		 // FIFO-регистр дл€ хранени€ коррел€ционных откликов
	deque<xip_complex> m_corrSumValuesReg;		 // FIFO-регистр дл€ хранени€ коррел€ционных откликов
	deque<xip_real> m_timingSyncReg;		 // FIFO-регистр дл€ тактовой синхронизации
	deque<xip_real> m_corrMnchReg;			 // FIFO-регистр дл€ сумм коррел€ционных откликов полусимволов манчестерского кода
	vector<xip_complex> m_preamble;          // –егистр с преамбулой (в виде комплексно-сопр€женных чисел)
	bool m_phaseEstMode;					 // режим оценки фазы (используетс€ дл€ переключени€ в режим оценки тактов)
	uint8_t m_symbolTimingProcCounter;		 // счетчик попыток оценить тактовый сдвиг (условие может не выполнитьс€)
	int m_maxCorrPos;						 // позици€ максимального отклика в регистре m_corrValuesReg

	uint16_t m_preambleLength;           // –азмер преамбулы
	xip_real m_burstEstML;               // ѕороговое значение дл€ коррел€ционного отклика (критерий максимального правдоподоби€)
};

#endif // PHCLCCORRELATORMANCHESTER_H