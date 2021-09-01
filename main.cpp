#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "SignalSource.h"
#include "xip_utils.h"
#include "debug.h"
#include "ChannelMatchedFir.h"
#include "LagrangeInterp.h"
#include "FirMultiplier.h"
#include "FirSummator.h"
#include "Pif.h"
#include "autoganecontrol.h"
#include "constellation.h"
#include "StsEstimate.h"
#include "Modulator.h"
#include "fx_point.h"

// Для тестов библиотек XIP
#include "XilinxIpTests.h"

using namespace std;

int main()
{
	test_xip_dds_bitacc_cmodel();
	return 0;

	// инициализация xip fir
	init_channel_matched_fir();

	// вид сигнала
	set_current_constell(Current_constell::PSK4);

	// настройка и запуск модулятора
	Modulator mdl("data.bin", "out_mod.pcm", 1115);
	mdl.process();

	destroy_channel_matched_fir();
	return 0;
	

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