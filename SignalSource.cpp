#include "SignalSource.h"

SignalSource::SignalSource(bool has_preamble, bool has_postamble):
	hasPreamble(has_preamble),
	hasPostamble(has_postamble)
{
	countDataLength(FRAME_DATA_SIZE);
}

SignalSource::SignalSource(const string& input_file, bool is_binary, bool has_preamble, bool has_postamble):
	hasPreamble(has_preamble),
	hasPostamble(has_postamble),
	binaryFile(is_binary)
{
	countDataLength(FRAME_DATA_SIZE);

	if (is_binary) {
		inFile.open(input_file, ios::in | ios::binary);
		if (current_constell == Current_constell::PSK2 || current_constell == Current_constell::PSK2_60) {
			bitShift = 1;
			bitMask = 0x1;
		}
		else if (current_constell == Current_constell::PSK4) {
			bitShift = 2;
			bitMask = 0x3;
		}

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
	__nextSampleManchester(false, out);
	return out;
}

bool SignalSource::nextSampleFromFile(xip_complex& out)
{
	return __nextSampleManchester(true, out);
}

void SignalSource::generateSymbolFile(int n_pos, size_t count, string file_name)
{
	ofstream out(file_name);
	if (!out.is_open())
		return;

	mls symbolSource{ /* 2^32 - длина M-последовательности */ 32, n_pos };
	for (size_t i = 0; i < count; i++)
		out << symbolSource.nextSymbol() << endl;

	out.close();
}

void SignalSource::generateBinFile(size_t byte_count, string file_name)
{
	ofstream out(file_name, ios::out | ios::binary);;
	if (!out.is_open())
		return;

	mls symbolSource{ /* 2^32 - длина M-последовательности */ 32, 2 };
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
	unsigned int current_symbol = 0;
	bool res = __nextSymbol(from_file, current_symbol);
	out = get_cur_constell_sample(current_symbol);
	return res;
}

bool SignalSource::__nextSampleManchester(bool from_file, xip_complex& out)
{
	bool res = true;
	if (manchesterState) {
		lastManchesterSymbol = (lastManchesterSymbol == 0) ? 1 : 0;
	}
	else {
		res = __nextSymbol(from_file, lastManchesterSymbol);
	}
	manchesterState = !manchesterState;
	out = get_cur_constell_sample(lastManchesterSymbol);
	return res;
}

bool SignalSource::__nextSymbol(bool from_file, unsigned& out)
{
	unsigned int current_symbol = 0;

	if (frameCounter < dataPos) {
		// вставка преамбулы
		out = PREAMBLE_DATA[frameCounter];
	}
	else if (frameCounter < postamblePos) {
		// вставка данных
		if (from_file) {
			if (inFile.eof())
				return false;
			if (binaryFile) {
				if (bitPos == 0)
					if (!inFile.read(&readByte, 1))
						return false;
				current_symbol = (readByte >> bitPos) & bitMask;
				bitPos += bitShift;
				if (bitPos >= 8)
					bitPos = 0;
			}
			else
				inFile >> current_symbol;
		}
		else
			current_symbol = symbolSource.nextSymbol();

		out = current_symbol;
	}
	else {
		// вставка концевика
		out = POSTAMBLE_DATA[frameCounter - postamblePos];
	}

	frameCounter++;
	if (frameCounter == endPos)
		frameCounter = 0;

	return true;
}

void SignalSource::countDataLength(size_t bytes)
{
	if (current_constell == Current_constell::PSK2 || current_constell == Current_constell::PSK2_60) {
		dataLength = bytes * 8;
	}
	else if (current_constell == Current_constell::PSK4) {
		dataLength = bytes * 4;
	}

	dataPos = hasPreamble ? PREAMBLE_LENGTH : 0;
	postamblePos = dataPos + dataLength;
	endPos = hasPostamble ? postamblePos + POSTAMBLE_LENGTH : postamblePos;
}
