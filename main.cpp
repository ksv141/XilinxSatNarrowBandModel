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
static const double PI_2 = PI / 2;
static const double PI_34 = PI + PI / 2;

// глобальные параметры
const int DDS_PHASE_MODULUS = 16384;				// диапазон изменения фазы [0, 16383] --> [0, 2pi]. Для ФАПЧ и петли Доплера
													// выбор этой величины будет влиять на коэффициент масштабирования 
													// оценки тактового смещения в классе PhaseTimingCorrelator

const int DDS_RAD_CONST = (int)(DDS_PHASE_MODULUS * 8 / _2_PI);	// радиан на одну позицию фазы << 3 == 20860 (16 бит)
const int AGC_WND_SIZE_LOG2 = 5;					// log2 окна усреднения АРУ

const double PIF_STS_Kp = 0.026311636311692643;		// коэффициент пропорциональной составляющей ПИФ СТС (при specific_locking_band = 0.01)
const double PIF_STS_Ki = 0.00035088206622480023;	// коэффициент интегральной составляющей ПИФ СТС (при specific_locking_band = 0.01)
const double PIF_PLL_Kp = 0.026311636311692643;		// коэффициент пропорциональной составляющей ПИФ ФАПЧ (при specific_locking_band = 0.01)
const double PIF_PLL_Ki = 0.00035088206622480023;	// коэффициент интегральной составляющей ПИФ ФАПЧ (при specific_locking_band = 0.01)
const double PIF_DOPL_Kp = 0.026311636311692643;	// коэффициент пропорциональной составляющей ПИФ Допл (при specific_locking_band = 0.01)
const double PIF_DOPL_Ki = 0.00035088206622480023;	// коэффициент интегральной составляющей ПИФ Допл (при specific_locking_band = 0.01)

//********* структура кадра
const size_t PREAMBLE_LENGTH = 32;					// размер преамбулы (бит)
//const size_t PREAMBLE_LENGTH = 16;

// преамбула 32 бита 0x1ACFFC1D
const int8_t PREAMBLE_DATA[PREAMBLE_LENGTH] = { 0,0,0,1,1,0,1,0,
												1,1,0,0,1,1,1,1,
												1,1,1,1,1,1,0,0,
												0,0,0,1,1,1,0,1 };
// преамбула 16 бит 0xEB90
//const int8_t PREAMBLE_DATA[PREAMBLE_LENGTH] = { 1,1,1,0,1,0,1,1,
//												  1,0,0,1,0,0,0,0 };
//const uint16_t FRAME_DATA_SIZE = 1115;				// размер данных в кадре (байт)
const uint16_t FRAME_DATA_SIZE = 100;				// размер данных в кадре (байт)

const size_t POSTAMBLE_LENGTH = 64;					// размер хвостовой последовательности (бит)
// хвостовая последовательность 64 бита 0x8153225B1D0D73DE
const int8_t POSTAMBLE_DATA[POSTAMBLE_LENGTH] = { 1,0,0,0,0,0,0,1,
												  0,1,0,1,0,0,1,1,
												  0,0,1,0,0,0,1,0,
												  0,1,0,1,1,0,1,1,
												  0,0,0,1,1,1,0,1,
												  0,0,0,0,1,1,0,1,
												  0,1,1,1,0,0,1,1,
												  1,1,0,1,1,1,1,0 };

const int BAUD_RATE = 8000;							// бодовая скорость в канале
/**
* Ряд скоростей (бод):
* 8000
* 16000
* 32000
* 64000
* 128000
* 256000
* 512000
*/

const int INIT_SAMPLE_RATE = 2 * BAUD_RATE;			// начальная частота дискретизации на выходе канального фильтра
const int HIGH_SAMPLE_RATE = 1600000;				// частота дискретизации на входе демодулятора

// параметры корреляторов
const uint32_t DPDI_BURST_ML_SATGE_1 = 3000;			// порог обнаружения сигнала коррелятора первой стадии (грубая оценка частоты)
const uint32_t DPDI_BURST_ML_SATGE_2 = 3500;			// порог обнаружения сигнала коррелятора второй стадии (точная оценка частоты)
const uint32_t PHASE_BURST_ML_SATGE_3 = 5000;			// порог обнаружения сигнала коррелятора третьей стадии (оценка фазы и тактов)

int main()
{
	//set_current_constell(Current_constell::PSK2_60);
	set_current_constell(Current_constell::PSK2);

	init_xip_multiplier();
	init_xip_cordic_sqrt();
	init_xip_cordic_rect_to_polar();
	init_channel_matched_fir();

	//************ Формирователь ПСП в виде бинарного файла *************
	//SignalSource::generateBinFile(10, "data_10.bin");

	//************ Модулятор *****************
	// Формирует кадры с добавлением преамбулы, без хвостовика (код Манчестера)
	//Modulator mdl("data.bin", "out_mod.pcm", true, false);		// точность [+/- 2^15]
	//Modulator mdl("data_10.bin", "out_mod.pcm", false, false);
	// Формирует кадры с добавлением преамбулы и хвостовика
	//Modulator mdl("data.bin", "out_mod.pcm", true, true);
	//mdl.process();

	//************ Фазовые и тактовые искажения ****************
	//signal_freq_phase_shift("out_mod.pcm", "out_mod_ph.pcm", 0, 1000);
	//signal_time_shift("out_mod_ph.pcm", "out_mod_tm.pcm", 512);

	signal_freq_shift("out_mod_tm.pcm", "out_mod_shifted.pcm", 300);

	// Тестирование коррелятора на манчестерском коде
	signal_lowpass("out_mod_shifted.pcm", "out_mod_matched.pcm", "rc_root_x2_25_19.fcf", 19); // точность [+/- 2^15]
	int16_t freq_est = 0;
	signal_freq_est_stage("out_mod_matched.pcm", 4, 8, DPDI_BURST_ML_SATGE_1, freq_est);
	cout << freq_est << endl;

	return 0;
	
	//************ Фазовые и тактовые искажения ****************
	//signal_freq_phase_shift("out_mod.pcm", "out_mod_ph.pcm", 0, 1000);
	//signal_time_shift("out_mod_ph.pcm", "out_mod_tm.pcm", 500);

	//************ Передискретизация до 1600 кГц ***************
	signal_resample("out_mod.pcm", "out_mod_25.pcm", INIT_SAMPLE_RATE, 25000);
	signal_interpolate("out_mod_25.pcm", "out_mod_interp_x64.pcm", 64);
	//signal_interpolate("out_mod_25.pcm", "out_mod_interp_x4.pcm", 4);
	//signal_interpolate("out_mod.pcm", "out_mod_interp_x4.pcm", 4);
	//signal_interpolate("out_mod_interp_x4.pcm", "out_mod_interp_x16.pcm", 4);
	//signal_interpolate("out_mod_interp_x16.pcm", "out_mod_interp_x64.pcm", 4);
	//signal_interpolate("out_mod.pcm", "out_mod_interp_x64.pcm", 64);

	//************ Моделирование произвольного смещения несущей в пределах полосы первого обнаружителя
	// Всего 4 обнаружителя с полосой 400 кГц каждый для перекрытия полосы 1600 кГц
	// Все обнаружители работают одинаково
	signal_freq_shift("out_mod_interp_x64.pcm", "out_mod_interp_x64_shifted.pcm", 383500, 1600000);

	//************ Моделирование доплеровского частотного смещения ***********
	signal_freq_shift_dopl("out_mod_interp_x64_shifted.pcm", "out_mod_interp_x64_dopl.pcm", 1600000, 600000, 280);
	//signal_freq_shift_dopl("out_mod_interp_x64_shifted.pcm", "out_mod_interp_x64_dopl.pcm", 1600000, 600000, 50000, 0);
	//signal_freq_shift_dopl("out_mod.pcm", "out_mod_dopl.pcm", INIT_SAMPLE_RATE, 6000, 10, 0);
	//signal_freq_shift_dopl("out_mod_25.pcm", "out_mod_dopl.pcm", 25000, 10000, 20, 0);
	//signal_freq_shift_dopl("out_mod_interp_x4.pcm", "out_mod_dopl.pcm", 100000, 30000, 50, 1);
	//signal_freq_shift_dopl("out_mod_interp_x16.pcm", "out_mod_dopl.pcm", 400000, 150000, 200, 1);
	//signal_freq_shift_dopl("out_mod_interp_x64.pcm", "out_mod_dopl.pcm", 1600000, 600000, 1000, 1);


	//************ Обнаружение сигнала и демодуляция
	signal_estimate_demodulate("out_mod_interp_x64_dopl.pcm", "out_mod_dmd_1B.pcm");
	//signal_estimate_demodulate("out_mod_interp_x64_383500.pcm", "out_mod_dmd_1B.pcm");
	//signal_estimate_demodulate("out_mod_interp_x64_shifted.pcm", "out_mod_dmd_1B.pcm");
	//signal_estimate_demodulate("out_mod_interp_x64_dopl.pcm", "out_mod_dmd_1B.pcm");

	//signal_estimate_demodulate_dopl_test("out_mod_dopl.pcm", "out_mod_dmd_1B.pcm");
	//signal_estimate_demodulate_dopl_test("out_mod_interp_x64.pcm", "out_mod_dmd_1B.pcm");


	destroy_xip_multiplier();
	destroy_xip_cordic_sqrt();
	destroy_xip_cordic_rect_to_polar();
	destroy_channel_matched_fir();

	return 0;
}