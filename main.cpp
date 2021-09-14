#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <thread>

#include "all_headers.h"

// ��� ������ ��������� XIP
//#include "XilinxIpTests.h"

using namespace std;

static const double PI = 3.141592653589793238463;
static const double _2_PI = PI * 2;

/**
* �������� � ������ (���/�):
* 9143
* 16000
* 24000
* 18286
* 48000
* 36570
* 96000
* 85710
* 171430
* 342860
* 685710
*/

// ���������� ���������
const int DDS_PHASE_MODULUS = 16384;				// �������� ��������� ���� [0, 16383] --> [0, 2pi]. ��� ���� � ����� �������
const int DDS_RAD_CONST = (int)(DDS_PHASE_MODULUS * 8 / _2_PI);	// ������ �� ���� ������� ���� << 3 == 20860 (16 ���)
const uint16_t FRAME_DATA_SIZE = 1115;					// ������ ������ � ����� (����)
const int AGC_WND_SIZE_LOG2 = 7;					// log2 ���� ���������� ���

const double PIF_STS_Kp = 0.026311636311692643;		// ����������� ���������������� ������������ ��� ��� (��� specific_locking_band = 0.01)
const double PIF_STS_Ki = 0.00035088206622480023;	// ����������� ������������ ������������ ��� ��� (��� specific_locking_band = 0.01)
const double PIF_PLL_Kp = 0.026311636311692643;		// ����������� ���������������� ������������ ��� ���� (��� specific_locking_band = 0.01)
const double PIF_PLL_Ki = 0.00035088206622480023;	// ����������� ������������ ������������ ��� ���� (��� specific_locking_band = 0.01)

const int BAUD_RATE = 9143;							// ������� �������� � ������
const int INIT_SAMPLE_RATE = 2 * BAUD_RATE;			// ��������� ������� ������������� �� ������ ���������� �������

// ��������� ������������
uint32_t DPDI_BURST_ML_SATGE_1 = 125;				// ����� ����������� ������� ����������� ������ ������ (������ ������ �������)
uint32_t DPDI_BURST_ML_SATGE_2 = 32;				// ����� ����������� ������� ����������� ������ ������ (������ ������ �������)

int main()
{
	set_current_constell(Current_constell::PSK4);


	// test DPDI
	CorrelatorDPDI corr_stage_1(FRAME_DATA_SIZE, (int8_t*)SignalSource::preambleData, SignalSource::preambleLength, 
								1, 32, 1, DPDI_BURST_ML_SATGE_1);
	return 0;

	init_xip_multiplier();
	init_xip_cordic_sqrt();
	init_channel_matched_fir();

	//Modulator mdl("data.bin", "out_mod.pcm", FRAME_DATA_SIZE);
	//Modulator mdl("1.zip", "out_mod.pcm", FRAME_DATA_SIZE);
	//mdl.process();

	//signal_time_shift("out_mod_fr_shift.pcm", "out_mod_tm_shift.pcm", 128);
	//signal_time_shift_dyn("out_mod_fr_shift.pcm", "out_mod_tm_shift.pcm", 10);
	double resample_coeff = 1.01;
	//signal_resample("out_mod.pcm", "out_mod_rsmpl.pcm", INIT_SAMPLE_RATE, 1600000);
	//signal_resample("out_mod.pcm", "out_mod_rsmpl.pcm", INIT_SAMPLE_RATE, INIT_SAMPLE_RATE* resample_coeff);
	//signal_freq_shift("out_mod_1600_shift_600.pcm", "out_mod_1600_downshift_600.pcm", -6144);
	//signal_lowpass("out_mod_1600_downshift_600.pcm", "out_mod_1600_downshift_600_lowpass.pcm", "lowpass_200kHz.fcf", 84);
	//signal_decimate("out_mod_1600_downshift_600_lowpass.pcm", "out_mod_400kHz.pcm", 4);
	Demodulator dmd("out_mod_400kHz.pcm", "out_mod_dmd.pcm", "out_mod.bin", FRAME_DATA_SIZE);
	dmd.process();

	destroy_xip_multiplier();
	destroy_xip_cordic_sqrt();
	destroy_channel_matched_fir();

	return 0;

	//ofstream dbg_out("dbg_out.txt");
	//DDS dds(DDS_PHASE_MODULUS);
	//double dds_phase, dds_sin, dds_cos;
	//for (int i = 0; i < 17000; i++) {
	//	double dph = 1.0;
	//	dds.process(dph, dds_phase, dds_sin, dds_cos);
	//	dbg_out << "ph = " << dds_phase << "\tsin = " << dds_sin << "\tcos = " << dds_cos << endl;
	//}

	//dbg_out.close();
	//return 0;

	//// ������������� xip fir
	//init_channel_matched_fir();

	//// ��� �������
	//set_current_constell(Current_constell::PSK4);

	//// ��������� � ������ ����������
	//Modulator mdl("data.bin", "out_mod.pcm", FRAME_DATA_SIZE);
	//mdl.process();

	//destroy_channel_matched_fir();
	//return 0;
	

	//ofstream dbg_out("dbg_out.txt");
	//// �������� �������
	//SignalSource signal_source("data.bin", true, 100);

	//xip_complex sample;
	//while (signal_source.nextSampleFromFile(sample)) {
	//	dbg_out << sample << endl;
	//}
	//return 0;

	//// ������������ ��� �������� ��������� ������ � ������
	//LagrangeInterp chan_interp(1);

	//// ������������ ��� ���
	//LagrangeInterp dmd_interp(1);

	//// ��� ��� �������� �������, ��� � ����
	//int agc_wnd = 128;
	//AutoGaneControl agc(agc_wnd, pwr_constell_psk4);

	//Pif pif_sts(0.01);

	//// ���� ������ ������ �������� ������
	//StsEstimate sts_est;

	//// �������� ���� ��������� ��������
	//int sample_count = symbol_count * 2;
	//for (int i = 0; i < sample_count; i++)
	//{
	//	xip_complex current_sample;
	//	// ��������� �������� �� 2B
	//	if (i % 2)
	//		// ������� 0 ��� ���������� Fd �� 2B
	//		current_sample = xip_complex{ 0, 0 };
	//	else
	//		// ��������� ���������� ������� (� ������ ����� � ���������)
	//		signal_source.nextSampleFromFile(current_sample);

	//	// ��������� ������ �� 2B
	//	process_sample_channel_matched_transmit(&current_sample, &current_sample);

	//	// �������� �������� ������ � ������
	//	chan_interp.process(current_sample, current_sample, 0.2);

	//	dmd_interp.process(current_sample);

	//	dmd_interp.next(current_sample);

	//	// ������������� ������ �� 2B
	//	process_sample_channel_matched_receive(&current_sample, &current_sample);

	//	agc.process(current_sample, current_sample);
	//	if (i / 2 < agc_wnd)
	//		continue;

	//	if (i % 2 == 0)
	//		continue;

	//	xip_complex est = nearest_point_psk4(current_sample);	// ������� �������
	//	xip_real sts_err = sts_est.getErr(current_sample, est);	// ������ ������ �������� ������
	//	pif_sts.process(sts_err, sts_err);
	//	dmd_interp.setShift(sts_err);
	//	//dbg_out << sts_err << endl;
	//	dbg_out << current_sample << endl;
	//}

	//// ���������������
	//destroy_channel_matched_fir();
	//destroy_fir_real_multiplier();
	//destroy_fir_real_summator();

	//dbg_out.close();
	//return 0;
}