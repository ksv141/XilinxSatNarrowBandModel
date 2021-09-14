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

bool CorrelatorDPDI::process(xip_complex in, xip_real& dph, xip_real& cur_est)
{
    // помещаем отсчет в FIFO
    m_correlationReg.pop_back();
    m_correlationReg.push_front(in);

    xip_complex rx_1{ 0, 0 };                           // текущее значение единичного коррелятора
    xip_complex reg_1{ 0, 0 };                          // предыдущее значение единичного коррелятора
    xip_complex SumL_1{ 0, 0 };                         // сумма по L дифференциальных корреляций (z)
    xip_complex SumL_2{ 0, 0 };                         // сумма по L дифференциальных корреляций, сдвинутых на 1 такт (z)

    // FIFO-регистр обходим с конца
    deque<xip_complex>::reverse_iterator reg_it = m_correlationReg.rbegin();

    for (int k = 0; k < m_correlatorF; k++) {           // Накопление корреляционного отклика для F преамбул
        vector<xip_complex>::iterator preamb_it = m_preamble.begin();
        for (int i = 0; i < m_correlatorL; i++) {       // Вычисление корреляционного отклика z для одной преамбулы
            rx_1 = xip_complex{ 0, 0 };
            for (int j = 0; j < m_correlatorM; j++) {   // Вычисление значения xk для единичного коррелятора
                xip_complex res;
                if (xip_multiply_complex(*preamb_it, *reg_it, res) == -1) // rm*cm для четных элементов регистра
                    return false;
                rx_1.re += res.re;
                rx_1.im += res.im;

                reg_it += 2;
                preamb_it++;
            }
            xip_complex res;
            xip_complex reg_1_conj{ reg_1.re, -reg_1.im };
            if (xip_multiply_complex(rx_1, reg_1_conj, res) == -1)  // Вычисление z для четных
                return false;
            SumL_1.re += res.re;
            SumL_1.im += res.im;

            reg_1 = rx_1;                               // Задержка на такт для четных
        }
        reg_it += 2 * m_dataLength;                     // Смещение к следующему пакету в регистре
    }

    SumL_2 = m_prev_sum_1;

    xip_real re_sqr = 0;
    xip_multiply_real(SumL_1.re, SumL_1.re, re_sqr);
    xip_real im_sqr = 0;
    xip_multiply_real(SumL_1.im, SumL_1.im, im_sqr);
    xip_real est_1 = re_sqr + im_sqr;                   // Абсолютное значение корреляционного отклика для четных

    xip_multiply_real(SumL_2.re, SumL_2.re, re_sqr);
    xip_multiply_real(SumL_2.im, SumL_2.im, im_sqr);
    xip_real est_2 = re_sqr + im_sqr;                   // Абсолютное значение корреляционного отклика для нечетных

    xip_real est;
    xip_complex maxSum;
    if (est_1 >= est_2) {           // Выбор наибольшего значения z из четного и нечетного
        est = est_1;
        maxSum = SumL_1;
    }
    else {
        est = est_2;
        maxSum = SumL_2;
    }

    m_prev_sum_1 = SumL_1;          // запоминаем сумму корреляции на текущем такте
    cur_est = est;

    if (est > m_burstEstML) {       // Порог в соответствии с критерием максимального правдоподобия
        //dph = std::arg(maxSum) / (2 * m_correlatorM);   // Оценка смещения по частоте (v = Arg{x}/2M), в рад/симв
        return true;
    }
    else {
        return false;
    }
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
