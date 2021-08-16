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
	SignalSource(size_t data_length);
	~SignalSource();

	xip_complex nextSample();

private:
	size_t sampleCounter = 0;	// ������� ��������
	size_t dataLength;			// ������ ����� ������ � �����
	size_t frameCounter = 0;	// ������� �������� � �����

	static const size_t preambleLength = 32;	// ������ ���������
	static const int preambleData[preambleLength]; // ���������

	// ��������������� �������� ��������
	mls symbolSource;
};

#endif // SIGNALSOURCE_H