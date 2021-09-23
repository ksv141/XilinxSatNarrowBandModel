#ifndef HALFBANDDDC_H
#define HALFBANDDDC_H

#include "DDS.h"
#include "LowpassFir.h"
#include "xip_utils.h"

extern const int DDS_PHASE_MODULUS;

/**
 * @brief ������������ DDC. �������:
 *	- ������������� ������������ ��������� �� +/- fs/4
 *	- ������������ ��� (raised-cosine)
 *	- ��������� �� 2
 * ������������ ����������� 2 ������, ��������� �� ������� ��� ���������� ����� ������������
*/
class HalfBandDDC
{
public:
	/**
	 * @brief �������������
	 * @param level ������� DDC � ������
	*/
	HalfBandDDC(unsigned level);

	/**
	 * @brief ��������� ���������� ������� (���������������). ������� ������ ���� ������� ������ ��� ��������� ��������� �������
	 * @param in ������ � �������������� ������� ��������, ������ �� ����� m_inChannels
	 * @param out ������ � �������������� �������� �������� (������ ������ ���� ������������ � 2 ���� ������ �� ��������)
	 * @return ���� ������ (��/���)
	*/
	bool process(const xip_complex* in, xip_complex* out);

private:
	DDS m_dds;
	LowpassFir m_lpFir;
	unsigned m_level;							// ������� DDC � ������
	unsigned m_inChannels;						// ���������� ������� ������� ������ --> 2^(m_level-1)
	unsigned m_outChannels;						// ���������� �������� ������� ������ --> 2^m_level

	int16_t m_dph = DDS_PHASE_MODULUS/4;		// ����� ���� --> [-DDS_PHASE_MODULUS/4, +DDS_PHASE_MODULUS/4]

	int m_inSampleCounter = 0;					// ������� ������� �������� [0, 1]
};

#endif // HALFBANDDDC_H