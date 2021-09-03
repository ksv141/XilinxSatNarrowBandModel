#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <string>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <fstream>
#include "func_templates.h"
#include "fx_cmpl_point.h"

using namespace std;
using namespace xilinx_m;

extern void signal_freq_shift(const string& in, const string& out, double dph);

#endif // TESTUTILS_H