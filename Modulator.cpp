#include "Modulator.h"


Modulator::Modulator(const string& input_file, const string& output_file, size_t data_length):
	m_signalSource(input_file, true, data_length)
{
	m_outFile = fopen(output_file.c_str(), "wb");
	if (!m_outFile)
		throw runtime_error("output file is not opened");
}

Modulator::~Modulator()
{
	if (m_outFile)
		fclose(m_outFile);
}

void Modulator::process()
{
	if (!m_outFile)
		return;

	// кодирование данных из файла и модул€ци€
	xip_complex sample;
	size_t sample_counter = 0;
	while (true) {
		// B --> 2B
		if (sample_counter % 2)
			sample = xip_complex{ 0, 0 };
		else
			if (!m_signalSource.nextSampleFromFile(sample))
				break;

		// канальный фильтр на 2B
		process_sample_channel_matched_transmit(&sample, &sample);

		// если диапазон сигнала +/-4096, то с выхода фильра 13-битный отсчет

		// повышаем точность на 2 бита (до 15 бит)
		xip_complex_shift(sample, 2);

		fx_cmpl_point fx_sample;
		fx_sample.set_val((int16_t)sample.re + 1, (int16_t)sample.im + 1);

		tC::write_real<int16_t>(m_outFile, fx_sample.real.i_val);
		tC::write_real<int16_t>(m_outFile, fx_sample.imag.i_val);

		sample_counter++;
	}
}
