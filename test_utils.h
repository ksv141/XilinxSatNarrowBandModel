#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <string>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <complex>
#include <cmath>
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
 * @param time_shift дискретное смещение на одном тактовом интервале. диапазон [-1024, 1023].
 * результирующий сигнал будет отставать от входного на величину T/time_shift
*/
extern void signal_time_shift(const string& in, const string& out, int32_t time_shift);

/**
 * @brief изменяющееся тактовое смещение сигнала
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит)
 * @param shift_step через сколько тактов смещение будет приращаться на 1 из диапазона [0, 1000]
*/
extern void signal_time_shift_dyn(const string& in, const string& out, int shift_step);

/**
 * @brief генератор гармонического квадратурного сигнала
 * @param out выходной файл (PCM стерео I/Q 16-бит)
 * @param freq частота сигнала (Гц)
 * @param sample_freq частота дискретизации (Гц)
 * @count количество генерируемых отсчетов
 * @bits точность отсчетов (бит)
*/
extern void generate_sin_signal(const string& out, double freq, double sample_freq, size_t count, int bits);

#endif // TESTUTILS_H