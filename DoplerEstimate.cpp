#include "DoplerEstimate.h"

DoplerEstimate::DoplerEstimate()
{
    m_reg.resize(4, { 0, 0 });
}

int16_t DoplerEstimate::getErr(const xip_complex& out_symb, const xip_complex& est)
{
    xip_complex est_sample{ est.re, -est.im };  // комплексно-сопряженное от текущего решения
    xip_multiply_complex(est_sample, out_symb, est_sample);     // --> [-2^26, 2^26]
    xip_complex_shift(est_sample, -10);                         // --> [-2^16, 2^16]
    m_reg.pop_back();
    m_reg.push_front(est_sample);

    xip_complex r1{ m_reg[0].re + m_reg[1].re, m_reg[0].im + m_reg[1].im };
    xip_complex r2{ m_reg[2].re + m_reg[3].re, m_reg[2].im + m_reg[3].im };
    r2.im = -r2.im;
    xip_complex r3{ r1.re + r2.re, r1.im + r2.im };

    // Оценка смещения по частоте (v = Arg{x}/2) на 1 символ [-8192, 8191]
    xip_real mag;
    xip_real arg;
    xip_cordic_rect_to_polar(r3, mag, arg);
    int16_t arg_int = (int16_t)arg;
    arg_int >>= 1;

    return arg_int;
}
