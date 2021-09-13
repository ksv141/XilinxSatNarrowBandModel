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
	int i = 0;	// счетчик для 2B --> B
	xip_real err_pll = 0;	// оценка частотного сдвига
	while (tC::read_real<int16_t, int16_t>(m_inFile, re) &&
		tC::read_real<int16_t, int16_t>(m_inFile, im)) {
		xip_complex sample{ re, im };

		// СТС
		dmd_interp.process(sample);
		while (dmd_interp.next(sample)) {

			// согласованный фильтр на 2B
			process_sample_channel_matched_receive(&sample, &sample);

			// 2B --> B
			if (i == 1) {	// берем каждый 2-й отсчет
				i = 0;
				continue;
			}
			i = 1;

			// АРУ для точной оценки ошибки тактовой синхры
			// уровень сигнала нормируется относительно уровня сигнального созвездия
			if (!m_agc.process(sample, sample))
				continue;
			// для сигнального созвездия +/-4096 сигнал с выхода АРУ будет в диапазоне [-2^14, 2^14]

			//******** петля ФАПЧ, компенсация частотного смещения ****************
			xip_real dds_phase, dds_sin, dds_cos;
			dds.process(err_pll, dds_phase, dds_sin, dds_cos);	// сигнал ГУН
			xip_complex pll_corr{ dds_cos, dds_sin };
			xip_multiply_complex(sample, pll_corr, sample);		// компенсация
			xip_complex_shift(sample, -(int)(dds.getOutputWidth() - 1));
			//*********************************************************************

			xip_complex est = nearest_point_psk4(sample);		// жесткое решение

			//******** оценка частотного сдвига ***********************************
			xip_complex err_pll_sample{ sample.re, -sample.im };  // комплексно-сопряженное от текущего отсчета
			xip_multiply_complex(err_pll_sample, est, err_pll_sample);
			err_pll = err_pll_sample.im;	// частотный сдвиг

			// примем максимальный диапазон отклонения фазы [-0.5, 0.5] рад
			// для сигнального созвездия +/-4096 сдвиг будет в диапазоне [-0.5*2^25, 0.5*2^25] --> рад << 25
			// приведем к диапазону [-2^15, 2^15] для работы ПИФ
			xip_real_shift(err_pll, -10);
			pif_pll.process(err_pll, err_pll);	// сглаживание и интеграция сигнала ошибки в ПИФ

			// переведем в диапазон работы DDS --> [0, 16384] --> [0, 2pi]
			xip_multiply_real(err_pll, DDS_RAD_CONST, err_pll);
			xip_real_shift(err_pll, -18);
			if (err_pll < 0)
				err_pll += DDS_PHASE_MODULUS;
			//*********************************************************************

			//************ оценка ошибки тактовой синхры **************************
			xip_real sts_err = m_stsEst.getErr(sample, est);
			// для сигнального созвездия +/-4096 ошибка будет в диапазоне [-2^26, 2^26]

			// уменьшаем динамический диапазон до диапазона ПИФ --> [-2^15, 2^15] 
			// величину сдвига нужно подобирать исходя из ресурсов. 
			// для максимальной точности можно без сдвига
			// для минимальной точности и экономии ресурса можно сдвинуть сразу до [-2^10, 2^10]
			// меньше [-2^10, 2^10] ученьшать нецелесообразно, т.к. это диапазон интерполятора
			//xip_real_shift(sts_err, -11);
			xip_real_shift(sts_err, -6);

			pif_sts.process(sts_err, sts_err);	// сглаживание и интеграция сигнала ошибки в ПИФ

			// уменьшаем динамический диапазон до диапазона интерполятора --> [-2^10, 2^10]
			//xip_real_shift(sts_err, -5);
			dbg_out << sts_err << endl;

			// коррекция смещения интерполятора
			dmd_interp.shift(-(int32_t)sts_err);
			//*********************************************************************

			// пишем в файл
			tC::write_real<int16_t>(m_outDmdFile, sample.re);
			tC::write_real<int16_t>(m_outDmdFile, sample.im);
		}
	}
	dbg_out.close();
}
