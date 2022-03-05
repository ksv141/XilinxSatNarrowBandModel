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
 * @brief �������� ������ ������������ DDC (4 ������)
*/
class HalfBandDDCTree
{
	static const unsigned n_levels = 4;					// ���������� �������
	static const unsigned n_ternimals = 1 << (n_levels + 1);	// ���������� ������������ ��������� ������ (� ������ ���������� ���������)
	static const unsigned in_fs = 400000;				// ������� ������������� �� ����� ������
	static const int terminal_fs = in_fs >> n_levels;	// ������� ������������� � ������������ �������� (25000 ��)
	static const int freq_shift = terminal_fs >> 2;		// ������� ������ ��� ���������� ����� ������ ������������ (6250 ��)

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
	 * @brief ���������� ����� ��������������� �����������, ������� ��������� ������, ������������� ������ ������������
	 * @param dph ��������� ����� ������ --> [-DDS_PHASE_MODULUS, DDS_PHASE_MODULUS]
	 * @return
	*/
	bool processTuneCorrelator(int16_t dph);

	/**
	 * @brief ���������� ����� ��������������� �����������, ������� ��������� ������, ������������ ��� ������ ���� � ������
	 * @param dph ��������� ����� ������ --> [-DDS_PHASE_MODULUS, DDS_PHASE_MODULUS]
	 * @return true - ���� ������ ��� ���� � ������ ���������, false - ���� �� ������� ���� ��� ��� ���������
	 * ���� ���� ��� ��� ��������� ������� �� �������, �� �� ���������� ����� 0
	*/
	bool processPhaseTimingCorrelator(int16_t dph);

	/**
	 * @brief ���������� ������ � �������� �������������� ��������
	 * @return 
	*/
	xip_complex* getData();

	/**
	 * @brief ���������� ����� �����������, ������������� ������
	 * @return
	*/
	unsigned getFreqEstCorrNum();

	/**
	 * @brief ���������� ������� � ������ ������� �����������
	 * @return
	*/
	int16_t getfreqEstStage_1();

	/**
	 * @brief ���������� ������� � ������ ������� �����������
	 * @return
	*/
	int16_t getfreqEstStage_2();

	/**
	 * @brief ���������� ����
	 * @return 
	*/
	int16_t getPhaseEst();

	/**
	 * @brief ���������� �������� ������
	 * @return 
	*/
	int16_t getSymbolTimingEst();

	/**
	 * @brief ���������� ����� ��������� ����� ������������� ������� � ������ ���� ������������
	 * [-DDS_PHASE_MODULUS, DDS_PHASE_MODULUS] ������������ Fs �� ����� ������
	 * @return 
	*/
	int16_t countTotalFreqShift();

private:
	AutoGaneControl m_agc;					// ��� �� ����� ������ DDC ��� ���������� ������� ������������

	HalfBandDDC m_ddc[n_levels] = { 1, 2, 3, 4 };

	xip_complex* out_ddc[n_levels + 1];		// ������� �� ����� � ������ ddc ������� ������
											// �� ������ ���������� ������ [0...15] - ����������� ������������, 
											// [16...31] - ��������� ������������

	xip_complex* out_itrp;					// ������ � ��������������� ������ �������������

	LagrangeInterp itrp;					// �������������� ������������

	DDS m_freqShifter;						// ��������� ��� ���������� ������ (����������� ���������� ����� ������ ������������)
	int16_t m_freqShiftMod;

	LowpassFir m_matchedFir;				// ������������� ������ �� 2B ����� ������������

	vector<CorrelatorDPDI> m_correlators;	// �������������� ���������� (1-� ������, ������ ������)
	CorrelatorDPDI m_tuneCorrelator;		// ������������� ���������� (2-� ������, ������ ������)
	PhaseTimingCorrelator m_phaseTimingCorrelator;	// ���������� ��� ������ ���� � ������ (3-� ������)

	int16_t m_freqEstStage_1;				// ������� � ������ ������� �����������
	unsigned m_freqEstCorrNum;				// ����� �����������, ������������� ������
	int16_t m_freqEstStage_2;				// ������� � ������ ������� �����������
	int16_t m_phaseEst;						// ���� � ������ ����������� 3-� ������
	int16_t m_symbolTimingEst;				// �������� �������� � ������ ����������� 3-� ������

	ofstream m_outCorrelator;				// ���� � ������� ����������� (��� �������)
};

#endif // HALFBANDDDCTREE_H