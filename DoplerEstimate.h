#ifndef DOPLERESTIMATE_H
#define DOPLERESTIMATE_H

#include <deque>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "constellation.h"
#include "xip_utils.h"

using namespace std;

// Блок оценки смещения Доплера
class DoplerEstimate
{
public:
	DoplerEstimate();

	// Оценка смещения Доплера (на 1B)
	int16_t getErr(const xip_complex& out_symb, const xip_complex& est);

private:
	deque<xip_complex> m_reg;     // FIFO-регистр для вычисления ошибки
};

#endif // DOPLERESTIMATE_H