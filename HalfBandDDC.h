#ifndef HALFBANDDDC_H
#define HALFBANDDDC_H

#include "DDS.h"
#include "LowpassFir.h"

extern const int DDS_PHASE_MODULUS;

/**
 * @brief Полуполосный DDC. Каскад модулятора на fs/2, полуполосного ФНЧ (raised-cosine), дециматора на 2
*/
class HalfBandDDC
{
public:
	HalfBandDDC();

private:
	DDS m_dds;
	LowpassFir m_lpFir;

	int16_t m_dph_up = DDS_PHASE_MODULUS/4;		// набег фазы --> [-DDS_PHASE_MODULUS/4, +DDS_PHASE_MODULUS/4]
	int16_t m_dph_down = DDS_PHASE_MODULUS - DDS_PHASE_MODULUS / 4;
};

#endif // HALFBANDDDC_H