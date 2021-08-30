#include "SignalSource.h"

// ���������, ����������� � ������ ������� �����
const int8_t SignalSource::preambleData[SignalSource::preambleLength] = {0,0,0,1,1,0,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,1};

SignalSource::SignalSource(size_t data_length) :
	dataLength(data_length)
{
}

SignalSource::SignalSource(string input_file, bool is_binary, size_t data_length) :
	binaryFile(is_binary),
	dataLength(data_length)
{
	if (is_binary) {
		inFile.open(input_file, ios::in | ios::binary);
		if (current_constell == Current_constell::PSK2 || current_constell == Current_constell::PSK2_60) {
			bitShift = 1;
		}
		else if (current_constell == Current_constell::PSK4)
			bitShift = 2;

		bitPos = 0;
	}
	else
		inFile.open(input_file);

	if (!inFile.is_open())
		throw runtime_error("input file is not opened");
}

SignalSource::~SignalSource()
{
	if (inFile.is_open())
		inFile.close();
}

xip_complex SignalSource::nextSample()
{
	xip_complex out;
	__nextSample(false, out);
	return out;
}

bool SignalSource::nextSampleFromFile(xip_complex& out)
{
	return __nextSample(true, out);
}

void SignalSource::generateSymbolFile(int n_pos, size_t count, string file_name)
{
	ofstream out(file_name);
	if (!out.is_open())
		return;

	mls symbolSource{ /* 2^32 - ����� M-������������������ */ 32, n_pos };
	for (size_t i = 0; i < count; i++)
		out << symbolSource.nextSymbol() << endl;

	out.close();
}

void SignalSource::generateBinFile(size_t byte_count, string file_name)
{
	ofstream out(file_name, ios::out | ios::binary);;
	if (!out.is_open())
		return;

	mls symbolSource{ /* 2^32 - ����� M-������������������ */ 32, 2 };
	for (size_t i = 0; i < byte_count; i++) {
		bitset<8> byte;
		for (int j = 0; j < 8; j++) {
			byte[j] = symbolSource.nextSymbol();
		}
		unsigned long b = byte.to_ulong();
		out.write((char*)&b, 1);
	}

	out.close();
}

bool SignalSource::__nextSample(bool from_file, xip_complex& out)
{
	int current_symbol = -1;

	if (frameCounter < preambleLength) {
		// ������ 32 ������� ����� - ���������
		current_symbol = preambleData[frameCounter];
		out = constell_preamble_psk4[current_symbol];
	}
	else {
		// ��������� ������� - ������
		if (from_file) {
			if (inFile.eof())
				return false;
			if (binaryFile) {
				bitPos += bitShift;
				if (bitPos >= 8) {
					inFile.read(&readByte, 1);
					bitPos = 0;
				}
				current_symbol = (readByte >> (8 - bitPos - bitShift)) << (8 - bitShift);
			}
			else
				inFile >> current_symbol;
		}
		else
			current_symbol = symbolSource.nextSymbol();

		out = constell_psk4[current_symbol];
	}

	frameCounter++;
	if (frameCounter == dataLength + preambleLength)
		frameCounter = 0;

	return true;
}
