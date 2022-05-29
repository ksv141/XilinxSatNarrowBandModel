#ifndef MODULATOR_H
#define MODULATOR_H

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

/**
 * @brief ���������
 * �������� ����� ����� � ������� ����������� ���������,
 * ������������� ������,
 * B --> 2B,
 * ��������� ������ (������ ������������ ��������)
*/
class Modulator
{
public:
	/**
	 * @brief ��������� ���������� ����������
	 * @param input_file ������� �������� ����
	 * @param output_file �������� PCM-���� (16-��� ������ I/Q)
	 * @param ������� ��������� ��������� � ����
	 * @param ������� ��������� ���������� � ����
	*/
	Modulator(const string& input_file, const string& output_file, bool has_preamble, bool has_postamble);

	~Modulator();

	/**
	 * @brief ������ ����������
	*/
	void process();

private:
	SignalSource m_signalSource;
	FILE* m_outFile;
};

#endif // MODULATOR_H