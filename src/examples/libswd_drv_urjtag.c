/*
 * $Id$
 *
 * Driver Bridge between LibSWD and UrJTAG.
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

/** \file liblibswd_drv_urjtag.c Driver Bridge between LibSWD and UrJTAG. */

#include <libswd.h>
#include <urjtag/urjtag.h>

/**
 * Use UrJTAG's driver to write 8-bit data (char type).
 * MOSI (Master Output Slave Input) is a SWD Write Operation.
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char data.
 * \bits tells how many bits to send (at most 8).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_mosi_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst){
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[8], mosidata[8];

 //UrJTAG drivers shift data LSB-First.
 for (i=0;i<8;i++) mosidata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?(i):(7-i)]=((1<<i)&(*data))?1:0;
 res=urj_tap_cable_transfer((urj_cable_t *)libswdctx->driver->device, bits, mosidata, misodata);
 if (res<0) return LIBSWD_ERROR_DRIVER;
 urj_tap_cable_flush((urj_cable_t *)libswdctx->driver->device, URJ_TAP_CABLE_COMPLETELY);
 return i;
}

/**
 * Use UrJTAG's driver to write 32-bit data (int type).
 * MOSI (Master Output Slave Input) is a SWD Write Operation.
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char buffer array.
 * \bits tells how many bits to send (at most 32).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_mosi_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst){
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[32], mosidata[32];

 //UrJTAG drivers shift data LSB-First.
 for (i=0;i<32;i++) mosidata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?(i):(31-i)]=((1<<i)&(*data))?1:0;
 res=urj_tap_cable_transfer((urj_cable_t *)libswdctx->driver->device, bits, mosidata, misodata);
 if (res<0) return LIBSWD_ERROR_DRIVER;
 urj_tap_cable_flush((urj_cable_t *)libswdctx->driver->device, URJ_TAP_CABLE_COMPLETELY);
 return i;
}

/**
 * Use UrJTAG's driver to read 8-bit data (char type).
 * MISO (Master Input Slave Output) is a SWD Read Operation.
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char buffer array.
 * \bits tells how many bits to send (at most 8).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_miso_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst){
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[8], mosidata[8];

 res=urj_tap_cable_transfer((urj_cable_t *)libswdctx->driver->device, bits, mosidata, misodata);
 if (res<0) return LIBSWD_ERROR_DRIVER;
 urj_tap_cable_flush((urj_cable_t *)libswdctx->driver->device, URJ_TAP_CABLE_COMPLETELY);
 //Now we need to reconstruct the data byte from shifted in LSBfirst byte array.
 *data=0;
 for (i=0;i<bits;i++) *data|=(misodata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?(bits-1-i):(i)]?(1<<i):0);
 return i;
}

/**
 * Use UrJTAG's driver to read 32-bit data (int type).
 * MISO (Master Input Slave Output) is a SWD Read Operation.
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char buffer array.
 * \bits tells how many bits to send (at most 32).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_miso_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst){
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[32], mosidata[32];

 res=urj_tap_cable_transfer((urj_cable_t *)libswdctx->driver->device, bits, mosidata, misodata);
 if (res<0) return LIBSWD_ERROR_DRIVER;
 urj_tap_cable_flush((urj_cable_t *)libswdctx->driver->device, URJ_TAP_CABLE_COMPLETELY);
 //Now we need to reconstruct the data byte from shifted in LSBfirst byte array.
 *data=0;
 for (i=0;i<bits;i++) *data|=(misodata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?(bits-1-i):(i)]?(1<<i):0);
 return i;
}

/**
 * This function sets interface buffers to MOSI direction.
 * MOSI (Master Output Slave Input) is a SWD Write operation.
 * \param *libswdctx is the swd context to work on.
 * \param bits specify how many clock cycles must be used for TRN.
 * \return number of bits transmitted or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_mosi_trn(libswd_ctx_t *libswdctx, int bits){
 if (bits<LIBSWD_TURNROUND_MIN_VAL && bits>LIBSWD_TURNROUND_MAX_VAL)
  return LIBSWD_ERROR_TURNAROUND;

 int res;
 res=urj_tap_cable_set_signal((urj_cable_t *)libswdctx->driver->device, URJ_POD_CS_RnW, 0);
 if (res<0) return LIBSWD_ERROR_DRIVER;
 /* void urj_tap_cable_clock (urj_cable_t *cable, int tms, int tdi, int n); */
 urj_tap_cable_clock((urj_cable_t *)libswdctx->driver->device, 1, 1, bits);

 return bits;
}

/**
 * This function sets interface buffers to MISO direction.
 * MISO (Master Input Slave Output) is a SWD Read operation.
 * \param *libswdctx is the swd context to work on.
 * \param bits specify how many clock cycles must be used for TRN.
 * \return number of bits transmitted or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_miso_trn(libswd_ctx_t *libswdctx, int bits){
 if (bits<LIBSWD_TURNROUND_MIN_VAL && bits>LIBSWD_TURNROUND_MAX_VAL)
  return LIBSWD_ERROR_TURNAROUND;

 static int res;

 res=urj_tap_cable_set_signal((urj_cable_t *)libswdctx->driver->device, URJ_POD_CS_RnW, URJ_POD_CS_RnW);
 if (res<0) return LIBSWD_ERROR_DRIVER;

 /* void urj_tap_cable_clock (urj_cable_t *cable, int tms, int tdi, int n); */
 urj_tap_cable_clock((urj_cable_t *)libswdctx->driver->device, 1, 1, bits);

 return bits;
}


/**
 * Set debug level according to UrJTAG settings.
 * \param *libswdctx is the context to work on.
 * \param loglevel is the UrJTAG's lovleve to be transformed into LibSWD one.
 * \return LIBSWD_OK on success, negative LIBSWD_ERROR code on failure.
 */
int libswd_log_level_inherit(libswd_ctx_t *libswdctx, int loglevel){
 if (libswdctx==NULL){
  urj_log(URJ_LOG_LEVEL_DEBUG, "libswd_log_level_inherit(): SWD Context not (yet) initialized...\n");
  return LIBSWD_OK;
 }

 libswd_loglevel_t new_swdlevel;
 switch (loglevel){
  case URJ_LOG_LEVEL_ALL:
  case URJ_LOG_LEVEL_COMM:
  case URJ_LOG_LEVEL_DEBUG:
   new_swdlevel=LIBSWD_LOGLEVEL_DEBUG;
   break;
  case URJ_LOG_LEVEL_DETAIL:
   new_swdlevel=LIBSWD_LOGLEVEL_INFO;
   break;
  case URJ_LOG_LEVEL_NORMAL:
   new_swdlevel=LIBSWD_LOGLEVEL_NORMAL;
   break;
  case URJ_LOG_LEVEL_WARNING:
   new_swdlevel=LIBSWD_LOGLEVEL_WARNING;
   break;
  case URJ_LOG_LEVEL_ERROR:
   new_swdlevel=LIBSWD_LOGLEVEL_ERROR;
   break;
  case URJ_LOG_LEVEL_SILENT:
   new_swdlevel=LIBSWD_LOGLEVEL_SILENT;
   break;
  default:
   new_swdlevel=LIBSWD_LOGLEVEL_NORMAL;
 }

 int res=libswd_log_level_set(libswdctx, new_swdlevel);
 if (res<0) {
  urj_log(URJ_LOG_LEVEL_ERROR, "libswd_log_level_set() failed (%s)\n", libswd_error_string(res));
  return URJ_ERROR_SYNTAX;
 } else return LIBSWD_OK;
}


