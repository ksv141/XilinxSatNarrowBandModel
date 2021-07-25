#pragma once

#include <cstdlib>

#include "xip_arrays.h"

#include "cordic_v6_0_bitacc_cmodel.h"

#include "fx_point.h"
#include "fx_cmpl_point.h"

namespace xilinx_m
{
	enum type_cordic
	{
		xip_rotate,
		xip_translate,
		xip_sin_cos,
		xip_atan,
		xip_sinh_cosh,
		xip_atanh,
		xip_sqrt
	};

	template <type_cordic Type_cordic>
	class cordic
	{
	private:
		xip_cordic_v6_0_config params;
		xip_cordic_v6_0* xip_cordic;

	public:
		cordic()
		{

		}

		void set_param(xip_cordic_v6_0_config c_p)
		{
			params = c_p;
			params.CordicFunction = Type_cordic;

			xip_cordic = xip_cordic_v6_0_create(&params, NULL, 0);
		}

		cordic(xip_cordic_v6_0_config c_p)
			: params(c_p)
		{
			params.CordicFunction = Type_cordic;

			/*switch (Type_cordic)
			{
			case xip_sqrt:
				params.DataFormat = 0;
				break;
			default:
				break;
			}*/

			xip_cordic = xip_cordic_v6_0_create(&params, NULL, 0);
		}
		~cordic()
		{
			xip_cordic_v6_0_destroy(xip_cordic);
		}

		template<type_cordic T = Type_cordic, std::enable_if_t<T == Type_cordic && T == xip_rotate, bool> = true>
		bool process(xip_arr_complex& in, xip_arr_real& phase_in, xip_arr_complex& out)
		{
			if (xip_cordic_v6_0_rotate(xip_cordic, in.get_xip_array(), phase_in.get_xip_array(), out.get_xip_array(), in.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				return false;
			}

			return true;
		}

		template<type_cordic T = Type_cordic, std::enable_if_t<T == Type_cordic && T == xip_translate, bool> = true>
		bool process(xip_arr_complex& in, xip_arr_real& mag_out, xip_arr_real& phase_out)
		{
			if (xip_cordic_v6_0_translate(xip_cordic, in.get_xip_array(), mag_out.get_xip_array(), phase_out.get_xip_array(), in.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				return false;
			}

			return true;
		}

		template<type_cordic T = Type_cordic, std::enable_if_t<T == Type_cordic && T == xip_atan, bool> = true>
		bool process(xip_arr_complex& in, xip_arr_real& phase_out)
		{
			if (xip_cordic_v6_0_atan(xip_cordic, in.get_xip_array(), phase_out.get_xip_array(), in.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				return false;
			}

			return true;
		}

		template<type_cordic T = Type_cordic, std::enable_if_t<T == Type_cordic && T == xip_atan, bool> = true>
		bool process(fx_cmpl_point& in, fx_point& phase_out)
		{
			xip_arr_real phase_out_arr(1);
			xip_arr_complex in_arr(1);

			xip_complex in_smpl;
			in_smpl.re = in.real.i_val;
			in_smpl.im = in.imag.i_val;

			in_arr.set_smpl(in_smpl, 0);

			if (xip_cordic_v6_0_atan(xip_cordic, in_arr.get_xip_array(), phase_out_arr.get_xip_array(), in_arr.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				return false;
			}

			phase_out.set_val((int64_t)phase_out_arr.get_smpl(0));

			return true;
		}

		template<type_cordic T = Type_cordic, std::enable_if_t<T == Type_cordic && T == xip_sinh_cosh, bool> = true>
		bool process(xip_arr_real& phase_in, xip_arr_complex& out)
		{
			if (xip_cordic_v6_0_sinh_cosh(xip_cordic, phase_in.get_xip_array(), out.get_xip_array(), phase_in.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				return false;
			}

			return true;
		}

		template<type_cordic T = Type_cordic, std::enable_if_t<T == Type_cordic && T == xip_atanh, bool> = true>
		bool process(xip_arr_complex& in, xip_arr_real& phase_out)
		{
			if (xip_cordic_v6_0_atanh(xip_cordic, in.get_xip_array(), phase_out.get_xip_array(), in.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				return false;
			}

			return true;
		}

		template<type_cordic T = Type_cordic, std::enable_if_t<T == Type_cordic && T == xip_sin_cos, bool> = true>
		bool process(xip_arr_real& phase_in, xip_arr_complex& out)
		{
			if (xip_cordic_v6_0_sin_cos(xip_cordic, phase_in.get_xip_array(), out.get_xip_array(), phase_in.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				//return false;
			}

			//return true;
		}

		template<type_cordic T = Type_cordic, std::enable_if_t<T == Type_cordic && T == xip_sin_cos, bool> = true>
		fx_cmpl_point process(fx_point& phase_in)
		{
			//phase_in.resize(params.InputWidth - 1, 2);
			xip_arr_real phase_in_arr(1);
			xip_arr_complex sin_cos_arr(1);

			phase_in_arr.set_smpl(phase_in.i_val, 0);

			if (xip_cordic_v6_0_sin_cos
			(xip_cordic, phase_in_arr.get_xip_array(), sin_cos_arr.get_xip_array(), phase_in_arr.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				//return false;
			}

			fx_cmpl_point out(params.OutputWidth - 1, 1);
			out.set_val((int64_t)sin_cos_arr.get_smpl(0).re, (int64_t)sin_cos_arr.get_smpl(0).im);

			return out;
		}

		template<type_cordic T = Type_cordic, std::enable_if_t<T == Type_cordic && T == xip_sqrt, bool> = true>
		bool process(xip_arr_real& in, xip_arr_real& phase_out)
		{
			if (xip_cordic_v6_0_sqrt(xip_cordic, in.get_xip_array(), phase_out.get_xip_array(), in.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				return false;
			}

			return true;
		}

		template<type_cordic T = Type_cordic, std::enable_if_t<T == Type_cordic && T == xip_sqrt, bool> = true>
		fx_point process(fx_point& in)
		{
			xip_arr_real in_arr(1);
			xip_arr_real out_arr(1);

			in_arr.set_smpl(in.i_val, 0);

			if (xip_cordic_v6_0_sqrt(xip_cordic, in_arr.get_xip_array(), out_arr.get_xip_array(), in_arr.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				//return false;
			}

			fx_point out;

			int8_t out_cap;
			int8_t out_size_i_pt;

			switch (params.DataFormat)
			{
			case 0:	//входные числа > 1 ¬х. отсчЄт. фикс. точка N:0 дробна€ часть отсутствует
				out_cap = in.get_capacity() / 2 + in.get_capacity() % 2;
				out_size_i_pt = in.get_size_i_pt() / 2 + in.get_size_i_pt() % 2;

				out.resize(out_cap, out_size_i_pt);
				break;

			case 1://входные числа < 1	¬х. отсчЄт. фикс. точка 0:N цела€ часть отсутствует
				out.resize(params.OutputWidth - 1, 0);
				break;
			}

			out.set_val((int64_t)out_arr.get_smpl(0));

			return out;
		}

		/*****************ѕќѕ–ќЅќ¬ј“№ —ѕ≈÷»јЋ»«ј÷»ё**************/
		/*template<> bool process<xip_rotate>(xip_arr_complex& in, xip_arr_real& phase_in, xip_arr_complex& out)
		{
			if (xip_cordic_v6_0_rotate(xip_cordic, in.get_xip_array(), phase_in.get_xip_array(), out.get_xip_array(), in.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				return false;
			}
		}

		template<> bool process<xip_translate>(xip_arr_complex& in, xip_arr_real& mag_out, xip_arr_real& phase_out)
		{
			if (xip_cordic_v6_0_translate(xip_cordic, in.get_xip_array(), mag_out.get_xip_array(), phase_out.get_xip_array(), in.get_all_data_size())
				!= XIP_STATUS_OK)
			{
				return false;
			}

			return true;
		}*/
	};
};