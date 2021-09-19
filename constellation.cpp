#include "constellation.h"

// используемый вид созвезди€
Current_constell current_constell = Current_constell::PSK2;
      
// нормальное значение уровн€ сигнала
const xip_real constell_psk_norm_val = 4096;

// сигнальное созвездие дл€ данных (PSK4)
xip_complex constell_psk4[4] = { {constell_psk_norm_val, constell_psk_norm_val},
								{constell_psk_norm_val, -constell_psk_norm_val},
								{-constell_psk_norm_val, constell_psk_norm_val},
								{-constell_psk_norm_val, -constell_psk_norm_val} };

// сигнальное созвездие дл€ данных (PSK2 +/-90)
xip_complex constell_psk2[2] = { {constell_psk_norm_val, 0},
								{-constell_psk_norm_val, 0} };

// сигнальное созвездие дл€ данных (PSK2 +/-60)
xip_complex constell_psk2_60[2] = { {4096, 2365},
								   {-4096, 2365} };

// сигнальное созвездие дл€ преамбулы (режим PSK4)
xip_complex constell_preamble_psk4[2] = { {constell_psk_norm_val, constell_psk_norm_val},
										{-constell_psk_norm_val, -constell_psk_norm_val} };

xip_complex* constell_current_ref = constell_psk2;
xip_complex* constell_preamble_current_ref = constell_psk2;

// нормированна€ мощность сигналов (используетс€ дл€ ј–” демодул€тора)
xip_real pwr_constell_psk4 = 2 * constell_psk4[0].re * constell_psk4[0].re;
xip_real pwr_constell_psk2 = constell_psk2[0].re * constell_psk2[0].re;
xip_real pwr_constell_psk2_60 = constell_psk2_60[0].re * constell_psk2_60[0].re + constell_psk2_60[0].im * constell_psk2_60[0].im;

xip_real pwr_constell_current = pwr_constell_psk2;

// жесткое решение
xip_complex nearest_point_psk4(const xip_complex& in)
{
	return constell_psk4[nearest_index_psk4(in)];
}

int nearest_index_psk4(const xip_complex& in)
{
	if (in.re < 0) {
		if (in.im < 0)
			return 3;
		else
			return 2;
	}
	else {
		if (in.im < 0)
			return 1;
		else
			return 0;
	}
}

xip_complex nearest_point_psk2(const xip_complex& in)
{
	return constell_psk2[nearest_index_psk2(in)];
}

int nearest_index_psk2(const xip_complex& in)
{
	if (in.re < 0) {
		return 1;
	}
	else {
		return 0;
	}
}

xip_complex nearest_point_psk2_60(const xip_complex& in)
{
	return constell_psk2_60[nearest_index_psk2_60(in)];
}

int nearest_index_psk2_60(const xip_complex& in)
{
	return nearest_index_psk2(in);
}

void set_current_constell(Current_constell cur_cnstl)
{
	current_constell = cur_cnstl;
	if (cur_cnstl == PSK2) {
		constell_current_ref = constell_psk2;
		constell_preamble_current_ref = constell_psk2;
		pwr_constell_current = pwr_constell_psk2;
	}
	else if (cur_cnstl == PSK2_60) {
		constell_current_ref = constell_psk2_60;
		constell_preamble_current_ref = constell_psk2_60;
		pwr_constell_current = pwr_constell_psk2_60;
	}
	else if (cur_cnstl == PSK4) {
		constell_current_ref = constell_psk4;
		constell_preamble_current_ref = constell_preamble_psk4;
		pwr_constell_current = pwr_constell_psk4;
	}
}

xip_complex get_cur_constell_sample(unsigned int symbol)
{
	return constell_current_ref[symbol];
}

xip_complex get_cur_constell_preamble_sample(unsigned int symbol)
{
	return constell_preamble_current_ref[symbol];
}

xip_real get_cur_constell_pwr()
{
	return pwr_constell_current;
}
