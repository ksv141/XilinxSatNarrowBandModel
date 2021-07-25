#pragma once

#include "array_func_templates.h"
#include "dsp_2_3/dsp/coeffs/rc_x2.hpp"

//#include "dsp_0_Template.h"
//
//#include "fir_compiler_v7_2_bitacc_cmodel.h"
//
//#include "xip_arrays.h"
//
//namespace xilinx_m
//{
//	enum type_filter{real, complex, other};
//
//	/*template <typename base_T, 
//	class filter
//	{
//
//	};*/
//
//	struct rc_filter_params
//	{
//		xip_fir_v7_2_config fir_config;
//
//		const double* arr_coeff;
//		unsigned char n_coeffs;
//
//		//type_filter type_filter;
//
//		unsigned int in_data_buff_size;
//	};
//
//	class rc_filter
//	{
//	private:
//		rc_filter_params flt_params;
//		xip_fir_v7_2* fir;
//
//		//xip_array_real* xip_arr_real_out;
//
//		unsigned short up;
//		unsigned short up_decim;
//
//	public:
//		rc_filter(rc_filter_params flt_params_)
//		{
//			flt_params = flt_params_;
//
//			switch (flt_params.fir_config.filter_type)
//			{
//				case XIP_FIR_SINGLE_RATE:
//					up = 1; 
//					up_decim = 1;
//					break;
//
//				case XIP_FIR_INTERPOLATION:
//					up = flt_params.fir_config.interp_rate; 
//					up_decim = 1;
//					break;
//
//				case XIP_FIR_DECIMATION:
//					up = 1; up_decim = flt_params.fir_config.decim_rate;
//					break;
//
//				default:
//					up = 1; up_decim = 1;
//					break;
//			}
//
//			/*switch (flt_params_.type_filter)
//			{
//				case real:
//					flt_params.fir_config.num_paths = 1;
//					flt_params.fir_config.num_channels = 1;
//					break;
//
//				case complex:
//					flt_params.fir_config.num_paths = 1;
//					flt_params.fir_config.num_channels = 2;
//					break;
//
//				default:
//					break;
//			}*/
//
//			if (flt_params_.arr_coeff != NULL)
//			{
//				unsigned int n_coeffs = flt_params_.n_coeffs;
//
//				double* new_coeffs = new double[n_coeffs];
//
//				for (int i = 0; i < n_coeffs; i++)
//				{
//					new_coeffs[i] = flt_params_.arr_coeff[i];
//				}
//
//				tC::set_capacity_data_in_bclock(new_coeffs, n_coeffs, flt_params.fir_config.coeff_width);
//
//				flt_params.fir_config.coeff = new_coeffs;
//				flt_params.fir_config.num_coeffs = n_coeffs;
//			}
//
//			fir = xip_fir_v7_2_create(&flt_params.fir_config, NULL, 0);  //объект фильтра (описатель)
//			if (!fir) {
//				printf("Error creating instance\n", flt_params.fir_config.name);
//			}
//			else {
//				printf("Created instance\n", flt_params.fir_config.name);
//			}
//
//
//			//xip_arr_real_out = xip_array_real_create();          //резервирование 
//			//xip_array_real_reserve_dim(xip_arr_real_out, 3); //3d arr
//			//xip_arr_real_out->dim_size = 3;
//			//xip_arr_real_out->dim[0] = flt_params.fir_config.num_paths;
//			//xip_arr_real_out->dim[1] = flt_params.fir_config.num_channels;
//
//			//flt_params.in_data_buff_size = flt_params.in_data_buff_size * up / up_decim;
//
//			//xip_arr_real_out->dim[2] = flt_params.in_data_buff_size;
//			//xip_arr_real_out->data_size = xip_arr_real_out->dim[0] * xip_arr_real_out->dim[1] * xip_arr_real_out->dim[2];
//
//			//if (xip_array_real_reserve_data(xip_arr_real_out, xip_arr_real_out->data_size) == XIP_STATUS_OK)
//			//	printf("Reserved data\n");
//			//else
//			//	printf("Unable to reserve data!\n");
//		}
//
//		void process( xip_arr_real& in_data, xip_arr_real& out_data)
//		{
//			/*if (in_data->data_size * up / up_decim!= xip_arr_real_out->data_size)
//			{
//				std::cout << "FIR_filter: ERROR set in data" << std::endl;
//			}
//
//			if (out_data->data_size != xip_arr_real_out->data_size)
//			{
//				std::cout << "FIR_filter: ERROR set out data" << std::endl;
//			}*/
//
//			xip_fir_v7_2_data_send(fir, in_data.get_xip_array());
//			xip_fir_v7_2_data_get(fir, out_data.get_xip_array(), 0);
//
//			//*out_data = *xip_arr_real_out;
//		}
//
//		~rc_filter()
//		{
//			//xip_array_real_destroy(xip_arr_real_out);
//			xip_fir_v7_2_destroy(fir);		
//		}
//
//		static rc_filter_params create_default_config()
//		{
//			rc_filter_params fir_cnfg;
//
//			xip_fir_v7_2_default_config(&fir_cnfg.fir_config);
//
//			return fir_cnfg;
//		}
//	};
//};