#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <string>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <iomanip>
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
#include "PhaseTimingCorrelator.h"
#include "NarrowBandDemodulator.h"
#include "ChannelMatchedFir.h"
#include "Pif.h"
#include "StsEstimate.h"
#include "DoplerEstimate.h"

using namespace std;
using namespace xilinx_m;

// утилиты для тестирования модема
extern const double PI;
extern const double _2_PI;
extern const int DDS_PHASE_MODULUS;
extern const uint16_t FRAME_DATA_SIZE;
extern const uint32_t DPDI_BURST_ML_SATGE_1;
extern const uint32_t DPDI_BURST_ML_SATGE_2;

extern const int DDS_RAD_CONST;		// радиан на одну позицию фазы << 3 == 20860 (16 бит)
extern const double PIF_PLL_Kp;		// коэффициент пропорциональной составляющей ПИФ ФАПЧ
extern const double PIF_PLL_Ki;		// коэффициент интегральной составляющей ПИФ ФАПЧ
extern const double PIF_STS_Kp;		// коэффициент пропорциональной составляющей ПИФ СТС
extern const double PIF_STS_Ki;		// коэффициент интегральной составляющей ПИФ СТС
extern const double PIF_DOPL_Kp;	// коэффициент пропорциональной составляющей ПИФ Допл (при specific_locking_band = 0.01)
extern const double PIF_DOPL_Ki;	// коэффициент интегральной составляющей ПИФ Допл (при specific_locking_band = 0.01)

extern const int BAUD_RATE;			// бодовая скорость в канале
extern const int HIGH_SAMPLE_RATE;	// частота дискретизации на входе демодулятора

/**
 * @brief частотное смещение сигнала
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит) 
 * @param freq_shift смещение частоты в Гц [-fs, fs]
 * @param fs частота дискретизации в Гц
*/
extern void signal_freq_shift(const string& in, const string& out, double freq_shift, double fs);

/**
 * @brief частотное смещение сигнала
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out_up выходной файл со сдвигом вверх (PCM стерео I/Q 16-бит)
 * @param out_down выходной файл со сдвигом вниз (PCM стерео I/Q 16-бит)
 * @param freq_shift смещение частоты в Гц [-fs, fs]
 * @param fs частота дискретизации в Гц
*/
extern void signal_freq_shift_symmetric(const string& in, const string& out_up, const string& out_down, double freq_shift, double fs);

/**
 * @brief частотное смещение сигнала
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит)
 * @param freq_shift_mod смещение частоты в единицах работы DDS --> [-8192, 8192]
*/
extern void signal_freq_shift(const string& in, const string& out, int16_t freq_shift_mod);


/**
 * @brief доплеровское частотное смещение сигнала
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит)
 * @param freq_ampl амплитуда изменения частоты в Гц
 * @param freq_peiod период изменения частоты в сек
*/
extern void signal_freq_shift_dopl(const string& in, const string& out, double fs, double freq_ampl, double freq_peiod);

/**
 * @brief частотное и фазовое смещение сигнала
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит)
 * @param freq_shift_mod смещение частоты в единицах работы DDS --> [-8192, 8192]
 * @param phase смещение фазы в единицах работы DDS --> [-8192, 8192]
*/
extern void signal_freq_phase_shift(const string& in, const string& out, int16_t freq_shift_mod, int16_t phase);

/**
 * @brief двухканальное симметричное частотное смещение сигнала
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out_up выходной файл со сдвигом вверх (PCM стерео I/Q 16-бит)
 * @param out_down выходной файл со сдвигом вниз (PCM стерео I/Q 16-бит)
 * @param freq_shift_mod смещение частоты вверх и вниз в единицах работы DDS --> [-8192, 8192]
*/
extern void signal_freq_shift_symmetric(const string& in, const string& out_up, const string& out_down, int16_t freq_shift_mod);

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
 * @brief изменение частоты дискретизации
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит)
 * @param from_sampling_freq исходная Fs
 * @param to_sampling_freq требуемая Fs
*/
extern void signal_resample(const string& in, const string& out, double from_sampling_freq, double to_sampling_freq);

/**
 * @brief генератор гармонического квадратурного сигнала
 * @param out выходной файл (PCM стерео I/Q 16-бит)
 * @param freq частота сигнала (Гц)
 * @param sample_freq частота дискретизации (Гц)
 * @count количество генерируемых отсчетов
 * @bits точность отсчетов (бит)
*/
extern void generate_sin_signal(const string& out, double freq, double sample_freq, size_t count, int bits);

/**
 * @brief децимация полифазным КИХ-фильтром в целое число раз
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит)
 * @decim_factor коэффициент децимации
*/
extern void signal_decimate(const string& in, const string& out, unsigned decim_factor);

/**
 * @brief интерполяция полифазным КИХ-фильтром в целое число раз
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит)
 * @interp_factor коэффициент интерполяции
*/
extern void signal_interpolate(const string& in, const string& out, unsigned interp_factor);

/**
 * @brief фильтрация заданным КИХ-фильтром
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит)
 * @param coeff_file файл с коэффициентами фильтра
 * @param num_coeff количество коэффициентов фильтра
*/
extern void signal_lowpass(const string& in, const string& out, const string& coeff_file, unsigned num_coeff);

/**
 * @brief фильтрация заданным КИХ-фильтром
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out выходной файл (PCM стерео I/Q 16-бит)
 * @param window_size_log2 - log2 окна усреднения АРУ
 * @param norm_power требуемая мощность сигнала
*/
extern void signal_agc(const string& in, const string& out, unsigned window_size_log2, double norm_power);

/**
 * @brief вычисление корреляционного отклика частотного коррелятора
 * @param in входной файл (PCM стерео I/Q 16-бит)
*/
extern bool signal_freq_est_stage(const string& in, uint16_t M, uint16_t L, uint16_t F, uint32_t burst_est, int16_t& freq_est);

/**
 * @brief вычисление корреляционного отклика фазового коррелятора
 * @param in входной файл (PCM стерео I/Q 16-бит)
*/
extern bool signal_phase_time_est_stage(const string& in, uint32_t burst_est, int16_t& phase, xip_real& time_shift, int& t_count);

/**
 * @brief разделение сигнала на половину полосы
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param out_up выходной файл (PCM стерео I/Q 16-бит)
 * @param out_down выходной файл (PCM стерео I/Q 16-бит)
*/
extern void signal_halfband_ddc(const string& in, const string& out_up, const string& out_down);

/**
 * @brief обнаружение сигнала и определение частотного смещения
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param corr_num номер коррелятора, обнаружившего сигнал
 * @param freq_est_stage_1 грубая оценка частотного смещения [-DDS_PHASE_MODULUS, DDS_PHASE_MODULUS] --> [0, 2pi]
 * @param freq_est_stage_2 точная оценка частотного смещения [-DDS_PHASE_MODULUS, DDS_PHASE_MODULUS] --> [0, 2pi]
 * @param phase_est оценка смещения фазы [-DDS_PHASE_MODULUS, DDS_PHASE_MODULUS] --> [0, 2pi]
 * @param symbol_timing_est оценка тактового смещения [-1024, 1024]
 * @param total_freq_est общая оценка частотного смещения в единицах входной частоты [-DDS_PHASE_MODULUS, DDS_PHASE_MODULUS] --> [0, 2pi]
*/
extern void signal_ddc_estimate(const string& in, unsigned& corr_num, int16_t& freq_est_stage_1, int16_t& freq_est_stage_2,
								int16_t& phase_est, int16_t& symbol_timing_est, int16_t& total_freq_est);

/**
 * @brief обнаружение сигнала, определение частотного смещения и демодуляция
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param dem_out сигнал на входе блока принятия решения на скорости 1B (PCM стерео I/Q 16-бит)
*/
extern void signal_estimate_demodulate(const string& in, const string& dem_out);

/**
 * @brief тестирование компенсации Доплера на скорости 2B
 * @param in входной файл (PCM стерео I/Q 16-бит)
 * @param dem_out сигнал на входе блока принятия решения на скорости 1B (PCM стерео I/Q 16-бит)
*/
extern void signal_estimate_demodulate_dopl_test(const string& in, const string& dem_out);

#endif // TESTUTILS_H