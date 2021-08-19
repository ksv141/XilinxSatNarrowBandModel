#include <iostream>
#include <fstream>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "SignalSource.h"
#include "utils.h"
#include "debug.h"
#include "ChannelMatchedFir.h"

// ��� ������ ��������� XIP
#include "XilinxIpTests.h"

using namespace std;

int main()
{
	// ����� ��������� XIP
	//test_cmpy_v6_0_bitacc_cmodel();
	//test_xip_fir_bitacc_cmodel();

	// ������������� ���� ������
	init_channel_matched_fir();
//	process_data_channel_matched_fir();

	// ���������� ������������ ��������
	int symbol_count = 10000;

	// ��������������� ��������� �������� � ���� ��� ������������
	//SignalSource gen_to_file;
	//gen_to_file.generateSamplesFile(symbol_count, "input_data.txt");
	//return 0;   

	// ���������� ����
	ofstream dbg_out("dbg_out.txt");

	// �������� �������
	SignalSource signal_source("input_data.txt", 20);

	// �������� ���� ��������� ��������
	int sample_count = symbol_count * 2;
	for (int i = 0; i < sample_count; i++)
	{
		xip_complex current_sample;
		if (i % 2)
			// ������� 0 ��� ���������� Fd �� 2B
			current_sample = xip_complex{ 0, 0 };
		else
			// ��������� ���������� ������� (� ������ ����� � ���������)
			current_sample = signal_source.nextSampleFromFile();

		xip_complex sample_filtered;
		process_sample_channel_matched_transmit(&current_sample, &sample_filtered);

		dbg_out << sample_filtered << endl;
		//cout << current_sample << endl;
	}

	// ���������������
	destroy_channel_matched_fir();

	dbg_out.close();
	return 0;
}