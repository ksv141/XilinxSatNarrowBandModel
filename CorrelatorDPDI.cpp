#include "CorrelatorDPDI.h"

CorrelatorDPDI::CorrelatorDPDI(uint16_t data_length, int8_t* preamble_data, uint16_t preamble_length, 
								uint16_t M, uint16_t L, uint16_t F, xip_real burst_est):
	m_dataLength(data_length),
	m_preambleLength(preamble_length),
	m_correlatorM(M),
	m_correlatorL(L),
	m_correlatorF(F),
	m_burstEstML(burst_est)
{
	init(preamble_data, preamble_length);
}

void CorrelatorDPDI::init(int8_t* preamble_data, uint16_t preamble_length)
{
    if (preamble_length < m_correlatorM * m_correlatorL)
        throw runtime_error("Preamble length incorrect");

    m_preamble.clear();

    // заполняем регистр коррелятора комплексно-сопряженной преамбулой
    for (int i = 0; i < preamble_length; i++) {
        xip_complex p = get_cur_constell_preamble_sample(preamble_data[i]);
        p.im = -p.im;
        m_preamble.push_back(p);
    }

    // размер регистра равен размеру пакета *2 с учетом удвоенной бодовой скорости и количества корреляторов
    m_correlationReg.resize((m_dataLength + m_preamble.size()) * m_correlatorF * 2, xip_complex{ 0, 0 });
}
