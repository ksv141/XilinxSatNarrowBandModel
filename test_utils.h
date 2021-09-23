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
#include "PolyphaseInterpolator.h"
#include "LowpassFir.h"
#include "CorrelatorDPDI.h"
#include "SignalSource.h"
#include "autoganecontrol.h"
#include "HalfBandDDC.h"
#include "HalfBandDDCTree.h"

using namespace std;
using namespace xilinx_m;

// ������� ��� ������������ ������
extern const double PI;
extern const double _2_PI;
extern const int DDS_PHASE_MODULUS;
extern const uint16_t FRAME_DATA_SIZE;
extern const uint32_t DPDI_BURST_ML_SATGE_1;
extern const uint32_t DPDI_BURST_ML_SATGE_2;

/**
 * @brief ��������� �������� �������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out �������� ���� (PCM ������ I/Q 16-���) 
 * @param freq_shift �������� ������� � �� [-fs, fs]
 * @param fs ������� ������������� � ��
*/
extern void signal_freq_shift(const string& in, const string& out, double freq_shift, double fs);

/**
 * @brief ��������� �������� �������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out_up �������� ���� �� ������� ����� (PCM ������ I/Q 16-���)
 * @param out_down �������� ���� �� ������� ���� (PCM ������ I/Q 16-���)
 * @param freq_shift �������� ������� � �� [-fs, fs]
 * @param fs ������� ������������� � ��
*/
extern void signal_freq_shift_symmetric(const string& in, const string& out_up, const string& out_down, double freq_shift, double fs);

/**
 * @brief ��������� �������� �������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out �������� ���� (PCM ������ I/Q 16-���)
 * @param freq_shift_mod �������� ������� � �������� ������ DDS --> [-8192, 8192]
*/
extern void signal_freq_shift(const string& in, const string& out, int16_t freq_shift_mod);

/**
 * @brief ������������� ������������ ��������� �������� �������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out_up �������� ���� �� ������� ����� (PCM ������ I/Q 16-���)
 * @param out_down �������� ���� �� ������� ���� (PCM ������ I/Q 16-���)
 * @param freq_shift_mod �������� ������� ����� � ���� � �������� ������ DDS --> [-8192, 8192]
*/
extern void signal_freq_shift_symmetric(const string& in, const string& out_up, const string& out_down, int16_t freq_shift_mod);

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

/**
 * @brief ������������ ���������� ���-�������� � ����� ����� ���
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out �������� ���� (PCM ������ I/Q 16-���)
 * @interp_factor ����������� ������������
*/
extern void signal_interpolate(const string& in, const string& out, unsigned interp_factor);

/**
 * @brief ���������� �������� ���-��������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out �������� ���� (PCM ������ I/Q 16-���)
 * @param coeff_file ���� � �������������� �������
 * @param num_coeff ���������� ������������� �������
*/
extern void signal_lowpass(const string& in, const string& out, const string& coeff_file, unsigned num_coeff);

/**
 * @brief ���������� �������� ���-��������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out �������� ���� (PCM ������ I/Q 16-���)
 * @param window_size_log2 - log2 ���� ���������� ���
 * @param norm_power ��������� �������� �������
*/
extern void signal_agc(const string& in, const string& out, unsigned window_size_log2, double norm_power);

/**
 * @brief ���������� ��������������� ������� �������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
*/
extern bool signal_freq_est_stage(const string& in, uint16_t M, uint16_t L, uint16_t F, uint32_t burst_est, int16_t& freq_est);

/**
 * @brief ���������� ������� �� �������� ������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param out_up �������� ���� (PCM ������ I/Q 16-���)
 * @param out_down �������� ���� (PCM ������ I/Q 16-���)
*/
extern void signal_halfband_ddc(const string& in, const string& out_up, const string& out_down);

/**
 * @brief ����������� ������� � ����������� ���������� ��������
 * @param in ������� ���� (PCM ������ I/Q 16-���)
 * @param corr_num ����� �����������, ������������� ������
 * @param freq_est_stage_1 ������ ������ ���������� �������� [-DDS_PHASE_MODULUS, DDS_PHASE_MODULUS] --> [0, 2pi].
*/
extern void signal_ddc_estimate(const string& in, unsigned& corr_num, int16_t& freq_est_stage_1, int16_t& freq_est_stage_2);

#endif // TESTUTILS_H