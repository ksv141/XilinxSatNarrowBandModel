#include "HalfBandDDCTree.h"

HalfBandDDCTree::HalfBandDDCTree():
	itrp(terminal_fs, INIT_SAMPLE_RATE, n_ternimals)
{
	for (int i = 0; i <= n_levels; i++)
		out_ddc[i] = new xip_complex[1 << i];
}

HalfBandDDCTree::~HalfBandDDCTree()
{
	for (int i = 0; i <= n_levels; i++)
		delete[] out_ddc[i];
}

bool HalfBandDDCTree::process(const xip_complex& in)
{
	out_ddc[0][0] = in;
	for (int i = 0; i < n_levels; i++)
		if (!m_ddc[i].process(out_ddc[i], out_ddc[i + 1]))
			return false;

	itrp.process(out_ddc[n_levels]);
	while (itrp.next(res)) {

	}
	return true;
}

xip_complex* HalfBandDDCTree::getData()
{
	return out_ddc[n_levels];
}
