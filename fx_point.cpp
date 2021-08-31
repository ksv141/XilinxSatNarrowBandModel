#include "fx_point.h"

namespace xilinx_m
{
    fx_point abs(fx_point& p)
    {
        fx_point out = p;
        out.i_val = std::abs(out.i_val);
        out.set_double_val();

        return out;
    }

    void print(fx_point* arr, int size_)
    {
        for (int i = 0; i < size_; i++)
        {
            std::cout << arr[i].i_val << " [" << arr[i].d_val << "] " << " ";
        }

        std::cout << std::endl;
    }
}