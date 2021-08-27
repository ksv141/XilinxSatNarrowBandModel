#ifndef STSESTIMATE_H
#define STSESTIMATE_H

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "FirMultiplier.h"
#include "FirSummator.h"

// Оценка тактового смещения (для СТС)
// Оценка ошибки тактовой синхронизации по Мюллеру-Меллеру
class StsEstimate
{
public:
	StsEstimate();
	~StsEstimate();

	xip_real getErr(const xip_complex& out_symb, const xip_complex& est);

private:
	xip_complex m_last_out_symb;
	xip_complex m_last_est;
};

#endif // STSESTIMATE_H