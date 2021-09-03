#ifndef DEMODULATOR_H
#define DEMODULATOR_H

#include <cstdio>
#include <cstring>
#include "fx_point.h"
#include "fx_cmpl_point.h"
#include "func_templates.h"
#include "SignalSource.h"
#include "ChannelMatchedFir.h"

using namespace std;
using namespace xilinx_m;

class Demodulator
{
public:
	/**
	 * @brief ��������� ���������� ������������
	 * @param input_file ������� PCM-���� (16-��� ������ I/Q)
	 * @param output_file �������� �������� ����
	 * @param data_length ������ ������ � ����� (����)
	*/
	Demodulator(const string& input_file, const string& output_file, size_t data_length);

	~Demodulator();
};

#endif // DEMODULATOR_H