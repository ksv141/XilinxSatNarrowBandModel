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
	SignalSource(size_t data_length);
	~SignalSource();

	xip_complex nextSample();

private:
	size_t sampleCounter = 0;	// счетчик отсчетов
	size_t dataLength;			// размер блока данных в кадре
	size_t frameCounter = 0;	// счетчик символов в кадре

	static const size_t preambleLength = 32;	// размер преамбулы
	static const int preambleData[preambleLength]; // преамбула

	// псевдослучайный источник символов
	mls symbolSource;
};

#endif // SIGNALSOURCE_H