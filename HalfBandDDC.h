#ifndef HALFBANDDDC_H
#define HALFBANDDDC_H

#include "DDS.h"
#include "LowpassFir.h"
#include "xip_utils.h"

extern const int DDS_PHASE_MODULUS;

/**
 * @brief Полуполосный DDC. Каскад модулятора на fs/2, полуполосного ФНЧ (raised-cosine), дециматора на 2
*/
class HalfBandDDC
{
public:
	HalfBandDDC();

	/**
	 * @brief обработка очередного отсчета. функция должна быть вызвана дважды перед получением выходного отсчета
	 * @param in вход
	*/
	void process(const xip_complex& in);

	/**
	 * @brief получить выходной отсчет. перед каждым вызовом next необходимо подать на вход два отсчета через process
	 * @param out 
	 * @return есть отсчет (да/нет)
	*/
	bool next(xip_complex& out);

private:
	DDS m_dds;
	LowpassFir m_lpFir;

	int16_t m_dph_up = DDS_PHASE_MODULUS/4;		// набег фазы --> [-DDS_PHASE_MODULUS/4, +DDS_PHASE_MODULUS/4]
	int16_t m_dph_down = DDS_PHASE_MODULUS - DDS_PHASE_MODULUS / 4;

	xip_complex m_lastSample{ 0, 0 };			// последний обработанный отсчет
	int m_inSampleCounter = 0;					// счетчик входных отсчетов [0, 1]
};

#endif // HALFBANDDDC_H