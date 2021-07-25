//  (c) Copyright 2013 Xilinx, Inc. All rights reserved.
//
//  This file contains confidential and proprietary information
//  of Xilinx, Inc. and is protected under U.S. and
//  international copyright and other intellectual property
//  laws.
//
//  DISCLAIMER
//  This disclaimer is not a license and does not grant any
//  rights to the materials distributed herewith. Except as
//  otherwise provided in a valid license issued to you by
//  Xilinx, and to the maximum extent permitted by applicable
//  law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
//  WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
//  AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
//  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
//  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
//  (2) Xilinx shall not be liable (whether in contract or tort,
//  including negligence, or under any other theory of
//  liability) for any loss or damage of any kind or nature
//  related to, arising under or in connection with these
//  materials, including for any direct, or any indirect,
//  special, incidental, or consequential loss or damage
//  (including loss of data, profits, goodwill, or any type of
//  loss or damage suffered as a result of any action brought
//  by a third party) even if such damage or loss was
//  reasonably foreseeable or Xilinx had been advised of the
//  possibility of the same.
//
//  CRITICAL APPLICATIONS
//  Xilinx products are not designed or intended to be fail-
//  safe, or for use in any application requiring fail-safe
//  performance, such as life-support or safety devices or
//  systems, Class III medical devices, nuclear facilities,
//  applications related to the deployment of airbags, or any
//  other applications that could lead to death, personal
//  injury, or severe property or environmental damage
//  (individually and collectively, "Critical
//  Applications"). Customer assumes the sole risk and
//  liability of any use of Xilinx products in Critical
//  Applications, subject only to applicable laws and
//  regulations governing limitations on product liability.
//
//  THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
//  PART OF THIS FILE AT ALL TIMES.
//-----------------------------------------------------------------------------

#ifndef XIP_CMPY_V6_0_BITACC_CMODEL_H
#define XIP_CMPY_V6_0_BITACC_CMODEL_H

// Common typedefs, constants and functions for use across Xilinx bit-accurate C models
#undef XIP_XILINX_XIP_TARGET
#define XIP_XILINX_XIP_TARGET cmpy_v6_0
#include "xip_common_bitacc_cmodel.h"
#include "xip_mpz_bitacc_cmodel.h"

#ifdef __cplusplus
extern "C" {
#endif

//Depreciated; these functions now defined within model
#define DEFINE_XIP_ARRAY(TYPE)

//Depreciated; use xip_status/XIP_STATUS_* instead
typedef xip_status xip_cmpy_v6_0_status;
#define XIP_CMPY_V6_0_STATUS_OK    XIP_STATUS_OK
#define XIP_CMPY_V6_0_STATUS_ERROR XIP_STATUS_ERROR

// Definition for RoundMode
#define XIP_CMPY_V6_0_TRUNCATE   0
#define XIP_CMPY_V6_0_ROUND      1

// Definitions for input/output data width limits
#define XIP_CMPY_V6_0_MIN_IN_DATA_WIDTH   8
#define XIP_CMPY_V6_0_MAX_IN_DATA_WIDTH   63
#define XIP_CMPY_V6_0_MIN_OUT_DATA_WIDTH  17
#define XIP_CMPY_V6_0_MAX_OUT_DATA_WIDTH  128

// cmpy_v6_0 configuration structure
typedef struct
{
  const char *name;    //@- Instance name (arbitrary)
  int APortWidth;      //@- Data width for real and imag components of input A
  int BPortWidth;      //@- Data width for real and imag components of input B
  int OutputWidth;     //@- Data width for real and imag components of result
  int RoundMode;
  int debug;
} xip_cmpy_v6_0_config;

// cmpy_v6_0 handle type (opaque to user)
struct _xip_cmpy_v6_0;
typedef struct _xip_cmpy_v6_0 xip_cmpy_v6_0;

//Helpers for accessing data
XIP_XILINX_XIP_IMPEXP const xip_status xip_cmpy_v6_0_xip_array_uint_set_data(xip_array_uint* p, const xip_uint  value, size_t sample);
XIP_XILINX_XIP_IMPEXP const xip_status xip_cmpy_v6_0_xip_array_uint_get_data(const xip_array_uint* p, xip_uint* value, size_t sample);
XIP_XILINX_XIP_IMPEXP const xip_status xip_cmpy_v6_0_xip_array_complex_set_data(xip_array_complex* p, const xip_complex  value, size_t sample);
XIP_XILINX_XIP_IMPEXP const xip_status xip_cmpy_v6_0_xip_array_complex_get_data(const xip_array_complex* p, xip_complex* value, size_t sample);
XIP_XILINX_XIP_IMPEXP const xip_status xip_cmpy_v6_0_xip_array_mpz_complex_set_data(xip_array_mpz_complex* p, const xip_mpz_complex  value, size_t sample);
XIP_XILINX_XIP_IMPEXP const xip_status xip_cmpy_v6_0_xip_array_mpz_complex_get_data(const xip_array_mpz_complex* p, xip_mpz_complex* value, size_t sample);

/**
 * Fill in a configuration structure with the core's default values.
 *
 * @param     config     The configuration structure to be populated
 * @returns   Exit code  XIP_STATUS_*
 */
XIP_XILINX_XIP_IMPEXP
const xip_status xip_cmpy_v6_0_default_config(xip_cmpy_v6_0_config *config);

/**
 * Get version of model.
 *
 * @returns   String  Textual representation of model version
 */
XIP_XILINX_XIP_IMPEXP
const char* xip_cmpy_v6_0_get_version(void);

/**
 * Create a new instance of the core based on some configuration values.
 *
 * @param     config      Pointer to a xip_cmpy_v6_0_config structure
 * @param     handler     Callback function for errors and warnings (providing a null
 *                        pointer means no messages are output)
 * @param     handle      Optional argument to be passed back to callback function
 *
 * @returns   Exit code   XIP_STATUS_*
 */
XIP_XILINX_XIP_IMPEXP
xip_cmpy_v6_0* xip_cmpy_v6_0_create(
  const xip_cmpy_v6_0_config *config,
  xip_msg_handler             handler,
  void                       *handle
);

/**
 * Execute the model. Complex data type. Use this for configurations
 * where the natural output width does not exceed 53 bits.
 *
 * @param     s           Pointer to xip_cmpy_v6_0 state structure
 * @param     req         Pointer to xip_cmpy_v6_0_data_req request structure
 * @param     resp        Pointer to xip_cmpy_v6_0_data_resp response structure
 * @returns   Exit code   XIP_STATUS_*
 */
XIP_XILINX_XIP_IMPEXP
const xip_status xip_cmpy_v6_0_data_do
  ( xip_cmpy_v6_0  *s,
    xip_array_complex *reqa,
    xip_array_complex *reqb,
    xip_array_uint *reqctrl,
    xip_array_complex *resp
  );

/**
 * Execute the model. mpz_complex data type. This can be used for any configurations
 * but may not be as fast as the variant using simpler types
 *
 * @param     s           Pointer to xip_cmpy_v6_0 state structure
 * @param     req         Pointer to xip_cmpy_v6_0_data_req request structure
 * @param     resp        Pointer to xip_cmpy_v6_0_data_resp response structure
 * @returns   Exit code   XIP_STATUS_*
 */
XIP_XILINX_XIP_IMPEXP
const xip_status xip_cmpy_v6_0_mpz_data_do
  ( xip_cmpy_v6_0  *s,
    xip_array_mpz_complex *reqa,
    xip_array_mpz_complex *reqb,
    xip_array_uint *reqctrl,
    xip_array_mpz_complex *resp
  );

/**
 * Destroy an instance of the core and free any resources allocated.
 *
 * @param     s           Pointer to xip_cmpy_v6_0 state structure
 * @returns   Exit code   XIP_STATUS_*
 *
 */
XIP_XILINX_XIP_IMPEXP
const xip_status xip_cmpy_v6_0_destroy(xip_cmpy_v6_0 *s);

/**
 * Fill a configuration structure with the core's current state
 */
XIP_XILINX_XIP_IMPEXP
const xip_status xip_cmpy_v6_0_get_config
  ( xip_cmpy_v6_0 *s,
    xip_cmpy_v6_0_config *config
  );



#ifdef __cplusplus
}
#endif

#endif
