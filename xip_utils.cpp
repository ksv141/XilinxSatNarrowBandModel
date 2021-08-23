#include "xip_utils.h"

ostream& operator<<(ostream& out, const xip_complex& data) {
	out << data.re;
	if (data.im >= 0)
		out << '+';
	out << data.im << 'i';
	return out;
}
