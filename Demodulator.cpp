#include "Demodulator.h"

Demodulator::Demodulator(const string& input_file, const string& output_file, size_t data_length)
{
	m_inFile = fopen(input_file.c_str(), "rb");
	if (!m_inFile)
		throw runtime_error("input file is not opened");
	m_outFile = fopen(output_file.c_str(), "wb");
	if (!m_outFile)
		throw runtime_error("output file is not opened");
}

Demodulator::~Demodulator()
{
	if (m_inFile)
		fclose(m_inFile);
	if (m_outFile)
		fclose(m_outFile);
}

void Demodulator::process()
{
	if (!m_inFile)
		return;
	if (!m_outFile)
		return;

	ofstream dbg_out("dbg_out.txt");

	int16_t re;
	int16_t im;
	int i = 0;	// счетчик для 2B --> B
	while (tC::read_real<int16_t, int16_t>(m_inFile, re) &&
		tC::read_real<int16_t, int16_t>(m_inFile, im)) {
		xip_complex sample{ re, im };

		// согласованный фильтр на 2B
		process_sample_channel_matched_receive(&sample, &sample);

		if (i == 1) {	// берем каждый 2-й отсчет
			i = 0;
			continue;
		}
		i = 1;

		dbg_out << sample << endl;
		//tC::write_real<int16_t>(out_file, res.re);
		//tC::write_real<int16_t>(out_file, res.im);
	}
	dbg_out.close();
}
