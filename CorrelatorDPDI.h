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
					uint16_t M, uint16_t L, uint16_t F, uint32_t burst_est);

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
	 * @brief оценка частоты сигнала во внешнем буфере (например, в другом корреляторе)
	 * @param samples входной буфер отсчетов
	 * @param dph оценка частоты (набег фазы за символ) --> [-8192, 8192]
	 * оценка правдоподобная только при превышении порога
	 * @return есть (true) или нет (false) срабатывание порога
	*/
	bool processBuffer(const deque<xip_complex>& samples, int16_t& dph);

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
	xip_complex m_prev_sum_1{ 0, 0 };		 // сумма корреляции на предыдущем такте

    uint16_t m_dataLength;               // Размер данных в кадре, не включая преамбулу
	uint16_t m_preambleLength;           // Размер преамбулы
	uint16_t m_correlatorM;              // Размер единичного коррелятора
    uint16_t m_correlatorL;              // Количество единичных корреляторов
    uint16_t m_correlatorF;              // Количество корреляторов ML для накопления корреляционного отклика
    xip_real m_burstEstML;               // Пороговое значение для корреляционного отклика (критерий максимального правдоподобия)

	uint16_t m_argShift;				 // величина битового сдвига для вычисления v = Arg{x}/2M
};

#endif // CORRELATORDPDI_H