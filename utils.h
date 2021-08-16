#ifndef UTILS_H
#define UTILS_H

#include <ostream>
#include "cmpy_v6_0_bitacc_cmodel.h"

using namespace std;

ostream& operator<<(ostream& out, const xip_complex& data) {
	out << data.re;
	if (data.im >= 0)
		out << '+';
	out << data.im << 'i';
	return out;
}

#endif // UTILS_H