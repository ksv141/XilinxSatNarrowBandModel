#ifndef FIRSUMMATOR_H
#define FIRSUMMATOR_H

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"

// сумматор вещественных чисел на основе Fir 2-й степени с единичными коэффициентами
// в данном проекте сумматор достаточно иметь один глобальный на все элементы схемы, т.к. код выполняется последовательно

extern int init_fir_real_summator();

extern int destroy_fir_real_summator();

extern int process_fir_real_sum(const xip_real& a, const xip_real& b, xip_real& out);

#endif // FIRSUMMATOR_H