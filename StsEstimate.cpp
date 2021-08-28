#include "StsEstimate.h"



StsEstimate::StsEstimate():
	m_last_out_symb{ 0, 0 },
	m_last_est{ 0, 0 }
{
}


StsEstimate::~StsEstimate()
{
}

xip_real StsEstimate::getErr(const xip_complex& out_symb, const xip_complex& est)
{
	// ������������� ��������, ����������� � [0, 1]
	// ������������� ����������� ������� ����� � fixed-point
	xip_complex out_symb_norm;
	out_symb_norm.re = out_symb.re / constell_qam4_norm_val;
	out_symb_norm.im = out_symb.im / constell_qam4_norm_val;
	xip_complex est_norm;
	est_norm.re = est.re / constell_qam4_norm_val;
	est_norm.im = est.im / constell_qam4_norm_val;

	// ������ �������� ������������� �� �������-�������
	xip_real a1, a2, b1, b2, c1, c2;
	xip_real e_val;

	// ������������ � ����� ������������� ����������� �����������
	process_fir_real_multiply(out_symb_norm.re, m_last_est.re, a1);
	process_fir_real_multiply(est_norm.re, m_last_out_symb.re, a2);
	process_fir_real_multiply(out_symb_norm.im, m_last_est.im, b1);
	process_fir_real_multiply(est_norm.im, m_last_out_symb.im, b2);

	process_fir_real_sum(a1, -a2, c1);
	process_fir_real_sum(b1, -b2, c2);
	process_fir_real_sum(c1, c2, e_val);

	//xip_real e_val_1 = out_symb.re * m_last_est.re - est.re * m_last_out_symb.re;
	//e_val_1 += out_symb.im * m_last_est.im - est.im * m_last_out_symb.im;
	//e_val_1 /= 2;

	//e_val /= 2;		// ��� �������� ����� ����������� ����� >> 1

	m_last_out_symb = out_symb_norm;
	m_last_est = est_norm;
	return e_val;
}

xip_real StsEstimate::getErrGardner(const xip_complex& sample)
{
	// ������������� ��������, ����������� � [0, 1]
	// ������������� ����������� ������� ����� � fixed-point
	xip_complex sample_norm;
	sample_norm.re = sample.re / constell_qam4_norm_val;
	sample_norm.im = sample.im / constell_qam4_norm_val;
	// ���������� ��������� 3 ��������, ����� ����������� �� ��������� ��������
	y[2] = y[1];
	y[1] = y[0];
	y[0] = sample_norm;

	xip_real err = (y[0].re - y[2].re) * y[1].re + (y[0].im - y[2].im) * y[1].im;

	return err;
}
