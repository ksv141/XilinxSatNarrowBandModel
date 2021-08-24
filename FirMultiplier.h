#ifndef FIRMULTIPLIER_H
#define FIRMULTIPLIER_H

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"

// умножитель вещественных чисел на основе Fir

extern int init_fir_real_multiplier();

extern int destroy_fir_real_multiplier();

extern int process_multiply_real(const xip_real& a, const xip_real& b, xip_real& out);

#endif // FIRMULTIPLIER_H