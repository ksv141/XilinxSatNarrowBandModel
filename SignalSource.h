#ifndef SIGNALSOURCE_H
#define SIGNALSOURCE_H

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "mls.h"
#include "constellation.h"

/**
* �������� ������������ �������
* - ������ ����������� � ���� ������ � ���������� 32 ����
* - ����������� ���������� ������� ������������� �� 2B ����� ������� 0
*/

class SignalSource
{
public:
	SignalSource();
	~SignalSource();

	xip_complex nextSample();

private:
	size_t sampleCounter = 0;

	// ��������������� �������� ��������
	mls symbolSource;
};

#endif // SIGNALSOURCE_H