#ifndef CONSTELLATION_H
#define CONSTELLATION_H

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "gmp.h"


xip_complex constell_qam4[4] = { {4096, 4096},
								{4096, -4096},
								{-4096, 4096},
								{-4096, -4096} };

#endif // CONSTELLATION_H
