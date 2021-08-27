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
	// ошибка тактовой синхронизации по Мюллеру-Меллеру
	xip_real a1, a2, b1, b2, c1, c2;
	xip_real e_val;

	// произведения и суммы целесообразно реализовать параллельно
	process_fir_real_multiply(out_symb.re, m_last_est.re, a1);
	process_fir_real_multiply(est.re, m_last_out_symb.re, a2);
	process_fir_real_multiply(out_symb.im, m_last_est.im, b1);
	process_fir_real_multiply(est.im, m_last_out_symb.im, b2);

	process_fir_real_sum(a1, -a2, c1);
	process_fir_real_sum(b1, -b2, c2);
	process_fir_real_sum(c1, c2, e_val);

	//xip_real e_val = out_symb.real() * m_last_est.real() - est.real() * m_last_out_symb.real();
	//e_val += out_symb.imag() * m_last_est.imag() - est.imag() * m_last_out_symb.imag();

	e_val /= 2;		// это наверное лучше реализовать через >> 1

	m_last_out_symb = out_symb;
	m_last_est = est;
	return e_val;
}
