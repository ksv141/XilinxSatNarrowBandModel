#ifndef	NARROWBANDDEMODULATOR_H
#define NARROWBANDDEMODULATOR_H

#include "HalfBandDDCTree.h"
#include "PhaseTimingCorrelator.h"

/**
 * @brief ������������ �����������
*/
class NarrowBandDemodulator
{
public:
	NarrowBandDemodulator();

private:
	HalfBandDDCTree m_ddcTree;						// �������� ������ ������������ DDC
	CorrelatorDPDI m_tuneCorrelator;				// ������������� ���������� (2-� ������, ������ ������)
	PhaseTimingCorrelator m_phaseTimingCorrelator;	// ���������� ��� ������ ���� � ������ (3-� ������)

	int16_t m_freqEstStage_2;						// ������� � ������ ������� �����������
	int16_t m_phaseEst;								// ���� � ������ ����������� 3-� ������
	int16_t m_symbolTimingEst;						// �������� �������� � ������ ����������� 3-� ������
};

#endif // NARROWBANDDEMODULATOR_H