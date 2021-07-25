#pragma once
#include <type_traits>

#include "xip_common_bitacc_cmodel.h"

namespace xilinx_m
{
	class xip_arr_real
	{
	private:

		int* dim_params;
		int* dim_index;
		int* min_dim_size;
		int n_dim;

		xip_array_real* data_real;

	public:
		template <typename... dim>
		xip_arr_real(int one_d_size, dim... Dimensions)
			
		{
			n_dim = sizeof...(Dimensions);
			n_dim += 1;
			dim_params = new int[n_dim];
			dim_params[0] = one_d_size;

			int index = 1;
			((dim_params[index++] = Dimensions), ...);

			dim_index = new int[n_dim];

			min_dim_size = new int[n_dim];

			min_dim_size[0] = 1;
			for (int i = 1; i < n_dim; i++)
			{
				min_dim_size[i] = dim_params[i - 1] * min_dim_size[i - 1];
			}

			data_real = xip_array_real_create();          //резервирование 
			xip_array_real_reserve_dim(data_real, n_dim); //Nd arr
			data_real->dim_size = n_dim;

			data_real->data_size = 1;

			for (int i = 0; i < n_dim; i++)
			{
				data_real->dim[i] = dim_params[i];
				data_real->data_size *= data_real->dim[i];
			}

			xip_array_real_reserve_data(data_real, data_real->data_size);
		}

		~xip_arr_real()
		{
			delete[] min_dim_size;
			delete[] dim_params;
			delete[] dim_index;

			xip_array_real_destroy(data_real);
		}

		template<typename T, typename... Idx>
		xip_real get_smpl(T first, Idx... idx)
		{
			dim_index[0] = first;
			int index = 1;
			((dim_index[index++] = idx), ...);

			index = 0;

			for (int i = 0; i < n_dim; i++)
			{
				index += min_dim_size[i] * dim_index[i];
			}
			
			return data_real->data[index];
		}

		template<typename T, typename... Idx>
		void set_smpl(double smpl, T first, Idx... idx)
		{
			dim_index[0] = first;
			int index = 1;
			((dim_index[index++] = idx), ...);

			index = 0;

			for (int i = 0; i < n_dim; i++)
			{
				int di = dim_index[i];
				int mi = min_dim_size[i];
				index += min_dim_size[i] * dim_index[i];
			}

			data_real->data[index] = smpl;
		}

		inline int get_all_data_size()
		{
			return data_real->data_size;
		}

		inline double* get_data()
		{
			return data_real->data;
		}

		inline xip_array_real* get_xip_array()
		{
			return data_real;
		}

		inline int get_data_capacity()
		{
			return data_real->data_capacity;
		}
	};

	class xip_arr_complex
	{
	private:

		int* dim_params;
		int* dim_index;
		int* min_dim_size;
		int n_dim;

		xip_array_complex* data_complex;

	public:
		template <typename... dim>
		xip_arr_complex(int one_d_size, dim... Dimensions)
		{
			n_dim = sizeof...(Dimensions);
			n_dim += 1;
			dim_params = new int[n_dim];
			dim_params[0] = one_d_size;

			int index = 1;
			((dim_params[index++] = Dimensions), ...);

			dim_index = new int[n_dim];

			min_dim_size = new int[n_dim];

			min_dim_size[0] = 1;
			for (int i = 1; i < n_dim; i++)
			{
				min_dim_size[i] = dim_params[i - 1] * min_dim_size[i - 1];
			}

			data_complex = xip_array_complex_create();          //резервирование 
			xip_array_complex_reserve_dim(data_complex, n_dim); //Nd arr
			data_complex->dim_size = n_dim;

			data_complex->data_size = 1;

			for (int i = 0; i < n_dim; i++)
			{
				data_complex->dim[i] = dim_params[i];
				data_complex->data_size *= data_complex->dim[i];
			}

			xip_array_complex_reserve_data(data_complex, data_complex->data_size);
		}

		~xip_arr_complex()
		{
			delete[] min_dim_size;
			delete[] dim_params;
			delete[] dim_index;

			xip_array_complex_destroy(data_complex);
		}

		template<typename T, typename... Idx>
		xip_complex get_smpl(T first, Idx... idx)
		{
			dim_index[0] = first;
			int index = 1;
			((dim_index[index++] = idx), ...);

			index = 0;

			for (int i = 0; i < n_dim; i++)
			{
				index += min_dim_size[i] * dim_index[i];
			}

			return data_complex->data[index];
		}

		template<typename T, typename... Idx>
		void set_smpl(xip_complex smpl, T first, Idx... idx)
		{
			dim_index[0] = first;
			int index = 1;
			((dim_index[index++] = idx), ...);

			index = 0;

			for (int i = 0; i < n_dim; i++)
			{
				int di = dim_index[i];
				int mi = min_dim_size[i];
				index += min_dim_size[i] * dim_index[i];
			}

			data_complex->data[index] = smpl;
		}

		inline int get_all_data_size(){return data_complex->data_size;}

		inline xip_complex* get_data(){return data_complex->data;}

		inline xip_array_complex* get_xip_array(){return data_complex;}

		inline int get_dimension_size(){return n_dim;}
		inline int get_data_capacity(){return data_complex->data_capacity;}
	};

};
