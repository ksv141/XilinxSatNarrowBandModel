#include "NarrowBandDemodulator.h"

NarrowBandDemodulator::NarrowBandDemodulator():
	m_tuneCorrelator(FRAME_DATA_SIZE, 
					(int8_t*)SignalSource::preambleData, 
					(uint16_t)SignalSource::preambleLength,
					8, 4, 1, 
					DPDI_BURST_ML_SATGE_2),
	m_phaseTimingCorrelator((int8_t*)SignalSource::preambleData, 
					(uint16_t)SignalSource::preambleLength, 
					PHASE_BURST_ML_SATGE_3)
{
}
