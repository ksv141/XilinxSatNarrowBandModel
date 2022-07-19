#ifndef	CORRELATORDPDIMANCHESTER_H
#define CORRELATORDPDIMANCHESTER_H

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
 * @brief Коррелятор для манчестерского кода на 4B
 * DPDI detector (Theory of Code and Frame Synchronization
 * with Applications to Advanced Wireless Communication Systems, p. 125)
*/
class CorrelatorDPDIManchester
{
public:
	/**
	 * @brief инициализация коррелятора
	 * @param preamble_data преамбула
	 * @param preamble_length размер преамбулы
	 * @param M Размер единичного коррелятора
	 * @param L Количество единичных корреляторов
	 * @param burst_est Пороговое значение для корреляционного отклика (критерий максимального правдоподобия)
	 * @param baud_mul Коэффициент увеличения бодовой скорости, на которой работает коррелятор (с манчестером - 4B/8B/16B...)
	*/
	CorrelatorDPDIManchester(int8_t* preamble_data, uint16_t preamble_length,
		uint16_t M, uint16_t L, uint32_t burst_est, uint16_t baud_mul = 4);

	/**
	 * @brief оценка частоты
	 * @param in входной отсчет
	 * @param corr_val текущий корреляционный отклик (устанавливается только при срабатывании порога)
	 * @param max_corr_pos позиция максимального отклика в регистре m_corrValuesReg
	 * @param cur_est энергия текущего корреляционного отклика (используется для отладки)
	 * @return есть (true) или нет (false) срабатывание порога
	*/
	bool freqEstimate(const xip_complex& in, xip_complex& corr_val, int& max_corr_pos, xip_real& cur_est);

	/**
	 * @brief поиск максимального значения энергии корреляции в регистре m_corrMnchReg
	 * @param corr_val значение корреляционного отклика, соответствующее найденному максимальному
	 * @param max_corr_pos позиция максимального отклика в регистре m_corrValuesReg
	 * @return максимальная энергия корреляции
	*/
	xip_real getMaxCorrVal(xip_complex& corr_val, int& max_corr_pos);

	/**
	 * @brief расчет частоты по корреляционному отклику
	 * @param corr_val текущий корреляционный отклик
	 * @return (набег фазы за символ) --> [-8192, 8192]
	*/
	int16_t countFreq(const xip_complex& corr_val);

	/**
	 * @brief для тестирования
	 * @param in 
	 * @param cur_corr 
	 * @param cur_est 
	*/
	void test_corr(const xip_complex& in, xip_real& est, xip_real& dph);

	/**
	 * @brief возвращает буфер коррелятора
	*/
	deque<xip_complex>& getBuffer();

private:
	/**
	 * @brief инициализация регистров коррелятора
	*/
	void init(int8_t* preamble_data, uint16_t preamble_length);

	/**
	 * @brief вычисление корреляции
	 * @param in входной отсчет
	*/
	void process(const xip_complex& in);

	deque<xip_complex> m_correlationReg;     // FIFO-регистр для вычисления корреляции
	vector<xip_complex> m_preamble;          // Регистр с преамбулой (в виде комплексно-сопряженных чисел)

	deque<xip_complex> m_corr;				// регистр корреляции на N суботсчетах для хранения откликов полусимволов (размер равен m_baudMul)
	deque<xip_complex> m_corrSumValuesReg;	// FIFO-регистр для хранения суммарных корреляционных откликов (для оценки частоты)
	deque<xip_real> m_corrMnchReg;			// FIFO-регистр для энергии сумм корреляционных откликов полусимволов манчестерского кода

	uint16_t m_preambleLength;           // Размер преамбулы
	uint16_t m_correlatorM;              // Размер единичного коррелятора
	uint16_t m_correlatorL;              // Количество единичных корреляторов
	xip_real m_burstEstML;               // Пороговое значение для корреляционного отклика (критерий максимального правдоподобия)
	uint16_t m_baudMul;					 // Коэффициент увеличения бодовой скорости, на которой работает коррелятор
										 // Без манчестера - 2B, с манчестером - 4B/8B/16B...
	unsigned m_sumMnchStep;				 // Шаг суммарной корреляции сопряженных манчестерских отсчетов (2/4/8...)

	uint16_t m_argShift;				 // величина битового сдвига для вычисления v = Arg{x}/(m_baudMul*M) (с учетом 2B и манчестера)
};

#endif // CORRELATORDPDIMANCHESTER_H