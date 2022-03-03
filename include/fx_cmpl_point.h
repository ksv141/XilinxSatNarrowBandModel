#ifndef FXCMPLPOINT_H
#define FXCMPLPOINT_H

#include <iostream>
#include <limits> 
#include <cmath>
#include <complex>
#include <algorithm>

#include <cassert>

#include "fx_point.h"

namespace xilinx_m
{
    class fx_cmpl_point
    {

        typedef int64_t base_type;

    private:
        int8_t cap_num;
        int8_t size_i_pt;
    public:

        friend fx_point;

        fx_point real;
        fx_point imag;

        double d_real = 0;
        double d_imag = 0;

        fx_cmpl_point()
            : real(0, 0), imag(0, 0),
            cap_num(0), size_i_pt(0)
        {

        }

        fx_cmpl_point(uint8_t capacity, uint8_t s_i_pt)
            : real(capacity, s_i_pt), imag(capacity, s_i_pt),
            cap_num(capacity), size_i_pt(s_i_pt)
        {

        }

        void set_capacity(uint8_t new_cap)
        {
            real.set_capacity(new_cap);
            imag.set_capacity(new_cap);

            cap_num = real.get_capacity();
            size_i_pt = real.get_size_i_pt();

            set_double_val();
        }

        inline uint8_t get_capacity()
        {
            return real.get_capacity();
        }

        void set_point_pos(uint8_t new_pos)
        {
            real.set_point_pos(new_pos);
            imag.set_point_pos(new_pos);

            size_i_pt = real.get_size_i_pt();

            set_double_val();
            set_double_val();
        }

        fx_cmpl_point& operator= (const fx_cmpl_point& copy_)
        {
            real = copy_.real;
            imag = copy_.imag;

            set_double_val();

            return *this;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        fx_cmpl_point& operator= (const std::complex<T>& copy_)
        {
            real = copy_.real();
            imag = copy_.imag();

            set_double_val();

            return *this;
        }

         void set_double_val()
         {
             d_real = real.d_val;
             d_imag = imag.d_val;
         }

        template<typename T1, typename T2 = T1>
        void set_val(T1 r_val, T2 i_val)
        {
            /*set_size_param();
            set_mask();*/

            real.set_val(r_val);
            imag.set_val(i_val);

            set_double_val();
        }

        fx_cmpl_point operator-() const
        {
            fx_cmpl_point out = *this;
            out.real *= -1;
            out.imag *= -1;

            out.set_double_val();

            return out;
        }

        fx_cmpl_point operator+ (fx_cmpl_point const& right_operand_)
        {
            fx_point new_real = real + right_operand_.real;
            fx_point new_imag = imag + right_operand_.imag;

            fx_cmpl_point out(new_real.get_capacity(), new_real.get_size_i_pt());
            out.real = new_real;
            out.imag = new_imag;

            out.set_double_val();

            return out;
        }

        fx_cmpl_point operator+ (fx_point const& right_operand_)
        {
            fx_point new_real = real + right_operand_;
            fx_point new_imag = real + right_operand_;

            int8_t out_capacity = std::max(new_real.get_capacity(), new_imag.get_capacity());
            int8_t out_size_i_pt = std::max(new_real.get_size_i_pt(), new_imag.get_size_i_pt());

            fx_cmpl_point out(out_capacity, out_size_i_pt);
            out.real = new_real;
            out.imag = new_imag;

            out.set_double_val();

            return out;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        fx_cmpl_point operator+ (T const& right_operand_) const
        {
            int8_t r_capacity = std::numeric_limits<T>::digits;
            fx_point r_operand_(r_capacity, r_capacity);
            r_operand_.set_val(right_operand_);

            fx_cmpl_point out = *this + r_operand_;

            return out;
        }

        template<typename T, 
            std::enable_if_t<std::is_integral<T>::value || std::is_same<T, fx_point>::value || std::is_same<T, fx_cmpl_point>::value, bool> = true>
            fx_cmpl_point& operator +=(T const& right_operand)
        {
            fx_cmpl_point out = *this + right_operand;
            *this = out;

            return *this;
        }

        friend fx_cmpl_point round(fx_cmpl_point& in, int8_t n_skip_bits)
        {
            fx_cmpl_point out = in;

            out.real.set_capacity(out.real.get_capacity() - n_skip_bits);
            out.imag.set_capacity(out.imag.get_capacity() - n_skip_bits);

            return out;
        }

        fx_cmpl_point operator* (fx_cmpl_point const& right_operand_)
        {
            fx_point new_real = (real * right_operand_.real) - (imag * right_operand_.imag);
            fx_point new_imag = (real * right_operand_.imag) + (imag * right_operand_.real);

            int8_t out_capacity = std::max(new_real.get_capacity(), new_imag.get_capacity());
            int8_t out_size_i_pt = std::max(new_real.get_size_i_pt(), new_imag.get_size_i_pt());

            fx_cmpl_point out(out_capacity, out_size_i_pt);
            out.real = new_real;
            out.imag = new_imag;

            out.set_double_val();

            return out;
        }

        fx_cmpl_point operator* (fx_point const& right_operand_)
        {
            fx_point new_real = real * right_operand_;
            fx_point new_imag = imag * right_operand_;

            int8_t out_capacity = std::max(new_real.get_capacity(), new_imag.get_capacity());
            int8_t out_size_i_pt = std::max(new_real.get_size_i_pt(), new_imag.get_size_i_pt());

            fx_cmpl_point out(out_capacity, out_size_i_pt);

            out.real = new_real;
            out.imag = new_imag;

            out.set_double_val();

            return out;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        fx_cmpl_point operator* (T const& right_operand_)
        {
            int8_t r_capacity = std::numeric_limits<T>::digits;
            fx_point r_operand_(r_capacity, r_capacity);
            r_operand_.set_val(right_operand_);

            fx_cmpl_point out = *this * r_operand_;

            return out;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        fx_cmpl_point operator* (std::complex<T> const& right_operand_)
        {
            fx_point new_real = real * right_operand_.real();
            new_real -=imag * right_operand_.imag();

            fx_point new_imag = real * right_operand_.imag() + imag * right_operand_.real();

            int8_t out_capacity = std::max(new_real.get_capacity(), new_imag.get_capacity());
            int8_t out_size_i_pt = std::max(new_real.get_size_i_pt(), new_imag.get_size_i_pt());

            fx_cmpl_point out(out_capacity, out_size_i_pt);

            out.real = new_real;
            out.imag = new_imag;

            out.set_double_val();

            return out;
        }

        template<typename T,
            std::enable_if_t<std::is_integral<T>::value || std::is_same<T, fx_point>::value || std::is_same<T, fx_cmpl_point>::value, bool> = true>
        fx_cmpl_point operator*= (T const& right_operand_)
        {
            fx_cmpl_point out = *this * right_operand_;
            *this = out;

            return *this;
        }

        template<typename T,
            std::enable_if_t<std::is_integral<T>::value, bool> = true>
            fx_cmpl_point operator*= (std::complex<T> const& right_operand_)
        {
            fx_cmpl_point out = *this * right_operand_;
            *this = out;

            return *this;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        fx_cmpl_point operator / (T const& right_operand_)
        {
            assert(is_num_pw_two(right_operand_) == true, 
                "fx_cmpl_point clas ERROR: operator '/' right_operand NOT pw_two");

            fx_cmpl_point out = *this;

            out.real /= right_operand_;
            out.imag /= right_operand_;

            out.set_double_val();

            return out;
        }

        template<typename T,
            std::enable_if_t<std::is_integral<T>::value || std::is_same<T, fx_point>::value || std::is_same<T, fx_cmpl_point>::value, bool> = true>
            fx_cmpl_point operator/= (T const& right_operand_)
        {
            *this = *this / right_operand_;

            return *this;
        }

        fx_cmpl_point conj()
        {
            fx_cmpl_point out = *this;
            out.imag.i_val *= -1;
            out.set_double_val();

            return out;
        }

        void move_point_pos(uint8_t new_point_pos)
        {
            real.move_point_pos(new_point_pos);
            imag.move_point_pos(new_point_pos);

            set_double_val();
        }

        void resize(uint8_t new_cap, uint8_t new_point_pos)
        {
            set_capacity(new_cap);
            move_point_pos(new_point_pos);
        }
    };

    /*fx_cmpl_point conj(fx_cmpl_point in)
    {
        fx_cmpl_point out = in;
        out.imag.i_val *= -1;
        out.imag.d_val *= -1;
        out.set_double_val();

        return out;
    }*/

    template<typename T, 
        std::enable_if_t<std::is_same<T, fx_point>::value || std::is_same<T, fx_cmpl_point>::value, bool> = true>
        bool fx_create_arr(T*& arr, int64_t size_, uint8_t cap_, uint8_t size_i_pt_)
    {
        if (arr)
            delete[]arr;

        arr = 0;

        arr = new T[size_];

        if (arr == NULL)
        {
            std::cout << "ERROR create fx_array" << std::endl;

            return NULL;
        }

        for (int i = 0; i < size_; i++)
        {
            arr[i].resize(cap_, size_i_pt_);
        }

        return true;
    }

    template<typename T,
        std::enable_if_t<std::is_same<T, fx_point>::value || std::is_same<T, fx_cmpl_point>::value, bool> = true>
        bool fx_init_arr(T arr[], int64_t size_, uint8_t cap_, uint8_t size_i_pt_)
    {
        if (arr == NULL)
        {
            std::cout << "ERROR initialize fx_array" << std::endl;

            return NULL;
        }

        for (int i = 0; i < size_; i++)
        {
            arr[i].resize(cap_, size_i_pt_);
        }

        return true;
    }

    fx_cmpl_point conj(fx_cmpl_point& in);

    fx_point power(fx_cmpl_point& in);

    fx_point abs(fx_cmpl_point& in);

    void print(fx_cmpl_point* arr, int size_);
};

#endif // FXCMPLPOINT_H