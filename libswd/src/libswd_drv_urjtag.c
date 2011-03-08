/*
 * $Id$
 *
 * Serial Wire Debug Open Library.
 * External Application Driver Interface File.
 *
 * Copyright (C) 2010, Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)
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
 * Written by Tomasz Boleslaw CEDRO <tomek.cedro@gmail.com>, 2010;
 *
 */

#include <libswd.h>
#include <urjtag/urjtag.h>
#include <stdlib.h>

int swd_drv_mosi_8(swd_ctx_t *swdctx, char *data, int bits, int nLSBfirst){
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return SWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return SWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[8], mosidata[8];

 //UrJTAG drivers shift data LSB-First.
 for (i=0;i<8;i++) mosidata[(nLSBfirst==SWD_DIR_LSBFIRST)?(i):(7-i)]=((1<<i)&(*data))?1:0; 
 res=urj_tap_cable_transfer((urj_cable_t *)swdctx->driver->device, bits, mosidata, misodata);
 if (res<0) return SWD_ERROR_DRIVER;
 urj_tap_cable_flush((urj_cable_t *)swdctx->driver->device, URJ_TAP_CABLE_COMPLETELY);
 return i;
}


int swd_drv_mosi_32(swd_ctx_t *swdctx, int *data, int bits, int nLSBfirst){
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return SWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return SWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[32], mosidata[32];

 //UrJTAG drivers shift data LSB-First.
 for (i=0;i<32;i++) mosidata[(nLSBfirst==SWD_DIR_LSBFIRST)?(i):(32-i)]=((1<<i)&(*data))?1:0; 
 res=urj_tap_cable_transfer((urj_cable_t *)swdctx->driver->device, bits, mosidata, misodata);
 if (res<0) return SWD_ERROR_DRIVER;
 urj_tap_cable_flush((urj_cable_t *)swdctx->driver->device, URJ_TAP_CABLE_COMPLETELY);
 return i;
}

int swd_drv_miso_8(swd_ctx_t *swdctx, char *data, int bits, int nLSBfirst){
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return SWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return SWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[8], mosidata[8]={0,0,0,0,0,0,0,0};

 //UrJTAG drivers shift data LSB-First.
 for (i=0;i<bits;i++) mosidata[(nLSBfirst==SWD_DIR_LSBFIRST)?(i):(7-i)]=((1<<i)&(*data))?1:0; 
 res=urj_tap_cable_transfer((urj_cable_t *)swdctx->driver->device, bits, mosidata, misodata);
 if (res<0) return SWD_ERROR_DRIVER;
 urj_tap_cable_flush((urj_cable_t *)swdctx->driver->device, URJ_TAP_CABLE_COMPLETELY);
 //Now we need to reconstruct the data byte from shifted in LSBfirst byte array.
 *data=0;
 for (i=0;i<bits;i++) *data|=(misodata[7-i]?(1<<i):0);
 return i;
}

int swd_drv_miso_32(swd_ctx_t *swdctx, int *data, int bits, int nLSBfirst){
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return SWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return SWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[32], mosidata[32];

 //UrJTAG drivers shift data LSB-First.
 for (i=0;i<bits;i++) mosidata[(nLSBfirst==SWD_DIR_LSBFIRST)?(i):(31-i)]=((1<<i)&(*data))?1:0; 
 res=urj_tap_cable_transfer((urj_cable_t *)swdctx->driver->device, bits, mosidata, misodata);
 if (res<0) return SWD_ERROR_DRIVER;
 urj_tap_cable_flush((urj_cable_t *)swdctx->driver->device, URJ_TAP_CABLE_COMPLETELY);
 //Now we need to reconstruct the data byte from shifted in LSBfirst byte array.
 *data=0;
 for (i=0;i<bits;i++) *data|=(misodata[31-i]?(1<<i):0);
 return i;
}       


/* This function sets interface buffers to MOSI direction.
 * Master Output Slave Input - SWD Write operation.
 * bits specify how many clock cycles must be used. */
int swd_drv_mosi_trn(swd_ctx_t *swdctx, int bits){
 if (bits<SWD_TURNROUND_MIN_VAL && bits>SWD_TURNROUND_MAX_VAL)
  return SWD_ERROR_TURNAROUND; 

 static int res;
// static char mosidata[4]={0xff, 0xff, 0xff, 0xff};

 res=urj_tap_cable_set_signal((urj_cable_t *)swdctx->driver->device, URJ_POD_CS_RnW, 0); 
 if (res<0) return SWD_ERROR_DRIVER;
// res=urj_tap_cable_transfer((urj_cable_t *)swdctx->driver->device, bits, mosidata, NULL); 
// if (res<0) return SWD_ERROR_DRIVER;
// urj_tap_cable_flush((urj_cable_t *)swdctx->driver->device, URJ_TAP_CABLE_COMPLETELY);

 /* void urj_tap_cable_clock (urj_cable_t *cable, int tms, int tdi, int n); */
 urj_tap_cable_clock((urj_cable_t *)swdctx->driver->device, 0, 0, bits); 

 return SWD_OK;
}

int swd_drv_miso_trn(swd_ctx_t *swdctx, int bits){
 if (bits<SWD_TURNROUND_MIN_VAL && bits>SWD_TURNROUND_MAX_VAL)
  return SWD_ERROR_TURNAROUND; 

 static int res;
 static char mosidata[4]={0xff, 0xff, 0xff, 0xff};

 res=urj_tap_cable_set_signal((urj_cable_t *)swdctx->driver->device, URJ_POD_CS_RnW, URJ_POD_CS_RnW); 
 if (res<0) return SWD_ERROR_DRIVER;
 res=urj_tap_cable_transfer((urj_cable_t *)swdctx->driver->device, bits, mosidata, NULL); 
 if (res<0) return SWD_ERROR_DRIVER;
 urj_tap_cable_flush((urj_cable_t *)swdctx->driver->device, URJ_TAP_CABLE_COMPLETELY);

 /* void urj_tap_cable_clock (urj_cable_t *cable, int tms, int tdi, int n); */
// urj_tap_cable_clock((urj_cable_t *)swdctx->driver->device, 0, 0, bits); 
 
 return SWD_OK;
}




