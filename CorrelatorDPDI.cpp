#include "CorrelatorDPDI.h"

CorrelatorDPDI::CorrelatorDPDI(uint16_t data_length, int8_t* preamble_data, uint16_t M, uint16_t L, uint16_t F, xip_real burst_est):
	m_dataLength(data_length),
	m_correlatorM(M),
	m_correlatorL(L),
	m_correlatorF(F),
	m_burstEstML(burst_est)
{
}
