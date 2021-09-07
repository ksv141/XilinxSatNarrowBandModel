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
extern const double PIF_STS_Kp;		// ����������� ���������������� ������������ ��� ���
extern const double PIF_STS_Ki;		// ����������� ������������ ������������ ��� ���
extern const double PIF_PLL_Kp;		// ����������� ���������������� ������������ ��� ����
extern const double PIF_PLL_Ki;		// ����������� ������������ ������������ ��� ����
extern const int DDS_PHASE_MODULUS; // �������� ��������� ���� [0, 16383] --> [0, 2pi]. ��� ���� � ����� �������
extern const int DDS_RAD_CONST;		// ������ �� ���� ������� ���� << 3 == 20860 (16 ���)

class Demodulator
{
public:
	/**
	 * @brief ��������� ���������� ������������
	 * @param input_file ������� PCM-���� (16-��� ������ I/Q)
	 * @output_dmd_file �������� PCM-���� �� ����� ����� �������� �������
	 * @param output_bin_file �������� �������� ���� � ��������� �������
	 * @param data_length ������ ������ � ����� (����)
	*/
	Demodulator(const string& input_file, const string& output_dmd_file, const string& output_bin_file, size_t data_length);

	~Demodulator();

	/**
	 * @brief ������ ����������
	*/
	void process();

private:
	FILE* m_inFile;
	FILE* m_outDmdFile;
	FILE* m_outBinFile;

	StsEstimate m_stsEst;		// ���� ������ ������ �������� ������
	Pif pif_sts;				// ��� ���
	Pif pif_pll;				// ��� ����
	LagrangeInterp dmd_interp;	// ������������ ���
	AutoGaneControl m_agc;		// ���
	DDS dds;					// ��� ����� ����
};

#endif // DEMODULATOR_H