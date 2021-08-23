#ifndef UTILS_H
#define UTILS_H

#include <ostream>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "debug.h"

using namespace std;

extern int init_xip_multiplier();

extern int destroy_xip_multiplier();

extern ostream& operator<<(ostream& out, const xip_complex& data);

#endif // UTILS_H