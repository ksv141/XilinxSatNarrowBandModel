#ifndef	NARROWBANDDEMODULATOR_H
#define NARROWBANDDEMODULATOR_H

#include "HalfBandDDCTree.h"
#include "PhaseTimingCorrelator.h"

/**
 * @brief Узкополосный демодулятор
*/
class NarrowBandDemodulator
{
public:
	NarrowBandDemodulator();

private:
	HalfBandDDCTree m_ddcTree;						// Бинарное дерево полуполосных DDC
	CorrelatorDPDI m_tuneCorrelator;				// одноканальный коррелятор (2-я стадия, точная оценка)
	PhaseTimingCorrelator m_phaseTimingCorrelator;	// коррелятор для оценки фазы и тактов (3-я стадия)

	int16_t m_freqEstStage_2;						// частота с выхода точного коррелятора
	int16_t m_phaseEst;								// фаза с выхода коррелятора 3-й стадии
	int16_t m_symbolTimingEst;						// тактовое смещение с выхода коррелятора 3-й стадии
};

#endif // NARROWBANDDEMODULATOR_H