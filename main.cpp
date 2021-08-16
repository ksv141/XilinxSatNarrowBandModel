#include <iostream>
#include <fstream>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "SignalSource.h"
#include "utils.h"

using namespace std;

int main()
{
	// отладочный файл
	ofstream dbg_out("dbg_out.txt");

	// источник сигнала
	SignalSource signal_source(20);

	// количество генерируемых символов
	int symbol_count = 1000;

	// основной цикл обработки символов
	for (int i = 0; i < symbol_count; i++)
	{
		// генерация очередного символа
		xip_complex current_sample = signal_source.nextSample();
		dbg_out << current_sample << endl;
		//cout << current_sample << endl;
	}

	dbg_out.close();
	return 0;
}