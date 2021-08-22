#ifndef LAGRANGEINTERP_H
#define LAGRANGEINTERP_H

#include <bitset>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"

extern int init_lagrange_interp();

extern int process_sample_lagrange_interp(xip_complex* in, xip_complex* out, uint32_t pos);

extern int destroy_lagrange_interp();

#endif // LAGRANGEINTERP_H