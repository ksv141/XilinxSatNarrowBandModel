#include "DDS.h"

DDS::DDS()
{
	init_dds_lib();
}

DDS::~DDS()
{
	destroy_dds_lib();
}

/**
 * @brief инициализация библиотеки xip dds
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
	dds_cnfg.Mode_of_Operation = XIP_DDS_MOO_RASTERIZED;
	dds_cnfg.Modulus = 3000;
	dds_cnfg.ParameterEntry = XIP_DDS_HARDWARE_PARAMS;
	dds_cnfg.Spurious_Free_Dynamic_Range = 45.0;
	dds_cnfg.Frequency_Resolution = 0.4;
	dds_cnfg.Noise_Shaping = XIP_DDS_NS_NONE;
	dds_cnfg.Phase_Increment = XIP_DDS_PINCPOFF_FIXED;
	dds_cnfg.Resync = XIP_DDS_ABSENT;
	dds_cnfg.Phase_Offset = XIP_DDS_PINCPOFF_NONE;
	dds_cnfg.Output_Selection = XIP_DDS_OUT_SIN_AND_COS;
	dds_cnfg.Negative_Sine = XIP_DDS_ABSENT;
	dds_cnfg.Negative_Cosine = XIP_DDS_ABSENT;
	dds_cnfg.Amplitude_Mode = XIP_DDS_FULL_RANGE;
	dds_cnfg.Memory_Type = XIP_DDS_MEM_AUTO;
	dds_cnfg.Optimization_Goal = XIP_DDS_OPTGOAL_AUTO;
	dds_cnfg.DSP48_Use = XIP_DDS_DSP_MIN;
	dds_cnfg.Has_TREADY = XIP_DDS_ABSENT;
	dds_cnfg.S_CONFIG_Sync_Mode = XIP_DDS_CONFIG_SYNC_VECTOR;
	dds_cnfg.Output_Form = XIP_DDS_OUTPUT_TWOS;
	dds_cnfg.Latency_Configuration = XIP_DDS_LATENCY_AUTO;
	dds_cnfg.Has_ARESETn = XIP_DDS_ABSENT;
	dds_cnfg.PINC[0] = 1; // 1101000110110111
	// dds_cnfg.PINC[1]					= 0;
	// ...
	dds_cnfg.POFF[0] = 0;
	// dds_cnfg.POFF[1]					= 0;
	// ...
	dds_cnfg.Latency = 4;
	dds_cnfg.Output_Width = 8;
	dds_cnfg.Phase_Width = 12;

	xip_dds_v6_0_config_pkt* pinc_poff_config = nullptr;

	// Create model object with your particular configuration
	dds_model = xip_dds_v6_0_create(&dds_cnfg, &msg_print, 0);
	if (!dds_model) {
		printf("Error creating instance %s\n", dds_cnfg.name);
		return -1;
	}

	return 0;
}

/**
 * @brief деинициализация библиотеки xip dds
 * @return 
*/
int DDS::destroy_dds_lib()
{
	if (xip_dds_v6_0_destroy(dds_model) != XIP_STATUS_OK) {
		return -1;
	}

	printf("Deleted instance of DDS and free memory\n");

	return 0;
}
