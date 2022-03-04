#ifndef HALFBANDDDCTREE_H
#define HALFBANDDDCTREE_H

#include <vector>
#include <fstream>
#include "constellation.h"
#include "HalfBandDDC.h"
#include "LagrangeInterp.h"
#include "DDS.h"
#include "LowpassFir.h"
#include "CorrelatorDPDI.h"
#include "SignalSource.h"
#include "autoganecontrol.h"
#include "PhaseTimingCorrelator.h"

using namespace std;

extern const int INIT_SAMPLE_RATE;
extern const int DDS_PHASE_MODULUS;
extern const int AGC_WND_SIZE_LOG2;
extern const uint16_t FRAME_DATA_SIZE;
extern const uint32_t DPDI_BURST_ML_SATGE_1;
extern const uint32_t DPDI_BURST_ML_SATGE_2;
extern const uint32_t PHASE_BURST_ML_SATGE_3;

/**
 * @brief Бинарное дерево полуполосных DDC (4 уровня)
*/
class HalfBandDDCTree
{
	static const unsigned n_levels = 4;					// количество уровней
	static const unsigned n_ternimals = 1 << (n_levels + 1);	// количество терминальных элементов дерева (с учетом сдвинутого диапазона)
	static const unsigned in_fs = 400000;				// частота дискретизации на входе дерева
	static const int terminal_fs = in_fs >> n_levels;	// частота дискретизации в терминальном элементе (25000 Гц)
	static const int freq_shift = terminal_fs >> 2;		// частота сдвига для перекрытия полос частот корреляторов (6250 Гц)

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
	 * @brief обработать буфер многоканального коррелятора, который обнаружил сигнал, одноканальным точным коррелятором
	 * @param dph частотный сдвиг буфера --> [-DDS_PHASE_MODULUS, DDS_PHASE_MODULUS]
	 * @return
	*/
	bool processTuneCorrelator(int16_t dph);

	/**
	 * @brief обработать буфер многоканального коррелятора, который обнаружил сигнал, коррелятором для оценки фазы и тактов
	 * @param dph частотный сдвиг буфера --> [-DDS_PHASE_MODULUS, DDS_PHASE_MODULUS]
	 * @return true - если оценка для фазы и тактов выполнена, false - если не оценены один или оба параметра
	 * если один или оба параметра оценить не удалось, то он становится равен 0
	*/
	bool processPhaseTimingCorrelator(int16_t dph);

	/**
	 * @brief возвращает массив с выходным многоканальным отсчетом
	 * @return 
	*/
	xip_complex* getData();

	/**
	 * @brief возвращает номер коррелятора, обнаружившего сигнал
	 * @return
	*/
	unsigned getFreqEstCorrNum();

	/**
	 * @brief возвращает частоту с выхода грубого коррелятора
	 * @return
	*/
	int16_t getfreqEstStage_1();

	/**
	 * @brief возвращает частоту с выхода точного коррелятора
	 * @return
	*/
	int16_t getfreqEstStage_2();

	/**
	 * @brief возвращает фазу
	 * @return 
	*/
	int16_t getPhaseEst();

	/**
	 * @brief возвращает смещение тактов
	 * @return 
	*/
	int16_t getSymbolTimingEst();

	/**
	 * @brief рассчитать общий частотный сдвиг обнаруженного сигнала с учетом всех корреляторов
	 * [-DDS_PHASE_MODULUS, DDS_PHASE_MODULUS] относительно Fs на входе дерева
	 * @return 
	*/
	int16_t countTotalFreqShift();

private:
	AutoGaneControl m_agc;					// АРУ на входе дерева DDC для нормировки уровней корреляторов

	HalfBandDDC m_ddc[n_levels] = { 1, 2, 3, 4 };

	xip_complex* out_ddc[n_levels + 1];		// массивы на входе и выходе ddc каждого уровня
											// на выходе последнего уровня [0...15] - несмещенные поддиапазоны, 
											// [16...31] - смещенные поддиапазоны

	xip_complex* out_itrp;					// массив с многоканального выхода интерполятора

	LagrangeInterp itrp;					// многоканальный интерполятор

	DDS m_freqShifter;						// генератор для частотного сдвига (обеспечение перекрытия полос частот корреляторов)
	int16_t m_freqShiftMod;

	LowpassFir m_matchedFir;				// согласованный фильтр на 2B перед коррелятором

	vector<CorrelatorDPDI> m_correlators;	// многоканальный коррелятор (1-я стадия, грубая оценка)
	CorrelatorDPDI m_tuneCorrelator;		// одноканальный коррелятор (2-я стадия, точная оценка)
	PhaseTimingCorrelator m_phaseTimingCorrelator;	// коррелятор для оценки фазы и тактов (3-я стадия)

	int16_t m_freqEstStage_1;				// частота с выхода грубого коррелятора
	unsigned m_freqEstCorrNum;				// номер коррелятора, обнаружившего сигнал
	int16_t m_freqEstStage_2;				// частота с выхода точного коррелятора
	int16_t m_phaseEst;						// фаза с выхода коррелятора 3-й стадии
	int16_t m_symbolTimingEst;				// тактовое смещение с выхода коррелятора 3-й стадии

	ofstream m_outCorrelator;				// файл с данными коррелятора (для отладки)
};

#endif // HALFBANDDDCTREE_H