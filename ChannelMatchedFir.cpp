#include "ChannelMatchedFir.h"

// +---------------+-------------+-------+--------+-------+--------------+
// |     Type      |   MagSpec   | Fc/Fs | Rollof | Order |    Window    |
// +---------------+-------------+-------+--------+-------+--------------+
// | raised-cosine | square root |  1/4  |  .25   |  18   | Kaiser, B=.5 |
// +---------------+-------------+-------+--------+-------+--------------+
const double rc_root_x2_25_19[19] =
{
	-0.0013789727382389502,
	 0.010108258970470938,
	-0.0088158332983470158,
	-0.018254667807044128,
	 0.032040090881415004,
	 0.026209138464556331,
	-0.08457623076097491,
	-0.032022493980703122,
	 0.31066605580209167,
	 0.53415494309189537,
	 0.31066605580209167,
	-0.032022493980703122,
	-0.08457623076097491,
	 0.026209138464556331,
	 0.032040090881415004,
	-0.018254667807044128,
	-0.0088158332983470158,
	 0.010108258970470938,
	-0.0013789727382389502
};

xip_fir_v7_2* fir_channel_matched;

// инициализация канального/согласованного фильтра (SRRC)
void init_channel_matched_fir()
{
	xip_fir_v7_2_config fir_cnfg;
	xip_fir_v7_2_default_config(&fir_cnfg);
	fir_cnfg.name = "srrc_fir";
	fir_cnfg.filter_type = XIP_FIR_SINGLE_RATE;
	fir_cnfg.coeff = rc_root_x2_25_19;
	fir_cnfg.num_coeffs = 19;
	fir_cnfg.quantization = XIP_FIR_QUANTIZED_ONLY;
	fir_cnfg.output_rounding_mode = XIP_FIR_FULL_PRECISION;

	//Create filter instances
	fir_channel_matched = xip_fir_v7_2_create(&fir_cnfg, &msg_print, 0);
	if (!fir_channel_matched) {
		printf("Error creating instance %s\n", fir_cnfg.name);
		return;
	}
	else {
		printf("Created instance %s\n", fir_cnfg.name);
	}
}

void destroy_channel_matched_fir()
{
	xip_fir_v7_2_destroy(fir_channel_matched);
	printf("Deleted instance of SRRC\n");
}
