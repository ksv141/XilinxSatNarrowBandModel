#include <iostream>
#include <fstream>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "SignalSource.h"
#include "utils.h"
#include "debug.h"
#include "ChannelMatchedFir.h"
#include "LagrangeInterp.h"

// Для тестов библиотек XIP
#include "XilinxIpTests.h"

using namespace std;

int main()
{
	// тесты библиотек XIP
	//test_cmpy_v6_0_bitacc_cmodel();
	//test_xip_fir_bitacc_cmodel();

	init_lagrange_interp();
	destroy_lagrange_interp();
	return 0;

	// инициализация всех блоков
	init_channel_matched_fir();

	// количество генерируемых символов
	int symbol_count = 10000;

	// предварительная генерация символов в файл для тестирования
	//SignalSource gen_to_file;
	//gen_to_file.generateSamplesFile(symbol_count, "input_data.txt");
	//return 0;   

	// отладочный файл
	ofstream dbg_out("dbg_out.txt");

	// источник сигнала
	SignalSource signal_source("input_data.txt", 20);

	// основной цикл обработки символов
	int sample_count = symbol_count * 2;
	for (int i = 0; i < sample_count; i++)
	{
		xip_complex current_sample;
		// генерация отсчетов на 2B
		if (i % 2)
			// вставка 0 для увеличения Fd до 2B
			current_sample = xip_complex{ 0, 0 };
		else
			// генерация очередного отсчета (с учетом кадра и преамбулы)
			current_sample = signal_source.nextSampleFromFile();

		// канальный фильтр на 2B
		xip_complex sample_filtered;
		process_sample_channel_matched_transmit(&current_sample, &sample_filtered);

		// согласованный фильтр на 2B
		xip_complex sample_matched_filtered;
		process_sample_channel_matched_receive(&sample_filtered, &sample_matched_filtered);

//		dbg_out << sample_filtered << endl;
		dbg_out << sample_matched_filtered << endl;
		//cout << current_sample << endl;
	}

	// деинициализация
	destroy_channel_matched_fir();

	dbg_out.close();
	return 0;
}