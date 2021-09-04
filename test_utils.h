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
#include "LagrangeInterp.h"

using namespace std;
using namespace xilinx_m;

// утилиты для тестирования модема

extern const int DDS_PHASE_MODULUS;
/**
 * @brief частотное смещение сигнала
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит) 
 * @param dph набег фазы в диапазоне [0, 16383] --> [0, 2pi]
*/
extern void signal_freq_shift(const string& in, const string& out, double dph);
/**
 * @brief тактовое смещение сигнала.
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит) 
 * @param time_shift дискретное смещение на одном тактовом интервале. диапазон [0, 1023].
 * результирующий сигнал будет отставать от входного на величину T/time_shift
*/
extern void signal_time_shift(const string& in, const string& out, int time_shift);

#endif // TESTUTILS_H