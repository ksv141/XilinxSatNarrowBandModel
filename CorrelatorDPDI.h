#ifndef	CORRELATORDPDI_H
#define CORRELATORDPDI_H

#include <deque>
#include <vector>
#include <stdexcept>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "xip_utils.h"
#include "debug.h"
#include "constellation.h"

using namespace std;

/**
 * @brief Коррелятор для частоты дискретизации Fs = 2*Fd
 * DPDI detector (Theory of Code and Frame Synchronization
 * with Applications to Advanced Wireless Communication Systems, p. 125)
*/
class CorrelatorDPDI
{
public:
	/**
	 * @brief инициализация коррелятора
	 * @param data_length Размер данных в кадре, не включая преамбулу
	 * @param preamble_data преамбула
	 * @param preamble_length размер преамбулы
	 * @param M Размер единичного коррелятора
	 * @param L Количество единичных корреляторов
	 * @param F Количество корреляторов ML для накопления корреляционного отклика
	 * @param burst_est Пороговое значение для корреляционного отклика (критерий максимального правдоподобия)
	*/
	CorrelatorDPDI(uint16_t data_length, int8_t* preamble_data, uint16_t preamble_length,
					uint16_t M, uint16_t L, uint16_t F, xip_real burst_est);

private:
	/**
	 * @brief инициализация регистров коррелятора
	*/
	void init(int8_t* preamble_data, uint16_t preamble_length);

	deque<xip_complex> m_correlationReg;     // регистр для вычисления корреляции
	vector<xip_complex> m_preamble;          // Регистр с преамбулой (в виде комплексно-сопряженных чисел)

    uint16_t m_dataLength;               // Размер данных в кадре, не включая преамбулу
	uint16_t m_preambleLength;           // Размер преамбулы
	uint16_t m_correlatorM;              // Размер единичного коррелятора
    uint16_t m_correlatorL;              // Количество единичных корреляторов
    uint16_t m_correlatorF;              // Количество корреляторов ML для накопления корреляционного отклика
    xip_real m_burstEstML;               // Пороговое значение для корреляционного отклика (критерий максимального правдоподобия)
};

#endif // CORRELATORDPDI_H