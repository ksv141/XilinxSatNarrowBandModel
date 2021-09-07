#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>

#include "all_headers.h"

// Для тестов библиотек XIP
//#include "XilinxIpTests.h"

using namespace std;

// глобальные параметры
const int DDS_PHASE_MODULUS = 16384;	// диапазон изменения фазы [0, 16383] --> [0, 2pi]. Для ФАПЧ и петли Доплера
const int FRAME_DATA_SIZE = 1115;		// размер данных в кадре (байт)
const int AGC_WND_SIZE = 128;			// окно усреднения АРУ
const double PIF_STS_Kp = 0.026311636311692643;		// коэффициент пропорциональной составляющей ПИФ СТС (при specific_locking_band = 0.01)
const double PIF_STS_Ki = 0.00035088206622480023;	// коэффициент интегральной составляющей ПИФ СТС (при specific_locking_band = 0.01)

int main()
{
	//Pif pif(0.1);
	//ofstream dbg_out("dbg_out.txt");
	//for (int i = 0; i < 100; i++) {
	//	xip_real x = 32;
	//	pif.process_1(x, x);
	//	dbg_out << x << endl;
	//}
	//dbg_out.close();
	//return 0;
	//ofstream dbg_out("dbg_out.txt");
	//LagrangeInterp itrp;
	//for (int i = -5000; i <= 5000; i++)
	//	dbg_out << i << "\t" << itrp.countPos(i) << endl;
	//dbg_out.close();
	//return 0;
	// 
//	generate_sin_signal("sin.pcm", 1, 20, 40, 13);
//	signal_time_shift("sin.pcm", "sin_shift.pcm", 256);
//	signal_time_shift("sin_shift.pcm", "sin_unshift.pcm", -256);
//	return 0;

	set_current_constell(Current_constell::PSK4);
	init_xip_multiplier();
	init_channel_matched_fir();
//	signal_freq_shift("out_mod.pcm", "out_mod_shift.pcm", 128);
	//Modulator mdl("data.bin", "out_mod.pcm", FRAME_DATA_SIZE);
	//mdl.process();
	signal_time_shift("out_mod.pcm", "out_mod_shift.pcm", 128);
	Demodulator dmd("out_mod_shift.pcm", "out_mod_dmd.pcm", "out_mod.bin", FRAME_DATA_SIZE);
	dmd.process();

	destroy_xip_multiplier();
	destroy_channel_matched_fir();

	return 0;

	//ofstream dbg_out("dbg_out.txt");
	//DDS dds(DDS_PHASE_MODULUS);
	//double dds_phase, dds_sin, dds_cos;
	//for (int i = 0; i < 17000; i++) {
	//	double dph = 1.0;
	//	dds.process(dph, dds_phase, dds_sin, dds_cos);
	//	dbg_out << "ph = " << dds_phase << "\tsin = " << dds_sin << "\tcos = " << dds_cos << endl;
	//}

	//dbg_out.close();
	//return 0;

	//// инициализация xip fir
	//init_channel_matched_fir();

	//// вид сигнала
	//set_current_constell(Current_constell::PSK4);

	//// настройка и запуск модулятора
	//Modulator mdl("data.bin", "out_mod.pcm", FRAME_DATA_SIZE);
	//mdl.process();

	//destroy_channel_matched_fir();
	//return 0;
	

	//ofstream dbg_out("dbg_out.txt");
	//// источник сигнала
	//SignalSource signal_source("data.bin", true, 100);

	//xip_complex sample;
	//while (signal_source.nextSampleFromFile(sample)) {
	//	dbg_out << sample << endl;
	//}
	//return 0;

	//// интерполятор для имитации тактового сдвига в канале
	//LagrangeInterp chan_interp(1);

	//// интерполятор для СТС
	//LagrangeInterp dmd_interp(1);

	//// АРУ для жесткого решения, СТС и ФАПЧ
	//int agc_wnd = 128;
	//AutoGaneControl agc(agc_wnd, pwr_constell_psk4);

	//Pif pif_sts(0.01);

	//// Блок оценки ошибки тактовой синхры
	//StsEstimate sts_est;

	//// основной цикл обработки символов
	//int sample_count = symbol_count * 2;
	//for (int i = 0; i < sample_count; i++)
	//{
	//	xip_complex current_sample;
	//	// генерация отсчетов на 2B
	//	if (i % 2)
	//		// вставка 0 для увеличения Fd до 2B
	//		current_sample = xip_complex{ 0, 0 };
	//	else
	//		// генерация очередного отсчета (с учетом кадра и преамбулы)
	//		signal_source.nextSampleFromFile(current_sample);

	//	// канальный фильтр на 2B
	//	process_sample_channel_matched_transmit(&current_sample, &current_sample);

	//	// имитация смещения тактов в канале
	//	chan_interp.process(current_sample, current_sample, 0.2);

	//	dmd_interp.process(current_sample);

	//	dmd_interp.next(current_sample);

	//	// согласованный фильтр на 2B
	//	process_sample_channel_matched_receive(&current_sample, &current_sample);

	//	agc.process(current_sample, current_sample);
	//	if (i / 2 < agc_wnd)
	//		continue;

	//	if (i % 2 == 0)
	//		continue;

	//	xip_complex est = nearest_point_psk4(current_sample);	// жесткое решение
	//	xip_real sts_err = sts_est.getErr(current_sample, est);	// оценка ошибки тактовой синхры
	//	pif_sts.process(sts_err, sts_err);
	//	dmd_interp.setShift(sts_err);
	//	//dbg_out << sts_err << endl;
	//	dbg_out << current_sample << endl;
	//}

	//// деинициализация
	//destroy_channel_matched_fir();
	//destroy_fir_real_multiplier();
	//destroy_fir_real_summator();

	//dbg_out.close();
	//return 0;
}