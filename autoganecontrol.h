#ifndef AUTOGANECONTROL_H
#define AUTOGANECONTROL_H

#include <cmath>
#include <deque>
#include "cmpy_v6_0_bitacc_cmodel.h"

// автоматический регулятор уровня сигнала методом скользящего среднего

class AutoGaneControl
{
public:
    AutoGaneControl(int window_size = 1, double norm_power = 1.0);

    // обработка одного отсчета. результат - готов ли результат.
    // результат не готов, когда регистр еще не заполнен
    bool process(const xip_complex& in, xip_complex& out);

    // сброс регистра в 0
    void reset();

private:
	double m_normPower;

    std::deque<double> m_pwrReg;          // регистр накопления мощности

	double m_currentPower;

    int m_windowSize;

    size_t m_counter;
};

#endif // AUTOGANECONTROL_H
