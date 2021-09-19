#include "HalfBandDDC.h"

HalfBandDDC::HalfBandDDC():
	m_dds(DDS_PHASE_MODULUS),
	m_lpFir("rc_normal_x2_25_19.fcf", 19)
{
}

int HalfBandDDC::process(const xip_complex& in, xip_complex& out)
{
	double dds_phase, dds_sin, dds_cos;
	m_dds.process(m_dph_down, dds_phase, dds_sin, dds_cos);
	xip_complex mod_sample{ dds_cos, dds_sin };
	xip_complex res;
	xip_multiply_complex(in, mod_sample, res);
	xip_complex_shift(res, -(int)(m_dds.getOutputWidth() - 1));	// ��������� ������������ �������� ���������� (��������� ������� �����)
	
	m_lpFir.process(res, res);
	out = res;
	return 0;
}
