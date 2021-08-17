#include "SignalSource.h"

// преамбула, вставляемая в начало каждого кадра
const int8_t SignalSource::preambleData[SignalSource::preambleLength] = {0,0,0,1,1,0,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,1};

SignalSource::SignalSource(size_t data_length) :
	dataLength(data_length)
{
}

SignalSource::SignalSource(string input_file, size_t data_length) :
	dataLength(data_length)
{
	inFile.open(input_file);
	if (!inFile.is_open())
		throw runtime_error("input file is not opened");
}

SignalSource::SignalSource()
{
}


SignalSource::~SignalSource()
{
	if (inFile.is_open())
		inFile.close();
}

xip_complex SignalSource::nextSample()
{
	return __nextSample(false);
}

xip_complex SignalSource::nextSampleFromFile()
{
	return __nextSample(true);
}

void SignalSource::generateSamplesFile(size_t count, string file_name)
{
	ofstream out(file_name);
	if (!out.is_open())
		return;

	for (size_t i = 0; i < count; i++)
		out << symbolSource.nextSymbol() << endl;

	out.close();
}

xip_complex SignalSource::__nextSample(bool from_file)
{
	xip_complex current_sample;
	int current_symbol;

	if (frameCounter < preambleLength) {
		// первые 32 символа кадра - преамбула
		current_symbol = preambleData[frameCounter];
		current_sample = constell_preamble[current_symbol];
	}
	else {
		// остальные символы - данные
		if (from_file)
			inFile >> current_symbol;
		else
			current_symbol = symbolSource.nextSymbol();
		current_sample = constell_qam4[current_symbol];
	}

	frameCounter++;
	if (frameCounter == dataLength + preambleLength)
		frameCounter = 0;

	return current_sample;
}
