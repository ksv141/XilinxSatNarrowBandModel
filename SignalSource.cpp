#include "SignalSource.h"


SignalSource::SignalSource(): 
	symbolSource(/* 2^32 - ����� M-������������������ */ 32, /* 4-����������� �������� */ 4)
{
}


SignalSource::~SignalSource()
{
}

xip_complex SignalSource::nextSample()
{
	int current_symbol = symbolSource.nextSymbol();
	return constell_qam4[current_symbol];
	sampleCounter++;
}
