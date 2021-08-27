#include "autoganecontrol.h"

AutoGaneControl::AutoGaneControl(int window_size, double norm_power):
    m_normPower(norm_power), m_windowSize(window_size), m_currentPower(0)
{
    m_pwrReg.resize(window_size, 0);
}

bool AutoGaneControl::process(const xip_complex& in, xip_complex& out)
{
	double pwr_x = (in.re * in.re + in.im * in.im)/m_windowSize;
    m_currentPower += pwr_x;
    m_currentPower -= m_pwrReg.back();
    m_pwrReg.pop_back();
    m_pwrReg.push_front(pwr_x);

	double norm = sqrt(m_currentPower/m_normPower);
	if (norm == 0)
		out = in;
	else
	{
		out.re = in.re / norm;
		out.im = in.im / norm;
	}

    return true;
}

void AutoGaneControl::reset()
{
    std::fill(m_pwrReg.begin(), m_pwrReg.end(), 0);
    m_currentPower = 0;
}
