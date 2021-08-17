#ifndef SIGNALSOURCE_H
#define SIGNALSOURCE_H

#include <cstdint>
#include <string>
#include <fstream>
#include <stdexcept>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "mls.h"
#include "constellation.h"

using namespace std;

/**
* �������� ������������ �������. ������� ������������ ���� �� ���, ���� �� ����� ���������� �����.
* ������ ����������� � ���� ������ � ���������� 32 ����
*/

class SignalSource
{
public:
	// ����������� ��� ��������� �� ���
	SignalSource(size_t data_length);

	// ����������� ��� ��������� �� �����
	SignalSource(string input_file, size_t data_length);

	SignalSource();

	~SignalSource();

	// ��������� ���������� ���������������� �������
	xip_complex nextSample();

	// ��������� ���������� ���������������� ������� �� �����
	xip_complex nextSampleFromFile();

	// ��������� ���-�������� � ������ � ����
	void generateSamplesFile(size_t count, string file_name);

private:
	size_t dataLength = 0;		// ������ ����� ������ � �����
	size_t frameCounter = 0;	// ������� �������� � �����

	static const size_t preambleLength = 32;	// ������ ���������
	static const int8_t preambleData[preambleLength]; // ���������

	ifstream inFile;			// ������� ���� � ���������

	// ��������������� �������� ��������
	mls symbolSource{ /* 2^32 - ����� M-������������������ */ 32, /* 4-����������� �������� */ 4 };

	xip_complex __nextSample(bool from_file);
};

#endif // SIGNALSOURCE_H