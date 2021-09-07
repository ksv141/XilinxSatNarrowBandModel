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
#include "autoganecontrol.h"
#include "DDS.h"

using namespace std;
using namespace xilinx_m;

extern const int AGC_WND_SIZE;
extern const double PIF_STS_Kp;		// коэффициент пропорциональной составляющей ПИФ СТС
extern const double PIF_STS_Ki;		// коэффициент интегральной составляющей ПИФ СТС
extern const double PIF_PLL_Kp;		// коэффициент пропорциональной составляющей ПИФ ФАПЧ
extern const double PIF_PLL_Ki;		// коэффициент интегральной составляющей ПИФ ФАПЧ
extern const int DDS_PHASE_MODULUS; // диапазон изменения фазы [0, 16383] --> [0, 2pi]. Для ФАПЧ и петли Доплера
extern const int DDS_RAD_CONST;		// радиан на одну позицию фазы << 3 == 20860 (16 бит)

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

	StsEstimate m_stsEst;		// блок оценки ошибки тактовой синхры
	Pif pif_sts;				// ПИФ СТС
	Pif pif_pll;				// ПИФ ФАПЧ
	LagrangeInterp dmd_interp;	// интерполятор СТС
	AutoGaneControl m_agc;		// АРУ
	DDS dds;					// ГУН петли ФАПЧ
};

#endif // DEMODULATOR_H