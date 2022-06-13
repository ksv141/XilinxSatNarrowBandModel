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

extern const uint16_t FRAME_DATA_SIZE;
extern const size_t PREAMBLE_LENGTH;
extern const int8_t PREAMBLE_DATA[];
extern const size_t POSTAMBLE_LENGTH;
extern const int8_t POSTAMBLE_DATA[];

/**
* Источник комплексного сигнала. Символы генерируются либо из ПСП, либо из ранее созданного файла.
* Сигнал формируется в виде кадров с преамбулой 32 бита
* Все данные кодируются Манчестерским кодом
*/

class SignalSource
{
public:
	/**
	 * @brief конструктор для генерации из ПСП (по-умолчанию 4-чный)
	 * @param признак включения преамбулы в кадр
	 * @param признак включения хвостовика в кадр
	*/
	SignalSource(bool has_preamble, bool has_postamble);

	/**
	 * @brief конструктор для генерации из файла (текстовый или бинарный)
	 * @param input_file файл
	 * @param is_binary признак бинарного файла
	 * @param признак включения преамбулы в кадр
	 * @param признак включения хвостовика в кадр
	*/
	SignalSource(const string& input_file, bool is_binary, bool has_preamble, bool has_postamble);

	~SignalSource();

	// генерация следующего псевдослучайного отсчета
	xip_complex nextSample();

	// генерация следующего отсчета для битов из файла
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
	bool hasPreamble;			// признак включения преамбулы в кадр
	bool hasPostamble;			// признак включения хвостовика в кадр
	size_t dataLength = 0;		// размер блока данных в кадре (в символах)
	size_t frameCounter = 0;	// счетчик символов в кадре
	unsigned dataPos;			// позиция начала данных
	unsigned postamblePos;		// позиция начала концевика
	unsigned endPos;			// позиция конца кадра

	ifstream inFile;			// входной файл с символами
	bool binaryFile;			// признак бинарного файла
	char readByte;				// очередной байт из файла
	int bitShift;				// количество считываемых битов из бинарного файла
	unsigned int bitMask;
	int bitPos;
	bool manchesterState = false;		// флаг состояния для генерации кода Манчестера
	unsigned lastManchesterSymbol;

	// псевдослучайный источник символов
	mls symbolSource{ /* 2^32 - длина M-последовательности */ 32, /* 4-позиционный источник */ 4 };

	bool __nextSample(bool from_file, xip_complex& out);

	bool __nextSampleManchester(bool from_file, xip_complex& out);

	bool __nextSymbol(bool from_file, unsigned& out);
	
	// пересчет длины данных из байтов в отсчеты
	void countDataLength(size_t bytes);
};

#endif // SIGNALSOURCE_H