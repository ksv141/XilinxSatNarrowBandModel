#ifndef FXPOINT_H
#define FXPOINT_H

#include <iostream>
#include <limits> 
#include <cmath>
#include <algorithm>

#define _USE_MATH_DEFINES

#include <cassert>

namespace xilinx_m
{
    template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
    bool is_num_pw_two(T val, uint8_t& mult_pw_two)
    {
        int64_t InitValue = 1;
        mult_pw_two = 0;

        while (InitValue < val)
        {
            InitValue *= 2;
            mult_pw_two++;
        }

        if (InitValue == val)
            return true;
    }

    template <typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        bool is_num_pw_two(T val)
    {
        int InitValue = 1;

        while (InitValue < val)
            InitValue *= 2;

        if (InitValue == val)
            return true;

        return false;
    }

    template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true> 
    uint8_t get_pos_high_bit(const T val)
    {
        if (val == 0)
            return 0;

        int64_t res = 1;
        uint8_t pos = 0;

        while (res < val)
        {
            pos++;
            res *=2; // или res = res << 1
        }

        return pos+1;
    }

    template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
    T round(T val, int8_t n_skip_bits)
    {
        assert((n_skip_bits <= (int8_t)std::numeric_limits<T>::digits) && (n_skip_bits >= 0)
            , "ERROR fx_point class: round error - new_size >= base_type size");

        if (n_skip_bits == 0)
            return val;

        int8_t diff_size = n_skip_bits;
        //int8_t diff_new_base_size = std::numeric_limits<base_type>::digits - new_size;

        T new_val = 0;

        T high_bit_skip_part = val >> (diff_size - 1);   //берем старший бит отбрасываемой части
        high_bit_skip_part &= (T)1;

        new_val = val >> diff_size;    //отбрасываем лишние биты
        new_val += high_bit_skip_part;
        new_val <<= diff_size;          //смешаем число к начальному положению

        return new_val;
    }

    enum fx_num_scaling_capacity_mod { scaling, not_scaling };

    class fx_point
    {

         typedef int64_t base_type;

    private:
        int8_t size_i_pt = 0;
        int8_t size_f_pt = 0;
        int8_t cap_num = 0;
        int8_t cap_bt = 0;

        base_type max_all_num = 0;
        base_type max_i_pt_num = 0;
        base_type max_f_pt_num = 0;

        base_type mask;

        int8_t diff_cap_bt = 0;
        int8_t diff_f_bt = 0;

        void set_size_param()
        {
            max_i_pt_num = pow(2, size_i_pt);
            max_f_pt_num = pow(2, size_f_pt);

            cap_num = size_i_pt + size_f_pt;

            max_all_num = pow(2, cap_num);

            diff_cap_bt = cap_bt - cap_num;
            diff_f_bt = cap_bt - size_f_pt;
        }

        void set_mask()
        {
            mask = std::numeric_limits<base_type>::max();

            for (int shift = 1; shift <= diff_f_bt; shift++)
            {
                mask &= ~((base_type)1 << (cap_bt - shift));
            }
        }

    public:

        base_type i_val;
        double d_val;

        void set_double_val()
        {
            base_type int_part = i_val >> size_f_pt;
            base_type float_part = i_val & mask;

            float d_float_part = (float)float_part / max_f_pt_num;

            if (float_part == 0 && max_f_pt_num == 0)
                d_float_part = 0;

            d_val = (double)int_part + d_float_part;
        }

        ~fx_point() {}

        fx_point()
            : size_i_pt(0), size_f_pt(0), cap_num(0), cap_bt(std::numeric_limits<base_type>::digits)
        {
            i_val = 0;

            set_size_param();
            set_mask();
            set_double_val();
        }

        fx_point(uint8_t capacity, uint8_t s_i_pt)
            : size_i_pt(s_i_pt), size_f_pt(capacity - s_i_pt), cap_num(size_i_pt + size_f_pt), 
            cap_bt(std::numeric_limits<base_type>::digits)
        {
            assert(capacity <= sizeof(base_type) * 8 - std::is_signed<base_type>::value,
                "ERROR fx_point class: capasity > base_type"); //

            assert(cap_num <= sizeof(base_type) * 8 - std::is_signed<base_type>::value,
                "ERROR fx_point class: int + fix > base_type"); //

            assert(std::is_integral<base_type>::value,
                "ERROR fx_point class: only integrer base_type"); //

            i_val = 0;

            set_size_param();
            set_mask();
            set_double_val();
        }

        template<typename T>
        fx_point(T val, uint8_t capacity, uint8_t s_i_pt)
            : size_i_pt(s_i_pt), size_f_pt(capacity - s_i_pt), cap_num(size_i_pt + size_f_pt),
            cap_bt(std::numeric_limits<base_type>::digits)
        {
            assert(capacity <= sizeof(base_type) * 8 - std::is_signed<base_type>::value,
                "ERROR fx_point class: capasity > base_type"); //

            assert(cap_num <= sizeof(base_type) * 8 - std::is_signed<base_type>::value,
                "ERROR fx_point class: int + fix > base_type"); //

            assert(std::is_integral<base_type>::value,
                "ERROR fx_point class: only integrer base_type"); //

            set_size_param();
            set_mask();
            set_double_val();

            set_val(val);
        }

        inline uint8_t get_capacity() const
        {
            return cap_num;
        }

        inline uint8_t get_size_f_pt() const 
        {
            return size_f_pt;
        }

        inline uint8_t get_size_i_pt() const 
        {
            return size_i_pt;
        }

        inline base_type get_i_part() const
        {
            return i_val >> size_f_pt;
        }

        inline base_type get_f_part() const
        {
            /*base_type new_i_val = i_val << diff_f_bt;
            new_i_val >>= diff_f_bt;*/

            return i_val & mask;
        }

        friend fx_point round(fx_point& in, int8_t n_skip_bits)
        {
            fx_point out = in;
            out.set_capacity(in.get_capacity() - n_skip_bits);

            return out;
        }
        void set_capacity(int8_t new_cap) 
        {
            assert((new_cap <= (int8_t)std::numeric_limits<base_type>::digits) && (new_cap > 0)
                , "ERROR fx_point class: new capacity error - new_cap >= base_type size");

            int8_t diff_cap = new_cap - cap_num;

            if (diff_cap == 0)
                return;

            if (diff_cap > 0)   //Если увеличиваем разрядность
            {
                size_i_pt += diff_cap;
            }

            /****/
            else if (diff_cap < 0)  //Если уменьшаем разрядность
            {
                diff_cap *= (int8_t)-1;

                size_f_pt -= diff_cap;

                i_val = round(i_val, diff_cap);
                i_val >>= diff_cap;

                if (size_f_pt < 0)
                {
                    size_i_pt += size_f_pt;
                    size_f_pt = 0;
                }

            }

            set_size_param();
            set_mask();
            set_double_val();
        }

        void resize(uint8_t new_cap, uint8_t new_point_pos)
        {
            set_capacity(new_cap);
            move_point_pos(new_point_pos);
        }

        template<typename T>
        void set_val(T val)    
        {
            /*set_size_param();
            set_mask();*/

            //assert(val <= (base_type)pow(2.0, cap_num), "ERROR fx_point class: input i_p_val > max_i_pt_num");
            //assert(val <= max_f_pt_num, "ERROR fx_point class: input i_p_val > max_i_pt_num");

            /*i_val = (base_type)val << size_f_pt;*/

            i_val = (base_type)val;

            set_double_val();
        }

        void set_val(fx_point& copy_)
        {
            *this = copy_;
        }

        template<>
        void set_val<double>(double d_val)
        //void set_val(double d_val)
        {
            /*set_size_param();
            set_mask();*/

            assert((base_type)d_val <= max_i_pt_num, "ERROR fx_point class: input i_p_val > max_i_pt_num");

            double d_i_pt_val = (base_type)d_val;
            double d_f_pt_val = 0;
            d_f_pt_val = std::modf(d_val, &d_i_pt_val);

            base_type i_pt_val = d_i_pt_val;
            /*base_type f_pt_val = (base_type)std::round(d_f_pt_val * max_f_pt_num);*/
            base_type f_pt_val = (base_type)std::round(d_f_pt_val * max_f_pt_num);

            i_val = (base_type)i_pt_val << size_f_pt;
            i_val += (base_type)f_pt_val;

            /********Уекаем все что больше capacity**********/
            i_val <<= diff_cap_bt;
            i_val >>= diff_cap_bt;
            /************************************************/

            set_double_val();
        }

        fx_point& operator= (const fx_point& copy_)
        {
            /*set_size_param();
            set_mask();*/

            int8_t copy_s_f_pt = copy_.get_size_f_pt();
            base_type copy_i_pt = copy_.get_i_part();
            base_type copy_f_pt = copy_.get_f_part();

            if (copy_i_pt > max_i_pt_num)
            {
                std::cout<<"ERROR fx_point: operator= - OVERLOW"<<std::endl;
                assert(false);
            }           

            i_val = copy_i_pt;
            i_val <<= size_f_pt;

            int8_t diff_f_pt = size_f_pt - copy_s_f_pt;

            if (diff_f_pt >= 0) //если float part копируемого объекта меньше, умножаем на 2 diff_f_pt раз
            {
                i_val += copy_f_pt << diff_f_pt;
            }
            else
            {
                diff_f_pt *= -1;
                copy_f_pt = round(copy_f_pt, diff_f_pt);
                copy_f_pt >>= diff_f_pt;
                i_val += copy_f_pt;
            }

            /*set_size_param();
            set_mask();*/
            set_double_val();

            return *this;
        }

        fx_point& operator=(double const& d_val)
        {
            assert((base_type)d_val <= max_i_pt_num, "ERROR fx_point class: input i_p_val > max_i_pt_num");

            double d_i_pt_val = (base_type)d_val;
            double d_f_pt_val = 0;
            d_f_pt_val = std::modf(d_val, &d_i_pt_val);

            base_type i_pt_val = d_i_pt_val;
            base_type f_pt_val = (base_type)std::round(d_f_pt_val * max_f_pt_num);

            i_val = (base_type)i_pt_val << size_f_pt;
            i_val += (base_type)f_pt_val;

            /********Уекаем все что больше capacity**********/
            i_val <<= diff_cap_bt;
            i_val >>= diff_cap_bt;
            /************************************************/

            set_double_val();

            return *this;
        }

        void set_point_pos(uint8_t new_point_pos)   //
        {
            int8_t diff_i_part = new_point_pos - size_i_pt;

            size_i_pt = new_point_pos;
            size_f_pt = cap_num - size_i_pt;

            if (diff_i_part >= 0)   //если целая часть болше предыдущей, смещаем вправо и округляем
            {
                i_val = round(i_val, diff_i_part);
                i_val >>= diff_i_part;
            }
            else//если целая часть меньше предыдущей, отбрасываем лишние биты и проверяем не будет ли переполнения
            {
                assert(max_i_pt_num >= get_i_part(),
                "ERROR fx_point: set new point pos - OVERLOW");

                i_val <<= -diff_i_part;
            }

            set_size_param();
            set_mask();
            set_double_val();
        }

        void move_point_pos(uint8_t new_point_pos)
        {
            size_i_pt = new_point_pos;
            size_f_pt = cap_num - size_i_pt;

            if (size_f_pt < 0)
            {
                size_i_pt = cap_num;
                size_f_pt = 0;
            }

            set_size_param();
            set_mask();
            set_double_val();
        }

        bool operator < (fx_point const& right_operand_) const
        {
            int8_t out_capacity = std::max((uint8_t)cap_num, right_operand_.get_capacity());
            int8_t out_size_i_pt = std::max((uint8_t)size_i_pt, right_operand_.get_size_i_pt());

            fx_point left_avrg(out_capacity, out_size_i_pt);
            left_avrg = *this;

            fx_point right_avrg(out_capacity, out_size_i_pt);
            right_avrg = right_operand_;

            return left_avrg.i_val < right_avrg.i_val;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        bool operator < (T const& right_operand_) const
        {
            base_type int_part = get_i_part();
            base_type float_part = get_f_part();

            if (int_part == right_operand_)
            {
                if (float_part > 0)
                    return true;
                else
                    return false;
            }

            return int_part < (base_type)right_operand_;
        }

        template<typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
        bool operator < (T const& right_operand_) const
        {
            return d_val < right_operand_.d_val;
        }

        bool operator == (fx_point const& right_operand_) const
        {
            int8_t out_capacity = std::max((uint8_t)cap_num, right_operand_.get_capacity());
            int8_t out_size_i_pt = std::max((uint8_t)size_i_pt, right_operand_.get_size_i_pt());

            fx_point left_avrg(out_capacity, out_size_i_pt);
            left_avrg = *this;

            fx_point right_avrg(out_capacity, out_size_i_pt);
            right_avrg = right_operand_;

            return left_avrg.i_val == right_avrg.i_val;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        bool operator == (T const& right_operand_) const
        {
            base_type int_part = get_i_part();
            base_type float_part = get_f_part();

            if (int_part == right_operand_)
            {
                if (float_part == 0)
                    return true;
                else
                    return false;
            }

            return int_part == (base_type)right_operand_;
        }

        template<typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
        bool operator == (T const& right_operand_) const
        {
            return d_val == right_operand_.d_val;
        }

        template<typename T,
            std::enable_if_t<std::is_arithmetic<T>::value, bool> = true>
            bool operator > (T const& right_operand_) const
        {
            return right_operand_ < this->i_val;
        }

        template<typename T,
            std::enable_if_t<std::is_same<T, fx_point>::value, bool> = true>
            bool operator > (T const& right_operand_) const
        {
            return right_operand_ < *this;
        }

        fx_point operator+ (fx_point const& right_operand_) const
        {
            if (right_operand_ == 0)
                return *this;

            uint8_t out_size_i_pt = std::max((uint8_t)size_i_pt, right_operand_.get_size_i_pt()) + 1;
            uint8_t out_size_f_pt = std::max((uint8_t)size_f_pt, right_operand_.get_size_f_pt());

            uint8_t out_capacity = out_size_i_pt + out_size_f_pt;

            if (out_capacity > cap_bt)
            {
                out_capacity = cap_bt;

                if (out_size_i_pt > cap_bt)
                    out_size_i_pt = cap_bt;
            }

            fx_point out(*this, out_capacity, out_size_i_pt);

            int8_t diff_f_pt = out.get_size_f_pt() - right_operand_.get_size_f_pt();
            base_type centred_r_val = right_operand_.i_val;

            if (diff_f_pt >= 0)
            {
                centred_r_val <<= diff_f_pt;
            }
            else if (diff_f_pt < 0)
            {
                centred_r_val >>= std::abs(diff_f_pt);
            }

            out.i_val += centred_r_val;
            out.set_double_val();

            return out;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        fx_point operator+ (T const& right_operand_) const
        {
            int8_t r_capacity = get_pos_high_bit(right_operand_);// std::numeric_limits<T>::digits;
            fx_point r_operand_(r_capacity, r_capacity);
            r_operand_.set_val(right_operand_);

            fx_point out = *this + r_operand_;

            return out;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value || std::is_same<T, fx_point>::value, bool> = true>
        fx_point& operator +=(T const& right_operand)
        {
            fx_point out = *this + right_operand;
            *this = out;

            return *this;
        }

        fx_point operator-() const
        {
            fx_point out = *this;
            out.i_val *= -1;
            out.set_double_val();

            return out;
        }

        template<typename T, 
            std::enable_if_t<std::is_integral<T>::value || std::is_same<T, fx_point>::value, bool> = true>
        fx_point operator- (T const& right_operand_) const
        {
            return *this + (T)-right_operand_;
        }

        template<typename T, 
            std::enable_if_t<std::is_integral<T>::value || std::is_same<T, fx_point>::value, bool> = true>
        fx_point& operator -=(T const& right_operand_)
        {
            fx_point out = *this + -right_operand_;
            *this = out;

            return *this;
        }

        fx_point operator* (const fx_point& right_operand_) const
        {
            uint8_t out_capacity = cap_num + right_operand_.get_capacity();
            uint8_t out_size_i_pt = size_i_pt +  right_operand_.get_size_i_pt();

            if (out_capacity > cap_bt)
            {
                out_capacity = cap_bt;
                
                if(out_size_i_pt > cap_bt)
                    out_size_i_pt = cap_bt;

                fx_point temp_right_operand_(right_operand_, out_capacity - cap_num, right_operand_.get_size_i_pt());

                fx_point out(out_capacity, out_size_i_pt);
                out.i_val = i_val;

                if (right_operand_.i_val == 0)
                {
                    out.set_val(0);
                    return out;
                }

                out.i_val *= temp_right_operand_.i_val;
                out.set_double_val();

                return out;
            }

            fx_point out(out_capacity, out_size_i_pt);
            out.i_val = i_val;

            if (right_operand_.i_val == 0)
            {
                out.set_val(0);
                return out;
            }

            out.i_val *= right_operand_.i_val;
            out.set_double_val();

            return out;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        fx_point operator * (T const& right_operand_) const 
        {
            uint8_t r_capacity = get_pos_high_bit(right_operand_);// std::numeric_limits<T>::digits;
            fx_point fx_right_operand_(r_capacity, r_capacity);
            fx_right_operand_ = right_operand_;

            fx_point out = *this * fx_right_operand_;

            return out;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value || std::is_same<T, fx_point>::value, bool> = true>
        fx_point operator *= (T const& right_operand_)
        {
            fx_point out = *this * right_operand_;
            *this = out;

            return *this;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        fx_point operator / (T const& right_operand_) const
        {
            uint8_t num_mult_pw_two = 0;
            assert(is_num_pw_two(right_operand_, num_mult_pw_two) || right_operand_ == 1,
                "operator / ERROR: right operand not mult pw two");

            is_num_pw_two(right_operand_, num_mult_pw_two);

            fx_point out = *this;
            out.i_val = round(out.i_val, num_mult_pw_two);
            out.i_val >>= num_mult_pw_two;
            out.set_double_val();

            return out;
        }

        template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
        fx_point operator /= (T const& right_operand_)
        {
            fx_point out = *this / right_operand_;
            *this = out;

            return *this;
        }

        fx_point abs() const
        {
            fx_point out = *this;
            out.i_val = std::abs(out.i_val);
            out.set_double_val();

            return out;
        }
    };

    const fx_point fx_PI(3.14159265358979323846, 63, 2);
    const fx_point fx_TWO_PI(2.0 * 3.14159265358979323846, 63, 3);

    fx_point abs(fx_point& p);

    void print(fx_point* arr, int size_);
};

#endif // FXPOINT_H