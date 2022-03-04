#include "HalfBandDDCTree.h"

HalfBandDDCTree::HalfBandDDCTree():
	m_agc(AGC_WND_SIZE_LOG2, get_cur_constell_pwr()),
	itrp(terminal_fs, INIT_SAMPLE_RATE, n_ternimals),
	m_freqShifter(DDS_PHASE_MODULUS),
	m_freqCorrector(DDS_PHASE_MODULUS),
	m_matchedFir("rc_root_x2_25_19.fcf", 19, 0, n_ternimals),
	m_correlators(n_ternimals, { FRAME_DATA_SIZE, (int8_t*)SignalSource::preambleData, (uint16_t)SignalSource::preambleLength,
								1, 32, 1, DPDI_BURST_ML_SATGE_1 }),
	m_tuneCorrelator(FRAME_DATA_SIZE, (int8_t*)SignalSource::preambleData, (uint16_t)SignalSource::preambleLength,
		8, 4, 1, DPDI_BURST_ML_SATGE_2),
	m_phaseTimingCorrelator((int8_t*)SignalSource::preambleData, (uint16_t)SignalSource::preambleLength, PHASE_BURST_ML_SATGE_3),
	m_estDone(false)
{
	for (int i = 0; i <= n_levels; i++)
		out_ddc[i] = new xip_complex[1 << (i + 1)];

	out_itrp = new xip_complex[1 << (n_levels + 1)];

	m_freqShiftMod = DDS_PHASE_MODULUS / (in_fs / freq_shift);

	//m_outCorrelator.open("correlators.out");
}

HalfBandDDCTree::~HalfBandDDCTree()
{
	for (int i = 0; i <= n_levels; i++)
		delete[] out_ddc[i];
	delete[] out_itrp;

	if (m_outCorrelator.is_open())
		m_outCorrelator.close();
}

bool HalfBandDDCTree::process(const xip_complex& in)
{
	// ј–” на входе дерева DDC дл€ нормировки уровней коррел€торов
	xip_complex in_agc{ 0, 0 };
	if (!m_agc.process(in, in_agc))
		return false;

	// частотный сдвиг дл€ перекрыти€ полос частот коррел€торов
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

	// интерпол€ци€  25000 --> 2B (только понижение!!!)
	itrp.process(out_ddc[n_levels]);
	if (itrp.next(out_itrp)) {
		m_matchedFir.process(out_itrp, out_itrp);
		// ищем коррел€ционный отклик по всем коррел€торам
		if (!m_estDone) {
			for (int i = 0; i < n_ternimals; i++) {
				xip_real corr_est = 0;
				if (m_correlators[i].process(out_itrp[i], m_freqEstStage_1, corr_est)) {
					// если коррел€тор обнаружил сигнал, то получаем от него грубую оценку частоты,
					// корректируем частоту сигнала в буфере и передаем его на второй коррел€тор дл€ точной оценки частоты
					m_freqEstCorrNum = i;									// номер коррел€тора, обнаружившего сигнал
					m_estDone = processTuneCorrelator(-m_freqEstStage_1);	// точный коррел€тор на буфере, где обнаружен сигнал
					//processPhaseTimingCorrelator(-m_freqEstStage_2);		// оценка фазы и тактов на буфере, где обнаружен сигнал
					m_freqEstSum = m_freqEstStage_1 + m_freqEstStage_2;
					if (m_freqEstSum < 0)
						m_freqEstSum += DDS_PHASE_MODULUS;
					break;
				}
				//m_outCorrelator << corr_est << "\t";
			}
		}
		else {	// если сигнал обнаружен коррел€тором, корректируем частоту и демодулируем
			xip_complex mod_sample{ 0, 0 };
			m_freqCorrector.process(m_freqEstSum, mod_sample);
			xip_multiply_complex(out_itrp[m_freqEstCorrNum], mod_sample, out_itrp[m_freqEstCorrNum]);
			xip_complex_shift(out_itrp[m_freqEstCorrNum], -(int)(m_freqCorrector.getOutputWidth() - 1));	// уменьшаем динамический диапазон результата (подобрано опытным путем)
		}
		//m_outCorrelator << endl;
		return m_estDone;
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
			//m_outCorrelator << phase_est << endl;
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

int16_t HalfBandDDCTree::countTotalFreqShift()
{
	const int16_t f1 = DDS_PHASE_MODULUS >> n_levels;
	const int16_t f2 = f1 >> 1;
	const int16_t f3 = f2 >> 1;
	const unsigned corr_count = n_ternimals >> 1;
	const xip_real resample_coeff = (xip_real)INIT_SAMPLE_RATE / (xip_real)in_fs;	// коэффициент пересчета частоты
	int16_t ddc_shift = 0;
	unsigned corr_num = m_freqEstCorrNum;
	if (corr_num >= corr_count) {		// принадлежит ли номер коррел€тора сдвинутому диапазону
		corr_num -= corr_count;
		ddc_shift = f3;					// поправка на сдвиг диапазона
	}

	// пересчет частоты из 2B во входную
	xip_real f1_resample;
	xip_multiply_real((xip_real)m_freqEstStage_1, resample_coeff, f1_resample);
	xip_real f2_resample;
	xip_multiply_real((xip_real)m_freqEstStage_2, resample_coeff, f2_resample);

	int16_t freq = corr_num * f1 + f2 - ddc_shift + f1_resample + f2_resample;
//	int16_t freq = corr_num * f1 + f2 - ddc_shift;
	return freq;
}
