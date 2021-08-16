#ifndef SIGNALSOURCE_H
#define SIGNALSOURCE_H

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "mls.h"
#include "constellation.h"

/**
* Источник сомплексного сигнала
* - сигнал формируется в виде кадров с преамбулой 32 бита
* - выполняется увеличение частоты дискретизации до 2B путем вставки 0
*/

class SignalSource
{
public:
	SignalSource();
	~SignalSource();

	xip_complex nextSample();

private:
	size_t sampleCounter = 0;

	// псевдослучайный источник символов
	mls symbolSource;
};

#endif // SIGNALSOURCE_H