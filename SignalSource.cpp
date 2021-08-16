#include "SignalSource.h"

// ���������, ����������� � ������ ������� �����
const int SignalSource::preambleData[SignalSource::preambleLength] = {0,0,0,1,1,0,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,1};

SignalSource::SignalSource(size_t data_length) :
	dataLength(data_length),
	symbolSource(/* 2^32 - ����� M-������������������ */ 32, /* 4-����������� �������� */ 4)
{
}


SignalSource::~SignalSource()
{
}

xip_complex SignalSource::nextSample()
{
	xip_complex current_sample;
	int current_symbol;

	if (sampleCounter % 2) {
		// �� �������� ������� 0
		current_sample = xip_complex{ 0, 0 };
	}
	else {
		if (frameCounter < preambleLength) {
			// ������ 32 ������� ����� - ���������
			int current_symbol = preambleData[frameCounter];
			current_sample = constell_preamble[current_symbol];
		}
		else {
			// ��������� ������� - ������
			int current_symbol = symbolSource.nextSymbol();
			current_sample = constell_qam4[current_symbol];
		}

		frameCounter++;
		if (frameCounter == dataLength + preambleLength)
			frameCounter = 0;
	}

	sampleCounter++;

	return current_sample;
}
