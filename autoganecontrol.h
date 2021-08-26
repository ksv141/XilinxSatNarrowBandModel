#ifndef AUTOGANECONTROL_H
#define AUTOGANECONTROL_H

#include "common/modemtypes.h"

// автоматический регулятор уровня сигнала методом скользящего среднего

class AutoGaneControl
{
public:
    AutoGaneControl(bool active = true, int window_size = 1, float norm_power = 1.0);

    // обработка одного отсчета. результат - готов ли результат.
    // результат не готов, когда регистр еще не заполнен
    bool process(cplx_fl in, cplx_fl& out);

    // сброс регистра в 0
    void reset();

    bool active() const;
    void setActive(bool active);

    float currentPower() const;     // текущая мощность сигнала

private:
    bool m_active;
    float m_normPower;

    d_fl m_pwrReg;          // регистр накопления мощности

    float m_currentPower;

    int m_counter;
    int m_windowSize;
};

#endif // AUTOGANECONTROL_H
