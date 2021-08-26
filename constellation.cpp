#include "constellation.h"

// ���������� ��������� ��� ������
xip_complex constell_qam4[4] = { {4096, 4096},
								{4096, -4096},
								{-4096, 4096},
								{-4096, -4096} };

// ���������� ��������� ��� ���������
xip_complex constell_preamble[2] = { {4096, 4096},
								{-4096, -4096} };

// ������������� �������� ������� qam4 (������������ ��� ��� ������������)
xip_real pwr_constell_qam4 = 2 * constell_qam4[0].re * constell_qam4[0].re;