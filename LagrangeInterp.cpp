#include "LagrangeInterp.h"

using namespace std;

// ������������ �������� 7-� ������� ��� �����������������, ������������ ��� ���
// �������� ����� ����� �������� ��������� ������� �� 1024 ����������, 
// ��� ������� �� ������� ������������ ���� ������� ������������ ����� �� 8 �������������

int32_t lagrange_n_intervals = 1024;	// ���������� ����������
int32_t lagrange_n_coeff = 8;			// ���������� �������������
int16_t* lagrange_coeff = nullptr;		// ������ ������������� �������, ������� �� �������

// �������� 1024 ������ �� 8 �������������
int lagrange_load_coeff()
{
	lagrange_coeff = new int16_t[lagrange_n_coeff * lagrange_n_intervals];

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
			lagrange_coeff[i + j * lagrange_n_coeff] = val_int;
		}
	}

	//ofstream out("coeff_out.txt");
	//for (int i = 0; i < lagrange_n_coeff * lagrange_n_intervals; i++)
	//{
	//	if (i % 8 == 0)
	//		out << "*********" << endl;
	//	out << lagrange_coeff[i] << endl;
	//}
	//out.close();

	return 0;
}

int destroy_lagrange_interp()
{
	if (lagrange_coeff)
		delete[] lagrange_coeff;

	return 0;
}