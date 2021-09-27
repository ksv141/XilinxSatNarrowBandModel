#ifndef DOPLERESTIMATE_H
#define DOPLERESTIMATE_H

#include <deque>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "constellation.h"
#include "xip_utils.h"

using namespace std;

// ���� ������ �������� �������
class DoplerEstimate
{
public:
	DoplerEstimate();

	// ������ �������� ������� (�� 1B)
	int16_t getErr(const xip_complex& out_symb, const xip_complex& est);

private:
	deque<xip_complex> m_reg;     // FIFO-������� ��� ���������� ������
};

#endif // DOPLERESTIMATE_H