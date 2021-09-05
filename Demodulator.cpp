#include "Demodulator.h"

Demodulator::Demodulator(const string& input_file, const string& output_dmd_file, const string& output_bin_file, size_t data_length):
	pif_sts(0.01)
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
	while (tC::read_real<int16_t, int16_t>(m_inFile, re) &&
		tC::read_real<int16_t, int16_t>(m_inFile, im)) {
		xip_complex sample{ re, im };

		// СТС
		dmd_interp.process(sample);
		dmd_interp.next(sample);

		// согласованный фильтр на 2B
		process_sample_channel_matched_receive(&sample, &sample);

		//agc.process(sample, sample);

		if (i == 0) {	// берем каждый 2-й отсчет
			i = 1;
			continue;
		}
		i = 0;

		// снижаем диапазон в 2 раза примерно до уровня исходного сигнала +/-4096 для более точной работы СТС и ФАПЧ
		xip_complex_shift(sample, -2);
		xip_complex est = nearest_point_psk4(sample);		// жесткое решение
		xip_real sts_err = m_stsEst.getErr(sample, est);	// оценка ошибки тактовой синхры

		// для сигнального созвездия +/-4096 ошибка будет в диапазоне [-2^26, 2^26]
		// уменьшаем динамический диапазон до [-2^15, 2^15]
		xip_real_shift(sts_err, -11);
		//dbg_out << sts_err << endl;

		pif_sts.process_1(sts_err, sts_err);	// сглаживание сигнала ошибки в ПИФ

		// уменьшаем динамический диапазон до [-2^10, 2^10] для интерполятора
		xip_real_shift(sts_err, -5);
		dmd_interp.setShift(-sts_err);

		dbg_out << sts_err << endl;
		tC::write_real<int16_t>(m_outDmdFile, sample.re);
		tC::write_real<int16_t>(m_outDmdFile, sample.im);
	}
	dbg_out.close();
}
