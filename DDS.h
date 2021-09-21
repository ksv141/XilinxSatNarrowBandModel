#ifndef DDS_H
#define DDS_H

#include <stdexcept>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "dds_compiler_v6_0_bitacc_cmodel.h"
#include "debug.h"

/**
 * @brief схема прямого цифрового синтеза
 * 2 режима: 
 *	- одноканальный
 *	- симметричный двухканальный (вниз и вверх с одинаковым набегом фазы) 
*/
class DDS
{
public:
	/**
	 * @brief инициализатор
	 * @param phase_modulus диапазон изменения фазы [0, phase_modulus]
	 * @param two_symmmetric_channels симметричный двухканальный режим (вниз и вверх с одинаковым набегом фазы)
	*/
	DDS(int phase_modulus, bool two_symmmetric_channels = false);

	~DDS();

	/**
	 * @brief одноканальная генерация
	 * @param dph набег фазы --> [0, phase_modulus]
	*/
	int process(double dph, xip_complex& out);

	/**
	 * @brief двухканальная генерация (вверх и вниз)
	 * @param dph набег фазы вверх --> [0, phase_modulus], вниз берется симметрично относительно phase_modulus
	*/
	int process(double dph, xip_complex& out_up, xip_complex& out_down);

	/**
	 * @brief возвращает ширину возвращаемых sin/cos в битах
	 * @return 
	*/
	unsigned int getOutputWidth();

private:
	int init_dds_lib(unsigned channels);
	int destroy_dds_lib();

	xip_dds_v6_0_config dds_cnfg;
	xip_dds_v6_0* dds_model;

	xip_array_real* dds_in;		// 3-D массив, содержащий текущий отсчет для обработки
	xip_array_real* dds_out;	// 3-D массив, содержащий результат обработки

	xip_uint no_of_input_fields;	// число полей во входной структуре
	xip_uint no_of_output_fields;	// число полей в выходной структуре

	int m_phaseModulus;				// диапазон изменения фазы [0, 16383] --> [0, 2pi]
};

#endif // DDS_H