#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include "cmpy_v6_0_bitacc_cmodel.h"
#include "fir_compiler_v7_2_bitacc_cmodel.h"

extern void msg_print(void* dummy, int error, const char* msg);

//---------------------------------------------------------------------------------------------------------------------
//Print a xip_array_real
extern void print_array_real(const xip_array_real* x);

//---------------------------------------------------------------------------------------------------------------------
// Print a xip_array_complex
extern void print_array_complex(const xip_array_complex* x);

//----------------------------------------------------------------------------------------------------------------------
// String arrays used by the print_config funtion
extern const char* filt_desc[5];
extern const char* seq_desc[2];

//----------------------------------------------------------------------------------------------------------------------
// Print a summary of a filter configuration
extern int print_config(const xip_fir_v7_2_config* cfg);

#endif // DEBUG_H