#include "NarrowBandDemodulator.h"

NarrowBandDemodulator::NarrowBandDemodulator():
	m_tuneCorrelator(FRAME_DATA_SIZE, 
					(int8_t*)PREAMBLE_DATA,
					(uint16_t)PREAMBLE_LENGTH,
					8, 4, 1, 
					DPDI_BURST_ML_SATGE_2),
	m_phaseTimingCorrelator((int8_t*)PREAMBLE_DATA,
					(uint16_t)PREAMBLE_LENGTH,
					PHASE_BURST_ML_SATGE_3)
{
}
