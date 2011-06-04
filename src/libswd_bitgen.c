/*
 * $Id$
 *
 * Serial Wire Debug Open Library.
 * Library Body File.
 *
 * Copyright (C) 2010-2011, Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the Tomasz Boleslaw CEDRO nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.*
 *
 * Written by Tomasz Boleslaw CEDRO <cederom@tlen.pl>, 2010-2011;
 *
 */

/** \file libswd_bitgen.c */

#include <libswd.h>


/*******************************************************************************
 * \defgroup swd_bitgen SWD Bitstream / Packet Payload generation routines.
 * @{
 ******************************************************************************/

/** Generate 8-bit SWD-REQUEST packet contents with provided parameters.
 * Note that parity bit value is calculated automatically.
 * \param *swdctx swd context pointer.
 * \param *APnDP AccessPort (high) or DebugPort (low) access type pointer.
 * \param *RnW Read (high) or Write (low) operation type pointer.
 * \param *addr target register address value pointer.
 * \param *request pointer where to store resulting packet.
 * \return number of generated packets (1), or SWD_ERROR_CODE on failure.
 */
int swd_bitgen8_request(swd_ctx_t *swdctx, char *APnDP, char *RnW, char *addr, char *request){
 /* Verify function parameters.*/
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (*APnDP!=0 && *APnDP!=1) return SWD_ERROR_APnDP;
 if (*RnW!=0 && *RnW!=1) return SWD_ERROR_RnW;
 if (*addr<SWD_ADDR_MINVAL && *addr>SWD_ADDR_MAXVAL) return SWD_ERROR_ADDR;

 /* Build request header content. */
 unsigned char reqhdr=0;
 char parity, req;
 int res;
 reqhdr|=(((*addr&(1<<2))?1:0)<<SWD_REQUEST_A2_BITNUM);
 reqhdr|=(((*addr&(1<<3))?1:0)<<SWD_REQUEST_A3_BITNUM);
 reqhdr|=((*APnDP?1:0)<<SWD_REQUEST_APnDP_BITNUM);
 reqhdr|=(((*RnW?1:0)<<SWD_REQUEST_RnW_BITNUM));
 req=reqhdr;
 res=swd_bin8_parity_even(&req, &parity);
 if (res<0) return res;
 if (parity<0 || parity>1) return SWD_ERROR_PARITY;
 reqhdr|=(res<<SWD_REQUEST_PARITY_BITNUM);
 reqhdr|=(SWD_REQUEST_START_VAL<<SWD_REQUEST_START_BITNUM);
 reqhdr|=(SWD_REQUEST_STOP_VAL<<SWD_REQUEST_STOP_BITNUM);
 reqhdr|=(SWD_REQUEST_PARK_VAL<<SWD_REQUEST_PARK_BITNUM);

 *request=reqhdr;
 return 1;
}
/** @} */
