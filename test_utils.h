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
#include "PolyphaseDecimator.h"

using namespace std;
using namespace xilinx_m;

// ������� ��� ������������ ������
extern const double PI;
extern const double _2_PI;
extern const int DDS_PHASE_MODULUS;

/**
 * @brief ��������� �������� �������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out �������� ���� (PCM ������ I/Q 16-���) 
 * @param dph ����� ���� �� ���� � ��������� [0, 16383] --> [0, 2pi]
*/
extern void signal_freq_shift(const string& in, const string& out, double dph);

/**
 * @brief �������� �������� �������.
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out �������� ���� (PCM ������ I/Q 16-���) 
 * @param time_shift ���������� �������� �� ����� �������� ���������. �������� [-1024, 1023].
 * �������������� ������ ����� ��������� �� �������� �� �������� T/time_shift
*/
extern void signal_time_shift(const string& in, const string& out, int32_t time_shift);

/**
 * @brief ������������ �������� �������� �������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out �������� ���� (PCM ������ I/Q 16-���)
 * @param shift_step ����� ������� ������ �������� ����� ����������� �� 1 �� ��������� [0, 1000]
*/
extern void signal_time_shift_dyn(const string& in, const string& out, int shift_step);

/**
 * @brief ��������� ������� �������������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out �������� ���� (PCM ������ I/Q 16-���)
 * @param from_sampling_freq �������� Fs
 * @param to_sampling_freq ��������� Fs
*/
extern void signal_resample(const string& in, const string& out, double from_sampling_freq, double to_sampling_freq);

/**
 * @brief ��������� �������������� ������������� �������
 * @param out �������� ���� (PCM ������ I/Q 16-���)
 * @param freq ������� ������� (��)
 * @param sample_freq ������� ������������� (��)
 * @count ���������� ������������ ��������
 * @bits �������� �������� (���)
*/
extern void generate_sin_signal(const string& out, double freq, double sample_freq, size_t count, int bits);

/**
 * @brief ��������� ���������� ���-�������� � ����� ����� ���
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out �������� ���� (PCM ������ I/Q 16-���)
 * @decim_factor ����������� ���������
*/
extern void signal_decimate(const string& in, const string& out, unsigned decim_factor);


extern int test_fir_order();

#endif // TESTUTILS_H