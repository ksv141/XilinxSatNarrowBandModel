#ifndef AUTOGANECONTROL_H
#define AUTOGANECONTROL_H

#include <cmath>
#include <deque>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "xip_utils.h"
#include "debug.h"

// автоматический регулятор уровня сигнала методом скользящего среднего
// сумматор реализован на КИХ-фильтре с единичными коэффициентами
class AutoGaneControl
{
public:
    /**
     * @brief 
     * @param window_size_log2 - log2 окна усреднения АРУ
     * @param norm_power требуемая мощность сигнала
    */
    AutoGaneControl(unsigned window_size_log2 = 0, double norm_power = 1.0);

    ~AutoGaneControl();

    // обработка одного отсчета. результат - готов ли результат.
    // результат не готов, когда регистр еще не заполнен
    bool process(const xip_complex& in, xip_complex& out);

    // сброс регистра в 0
    void reset();

private:
	double m_normPower;

    std::deque<double> m_pwrReg;          // регистр накопления мощности

	double m_currentPower;

    unsigned m_windowSizeLog2;               // log2 окна усреднения АРУ
    unsigned m_windowSize;                   // окно усреднения АРУ

    size_t m_counter;

    int init_xip_fir(int window_size);

    int destroy_xip_fir();

    int xip_fir_process(const xip_real& in_re, const xip_real& in_im, xip_real& out);

    xip_real m_normPowerSqrt;

    xip_fir_v7_2* xip_fir;				// КИХ-фильтр сумматора
    xip_fir_v7_2_config xip_fir_cnfg;	// конфиг фильтра
    xip_array_real* xip_fir_in;			// 3-D массив, содержащий текущий отсчет для обработки
    xip_array_real* xip_fir_out;		// 3-D массив, содержащий результат обработки

};

#endif // AUTOGANECONTROL_H
