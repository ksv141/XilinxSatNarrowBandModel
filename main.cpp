#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "SignalSource.h"
#include "xip_utils.h"
#include "debug.h"
#include "ChannelMatchedFir.h"
#include "LagrangeInterp.h"
#include "FirMultiplier.h"
#include "FirSummator.h"
#include "Pif.h"
#include "autoganecontrol.h"
#include "constellation.h"
#include "StsEstimate.h"

// ��� ������ ��������� XIP
//#include "XilinxIpTests.h"

#define M_PI		3.14159265358979323846

using namespace std;

int main()
{
	// ����� ��������� XIP
	//test_cmpy_v6_0_bitacc_cmodel();
	//test_xip_fir_bitacc_cmodel();

	// ���� �������� ������� �������
	// init_fir_real_summator();
	// destroy_fir_real_summator();

	// ���� ������������� ��������
	//ofstream dbg_out_sin("sin.txt");
	//ofstream dbg_out_sin_int("sin_interp.txt");

	//LagrangeInterp test_interp(1);
	//int n = 30;
	//double step = M_PI * 2 / n;

	//for (int i = 0; i < n; i++)
	//{
	//	xip_complex current_sample{ sin(i*step), 0 };
	//	xip_complex sample_filtered;
	//	test_interp.process(current_sample, sample_filtered, 0.1);
	//	dbg_out_sin << current_sample.re << endl;
	//	if (i > 2)
	//		dbg_out_sin_int << sample_filtered.re << endl;
	//}

	//dbg_out_sin.close();
	//dbg_out_sin_int.close();

	//SignalSource::generateBinFile(3000, "data.bin");

	//return 0;

	// ������������� ���� ������
	init_channel_matched_fir();
	init_fir_real_multiplier();
	init_fir_real_summator();

	// ���������� ������������ ��������
	int symbol_count = 10000;

	// ��������������� ��������� �������� � ���� ��� ������������
	//SignalSource gen_to_file;
	//gen_to_file.generateSamplesFile(symbol_count, "input_data.txt");
	//return 0;   

	ofstream dbg_out("dbg_out.txt");
	// �������� �������
	SignalSource signal_source("input_data.txt", true, 20);

	// ������������ ��� �������� ��������� ������ � ������
	LagrangeInterp chan_interp(1);

	// ������������ ��� ���
	LagrangeInterp dmd_interp(1);

	// ��� ��� �������� �������, ��� � ����
	int agc_wnd = 128;
	AutoGaneControl agc(agc_wnd, pwr_constell_psk4);

	Pif pif_sts(0.01);

	// ���� ������ ������ �������� ������
	StsEstimate sts_est;

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
		process_sample_channel_matched_transmit(&current_sample, &current_sample);

		// �������� �������� ������ � ������
		chan_interp.process(current_sample, current_sample, 0.2);

		dmd_interp.process(current_sample);

		dmd_interp.next(current_sample);

		// ������������� ������ �� 2B
		process_sample_channel_matched_receive(&current_sample, &current_sample);

		agc.process(current_sample, current_sample);
		if (i / 2 < agc_wnd)
			continue;

		if (i % 2 == 0)
			continue;

		xip_complex est = nearest_point_psk4(current_sample);	// ������� �������
		xip_real sts_err = sts_est.getErr(current_sample, est);	// ������ ������ �������� ������
		pif_sts.process(sts_err, sts_err);
		dmd_interp.setShift(sts_err);
		//dbg_out << sts_err << endl;
		dbg_out << current_sample << endl;
	}

	// ���������������
	destroy_channel_matched_fir();
	destroy_fir_real_multiplier();
	destroy_fir_real_summator();

	dbg_out.close();
	return 0;
}