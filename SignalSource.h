#ifndef SIGNALSOURCE_H
#define SIGNALSOURCE_H

#include <cstdint>
#include <string>
#include <fstream>
#include <stdexcept>
#include <bitset>

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
	/**
	 * @brief ����������� ��� ��������� �� ��� (��-��������� 4-����)
	 * @param data_length ������ ����� ������ � ����� (����)
	*/
	SignalSource(size_t data_length);

	/**
	 * @brief ����������� ��� ��������� �� ����� (��������� ��� ��������)
	 * @param input_file ����
	 * @param is_binary ������� ��������� �����
	 * @param data_length ������ ����� ������ � ����� (����)
	*/
	SignalSource(string input_file, bool is_binary, size_t data_length);

	~SignalSource();

	// ��������� ���������� ���������������� �������
	xip_complex nextSample();

	// ��������� ���������� ���������������� ������� �� �����
	bool nextSampleFromFile(xip_complex& out);

	/**
	 * @brief helper-�������. ��������� ���-�������� � ������ � ��������� ����
	 * @param n_pos ����� ������� (��� ����������� ���������)
	 * @param count ����� ��������
	 * @param file_name ��������� ����
	*/
	static void generateSymbolFile(int n_pos, size_t count, string file_name);

	/**
	 * @brief helper-�������. ��������� 2-���� ���-������������������ � ������ � �������� ����
	 * @param byte_count ����� ������
	 * @param file_name �������� ����
	*/
	static void generateBinFile(size_t byte_count, string file_name);

private:
	size_t dataLength = 0;		// ������ ����� ������ � �����
	size_t frameCounter = 0;	// ������� �������� � �����

	static const size_t preambleLength = 32;	// ������ ���������
	static const int8_t preambleData[preambleLength]; // ���������

	ifstream inFile;			// ������� ���� � ���������
	bool binaryFile;			// ������� ��������� �����
	char readByte;				// ��������� ���� �� �����
	int bitShift;				// ���������� ����������� ����� �� ��������� �����
	unsigned int bitMask;
	int bitPos;

	// ��������������� �������� ��������
	mls symbolSource{ /* 2^32 - ����� M-������������������ */ 32, /* 4-����������� �������� */ 4 };

	bool __nextSample(bool from_file, xip_complex& out);
};

#endif // SIGNALSOURCE_H