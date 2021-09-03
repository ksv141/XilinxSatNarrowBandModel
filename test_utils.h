#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <string>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <fstream>
#include "func_templates.h"
#include "fx_cmpl_point.h"
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "DDS.h"
#include "xip_utils.h"

using namespace std;
using namespace xilinx_m;

extern const int DDS_PHASE_MODULUS;

extern void signal_freq_shift(const string& in, const string& out, double dph);

#endif // TESTUTILS_H