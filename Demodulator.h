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
#include "constellation.h"
#include "StsEstimate.h"
#include "Pif.h"
#include "LagrangeInterp.h"
#include "xip_utils.h"

using namespace std;
using namespace xilinx_m;

extern const int AGC_WND_SIZE;

class Demodulator
{
public:
	/**
	 * @brief Установка параметров демодулятора
	 * @param input_file входной PCM-файл (16-бит стерео I/Q)
	 * @output_dmd_file выходной PCM-файл на входе блока принятия решения
	 * @param output_bin_file выходной бинарный файл с принятыми данными
	 * @param data_length размер данных в кадре (байт)
	*/
	Demodulator(const string& input_file, const string& output_dmd_file, const string& output_bin_file, size_t data_length);

	~Demodulator();

	/**
	 * @brief Запуск модулятора
	*/
	void process();

private:
	FILE* m_inFile;
	FILE* m_outDmdFile;
	FILE* m_outBinFile;

	StsEstimate m_stsEst;
	Pif pif_sts;
	LagrangeInterp dmd_interp;
};

#endif // DEMODULATOR_H