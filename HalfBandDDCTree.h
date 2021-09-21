#ifndef HALFBANDDDCTREE_H
#define HALFBANDDDCTREE_H

#include "HalfBandDDC.h"
#include "LagrangeInterp.h"

extern const int INIT_SAMPLE_RATE;

/**
 * @brief Бинарное дерево полуполосных DDC (5 уровней)
*/
class HalfBandDDCTree
{
	static const unsigned n_levels = 4;					// количество уровней
	static const unsigned n_ternimals = 1 << n_levels;	// количество терминальных элементов дерева
	static const int terminal_fs = 25000;				// частота дискретизации в терминальном элементе
public:
	/**
	 * @brief 
	*/
	HalfBandDDCTree();

	~HalfBandDDCTree();

	/**
	 * @brief обработать очередной отсчет. для получения выходных данных требуется вызвать 2^n_levels раз
	 * @param in 
	 * @return 
	*/
	bool process(const xip_complex& in);

	/**
	 * @brief возвращает массив с выходным многоканальным отсчетом
	 * @return 
	*/
	xip_complex* getData();

private:
	HalfBandDDC m_ddc[n_levels] = { 1, 2, 3, 4 };

	xip_complex* out_ddc[n_levels + 1];		// массивы на входе и выходе ddc каждого уровня

	LagrangeInterp itrp;					// многоканальный интерполятор
};

#endif // HALFBANDDDCTREE_H