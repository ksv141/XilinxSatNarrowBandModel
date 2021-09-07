#include "Demodulator.h"

Demodulator::Demodulator(const string& input_file, const string& output_dmd_file, const string& output_bin_file, size_t data_length):
	m_agc(AGC_WND_SIZE, pwr_constell_psk4),
	pif_sts(PIF_STS_Kp, PIF_STS_Ki)
{
	m_inFile = fopen(input_file.c_str(), "rb");
	if (!m_inFile)
		throw runtime_error("input file is not opened");
	m_outDmdFile = fopen(output_dmd_file.c_str(), "wb");
	if (!m_outDmdFile)
		throw runtime_error("output file is not opened");
	m_outBinFile = fopen(output_bin_file.c_str(), "wb");
	if (!m_outBinFile)
		throw runtime_error("output file is not opened");
}

Demodulator::~Demodulator()
{
	if (m_inFile)
		fclose(m_inFile);
	if (m_outDmdFile)
		fclose(m_outDmdFile);
	if (m_outBinFile)
		fclose(m_outBinFile);
}

void Demodulator::process()
{
	if (!m_inFile)
		return;
	if (!m_outDmdFile)
		return;
	if (!m_outBinFile)
		return;

	ofstream dbg_out("dbg_out.txt");

	int16_t re;
	int16_t im;
	int i = 0;	// ������� ��� 2B --> B
	while (tC::read_real<int16_t, int16_t>(m_inFile, re) &&
		tC::read_real<int16_t, int16_t>(m_inFile, im)) {
		xip_complex sample{ re, im };

		// ���
		dmd_interp.process(sample);
		if (!dmd_interp.next(sample))
			continue;

		// ������������� ������ �� 2B
		process_sample_channel_matched_receive(&sample, &sample);

		//agc.process(sample, sample);

		if (i == 1) {	// ����� ������ 2-� ������
			i = 0;
			continue;
		}
		i = 1;

		// ��� ��� ������ ������ ������ �������� ������
		if (!m_agc.process(sample, sample))
			continue;

		xip_complex est = nearest_point_psk4(sample);		// ������� �������
		xip_real sts_err = m_stsEst.getErr(sample, est);	// ������ ������ �������� ������

		// ��� ����������� ��������� +/-4096 ������ ����� � ��������� [-2^24, 2^24]

		// ��������� ������������ �������� �� ��������� ��� --> [-2^16, 2^16] 
		// �������� ������ ����� ���������� ������ �� ��������. 
		// ��� ������������ �������� ����� ��� ������
		// ��� ����������� �������� � �������� ������� ����� �������� ����� �� [-2^10, 2^10]
		// ������ [-2^10, 2^10] ��������� ���������������, �.�. ��� �������� �������������
		xip_real_shift(sts_err, -8);

		pif_sts.process(sts_err, sts_err);	// ����������� ������� ������ � ���
		//dbg_out << sts_err << endl;

		// ��������� ������������ �������� �� ��������� ������������� --> [-2^10, 2^10]
		xip_real_shift(sts_err, -6);
		dmd_interp.shift(-(int32_t)sts_err);

		//dbg_out << sts_err << endl;
		//dbg_out << dmd_interp.getPos() << endl;
		tC::write_real<int16_t>(m_outDmdFile, sample.re);
		tC::write_real<int16_t>(m_outDmdFile, sample.im);
	}
	dbg_out.close();
}
