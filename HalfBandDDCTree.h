#ifndef HALFBANDDDCTREE_H
#define HALFBANDDDCTREE_H

#include "HalfBandDDC.h"
#include "LagrangeInterp.h"

extern const int INIT_SAMPLE_RATE;

/**
 * @brief �������� ������ ������������ DDC (5 �������)
*/
class HalfBandDDCTree
{
	static const unsigned n_levels = 4;					// ���������� �������
	static const unsigned n_ternimals = 1 << n_levels;	// ���������� ������������ ��������� ������
	static const int terminal_fs = 25000;				// ������� ������������� � ������������ ��������
public:
	/**
	 * @brief 
	*/
	HalfBandDDCTree();

	~HalfBandDDCTree();

	/**
	 * @brief ���������� ��������� ������. ��� ��������� �������� ������ ��������� ������� 2^n_levels ���
	 * @param in 
	 * @return 
	*/
	bool process(const xip_complex& in);

	/**
	 * @brief ���������� ������ � �������� �������������� ��������
	 * @return 
	*/
	xip_complex* getData();

private:
	HalfBandDDC m_ddc[n_levels] = { 1, 2, 3, 4 };

	xip_complex* out_ddc[n_levels + 1];		// ������� �� ����� � ������ ddc ������� ������

	LagrangeInterp itrp;					// �������������� ������������
};

#endif // HALFBANDDDCTREE_H