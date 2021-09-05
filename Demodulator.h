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

	StsEstimate m_stsEst;
	Pif pif_sts;
	LagrangeInterp dmd_interp;
};

#endif // DEMODULATOR_H