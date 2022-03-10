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
													// выбор этой величины будет влиять на коэффициент масштабирования 
													// оценки тактового смещения в классе PhaseTimingCorrelator

const int DDS_RAD_CONST = (int)(DDS_PHASE_MODULUS * 8 / _2_PI);	// радиан на одну позицию фазы << 3 == 20860 (16 бит)
const uint16_t FRAME_DATA_SIZE = 1115;					// размер данных в кадре (байт)
const int AGC_WND_SIZE_LOG2 = 5;					// log2 окна усреднения АРУ

const double PIF_STS_Kp = 0.026311636311692643;		// коэффициент пропорциональной составляющей ПИФ СТС (при specific_locking_band = 0.01)
const double PIF_STS_Ki = 0.00035088206622480023;	// коэффициент интегральной составляющей ПИФ СТС (при specific_locking_band = 0.01)
const double PIF_PLL_Kp = 0.026311636311692643;		// коэффициент пропорциональной составляющей ПИФ ФАПЧ (при specific_locking_band = 0.01)
const double PIF_PLL_Ki = 0.00035088206622480023;	// коэффициент интегральной составляющей ПИФ ФАПЧ (при specific_locking_band = 0.01)
const double PIF_DOPL_Kp = 0.026311636311692643;	// коэффициент пропорциональной составляющей ПИФ Допл (при specific_locking_band = 0.01)
const double PIF_DOPL_Ki = 0.00035088206622480023;	// коэффициент интегральной составляющей ПИФ Допл (при specific_locking_band = 0.01)

const int BAUD_RATE = 9143;							// бодовая скорость в канале
const int INIT_SAMPLE_RATE = 2 * BAUD_RATE;			// начальная частота дискретизации на выходе канального фильтра
const int HIGH_SAMPLE_RATE = 1600000;				// частота дискретизации на входе демодулятора

// параметры корреляторов
const uint32_t DPDI_BURST_ML_SATGE_1 = 3000;			// порог обнаружения сигнала коррелятора первой стадии (грубая оценка частоты)
const uint32_t DPDI_BURST_ML_SATGE_2 = 3500;			// порог обнаружения сигнала коррелятора второй стадии (точная оценка частоты)
const uint32_t PHASE_BURST_ML_SATGE_3 = 5000;			// порог обнаружения сигнала коррелятора третьей стадии (оценка фазы и тактов)

int main()
{
	set_current_constell(Current_constell::PSK4);

	init_xip_multiplier();
	init_xip_cordic_sqrt();
	init_xip_cordic_rect_to_polar();
	init_channel_matched_fir();

	//int16_t x = -9;
	//if (x & 1)
	//	cout << 1 << endl;
	//x >>= 1;
	//cout << x << endl;

	//************ Формирователь ПСП в виде бинарного файла *************
	//SignalSource::generateBinFile(30000, "data_30.bin");

	//************ Модулятор *****************
	// Формирует кадры с добавлением преамбулы
	//Modulator mdl("data.bin", "out_mod.pcm", FRAME_DATA_SIZE);
	//Modulator mdl("1.zip", "out_mod.pcm", FRAME_DATA_SIZE);
	//mdl.process();
	
	//************ Фазовые и тактовые искажения ****************
	//signal_freq_phase_shift("out_mod.pcm", "out_mod_ph.pcm", 0, 1000);
	//signal_time_shift("out_mod_ph.pcm", "out_mod_tm.pcm", 500);

	//************ Передискретизация до 1600 кГц ***************
	//signal_resample("out_mod.pcm", "out_mod_25.pcm", INIT_SAMPLE_RATE, 25000);
	//signal_interpolate("out_mod_25.pcm", "out_mod_interp_x64.pcm", 64);
	//signal_interpolate("out_mod_25.pcm", "out_mod_interp_x4.pcm", 4);
	//signal_interpolate("out_mod.pcm", "out_mod_interp_x4.pcm", 4);

	//************ Моделирование произвольного смещения несущей в пределах полосы первого обнаружителя
	// Всего 4 обнаружителя с полосой 400 кГц каждый для перекрытия полосы 1600 кГц
	// Все обнаружители работают одинаково
	//signal_freq_shift("out_mod_interp_x64.pcm", "out_mod_interp_x64_shifted.pcm", 383500, 1600000);
	//signal_freq_shift("out_mod_interp_x64.pcm", "out_mod_interp_x64_383500.pcm", 383500, 1600000);

	//************ Моделирование доплеровского частотного смещения ***********
	//signal_freq_shift_dopl("out_mod_interp_x64_shifted.pcm", "out_mod_interp_x64_dopl.pcm", 1600000, 600000, 280);
	//signal_freq_shift_dopl("out_mod_interp_x64_shifted.pcm", "out_mod_interp_x64_dopl.pcm", 1600000, 600000, 50000);
	//signal_freq_shift_dopl("out_mod.pcm", "out_mod_dopl.pcm", INIT_SAMPLE_RATE, 6000, 10, 0);
	//signal_freq_shift_dopl("out_mod_25.pcm", "out_mod_dopl.pcm", 25000, 10000, 20, 0);
	//signal_freq_shift_dopl("out_mod_interp_x4.pcm", "out_mod_dopl.pcm", 100000, 30000, 50, 0);


	//************ Обнаружение сигнала и демодуляция
	//signal_estimate_demodulate("out_mod_decim_x16.pcm", "out_mod_dmd_1B.pcm");
	//signal_estimate_demodulate("out_mod_interp_x64_383500.pcm", "out_mod_dmd_1B.pcm");
	//signal_estimate_demodulate("out_mod_interp_x64_shifted.pcm", "out_mod_dmd_1B.pcm");
	//signal_estimate_demodulate("out_mod_interp_x64_dopl.pcm", "out_mod_dmd_1B.pcm");

	//signal_estimate_demodulate_dopl_test("out_mod_interp_x4.pcm", "out_mod_dmd_1B.pcm");
	signal_estimate_demodulate_dopl_test("out_mod.pcm", "out_mod_dmd_1B.pcm");


	destroy_xip_multiplier();
	destroy_xip_cordic_sqrt();
	destroy_xip_cordic_rect_to_polar();
	destroy_channel_matched_fir();

	return 0;
}