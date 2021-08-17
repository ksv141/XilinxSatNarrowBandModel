#include <iostream>
#include <fstream>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "SignalSource.h"
#include "utils.h"

using namespace std;

int main()
{
	// ���������� ����
	ofstream dbg_out("dbg_out.txt");

	// �������� �������
	SignalSource signal_source("input_data.txt", 20);

	// ���������� ������������ ��������
	int symbol_count = 1000;

	//signal_source.generateSamplesFile(symbol_count, "input_data.txt");

	// �������� ���� ��������� ��������
	for (int i = 0; i < symbol_count; i++)
	{
		// ��������� ���������� �������
		xip_complex current_sample = signal_source.nextSampleFromFile();
		dbg_out << current_sample << endl;
		//cout << current_sample << endl;
	}

	dbg_out.close();
	return 0;
}