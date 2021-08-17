#ifndef SIGNALSOURCE_H
#define SIGNALSOURCE_H

#include <cstdint>
#include <string>
#include <fstream>
#include <stdexcept>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "mls.h"
#include "constellation.h"

using namespace std;

/**
* Источник сомплексного сигнала. Символы генерируются либо из ПСП, либо из ранее созданного файла.
* Сигнал формируется в виде кадров с преамбулой 32 бита
*/

class SignalSource
{
public:
	// конструктор для генерации из ПСП
	SignalSource(size_t data_length);

	// конструктор для генерации из файла
	SignalSource(string input_file, size_t data_length);

	SignalSource();

	~SignalSource();

	// генерация следующего псевдослучайного отсчета
	xip_complex nextSample();

	// генерация следующего псевдослучайного отсчета из файла
	xip_complex nextSampleFromFile();

	// генерация ПСП-отсчетов и запись в файл
	void generateSamplesFile(size_t count, string file_name);

private:
	size_t dataLength = 0;		// размер блока данных в кадре
	size_t frameCounter = 0;	// счетчик символов в кадре

	static const size_t preambleLength = 32;	// размер преамбулы
	static const int8_t preambleData[preambleLength]; // преамбула

	ifstream inFile;			// входной файл с символами

	// псевдослучайный источник символов
	mls symbolSource{ /* 2^32 - длина M-последовательности */ 32, /* 4-позиционный источник */ 4 };

	xip_complex __nextSample(bool from_file);
};

#endif // SIGNALSOURCE_H