#include "constellation.h"

// нормальное значение уровня сигнала
const xip_real constell_qam4_norm_val = 4096;

// сигнальное созвездие для данных
xip_complex constell_qam4[4] = { {constell_qam4_norm_val, constell_qam4_norm_val},
								{constell_qam4_norm_val, -constell_qam4_norm_val},
								{-constell_qam4_norm_val, constell_qam4_norm_val},
								{-constell_qam4_norm_val, -constell_qam4_norm_val} };

// сигнальное созвездие для преамбулы
xip_complex constell_preamble[2] = { {constell_qam4_norm_val, constell_qam4_norm_val},
								{-constell_qam4_norm_val, -constell_qam4_norm_val} };

// нормированная мощность сигнала qam4 (используется для АРУ демодулятора)
xip_real pwr_constell_qam4 = 2 * constell_qam4[0].re * constell_qam4[0].re;

// жесткое решение
xip_complex nearest_point_qam4(const xip_complex& in)
{
	return constell_qam4[nearest_index_qam4(in)];
}

int nearest_index_qam4(const xip_complex& in)
{
	if (in.re < 0) {
		if (in.im < 0)
			return 3;
		else
			return 2;
	}
	else {
		if (in.im < 0)
			return 1;
		else
			return 0;
	}
	return 0;
}
