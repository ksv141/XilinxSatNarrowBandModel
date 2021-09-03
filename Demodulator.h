#ifndef DEMODULATOR_H
#define DEMODULATOR_H

#include <cstdio>
#include <cstring>
#include "fx_point.h"
#include "fx_cmpl_point.h"
#include "func_templates.h"
#include "SignalSource.h"
#include "ChannelMatchedFir.h"
#include "xip_utils.h"

using namespace std;
using namespace xilinx_m;

class Demodulator
{
public:
	/**
	 * @brief Установка параметров демодулятора
	 * @param input_file входной PCM-файл (16-бит стерео I/Q)
	 * @param output_file выходной бинарный файл
	 * @param data_length размер данных в кадре (байт)
	*/
	Demodulator(const string& input_file, const string& output_file, size_t data_length);

	~Demodulator();

	/**
	 * @brief Запуск модулятора
	*/
	void process();

private:
	FILE* m_inFile;
	FILE* m_outFile;
};

#endif // DEMODULATOR_H