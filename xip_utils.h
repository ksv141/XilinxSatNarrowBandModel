#ifndef UTILS_H
#define UTILS_H

#include <ostream>
#include <cmath>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "cordic_v6_0_bitacc_cmodel.h"
#include "debug.h"

using namespace std;

/**
 * @brief ������������� ����������. ���� ���������� ������������ ��� ���� ������
 * @return 
*/
extern int init_xip_multiplier();

/**
 * @brief ��������������� ����������
 * @return 
*/
extern int destroy_xip_multiplier();

/**
 * @brief ��������� ���� ����������� �����
 * @param a 
 * @param b 
 * @param out 
 * @return 
*/
extern int xip_multiply_complex(const xip_complex& a, const xip_complex& b, xip_complex& out);

/**
 * @brief ��������� ���� ������������ �����
 * @param a 
 * @param b 
 * @param out 
 * @return 
*/
extern int xip_multiply_real(const xip_real& a, const xip_real& b, xip_real& out);

/**
 * @brief ������������� ����������� �����. ���� ���������� ������������ ��� ���� ������
 * @return 
*/
extern int init_xip_cordic_sqrt();

/**
 * @brief ��������������� ����������� �����
 * @return 
*/
extern int destroy_xip_cordic_sqrt();

/**
 * @brief ��������������� ����� � 2^n
 * @param in_out ���� � �����
 * @param pos ����� �������� ������� ������, ����� ���� + ��� -
*/
extern void xip_real_shift(xip_real& in_out, int pos);

/**
 * @brief ��������������� ������������ ����� � 2^n
 * @param in_out ���� � �����
 * @param pos ����� �������� ������� ������, ����� ���� + ��� -
*/
extern void xip_complex_shift(xip_complex& in_out, int pos);

extern ostream& operator<<(ostream& out, const xip_complex& data);

#endif // UTILS_H