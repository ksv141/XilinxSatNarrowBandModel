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

extern const uint16_t FRAME_DATA_SIZE;
extern const size_t PREAMBLE_LENGTH;
extern const int8_t PREAMBLE_DATA[];
extern const size_t POSTAMBLE_LENGTH;
extern const int8_t POSTAMBLE_DATA[];

/**
* �������� ������������ �������. ������� ������������ ���� �� ���, ���� �� ����� ���������� �����.
* ������ ����������� � ���� ������ � ���������� 32 ����
* ��� ������ ���������� ������������� �����
*/

class SignalSource
{
public:
	/**
	 * @brief ����������� ��� ��������� �� ��� (��-��������� 4-����)
	 * @param ������� ��������� ��������� � ����
	 * @param ������� ��������� ���������� � ����
	*/
	SignalSource(bool has_preamble, bool has_postamble);

	/**
	 * @brief ����������� ��� ��������� �� ����� (��������� ��� ��������)
	 * @param input_file ����
	 * @param is_binary ������� ��������� �����
	 * @param ������� ��������� ��������� � ����
	 * @param ������� ��������� ���������� � ����
	*/
	SignalSource(const string& input_file, bool is_binary, bool has_preamble, bool has_postamble);

	~SignalSource();

	// ��������� ���������� ���������������� �������
	xip_complex nextSample();

	// ��������� ���������� ������� ��� ����� �� �����
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
	bool hasPreamble;			// ������� ��������� ��������� � ����
	bool hasPostamble;			// ������� ��������� ���������� � ����
	size_t dataLength = 0;		// ������ ����� ������ � ����� (� ��������)
	size_t frameCounter = 0;	// ������� �������� � �����
	unsigned dataPos;			// ������� ������ ������
	unsigned postamblePos;		// ������� ������ ���������
	unsigned endPos;			// ������� ����� �����

	ifstream inFile;			// ������� ���� � ���������
	bool binaryFile;			// ������� ��������� �����
	char readByte;				// ��������� ���� �� �����
	int bitShift;				// ���������� ����������� ����� �� ��������� �����
	unsigned int bitMask;
	int bitPos;
	bool manchesterState = false;		// ���� ��������� ��� ��������� ���� ����������
	unsigned lastManchesterSymbol;

	// ��������������� �������� ��������
	mls symbolSource{ /* 2^32 - ����� M-������������������ */ 32, /* 4-����������� �������� */ 4 };

	bool __nextSample(bool from_file, xip_complex& out);

	bool __nextSampleManchester(bool from_file, xip_complex& out);

	bool __nextSymbol(bool from_file, unsigned& out);
	
	// �������� ����� ������ �� ������ � �������
	void countDataLength(size_t bytes);
};

#endif // SIGNALSOURCE_H