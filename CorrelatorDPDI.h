#ifndef	CORRELATORDPDI_H
#define CORRELATORDPDI_H

#include <deque>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "xip_utils.h"
#include "debug.h"

using namespace std;

/**
 * @brief Коррелятор для частоты дискретизации Fs = 2*Fd
 * DPDI detector (Theory of Code and Frame Synchronization
 * with Applications to Advanced Wireless Communication Systems, p. 125)
*/
class CorrelatorDPDI
{
public:
	CorrelatorDPDI(uint16_t data_length, int8_t* preamble_data, uint16_t M, uint16_t L, uint16_t F, xip_real burst_est);

private:
	deque<xip_complex> m_correlationReg;     // регистр для вычисления корреляции

    uint16_t m_dataLength;               // Размер данных в кадре, не включая преамбулу
    uint16_t m_correlatorM;              // Размер единичного коррелятора
    uint16_t m_correlatorL;              // Количество единичных корреляторов
    uint16_t m_correlatorF;              // Количество корреляторов ML для накопления корреляционного отклика
    xip_real m_burstEstML;               // Пороговое значение для корреляционного отклика (критерий максимального правдоподобия)
};

#endif // CORRELATORDPDI_H