#ifndef	CORRELATORDPDIMANCHESTER_H
#define CORRELATORDPDIMANCHESTER_H

#include <deque>
#include <vector>
#include <stdexcept>

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
	*/
	CorrelatorDPDIManchester(int8_t* preamble_data, uint16_t preamble_length,
		uint16_t M, uint16_t L, uint32_t burst_est, uint16_t baud_mul);

	/**
	 * @brief оценка частоты
	 * @param in входной отсчет
	 * @param dph оценка частоты (набег фазы за символ) --> [-8192, 8192]
	 * оценка правдоподобная только при превышении порога
	 * @param cur_est текущий корреляционный отклик (используется для отладки),
	 * @return есть (true) или нет (false) срабатывание порога
	*/
	bool process(xip_complex in, int16_t& dph, xip_real& cur_est);

	/**
	 * @brief для тестирования
	 * @param in 
	 * @param cur_corr 
	 * @param cur_est 
	*/
	void test_corr(xip_complex in, xip_real* est, xip_real* dph);

	/**
	 * @brief возвращает буфер коррелятора
	*/
	deque<xip_complex>& getBuffer();

private:
	/**
	 * @brief инициализация регистров коррелятора
	*/
	void init(int8_t* preamble_data, uint16_t preamble_length);

	deque<xip_complex> m_correlationReg;     // FIFO-регистр для вычисления корреляции
	vector<xip_complex> m_preamble;          // Регистр с преамбулой (в виде комплексно-сопряженных чисел)

	xip_complex m_corr_1{ 0, 0 };			// корреляция на 4-х суботсчетах
	xip_complex m_corr_2{ 0, 0 };
	xip_complex m_corr_3{ 0, 0 };
	xip_complex m_corr_4{ 0, 0 };

	deque<xip_complex> m_corr;				// регистр корреляции на N суботсчетах (размер равен m_baudMul)

	uint16_t m_preambleLength;           // Размер преамбулы
	uint16_t m_correlatorM;              // Размер единичного коррелятора
	uint16_t m_correlatorL;              // Количество единичных корреляторов
	xip_real m_burstEstML;               // Пороговое значение для корреляционного отклика (критерий максимального правдоподобия)
	uint16_t m_baudMul;					 // Коэффициент увеличения бодовой скорости, на которой работает коррелятор
										 // Без манчестера - 2B, с манчестером - 4B/8B/16B...

	uint16_t m_argShift;				 // величина битового сдвига для вычисления v = Arg{x}/4M (с учетом 2B и манчестера)
};

#endif // CORRELATORDPDIMANCHESTER_H