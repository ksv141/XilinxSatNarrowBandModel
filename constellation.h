#ifndef CONSTELLATION_H
#define CONSTELLATION_H

#include "cmpy_v6_0_bitacc_cmodel.h"

enum Current_constell {PSK2, PSK2_60, PSK4};

// ������������ ��� �������
extern Current_constell current_constell;

extern const xip_real constell_psk_norm_val;

extern xip_complex constell_psk4[4];

extern xip_complex constell_psk2[2];

extern xip_complex constell_psk2_60[2];

extern xip_complex constell_preamble_psk4[2];

// ������������� �������� �������� (������������ ��� ��� ������������)
extern xip_real pwr_constell_psk4;
extern xip_real pwr_constell_psk2;
extern xip_real pwr_constell_psk2_60;

// ������� �������
extern xip_complex nearest_point_psk4(const xip_complex& in);

extern int nearest_index_psk4(const xip_complex& in);

extern xip_complex nearest_point_psk2(const xip_complex& in);

extern int nearest_index_psk2(const xip_complex& in);

extern xip_complex nearest_point_psk2_60(const xip_complex& in);

extern int nearest_index_psk2_60(const xip_complex& in);

// ���������� ��� ����������� ���������
extern void set_current_constell(Current_constell cur_cnstl);

// �������� ������ ������� �� ������������� ���������
extern xip_complex get_cur_constell_sample(unsigned int symbol);

// �������� ������ ������� ��������� �� ������������� ���������
extern xip_complex get_cur_constell_preamble_sample(unsigned int symbol);

#endif // CONSTELLATION_H
