#include "DDS.h"

DDS::DDS()
{
	init_dds_lib();
}

DDS::~DDS()
{
	destroy_dds_lib();
}

int DDS::process(double dph, double& out_phase, double& out_sin, double& out_cos)
{
	if (xip_dds_v6_0_xip_array_real_set_data(dds_in, dph, 0, 0, 0) != XIP_STATUS_OK) {
		printf("ERROR: Could not set data to dds input array\n");
		return -1;
	}

	if (xip_dds_v6_0_data_do(dds_model,		//pointer to c model instance
		dds_in,								//pointer to input data structure
		dds_out,							//pointer to output structure
		dds_in->dim[0],						//first dimension of either data structure
		dds_cnfg.Channels,					//2nd dimension of either data structure
		no_of_input_fields,					//3rd dimension of input
		no_of_output_fields					//3rd dimension of output
	) != XIP_STATUS_OK) {
		printf("ERROR: C model did not complete successfully\n");
		return -1;
	}

	xip_real value;
	if (xip_dds_v6_0_xip_array_real_get_data(dds_out, &value, 0, 0, 0) != XIP_STATUS_OK) {
		printf("ERROR: Could not get data from dds output array\n");
		return -1;
	}
	out_phase = value;

	if (xip_dds_v6_0_xip_array_real_get_data(dds_out, &value, 0, 0, 1) != XIP_STATUS_OK) {
		printf("ERROR: Could not get data from dds output array\n");
		return -1;
	}
	out_sin = value;

	if (xip_dds_v6_0_xip_array_real_get_data(dds_out, &value, 0, 0, 2) != XIP_STATUS_OK) {
		printf("ERROR: Could not get data from dds output array\n");
		return -1;
	}
	out_cos = value;

	return 0;
}

/**
 * @brief ������������� ���������� xip dds
 * @return 
*/
int DDS::init_dds_lib()
{
	xip_status status = xip_dds_v6_0_default_config(&dds_cnfg);

	if (status != XIP_STATUS_OK) {
		printf("ERROR: Could not get C model default configuration\n");
		return -1;
	}

	dds_cnfg.name = "dds_compiler_v6_0";
	dds_cnfg.PartsPresent = XIP_DDS_PHASE_GEN_AND_SIN_COS_LUT;
	dds_cnfg.DDS_Clock_Rate = 100.0;
	dds_cnfg.Channels = 1;
	dds_cnfg.Mode_of_Operation = XIP_DDS_MOO_CONVENTIONAL; // XIP_DDS_MOO_RASTERIZED;
	dds_cnfg.Modulus = 3000;
	dds_cnfg.ParameterEntry = XIP_DDS_HARDWARE_PARAMS;
	dds_cnfg.Spurious_Free_Dynamic_Range = 45.0;
	dds_cnfg.Frequency_Resolution = 0.4;
	dds_cnfg.Noise_Shaping = XIP_DDS_NS_AUTO;
	dds_cnfg.Phase_Increment = XIP_DDS_PINCPOFF_STREAM;	// � ���� � ����� ������� ���� ����� ��������� �������� 
	dds_cnfg.Resync = XIP_DDS_ABSENT;
	dds_cnfg.Phase_Offset = XIP_DDS_PINCPOFF_NONE;
	dds_cnfg.Output_Selection = XIP_DDS_OUT_SIN_AND_COS;
	dds_cnfg.Negative_Sine = XIP_DDS_ABSENT;			// �������� ����������, ����� ����� �������� ����������-����������� ������
	dds_cnfg.Negative_Cosine = XIP_DDS_ABSENT;			
	dds_cnfg.Amplitude_Mode = XIP_DDS_FULL_RANGE;	// ��� 16-��� ����� ����� � ��������� [-0.99994, +0.99994]
	dds_cnfg.Memory_Type = XIP_DDS_MEM_AUTO;
	dds_cnfg.Optimization_Goal = XIP_DDS_OPTGOAL_AUTO;
	dds_cnfg.DSP48_Use = XIP_DDS_DSP_MIN;
	dds_cnfg.Has_TREADY = XIP_DDS_ABSENT;				// ???????????
	dds_cnfg.S_CONFIG_Sync_Mode = XIP_DDS_CONFIG_SYNC_VECTOR; // ????????????
	dds_cnfg.Output_Form = XIP_DDS_OUTPUT_TWOS;
	dds_cnfg.Latency_Configuration = XIP_DDS_LATENCY_AUTO;
	dds_cnfg.Has_ARESETn = XIP_DDS_ABSENT;
	dds_cnfg.PINC[0] = 0; 
	// dds_cnfg.PINC[1]					= 0;
	// ...
	dds_cnfg.POFF[0] = 0;
	// dds_cnfg.POFF[1]					= 0;
	// ...
	dds_cnfg.Latency = 4;
	dds_cnfg.Output_Width = 16;
	dds_cnfg.Phase_Width = 16;

	xip_dds_v6_0_config_pkt* pinc_poff_config = nullptr;

	// Create model object with your particular configuration
	dds_model = xip_dds_v6_0_create(&dds_cnfg, &msg_print, 0);
	if (!dds_model) {
		printf("Error creating instance %s\n", dds_cnfg.name);
		return -1;
	}

	//------------------------------------------------------------------------------------
	// Set up fields and reserve memory for data and config packets, for this configuration
	//------------------------------------------------------------------------------------

	//Calculate the number of input fields
	no_of_input_fields = 0;
	if (dds_cnfg.PartsPresent == XIP_DDS_SIN_COS_LUT_ONLY) {
		no_of_input_fields++; //Phase_In
	}
	else {
		if (dds_cnfg.Phase_Increment == XIP_DDS_PINCPOFF_STREAM) {
			no_of_input_fields++; //PINC
			if (dds_cnfg.Resync == XIP_DDS_PRESENT) {
				no_of_input_fields++; //RESYNC
			}
		}
		if (dds_cnfg.Phase_Offset == XIP_DDS_PINCPOFF_STREAM) {
			no_of_input_fields++; //POFF
		}
	}

	//Calculate the number of output fields
	no_of_output_fields = 0; //phase output is not optional in the c model.
	if (dds_cnfg.PartsPresent != XIP_DDS_SIN_COS_LUT_ONLY) {
		no_of_output_fields = 1; //PHASE_OUT
	}
	if (dds_cnfg.PartsPresent != XIP_DDS_PHASE_GEN_ONLY) {
		if (dds_cnfg.Output_Selection == XIP_DDS_OUT_SIN_ONLY) no_of_output_fields++; //SIN
		if (dds_cnfg.Output_Selection == XIP_DDS_OUT_COS_ONLY) no_of_output_fields++; //COS
		if (dds_cnfg.Output_Selection == XIP_DDS_OUT_SIN_AND_COS) no_of_output_fields += 2; //SIN and COS
	}

	// Create input data packet
	dds_in = xip_array_real_create();
	xip_array_real_reserve_dim(dds_in, 3); //dimensions are (Number of samples, channels, PINC/POFF/Phase)
	dds_in->dim_size = 3;
	dds_in->dim[0] = 1;		// �� ������ �������
	dds_in->dim[1] = dds_cnfg.Channels;
	dds_in->dim[2] = no_of_input_fields;
	dds_in->data_size = dds_in->dim[0] * dds_in->dim[1] * dds_in->dim[2];
	if (xip_array_real_reserve_data(dds_in, dds_in->data_size) != XIP_STATUS_OK) {
		printf("ERROR: Unable to reserve memory for input data packet!\n");
		return -1;
	}

	// Request memory for output data
	dds_out = xip_array_real_create();
	xip_array_real_reserve_dim(dds_out, 3); //dimensions are (Number of samples, channels, PINC/POFF/Phase)
	dds_out->dim_size = 3;
	dds_out->dim[0] = 1;
	dds_out->dim[1] = dds_cnfg.Channels;
	dds_out->dim[2] = no_of_output_fields;
	dds_out->data_size = dds_out->dim[0] * dds_out->dim[1] * dds_out->dim[2];
	if (xip_array_real_reserve_data(dds_out, dds_out->data_size) != XIP_STATUS_OK) {
		printf("ERROR: Unable to reserve memory for output data packet!\n");
		return -1;
	}

	return 0;
}

/**
 * @brief ��������������� ���������� xip dds
 * @return 
*/
int DDS::destroy_dds_lib()
{
	if (xip_array_real_destroy(dds_in) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_array_real_destroy(dds_out) != XIP_STATUS_OK) {
		return -1;
	}
	if (xip_dds_v6_0_destroy(dds_model) != XIP_STATUS_OK) {
		return -1;
	}

	printf("Deleted instance of DDS and free memory\n");

	return 0;
}
