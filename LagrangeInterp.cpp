#include <bitset>
#include <string>

#include "LagrangeInterp.h"

using namespace std;

// ������������ �������� 7-� ������� ��� �����������������, ������������ ��� ���
// �������� ����� ����� �������� ��������� ������� �� 1024 ����������, 
// ��� ������� �� ������� ������������ ���� ������� ������������ ����� �� 8 �������������

// �������� 1024 ������ �� 8 �������������
int lagrange_load_coeff()
{
	// test bitset
	string s = "10000000";
	bitset<8> bs(s);
	unsigned long ul = bs.to_ulong();
	int8_t i8 = (int8_t)ul;
	return 0;
}