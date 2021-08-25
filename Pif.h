#ifndef PIF_H
#define PIF_H

#include <cmath>
#include <stdexcept>
#include <algorithm>

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"

// Пиф (Nezami - Loop Filter topology III)
// Реализован как совокупность КИХ-фильтра 2-го порядка и интегратора
class Pif
{
public:
	Pif(double g1, double g2);

	Pif(double specific_locking_band = 0.001); // specific_locking_band = lock_band / sampling_freq
		                                       // примеры: 0.0005 или 4.0 / 8000.0
	~Pif();

	int process(const xip_real& in, xip_real& out);

private:
	void calculate_g1_g2(double slb);

	int init_xip_fir();

	int destroy_xip_fir();

	double g[2];	// g[0] - пропорциональная составляющая, g[1] - интегральная составляющая
	xip_real reg;		// регистр для интегратора

	xip_fir_v7_2* xip_fir;				// КИХ-фильтр
	xip_fir_v7_2_config xip_fir_cnfg;	// конфиг фильтра
	xip_array_real* xip_fir_in;			// 3-D массив, содержащий текущий отсчет для обработки
	xip_array_real* xip_fir_out;		// 3-D массив, содержащий результат обработки
};

#endif // PIF_H