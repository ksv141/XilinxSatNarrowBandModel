#ifndef MODULATOR_H
#define MODULATOR_H

#include <cstdio>
#include <cstring>
#include "fx_point.h"
#include "fx_cmpl_point.h"
#include "func_templates.h"
#include "SignalSource.h"
#include "ChannelMatchedFir.h"

using namespace std;
using namespace xilinx_m;

/**
 * @brief модулятор
 * включает кодер битов в символы сигнального созвездия,
 * формирователь кадров,
 * B --> 2B,
 * канальный фильтр (корень приподнятого косинуса)
*/
class Modulator
{
public:
	/**
	 * @brief Установка параметров модулятора
	 * @param input_file входной бинарный файл
	 * @param output_file выходной PCM-файл (16-бит стерео I/Q)
	 * @param data_length размер данных в кадре (байт)
	*/
	Modulator(const string& input_file, const string& output_file, size_t data_length);

	~Modulator();

	/**
	 * @brief Запуск модулятора
	*/
	void process();

private:
	SignalSource m_signalSource;
	FILE* m_outFile;
};

#endif // MODULATOR_H