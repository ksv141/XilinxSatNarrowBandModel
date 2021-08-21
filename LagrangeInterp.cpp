#include <bitset>
#include <string>

#include "LagrangeInterp.h"

using namespace std;

// Интерполятор Лагранжа 7-й степени без передискретизации, используется для СТС
// интервал между двумя смежными отсчетами делится на 1024 интервалов, 
// для каждого из которых используется свой заранее рассчитанный набор из 8 коэффициентов

// загрузка 1024 набора из 8 коэффициентов
int lagrange_load_coeff()
{
	// test bitset
	string s = "10000000";
	bitset<8> bs(s);
	unsigned long ul = bs.to_ulong();
	int8_t i8 = (int8_t)ul;
	return 0;
}