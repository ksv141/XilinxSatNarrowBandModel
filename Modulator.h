#ifndef MODULATOR_H
#define MODULATOR_H

#include <fstream>
#include "fx_point.h"
#include "fx_cmpl_point.h"
#include "SignalSource.h"

/**
 * @brief ���������
 * �������� ����� ����� � ������� ����������� ���������,
 * ������������� ������,
 * B --> 2B,
 * ��������� ������ (������ ������������ ��������)
*/
class Modulator
{
public:
	/**
	 * @brief ��������� ���������� ����������
	 * @param input_file ������� �������� ����
	 * @param output_file �������� PCM-���� (16-��� ������ I/Q)
	 * @param data_length ������ ������ � ����� (����)
	*/
	Modulator(const string& input_file, const string& output_file, size_t data_length);

	~Modulator();

	/**
	 * @brief ������ ����������
	*/
	void process();

private:
	SignalSource m_signalSource;
	ofstream m_outFile;
};

#endif // MODULATOR_H