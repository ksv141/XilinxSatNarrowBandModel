#ifndef HALFBANDDDCTREE_H
#define HALFBANDDDCTREE_H

#include "HalfBandDDC.h"

/**
 * @brief Бинарное дерево полуполосных DDC
*/
class HalfBandDDCTree
{
public:
	HalfBandDDCTree();

	void process(const xip_complex& in);

	bool next(xip_complex& out);

private:
	//HalfBandDDC m_ddc;
};

#endif // HALFBANDDDCTREE_H