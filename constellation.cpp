#include "constellation.h"

// сигнальное созвездие для данных
xip_complex constell_qam4[4] = { {4096, 4096},
								{4096, -4096},
								{-4096, 4096},
								{-4096, -4096} };

// сигнальное созвездие для преамбулы
xip_complex constell_preamble[2] = { {4096, 4096},
								{-4096, -4096} };

// нормированная мощность сигнала qam4 (используется для АРУ демодулятора)
xip_real pwr_constell_qam4 = 2 * constell_qam4[0].re * constell_qam4[0].re;