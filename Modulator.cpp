#include "Modulator.h"



Modulator::Modulator(const string& input_file, const string& output_file, size_t data_length):
	m_signalSource(input_file, true, data_length)
{
	m_outFile.open(output_file, ios::out | ios::binary);
	if (!m_outFile.is_open())
		throw runtime_error("input file is not opened");
}


Modulator::~Modulator()
{
	if (m_outFile.is_open())
		m_outFile.close();
}

void Modulator::process()
{
	if (!m_outFile.is_open())
		return;

	xip_complex sample;
	while (m_signalSource.nextSampleFromFile(sample)) {
		m_outFile.write((char*)&sample.re, sizeof(sample.re));
		m_outFile.write((char*)&sample.im, sizeof(sample.im));
	}
}
