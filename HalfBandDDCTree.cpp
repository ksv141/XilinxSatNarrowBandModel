#include "HalfBandDDCTree.h"

HalfBandDDCTree::HalfBandDDCTree():
	m_agc(AGC_WND_SIZE_LOG2, get_cur_constell_pwr()),
	itrp(terminal_fs, INIT_SAMPLE_RATE, n_ternimals),
	m_freqShifter(DDS_PHASE_MODULUS),
	m_matchedFir("rc_root_x2_25_19.fcf", 19, 0, n_ternimals),
	m_correlators(n_ternimals, { FRAME_DATA_SIZE, (int8_t*)SignalSource::preambleData, SignalSource::preambleLength,
								1, 32, 1, DPDI_BURST_ML_SATGE_1 }),
	m_tuneCorrelator(FRAME_DATA_SIZE, (int8_t*)SignalSource::preambleData, SignalSource::preambleLength,
		32, 1, 1, DPDI_BURST_ML_SATGE_2)
{
	for (int i = 0; i <= n_levels; i++)
		out_ddc[i] = new xip_complex[1 << (i + 1)];

	out_itrp = new xip_complex[1 << (n_levels + 1)];

	m_freqShiftMod = DDS_PHASE_MODULUS / (in_fs / freq_shift);

	m_outCorrelator.open("correlators.out");
}

HalfBandDDCTree::~HalfBandDDCTree()
{
	for (int i = 0; i <= n_levels; i++)
		delete[] out_ddc[i];
	delete[] out_itrp;

	m_outCorrelator.close();
}

bool HalfBandDDCTree::process(const xip_complex& in)
{
	// ��� �� ����� ������ DDC ��� ���������� ������� ������������
	xip_complex in_agc{ 0, 0 };
	if (!m_agc.process(in, in_agc))
		return false;

	// ��������� ����� ��� ���������� ����� ������ ������������
	xip_complex mod_sample{ 0, 0 };
	m_freqShifter.process(m_freqShiftMod, mod_sample);
	xip_complex in_shifted;
	xip_multiply_complex(in_agc, mod_sample, in_shifted);
	xip_complex_shift(in_shifted, -(int)(m_freqShifter.getOutputWidth() - 1));	// ��������� ������������ �������� ���������� (��������� ������� �����)


	out_ddc[0][0] = in_agc;
	out_ddc[0][1] = in_shifted;
	for (int i = 0; i < n_levels; i++)
		if (!m_ddc[i].process(out_ddc[i], out_ddc[i + 1]))
			return false;

	// ������������  25000 --> 2B (������ ���������!!!)
	itrp.process(out_ddc[n_levels]);
	if (itrp.next(out_itrp)) {
		m_matchedFir.process(out_itrp, out_itrp);
		bool est = false;
		for (int i = 0; i < n_ternimals; i++) {
			xip_real corr_est = 0;
			if (m_correlators[i].process(out_itrp[i], m_freqEstStage_1, corr_est)) {
				m_freqEstCorrNum = i;
				freqShiftBuffer(-m_freqEstStage_1);	// ��������� ����� ������ ��������������� �����������, ������� ��������� ������
				est = processTuneCorrelator();		// ������ ���������� �� ������, ��� ��������� ������
				break;
			}
			//m_outCorrelator << corr_est << "\t";
		}
		//m_outCorrelator << endl;
		return est;
	}
	return false;
}

bool HalfBandDDCTree::processTuneCorrelator()
{
	deque<xip_complex>& corr_reg = m_correlators[m_freqEstCorrNum].getBuffer();
	return m_tuneCorrelator.processBuffer(corr_reg, m_freqEstStage_2);
}

void HalfBandDDCTree::freqShiftBuffer(int16_t dph)
{
	deque<xip_complex>& corr_reg = m_correlators[m_freqEstCorrNum].getBuffer();

	if (dph < 0)
		dph += DDS_PHASE_MODULUS;

	DDS dds(DDS_PHASE_MODULUS);

	for (xip_complex& sample : corr_reg) {
		xip_complex mod_sample{ 0, 0 };
		dds.process(dph, mod_sample);
		xip_multiply_complex(sample, mod_sample, sample);
		xip_complex_shift(sample, -(int)(dds.getOutputWidth() - 1));	// ��������� ������������ �������� ���������� (��������� ������� �����)
	}
}

xip_complex* HalfBandDDCTree::getData()
{
	return out_itrp;
}

unsigned HalfBandDDCTree::getFreqEstCorrNum()
{
	return m_freqEstCorrNum;
}

int16_t HalfBandDDCTree::getfreqEstStage_1()
{
	return m_freqEstStage_1;
}
