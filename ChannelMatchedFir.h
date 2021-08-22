#ifndef CHANNELMATCHEDFIR_H
#define CHANNELMATCHEDFIR_H

#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"
#include "debug.h"

extern int init_channel_matched_fir();

extern int destroy_channel_matched_fir();

// для тестирования
extern int process_data_channel_matched_fir();

extern int process_sample_channel_matched_transmit(xip_complex* in, xip_complex* out);

extern int process_sample_channel_matched_receive(xip_complex* in, xip_complex* out);

#endif // CHANNELMATCHEDFIR_H