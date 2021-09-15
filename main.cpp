#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <thread>

#include "all_headers.h"

// Для тестов библиотек XIP
//#include "XilinxIpTests.h"

using namespace std;

static const double PI = 3.141592653589793238463;
static const double _2_PI = PI * 2;

/**
* Скорости в канале (бод/с):
* 9143
* 16000
* 24000
* 18286
* 48000
* 36570
* 96000
* 85710
* 171430
* 342860
* 685710
*/

// глобальные параметры
const int DDS_PHASE_MODULUS = 16384;				// диапазон изменения фазы [0, 16383] --> [0, 2pi]. Для ФАПЧ и петли Доплера
const int DDS_RAD_CONST = (int)(DDS_PHASE_MODULUS * 8 / _2_PI);	// радиан на одну позицию фазы << 3 == 20860 (16 бит)
const uint16_t FRAME_DATA_SIZE = 1115;					// размер данных в кадре (байт)
const int AGC_WND_SIZE_LOG2 = 5;					// log2 окна усреднения АРУ

const double PIF_STS_Kp = 0.026311636311692643;		// коэффициент пропорциональной составляющей ПИФ СТС (при specific_locking_band = 0.01)
const double PIF_STS_Ki = 0.00035088206622480023;	// коэффициент интегральной составляющей ПИФ СТС (при specific_locking_band = 0.01)
const double PIF_PLL_Kp = 0.026311636311692643;		// коэффициент пропорциональной составляющей ПИФ ФАПЧ (при specific_locking_band = 0.01)
const double PIF_PLL_Ki = 0.00035088206622480023;	// коэффициент интегральной составляющей ПИФ ФАПЧ (при specific_locking_band = 0.01)

const int BAUD_RATE = 9143;							// бодовая скорость в канале
const int INIT_SAMPLE_RATE = 2 * BAUD_RATE;			// начальная частота дискретизации на выходе канального фильтра

// параметры корреляторов
uint32_t DPDI_BURST_ML_SATGE_1 = 125;				// порог обнаружения сигнала коррелятора первой стадии (грубая оценка частоты)
uint32_t DPDI_BURST_ML_SATGE_2 = 32;				// порог обнаружения сигнала коррелятора второй стадии (точная оценка частоты)

int main()
{
	set_current_constell(Current_constell::PSK4);

	init_xip_multiplier();
	init_xip_cordic_sqrt();
	init_channel_matched_fir();

	//Modulator mdl("data.bin", "out_mod.pcm", FRAME_DATA_SIZE);
	//Modulator mdl("1.zip", "out_mod.pcm", FRAME_DATA_SIZE);
	//mdl.process();

	//signal_time_shift("out_mod_fr_shift.pcm", "out_mod_tm_shift.pcm", 128);
	//signal_time_shift_dyn("out_mod_fr_shift.pcm", "out_mod_tm_shift.pcm", 10);
	double resample_coeff = 1.0;
	//signal_resample("out_mod.pcm", "out_mod_rsmpl.pcm", INIT_SAMPLE_RATE, INIT_SAMPLE_RATE* resample_coeff);

	//signal_resample("out_mod.pcm", "out_mod_25.pcm", INIT_SAMPLE_RATE, 25000);
	//signal_interpolate("out_mod_25.pcm", "out_mod_interp_x64.pcm", 64);
	//signal_freq_shift("out_mod_interp_x64.pcm", "out_mod_interp_x64_shift_780.pcm", 780000, 1600000);
	//signal_freq_shift("out_mod_interp_x64_shift_780.pcm", "out_mod_interp_x64_downshift_780.pcm", -780000, 1600000);
	//signal_lowpass("out_mod_interp_x64_downshift_780.pcm", "out_mod_interp_x64_downshift_780_lowpass.pcm", "lowpass_200kHz.fcf", 51);
	//signal_decimate("out_mod_interp_x64_downshift_780_lowpass.pcm", "out_mod_decim_x16.pcm", 4);
	//signal_decimate("out_mod_decim_x16.pcm", "out_mod_decim_x4.pcm", 4);
	//signal_decimate("out_mod_decim_x4.pcm", "out_mod_decim_x1.pcm", 4);

	signal_resample("out_mod_decim_x1.pcm", "out_mod_res25.pcm", 25000, INIT_SAMPLE_RATE);
	signal_agc("out_mod_res25.pcm", "out_mod_agc.pcm", AGC_WND_SIZE_LOG2, get_cur_constell_pwr());
	signal_lowpass("out_mod_agc.pcm", "out_mod_mf.pcm", "rc_root_x2_25_19.fcf", 19);
	signal_correlation("out_mod_mf.pcm");

	//Demodulator dmd("out_mod_decim_x1.pcm", "out_mod_dmd.pcm", "out_mod.bin", FRAME_DATA_SIZE);
	//dmd.process();

	destroy_xip_multiplier();
	destroy_xip_cordic_sqrt();
	destroy_channel_matched_fir();

	return 0;
}