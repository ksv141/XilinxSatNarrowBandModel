#ifndef CONSTELLATION_H
#define CONSTELLATION_H

#include "cmpy_v6_0_bitacc_cmodel.h"

extern const xip_real constell_qam4_norm_val;

extern xip_complex constell_qam4[4];

extern xip_complex constell_preamble[2];

extern xip_real pwr_constell_qam4;

// жесткое решение
extern xip_complex nearest_point_qam4(const xip_complex& in);

extern int nearest_index_qam4(const xip_complex& in);

#endif // CONSTELLATION_H
