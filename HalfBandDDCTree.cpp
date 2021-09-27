#include "HalfBandDDCTree.h"

HalfBandDDCTree::HalfBandDDCTree():
	m_agc(AGC_WND_SIZE_LOG2, get_cur_constell_pwr()),
	itrp(terminal_fs, INIT_SAMPLE_RATE, n_ternimals),
	m_freqShifter(DDS_PHASE_MODULUS),
	m_matchedFir("rc_root_x2_25_19.fcf", 19, 0, n_ternimals),
	m_correlators(n_ternimals, { FRAME_DATA_SIZE, (int8_t*)SignalSource::preambleData, (uint16_t)SignalSource::preambleLength,
								1, 32, 1, DPDI_BURST_ML_SATGE_1 }),
	m_tuneCorrelator(FRAME_DATA_SIZE, (int8_t*)SignalSource::preambleData, (uint16_t)SignalSource::preambleLength,
		8, 4, 1, DPDI_BURST_ML_SATGE_2),
	m_phaseTimingCorrelator((int8_t*)SignalSource::preambleData, (uint16_t)SignalSource::preambleLength, PHASE_BURST_ML_SATGE_3)
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
	// АРУ на входе дерева DDC для нормировки уровней корреляторов
	xip_complex in_agc{ 0, 0 };
	if (!m_agc.process(in, in_agc))
		return false;

	// частотный сдвиг для перекрытия полос частот корреляторов
	xip_complex mod_sample{ 0, 0 };
	m_freqShifter.process(m_freqShiftMod, mod_sample);
	xip_complex in_shifted;
	xip_multiply_complex(in_agc, mod_sample, in_shifted);
	xip_complex_shift(in_shifted, -(int)(m_freqShifter.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)


	out_ddc[0][0] = in_agc;
	out_ddc[0][1] = in_shifted;
	for (int i = 0; i < n_levels; i++)
		if (!m_ddc[i].process(out_ddc[i], out_ddc[i + 1]))
			return false;

	// интерполяция  25000 --> 2B (только понижение!!!)
	itrp.process(out_ddc[n_levels]);
	if (itrp.next(out_itrp)) {
		m_matchedFir.process(out_itrp, out_itrp);
		bool est = false;
		for (int i = 0; i < n_ternimals; i++) {
			xip_real corr_est = 0;
			if (m_correlators[i].process(out_itrp[i], m_freqEstStage_1, corr_est)) {
				m_freqEstCorrNum = i;
				est = processTuneCorrelator(-m_freqEstStage_1);		// точный коррелятор на буфере, где обнаружен сигнал
				break;
			}
			//m_outCorrelator << corr_est << "\t";
		}
		//m_outCorrelator << endl;
		return est;
	}
	return false;
}

bool HalfBandDDCTree::processTuneCorrelator(int16_t dph)
{
	deque<xip_complex>& corr_reg = m_correlators[m_freqEstCorrNum].getBuffer();

	if (dph < 0)
		dph += DDS_PHASE_MODULUS;
	DDS dds(DDS_PHASE_MODULUS);

	xip_real corr_est = 0;
	for (deque<xip_complex>::reverse_iterator it = corr_reg.rbegin(); it != corr_reg.rend(); it++) {
		xip_complex mod_sample{ 0, 0 };
		dds.process(dph, mod_sample);
		xip_multiply_complex(*it, mod_sample, *it);
		xip_complex_shift(*it, -(int)(dds.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)

		if (m_tuneCorrelator.process(*it, m_freqEstStage_2, corr_est)) {
			return true;
		}
		//m_outCorrelator << corr_est << endl;
	}
	return false;
}

bool HalfBandDDCTree::processPhaseTimingCorrelator(int16_t dph)
{
	deque<xip_complex>& corr_reg = m_correlators[m_freqEstCorrNum].getBuffer();

	if (dph < 0)
		dph += DDS_PHASE_MODULUS;
	DDS dds(DDS_PHASE_MODULUS);

	m_phaseEst = 0;
	m_symbolTimingEst = 0;
	for (deque<xip_complex>::reverse_iterator it = corr_reg.rbegin(); it != corr_reg.rend(); it++) {
		xip_complex mod_sample{ 0, 0 };
		dds.process(dph, mod_sample);
		xip_multiply_complex(*it, mod_sample, *it);
		xip_complex_shift(*it, -(int)(dds.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)

		if (m_phaseTimingCorrelator.isPhaseEstMode()) {
			int16_t ph = 0;
			xip_real phase_est = 0;
			m_phaseTimingCorrelator.phaseEstimate(*it, m_phaseEst, phase_est);
		}
		else {
			int16_t t_shift = 0;
			xip_real time_est = 0;
			if (m_phaseTimingCorrelator.getSymbolTimingProcCounter()) {
				if (m_phaseTimingCorrelator.symbolTimingEstimate(*it, t_shift, time_est)) {
					m_symbolTimingEst = (int16_t)t_shift;
					return true;
				}
			}
		}
		//m_outCorrelator << corr_est << endl;
	}
	return false;
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

int16_t HalfBandDDCTree::getfreqEstStage_2()
{
	return m_freqEstStage_2;
}

int16_t HalfBandDDCTree::getPhaseEst()
{
	return m_phaseEst;
}

int16_t HalfBandDDCTree::getSymbolTimingEst()
{
	return m_symbolTimingEst;
}
