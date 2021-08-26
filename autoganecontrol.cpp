#include <complex>
#include "autoganecontrol.h"

AutoGaneControl::AutoGaneControl(bool active, int window_size, float norm_power):
    m_active(active), m_normPower(norm_power), m_windowSize(window_size), m_counter(0), m_currentPower(0)
{
    if (active) {
        m_pwrReg.resize(window_size, 0);
    }
}

bool AutoGaneControl::process(cplx_fl in, cplx_fl &out)
{
    if (!m_active) {
        out = in;
        return true;
    }

    float pwr_x = std::norm(in)/m_windowSize;
    m_currentPower += pwr_x;
    m_currentPower -= m_pwrReg.back();
    m_pwrReg.pop_back();
    m_pwrReg.push_front(pwr_x);

    if (m_counter < m_windowSize) {
        m_counter++;
        return false;
    }

    float norm = sqrt(m_currentPower/m_normPower);
    out = in/norm;

    return true;
}

void AutoGaneControl::reset()
{
    std::fill(m_pwrReg.begin(), m_pwrReg.end(), 0);
    m_currentPower = 0;
}

bool AutoGaneControl::active() const
{
    return m_active;
}

void AutoGaneControl::setActive(bool active)
{
    m_active = active;
}

float AutoGaneControl::currentPower() const
{
    return m_currentPower;
}
