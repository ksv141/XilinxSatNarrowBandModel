#ifndef UTILS_H
#define UTILS_H

#include <ostream>
#include <cmath>
#include <stdint.h>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "cordic_v6_0_bitacc_cmodel.h"
#include "debug.h"

using namespace std;

/**
 * @brief инициализаци€ умножител€. один умножитель используетс€ дл€ всей модели
 * @return 
*/
extern int init_xip_multiplier();

/**
 * @brief деинициализаци€ умножител€
 * @return 
*/
extern int destroy_xip_multiplier();

/**
 * @brief умножение двух комплексных чисел
 * @param a 
 * @param b 
 * @param out 
 * @return 
*/
extern int xip_multiply_complex(const xip_complex& a, const xip_complex& b, xip_complex& out);

/**
 * @brief умножение двух вещественных чисел
 * @param a 
 * @param b 
 * @param out 
 * @return 
*/
extern int xip_multiply_real(const xip_real& a, const xip_real& b, xip_real& out);

/**
 * @brief инициализаци€ вычислител€ корн€. один умножитель используетс€ дл€ всей модели
 * @return 
*/
extern int init_xip_cordic_sqrt();

/**
 * @brief деинициализаци€ вычислител€ корн€
 * @return 
*/
extern int destroy_xip_cordic_sqrt();

/**
 * @brief вычисление корн€
 * @param arg 
 * @param out 
 * @return 
*/
extern int xip_sqrt_real(const xip_real& arg, xip_real& out);

/**
 * @brief инициализаци€ комплексного конвертора
 * @return 
*/
extern int init_xip_cordic_rect_to_polar();

/**
 * @brief деинициализаци€ комплексного конвертора
 * @return 
*/
extern int destroy_xip_cordic_rect_to_polar();

/**
 * @brief конвертаци€ комплексного числа в пол€рные координаты
 * @param in вход
 * @param mag модуль
 * @param arg аргумент
 * @return
*/
extern int xip_cordic_rect_to_polar(const xip_complex& in, xip_real& mag, xip_real& arg);

/**
 * @brief целочисленное деление. 
 * вещественные аргументы перевод€тс€ в int32 максимального диапазона.
 * деление возможно при a >= b
 * @param a 
 * @param b 
 * @param res 
 * @return 
*/
extern int int32_division(double a, double b, double& res);

/**
 * @brief масштабирование числа в 2^n
 * @param in_out вход и выход
 * @param pos число двоичных позиций сдвига, может быть + или -
*/
extern void xip_real_shift(xip_real& in_out, int pos);

/**
 * @brief масштабирование комплексного числа в 2^n
 * @param in_out вход и выход
 * @param pos число двоичных позиций сдвига, может быть + или -
*/
extern void xip_complex_shift(xip_complex& in_out, int pos);

extern ostream& operator<<(ostream& out, const xip_complex& data);

#endif // UTILS_H