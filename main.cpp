#include <iostream>
#include <fstream>
#include <cmath>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "SignalSource.h"
#include "xip_utils.h"
#include "debug.h"
#include "ChannelMatchedFir.h"
#include "LagrangeInterp.h"

// ��� ������ ��������� XIP
#include "XilinxIpTests.h"

#define M_PI		3.14159265358979323846

using namespace std;

int main()
{
	// ����� ��������� XIP
	//test_cmpy_v6_0_bitacc_cmodel();
	//test_xip_fir_bitacc_cmodel();

	// ���� ������������� ��������
	//ofstream dbg_out_sin("sin.txt");
	//ofstream dbg_out_sin_int("sin_interp.txt");

	//init_lagrange_interp();
	//int n = 30;
	//double step = M_PI * 2 / n;

	//for (int i = 0; i < n; i++)
	//{
	//	xip_complex current_sample{ sin(i*step), 0 };
	//	xip_complex sample_filtered;
	//	process_sample_lagrange_interp(&current_sample, &sample_filtered, 1023);
	//	dbg_out_sin << current_sample.re << endl;
	//	if (i > 2)
	//		dbg_out_sin_int << sample_filtered.re << endl;
	//}

	//destroy_lagrange_interp();
	//dbg_out_sin.close();
	//dbg_out_sin_int.close();


	return 0;

	// ������������� ���� ������
	init_channel_matched_fir();

	// ���������� ������������ ��������
	int symbol_count = 10000;

	// ��������������� ��������� �������� � ���� ��� ������������
	//SignalSource gen_to_file;
	//gen_to_file.generateSamplesFile(symbol_count, "input_data.txt");
	//return 0;   

	ofstream dbg_out("dbg_out.txt");
	// �������� �������
	SignalSource signal_source("input_data.txt", 20);

	// �������� ���� ��������� ��������
	int sample_count = symbol_count * 2;
	for (int i = 0; i < sample_count; i++)
	{
		xip_complex current_sample;
		// ��������� �������� �� 2B
		if (i % 2)
			// ������� 0 ��� ���������� Fd �� 2B
			current_sample = xip_complex{ 0, 0 };
		else
			// ��������� ���������� ������� (� ������ ����� � ���������)
			current_sample = signal_source.nextSampleFromFile();

		// ��������� ������ �� 2B
		xip_complex sample_filtered;
		process_sample_channel_matched_transmit(&current_sample, &sample_filtered);

		// ������������� ������ �� 2B
		xip_complex sample_matched_filtered;
		process_sample_channel_matched_receive(&sample_filtered, &sample_matched_filtered);

//		dbg_out << sample_filtered << endl;
		dbg_out << sample_matched_filtered << endl;
		//cout << current_sample << endl;
	}

	// ���������������
	destroy_channel_matched_fir();

	dbg_out.close();
	return 0;
}