#include "HalfBandDDC.h"

HalfBandDDC::HalfBandDDC():
	m_dds(DDS_PHASE_MODULUS),
	m_lpFir("rc_normal_x2_25_19.fcf", 19)
{
}
