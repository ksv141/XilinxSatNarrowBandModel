#include "LagrangeInterp.h"

using namespace std;

// ������������ �������� 7-� ������� ��� �����������������, ������������ ��� ���
// �������� ����� ����� �������� ��������� ������� �� 1024 ����������, 
// ��� ������� �� ������� ������������ ���� ������� ������������ ����� �� 8 �������������

int32_t lagrange_n_intervals = 1024;	// ���������� ����������
int32_t lagrange_n_coeff = 8;			// ���������� �������������

// �������� 1024 ������ �� 8 �������������
int lagrange_load_coeff()
{
	for (int i = 0; i < lagrange_n_coeff; i++) {
		string fname;
		stringstream fname_ss;
		fname_ss << "lagrange_coeff\\LagrangeFixed" << i << ".coe";
		fname_ss >> fname;

		ifstream in(fname);
		if (!in.is_open())
			return -1;

		for (int j = 0; j < lagrange_n_intervals; j++) {
			string val_str;
			in >> val_str;
			bitset<16> val_bs(val_str);
			int16_t val_int = (int16_t)val_bs.to_ulong();

		}
	}

	return 0;
}