#ifndef HALFBANDDDC_H
#define HALFBANDDDC_H

#include "DDS.h"
#include "LowpassFir.h"
#include "xip_utils.h"

extern const int DDS_PHASE_MODULUS;

/**
 * @brief ������������ DDC. ������ ���������� �� fs/2, ������������� ��� (raised-cosine), ���������� �� 2
*/
class HalfBandDDC
{
public:
	HalfBandDDC();

	/**
	 * @brief ��������� ���������� �������. ������� ������ ���� ������� ������ ����� ���������� ��������� �������
	 * @param in ����
	*/
	void process(const xip_complex& in);

	/**
	 * @brief �������� �������� ������. ����� ������ ������� next ���������� ������ �� ���� ��� ������� ����� process
	 * @param out 
	 * @return ���� ������ (��/���)
	*/
	bool next(xip_complex& out);

private:
	DDS m_dds;
	LowpassFir m_lpFir;

	int16_t m_dph_up = DDS_PHASE_MODULUS/4;		// ����� ���� --> [-DDS_PHASE_MODULUS/4, +DDS_PHASE_MODULUS/4]
	int16_t m_dph_down = DDS_PHASE_MODULUS - DDS_PHASE_MODULUS / 4;

	xip_complex m_lastSample{ 0, 0 };			// ��������� ������������ ������
	int m_inSampleCounter = 0;					// ������� ������� �������� [0, 1]
};

#endif // HALFBANDDDC_H