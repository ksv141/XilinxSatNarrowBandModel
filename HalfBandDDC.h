#ifndef HALFBANDDDC_H
#define HALFBANDDDC_H

#include "DDS.h"
#include "LowpassFir.h"
#include "xip_utils.h"

extern const int DDS_PHASE_MODULUS;

/**
 * @brief Полуполосный DDC. Состоит:
 *	- двухканальный симметричный модулятор на +/- fs/4
 *	- полуполосный ФНЧ (raised-cosine)
 *	- дециматор на 2
 * Обрабатывает параллельно 2 потока, смещенные по частоте для перекрытия полос корреляторов
*/
class HalfBandDDC
{
public:
	/**
	 * @brief инициализация
	 * @param level уровень DDC в дереве
	*/
	HalfBandDDC(unsigned level);

	/**
	 * @brief обработка очередного отсчета (многоканального). функция должна быть вызвана дважды для получения выходного отсчета
	 * @param in массив с многоканальным входным отсчетом, размер не менее m_inChannels
	 * @param out массив с многоканальным выходным отсчетом (память должна быть аллоцирована в 2 раза больше от входного)
	 * @return есть отсчет (да/нет)
	*/
	bool process(const xip_complex* in, xip_complex* out);

private:
	DDS m_dds;
	LowpassFir m_lpFir;
	unsigned m_level;							// уровень DDC в дереве
	unsigned m_inChannels;						// количество входных потоков данных --> 2^(m_level-1)
	unsigned m_outChannels;						// количество выходных потоков данных --> 2^m_level

	int16_t m_dph = DDS_PHASE_MODULUS/4;		// набег фазы --> [-DDS_PHASE_MODULUS/4, +DDS_PHASE_MODULUS/4]

	int m_inSampleCounter = 0;					// счетчик входных отсчетов [0, 1]
};

#endif // HALFBANDDDC_H