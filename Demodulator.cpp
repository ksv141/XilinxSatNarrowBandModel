#include "Demodulator.h"

Demodulator::Demodulator(const string& input_file, const string& output_dmd_file, const string& output_bin_file, size_t data_length):
	m_agc(AGC_WND_SIZE_LOG2, pwr_constell_psk4),
	pif_sts(PIF_STS_Kp, PIF_STS_Ki),
	pif_pll(PIF_PLL_Kp, PIF_PLL_Ki),
	dds(DDS_PHASE_MODULUS)
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
	xip_real err_pll = 0;	// ������ ���������� ������
	while (tC::read_real<int16_t, int16_t>(m_inFile, re) &&
		tC::read_real<int16_t, int16_t>(m_inFile, im)) {
		xip_complex sample{ re, im };

		// ���
		dmd_interp.process(sample);
		while (dmd_interp.next(sample)) {

			// ������������� ������ �� 2B
			process_sample_channel_matched_receive(&sample, &sample);

			// 2B --> B
			if (i == 1) {	// ����� ������ 2-� ������
				i = 0;
				continue;
			}
			i = 1;

			// ��� ��� ������ ������ ������ �������� ������
			// ������� ������� ����������� ������������ ������ ����������� ���������
			if (!m_agc.process(sample, sample))
				continue;
			// ��� ����������� ��������� +/-4096 ������ � ������ ��� ����� � ��������� [-2^14, 2^14]

			//******** ����� ����, ����������� ���������� �������� ****************
			xip_real dds_phase, dds_sin, dds_cos;
			dds.process(err_pll, dds_phase, dds_sin, dds_cos);	// ������ ���
			xip_complex pll_corr{ dds_cos, dds_sin };
			xip_multiply_complex(sample, pll_corr, sample);		// �����������
			xip_complex_shift(sample, -(int)(dds.getOutputWidth() - 1));
			//*********************************************************************

			xip_complex est = nearest_point_psk4(sample);		// ������� �������

			//******** ������ ���������� ������ ***********************************
			xip_complex err_pll_sample{ sample.re, -sample.im };  // ����������-����������� �� �������� �������
			xip_multiply_complex(err_pll_sample, est, err_pll_sample);
			err_pll = err_pll_sample.im;	// ��������� �����

			// ������ ������������ �������� ���������� ���� [-0.5, 0.5] ���
			// ��� ����������� ��������� +/-4096 ����� ����� � ��������� [-0.5*2^25, 0.5*2^25] --> ��� << 25
			// �������� � ��������� [-2^15, 2^15] ��� ������ ���
			xip_real_shift(err_pll, -10);
			pif_pll.process(err_pll, err_pll);	// ����������� � ���������� ������� ������ � ���

			// ��������� � �������� ������ DDS --> [0, 16384] --> [0, 2pi]
			xip_multiply_real(err_pll, DDS_RAD_CONST, err_pll);
			xip_real_shift(err_pll, -18);
			if (err_pll < 0)
				err_pll += DDS_PHASE_MODULUS;
			//*********************************************************************

			//************ ������ ������ �������� ������ **************************
			xip_real sts_err = m_stsEst.getErr(sample, est);
			// ��� ����������� ��������� +/-4096 ������ ����� � ��������� [-2^26, 2^26]

			// ��������� ������������ �������� �� ��������� ��� --> [-2^15, 2^15] 
			// �������� ������ ����� ���������� ������ �� ��������. 
			// ��� ������������ �������� ����� ��� ������
			// ��� ����������� �������� � �������� ������� ����� �������� ����� �� [-2^10, 2^10]
			// ������ [-2^10, 2^10] ��������� ���������������, �.�. ��� �������� �������������
			//xip_real_shift(sts_err, -11);
			xip_real_shift(sts_err, -6);

			pif_sts.process(sts_err, sts_err);	// ����������� � ���������� ������� ������ � ���

			// ��������� ������������ �������� �� ��������� ������������� --> [-2^10, 2^10]
			//xip_real_shift(sts_err, -5);
			dbg_out << sts_err << endl;

			// ��������� �������� �������������
			dmd_interp.shift(-(int32_t)sts_err);
			//*********************************************************************

			// ����� � ����
			tC::write_real<int16_t>(m_outDmdFile, sample.re);
			tC::write_real<int16_t>(m_outDmdFile, sample.im);
		}
	}
	dbg_out.close();
}
