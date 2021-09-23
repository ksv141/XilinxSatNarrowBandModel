#include "HalfBandDDC.h"

HalfBandDDC::HalfBandDDC(unsigned level):
	m_dds(DDS_PHASE_MODULUS, true),
	m_lpFir("rc_normal_x2_25_19.fcf", 19, 1, 1 << (level + 1)),
	m_level(level),
	m_inChannels(1 << level),
	m_outChannels(1 << (level + 1))
{
}

bool HalfBandDDC::process(const xip_complex* in, xip_complex* out)
{
	xip_complex mod_sample_up{ 0, 0 };
	xip_complex mod_sample_down{ 0, 0 };
	m_dds.process(m_dph, mod_sample_up, mod_sample_down);

	for (int i = 0; i < m_inChannels; i++) {
		xip_multiply_complex(in[i], mod_sample_up, out[2*i]);			// в четные пишем смещенный вверх
		xip_complex_shift(out[2*i], -(int)(m_dds.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)
		xip_multiply_complex(in[i], mod_sample_down, out[2*i+1]);		// в нечетные пишем смещенный вниз
		xip_complex_shift(out[2*i+1], -(int)(m_dds.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)
	}

	m_lpFir.process(out, out);

	m_inSampleCounter++;
	if (m_inSampleCounter == 2) {
		m_inSampleCounter = 0;
		return true;
	}
	return false;
}
