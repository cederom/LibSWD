/*
 * $Id$
 *
 * Driver Bridge between LibSWD and OpenOCD.
 *
 * Copyright (C) 2010-2011 Tomasz Boleslaw CEDRO
 * cederom@tlen.pl, http://www.tomek.cedro.info
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

/** \file liblibswd_drv_openocd.c Driver Bridge between LibSWD and OpenOCD. */

#include <transport/swd.h>

/** OpenOCD as for now use global pointer to driver structure. */
extern struct jtag_interface *jtag_interface;

/**
 * Use UrJTAG's driver to write 8-bit data (char type).
 * MOSI (Master Output Slave Input) is a SWD Write Operation.
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char data.
 * \bits tells how many bits to send (at most 8).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
ar)*/
int libswd_drv_mosi_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst){
 LOG_DEBUG("OpenOCD's libswd_drv_mosi_8(libswdctx=@%p, cmd=@%p, data=0x%02X, bits=%d, nLSBfirst=0x%02X)", (void*)libswdctx, (void*)cmd, *data, bits, nLSBfirst);
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[8], mosidata[8];

 /* Split output data into char array. */
 for (i=0;i<8;i++) mosidata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?(i):(bits-1-i)]=((1<<i)&(*data))?1:0;
 /* Then send that array into interface hardware. */
 res=jtag_interface->transfer(NULL, bits, mosidata, misodata, 0);
 if (res<0) return LIBSWD_ERROR_DRIVER;

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
 LOG_DEBUG("OpenOCD's libswd_drv_mosi_32(libswdctx=@%p, cmd=@%p, data=0x%08X, bits=%d, nLSBfirst=0x%02X)", (void*)libswdctx, (void*)cmd, *data, bits, nLSBfirst);
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[32], mosidata[32];

 //UrJTAG drivers shift data LSB-First.
 for (i=0;i<32;i++) mosidata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?(i):(bits-1-i)]=((1<<i)&(*data))?1:0;
 res=jtag_interface->transfer(NULL, bits, mosidata, misodata, 0);
 if (res<0) return LIBSWD_ERROR_DRIVER;
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

 static int i;
 static signed int res;
 static char misodata[8], mosidata[8];

 res=jtag_interface->transfer(NULL, bits, mosidata, misodata, LIBSWD_DIR_LSBFIRST);
 if (res<0) return LIBSWD_ERROR_DRIVER;
 /* Now we need to reconstruct the data byte from shifted in LSBfirst byte array. */
 *data=0;
 for (i=0;i<bits;i++) *data|=misodata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?(i):(bits-1-i)]?(1<<i):0;
 LOG_DEBUG("OpenOCD's libswd_drv_miso_8(libswdctx=@%p, cmd=@%p, data=@%p, bits=%d, nLSBfirst=0x%02X) reads: 0x%02X", (void*)libswdctx, (void*)cmd, (void*)data, bits, nLSBfirst, *data);
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

 static int i;
 static signed int res;
 static char misodata[32], mosidata[32];

 res=jtag_interface->transfer(NULL, bits, mosidata, misodata, LIBSWD_DIR_LSBFIRST);
 if (res<0) return LIBSWD_ERROR_DRIVER;
 /* Now we need to reconstruct the data byte from shifted in LSBfirst byte array. */
 *data=0;
 for (i=0;i<bits;i++) *data|=(misodata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?(i):(bits-1-i)]?(1<<i):0);
 LOG_DEBUG("OpenOCD's libswd_drv_miso_32(libswdctx=@%p, cmd=@%p, data=@%p, bits=%d, nLSBfirst=0x%02X) reads: 0x%08X", (void*)libswdctx, (void*)cmd, (void*)data, bits, nLSBfirst, *data);
 LOG_DEBUG("OpenOCD's libswd_drv_miso_32() reads: 0x%08X\n", *data);
 return i;
}

/**
 * This function sets interface buffers to MOSI direction.
 * MOSI (Master Output Slave Input) is a SWD Write operation.
 * OpenOCD use global "struct jtag_interface" pointer as interface driver.
 * OpenOCD driver must support "RnW" signal to drive output buffers for TRN.
 * \param *libswdctx is the swd context to work on.
 * \param bits specify how many clock cycles must be used for TRN.
 * \return number of bits transmitted or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_mosi_trn(libswd_ctx_t *libswdctx, int bits){
 LOG_DEBUG("OpenOCD's libswd_drv_mosi_trn(libswdctx=@%p, bits=%d)\n", (void*)libswdctx, bits);
 if (bits<LIBSWD_TURNROUND_MIN_VAL && bits>LIBSWD_TURNROUND_MAX_VAL)
  return LIBSWD_ERROR_TURNAROUND;

 int res, val=0;
 static char buf[LIBSWD_TURNROUND_MAX_VAL];
 /* Use driver method to set low (write) signal named RnW. */
 res=jtag_interface->bitbang(NULL, "RnW", 0, &val);
 if (res<0) return LIBSWD_ERROR_DRIVER;

 /* Clock specified number of bits for proper TRN transaction. */
 res=jtag_interface->transfer(NULL, bits, buf, buf, 0);
 if (res<0) return LIBSWD_ERROR_DRIVER;

 return bits;
}

/**
 * This function sets interface buffers to MISO direction.
 * MISO (Master Input Slave Output) is a SWD Read operation.
 * OpenOCD use global "struct jtag_interface" pointer as interface driver.
 * OpenOCD driver must support "RnW" signal to drive output buffers for TRN.
 * \param *libswdctx is the swd context to work on.
 * \param bits specify how many clock cycles must be used for TRN.
 * \return number of bits transmitted or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_miso_trn(libswd_ctx_t *libswdctx, int bits){
 LOG_DEBUG("OpenOCD's libswd_drv_miso_trn(libswdctx=@%p, bits=%d)\n", (void*)libswdctx, bits);
 if (bits<LIBSWD_TURNROUND_MIN_VAL && bits>LIBSWD_TURNROUND_MAX_VAL)
  return LIBSWD_ERROR_TURNAROUND;

 static int res, val=1;
 static char buf[LIBSWD_TURNROUND_MAX_VAL];

 /* Use driver method to set high (read) signal named RnW. */
 res=jtag_interface->bitbang(NULL, "RnW", 0xFFFFFFFF, &val);
 if (res<0) return LIBSWD_ERROR_DRIVER;

 /* Clock specified number of bits for proper TRN transaction. */
 res=jtag_interface->transfer(NULL, bits, buf, buf, 0);
 if (res<0) return LIBSWD_ERROR_DRIVER;

 return bits;
}


/**
 * Set SWD debug level according to OpenOCD settings.
 * \param *libswdctx is the context to work on.
 * \param loglevel is the OpenOCD numerical value of actual loglevel to force
 *  on LibSWD, or -1 to inherit from actual global settings of OpenOCD.
 * \return LIBSWD_OK on success, negative LIBSWD_ERROR code on failure.
 */
int libswd_log_level_inherit(libswd_ctx_t *libswdctx, int loglevel){
 LOG_DEBUG("OpenOCD's libswd_log_level_inherit(libswdctx=@%p, loglevel=%d)\n", (void*)libswdctx, loglevel);
 if (libswdctx==NULL){
  LOG_WARNING("libswd_log_level_inherit(): SWD Context not (yet) initialized...\n");
  return LIBSWD_OK;
 }

 libswd_loglevel_t new_swdlevel;
 switch ((loglevel==-1)?debug_level:loglevel){
  case LOG_LVL_DEBUG:
   new_swdlevel=LIBSWD_LOGLEVEL_PAYLOAD;
   break;
  case LOG_LVL_INFO:
   new_swdlevel=LIBSWD_LOGLEVEL_INFO;
   break;
  case LOG_LVL_WARNING:
   new_swdlevel=LIBSWD_LOGLEVEL_WARNING;
   break;
  case LOG_LVL_ERROR:
   new_swdlevel=LIBSWD_LOGLEVEL_ERROR;
   break;
  case LOG_LVL_USER:
  case LOG_LVL_OUTPUT:
   new_swdlevel=LIBSWD_LOGLEVEL_NORMAL;
   break;
  case LOG_LVL_SILENT:
   new_swdlevel=LIBSWD_LOGLEVEL_SILENT;
   break;
  default:
   new_swdlevel=LIBSWD_LOGLEVEL_NORMAL;
 }

 int res=libswd_log_level_set(libswdctx, new_swdlevel);
 if (res<0) {
  LOG_ERROR("libswd_log_level_set() failed (%s)\n", libswd_error_string(res));
  return ERROR_FAIL;
 } return new_swdlevel;
}

/** We will use OpenOCD's logging mechanisms to show LibSWD messages.
  * SWD can have different loglevel set than the OpenOCD itself, so we need to
  * log all messages at openocd level that will not block swd messages.
  * It is also possible to 'inherit' loglevel to swd from openocd.
  */
int libswd_log(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (loglevel > LIBSWD_LOGLEVEL_MAX) return LIBSWD_ERROR_PARAM;

 if (loglevel > libswdctx->config.loglevel) return LIBSWD_OK;
 va_list ap;
 va_start(ap, msg);
 // Calling OpenOCD log functions here will cause program crash (va recurrent).
 vprintf(msg, ap);
 va_end(ap);
 return LIBSWD_OK;
}

