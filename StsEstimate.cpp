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
	xip_real a1, a2, b1, b2;
	xip_multiply_real(m_last_est.re, out_symb.re, a1);
	xip_multiply_real(est.re, m_last_out_symb.re, a2);
	xip_multiply_real(m_last_est.im, out_symb.im, b1);
	xip_multiply_real(est.im, m_last_out_symb.im, b2);

	xip_real e_val = (a1 - a2) + (b1 - b2);	// для сигнального созвездия размером 4096 ошибка будет в диапазоне [-2^26, 2^26]

	m_last_out_symb = out_symb;
	m_last_est = est;
	return e_val;
}

xip_real StsEstimate::getErrGardner(const xip_complex& sample)
{
	// нормированное значение, приведенное к [0, 1]
	// целесообразно реализовать сдвигом точки в fixed-point
	xip_complex sample_norm;
	sample_norm.re = sample.re / constell_psk_norm_val;
	sample_norm.im = sample.im / constell_psk_norm_val;
	// обновление последних 3 отсчетов, лучше реализовать на сдвиговом регистре
	y[2] = y[1];
	y[1] = y[0];
	y[0] = sample_norm;

	xip_real err = (y[0].re - y[2].re) * y[1].re + (y[0].im - y[2].im) * y[1].im;

	return err;
}
