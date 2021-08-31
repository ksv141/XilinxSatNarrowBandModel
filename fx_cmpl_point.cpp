#include "fx_cmpl_point.h"

namespace xilinx_m
{
    fx_cmpl_point conj(fx_cmpl_point& in)
    {
        fx_cmpl_point out = in;
        out.imag.i_val *= -1;
        out.imag.d_val *= -1;
        out.set_double_val();

        return out;
    }

	fx_point power(fx_cmpl_point& in)
	{
		fx_point out = in.real * in.real + in.imag * in.imag;

		return out;
	}

	fx_point abs(fx_cmpl_point& in)//!!!!!
	{
		fx_point out = in.real * in.real + in.imag * in.imag;

		return out;
	}

	void print(fx_cmpl_point* arr, int size_)
	{
		std::cout << std::endl;

		for (int i = 0; i < size_; i++)
		{
			std::cout << arr[i].real.i_val << " " << arr[i].imag.i_val << std::endl;
		}

		std::cout << std::endl;
	}
}