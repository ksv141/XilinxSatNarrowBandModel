#ifndef HALFBANDDDC_H
#define HALFBANDDDC_H

#include "DDS.h"
#include "LowpassFir.h"

extern const int DDS_PHASE_MODULUS;

/**
 * @brief ������������ DDC. ������ ���������� �� fs/2, ������������� ��� (raised-cosine), ���������� �� 2
*/
class HalfBandDDC
{
public:
	HalfBandDDC();

private:
	DDS m_dds;
	LowpassFir m_lpFir;

	int16_t m_dph_up = DDS_PHASE_MODULUS/4;		// ����� ���� --> [-DDS_PHASE_MODULUS/4, +DDS_PHASE_MODULUS/4]
	int16_t m_dph_down = DDS_PHASE_MODULUS - DDS_PHASE_MODULUS / 4;
};

#endif // HALFBANDDDC_H