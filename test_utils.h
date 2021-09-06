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

// ������� ��� ������������ ������

extern const int DDS_PHASE_MODULUS;
/**
 * @brief ��������� �������� �������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out �������� ���� (PCM ������ I/Q 16-���) 
 * @param dph ����� ���� � ��������� [0, 16383] --> [0, 2pi]
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
 * @brief ��������� �������������� ������������� �������
 * @param out �������� ���� (PCM ������ I/Q 16-���)
 * @param freq ������� ������� (��)
 * @param sample_freq ������� ������������� (��)
 * @count ���������� ������������ ��������
 * @bits �������� �������� (���)
*/
extern void generate_sin_signal(const string& out, double freq, double sample_freq, size_t count, int bits);

#endif // TESTUTILS_H