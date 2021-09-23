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
const uint32_t DPDI_BURST_ML_SATGE_1 = 3000;			// порог обнаружения сигнала коррелятора первой стадии (грубая оценка частоты)
const uint32_t DPDI_BURST_ML_SATGE_2 = 3500;			// порог обнаружения сигнала коррелятора второй стадии (точная оценка частоты)
const uint32_t PHASE_BURST_ML_SATGE_3 = 3500;			// порог обнаружения сигнала коррелятора третьей стадии (оценка фазы и тактов)

int main()
{
	set_current_constell(Current_constell::PSK4);

	init_xip_multiplier();
	init_xip_cordic_sqrt();
	init_xip_cordic_rect_to_polar();
	init_channel_matched_fir();

	//signal_halfband_ddc("out_mod_shift_up_6.pcm", "out_mod_shift_ddc_up.pcm", "out_mod_shift_ddc_down.pcm");
	//signal_halfband_ddc("out_mod_shift_ddc_down.pcm", "out_mod_shift_ddc_up_1.pcm", "out_mod_shift_ddc_down_1.pcm");

	//Modulator mdl("data.bin", "out_mod.pcm", FRAME_DATA_SIZE);
	//Modulator mdl("1.zip", "out_mod.pcm", FRAME_DATA_SIZE);
	//mdl.process();

	//signal_time_shift("out_mod_fr_shift.pcm", "out_mod_tm_shift.pcm", 128);
	//signal_time_shift_dyn("out_mod_fr_shift.pcm", "out_mod_tm_shift.pcm", 10);
	//double resample_coeff = 1.0;
	//signal_resample("out_mod.pcm", "out_mod_rsmpl.pcm", INIT_SAMPLE_RATE, INIT_SAMPLE_RATE* resample_coeff);

	//signal_resample("out_mod.pcm", "out_mod_25.pcm", INIT_SAMPLE_RATE, 25000);
	//signal_interpolate("out_mod_25.pcm", "out_mod_interp_x64.pcm", 64);
	//signal_freq_shift("out_mod_interp_x64.pcm", "out_mod_interp_x64_383500.pcm", 383500, 1600000);
	//signal_freq_shift("out_mod_interp_x64_383500.pcm", "out_mod_interp_x64_downshift_200.pcm", -200000, 1600000);
	//signal_lowpass("out_mod_interp_x64_downshift_200.pcm", "out_mod_interp_x64_downshift_200_lowpass.pcm", "lowpass_200kHz.fcf", 51);
	//signal_decimate("out_mod_interp_x64_downshift_200_lowpass.pcm", "out_mod_decim_x16.pcm", 4);

	//signal_halfband_ddc("out_mod_decim_x16.pcm", "out_mod_halfband_25_up.pcm", "out_mod_halfband_25_down.pcm");

	unsigned corr_num = 0;
	int16_t freq_1 = 0;
	int16_t freq_2 = 0;
	signal_ddc_estimate("out_mod_decim_x16.pcm", corr_num, freq_1, freq_2);
	cout << "corr = " << corr_num << endl << "freq 1 = " << freq_1 << endl << "freq 2 = " << freq_2 << endl;

	//signal_halfband_ddc("out_mod_halfband_200_down.pcm", "out_mod_halfband_100_up.pcm", "out_mod_halfband_100_down.pcm");

	//signal_decimate("out_mod_halfband_100.pcm", "out_mod_decim_x1.pcm", 4);
	//signal_decimate("out_mod_decim_x4.pcm", "out_mod_decim_x1.pcm", 4);

	//signal_resample("out_mod_halfband_25.pcm", "out_mod_rcv.pcm", 25000, INIT_SAMPLE_RATE);
	//signal_lowpass("out_mod_rcv.pcm", "out_mod_mf.pcm", "rc_root_x2_25_19.fcf", 19);
	//signal_agc("out_mod_mf.pcm", "out_mod_rcv_agc.pcm", AGC_WND_SIZE_LOG2, get_cur_constell_pwr());

	//int16_t freq_est_stage_1 = 0;
	//signal_freq_est_stage("out_mod_rcv_agc.pcm", 1, 32, 1, DPDI_BURST_ML_SATGE_1, freq_est_stage_1);
	//signal_freq_shift("out_mod_rcv.pcm", "out_mod_rcv_st_1.pcm", -freq_est_stage_1);

	//signal_lowpass("out_mod_rcv_st_1.pcm", "out_mod_rcv_st_1_mf.pcm", "rc_root_x2_25_19.fcf", 19);
	//signal_agc("out_mod_rcv_st_1_mf.pcm", "out_mod_rcv_st_1_agc.pcm", AGC_WND_SIZE_LOG2, get_cur_constell_pwr());

	//int16_t freq_est_stage_2 = 0;
	//signal_freq_est_stage("out_mod_rcv_st_1_agc.pcm", 8, 4, 1, DPDI_BURST_ML_SATGE_2, freq_est_stage_2);
	//signal_freq_shift("out_mod_rcv_st_1.pcm", "out_mod_rcv_st_2.pcm", -freq_est_stage_2);

	//int16_t freq_est_sum = (387500./1600000.)*DDS_PHASE_MODULUS + freq_est_stage_1*(BAUD_RATE*2./1600000.) + freq_est_stage_2* (BAUD_RATE * 2. / 1600000.);
	//signal_freq_shift("out_mod_interp_x64_390500.pcm", "out_mod_rcv_x64.pcm", -freq_est_sum);
	//
	//signal_decimate("out_mod_rcv_x64.pcm", "out_mod_rcv_x1.pcm", 64);

	//Demodulator dmd("out_mod_halfband_25_down.pcm", "out_mod_dmd.pcm", "out_mod.bin", FRAME_DATA_SIZE);
	//dmd.process();

	destroy_xip_multiplier();
	destroy_xip_cordic_sqrt();
	destroy_xip_cordic_rect_to_polar();
	destroy_channel_matched_fir();

	return 0;
}