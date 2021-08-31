#ifndef SIGNALSOURCE_H
#define SIGNALSOURCE_H

#include <cstdint>
#include <string>
#include <fstream>
#include <stdexcept>
#include <bitset>

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
	/**
	 * @brief конструктор для генерации из ПСП (по-умолчанию 4-чный)
	 * @param data_length размер блока данных в кадре (байт)
	*/
	SignalSource(size_t data_length);

	/**
	 * @brief конструктор для генерации из файла (текстовый или бинарный)
	 * @param input_file файл
	 * @param is_binary признак бинарного файла
	 * @param data_length размер блока данных в кадре (байт)
	*/
	SignalSource(string input_file, bool is_binary, size_t data_length);

	~SignalSource();

	// генерация следующего псевдослучайного отсчета
	xip_complex nextSample();

	// генерация следующего псевдослучайного отсчета из файла
	bool nextSampleFromFile(xip_complex& out);

	/**
	 * @brief helper-функция. Генерация ПСП-символов и запись в текстовый файл
	 * @param n_pos число позиций (для сигнального созвездия)
	 * @param count чисор символов
	 * @param file_name текстовый файл
	*/
	static void generateSymbolFile(int n_pos, size_t count, string file_name);

	/**
	 * @brief helper-функция. Генерация 2-чной ПСП-последовательности и запись в бинарный файл
	 * @param byte_count число байтов
	 * @param file_name бинарный файл
	*/
	static void generateBinFile(size_t byte_count, string file_name);

private:
	size_t dataLength = 0;		// размер блока данных в кадре
	size_t frameCounter = 0;	// счетчик символов в кадре

	static const size_t preambleLength = 32;	// размер преамбулы
	static const int8_t preambleData[preambleLength]; // преамбула

	ifstream inFile;			// входной файл с символами
	bool binaryFile;			// признак бинарного файла
	char readByte;				// очередной байт из файла
	int bitShift;				// количество считываемых битов из бинарного файла
	unsigned int bitMask;
	int bitPos;

	// псевдослучайный источник символов
	mls symbolSource{ /* 2^32 - длина M-последовательности */ 32, /* 4-позиционный источник */ 4 };

	bool __nextSample(bool from_file, xip_complex& out);
};

#endif // SIGNALSOURCE_H