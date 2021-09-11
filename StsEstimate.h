#ifndef STSESTIMATE_H
#define STSESTIMATE_H

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "constellation.h"
#include "FirMultiplier.h"
#include "FirSummator.h"
#include "xip_utils.h"

// Оценка тактового смещения (для СТС)
class StsEstimate
{
public:
	StsEstimate();
	~StsEstimate();

	// Оценка ошибки тактовой синхронизации по Мюллеру-Меллеру (на 1B)
	// для сигнального созвездия размером 4096 ошибка будет в диапазоне [-2^26, 2^26]
	xip_real getErr(const xip_complex& out_symb, const xip_complex& est);

	// Оценка ошибки тактовой синхронизации по Гарднеру (на 2B)
	xip_real getErrGardner(const xip_complex& sample);

private:
	xip_complex m_last_out_symb;
	xip_complex m_last_est;

	xip_complex y[3] = { {0,0},{0,0},{0,0} };	// последние 3 отсчета на 2B
};

#endif // STSESTIMATE_H