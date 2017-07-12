/*
 * Serial Wire Debug Open Library.
 * External Handlers Definition File.
 *
 * Copyright (C) 2010-2013, Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)
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
 * Written by Tomasz Boleslaw CEDRO <cederom@tlen.pl>, 2010-2013;
 *
 */

/** \file libswd_externs.c Template for driver bridge between libswd and your application. */

#include <libswd.h>
#include <stdlib.h>

__attribute__ ((weak)) int libswd_drv_mosi_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst){
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 // Your code goes here...

 return bits;
}


__attribute__ ((weak)) int libswd_drv_mosi_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst){
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 // Your code goes here...

 return bits;
}

__attribute__ ((weak)) int libswd_drv_miso_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst){
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 // Your code goes here...

 return bits;
}

__attribute__ ((weak)) int libswd_drv_miso_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst){
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 && bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 // Your code goes here...

 return bits;
}


/* This function sets interface buffers to MOSI direction.
 * Master Output Slave Input - SWD Write operation.
 * bits specify how many clock cycles must be used. */
__attribute__ ((weak)) int libswd_drv_mosi_trn(libswd_ctx_t *libswdctx, int bits){
 if (bits<LIBSWD_TURNROUND_MIN_VAL && bits>LIBSWD_TURNROUND_MAX_VAL)
  return LIBSWD_ERROR_TURNAROUND;

 // Your code goes here...

 return bits;
}

__attribute__ ((weak)) int libswd_drv_miso_trn(libswd_ctx_t *libswdctx, int bits){
 if (bits<LIBSWD_TURNROUND_MIN_VAL && bits>LIBSWD_TURNROUND_MAX_VAL)
  return LIBSWD_ERROR_TURNAROUND;

 // Your code goes here...

 return bits;
}


/** Set debug level according to caller's application settings.
 * \params *libswdctx swd context to work on.
 * \params loglevel caller's application log level to be converted.
 * \return LIBSWD_OK on success, of error code on failure.
 */
__attribute__ ((weak)) int libswd_log_level_inherit(libswd_ctx_t *libswdctx, int loglevel){
 if (libswdctx==NULL){
  // log(LOG_LEVEL_DEBUG, "libswd_log_level_inherit(): SWD Context not (yet) initialized...\n");
  return LIBSWD_OK;
 }

 libswd_loglevel_t new_swdlevel;
 switch (loglevel){
  // Your code goes here...
  default:
   new_swdlevel=LIBSWD_LOGLEVEL_NORMAL;
 }

 int res=libswd_log_level_set(libswdctx, new_swdlevel);
 if (res<0) {
  // Your error routine goes here...
  // return URJ_ERROR_SYNTAX;
 } else return LIBSWD_OK;
}

/** By default we want to use internal logging mechanisms.
 * It is possible however to use target program mechanisms to log messages.
 * In order to correctly parse variable number of arguments we need to use
 * dedicated libswd_log_internal_va() function...
 */
__attribute__ ((weak)) int libswd_log(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...){
 int retval;
 va_list ap;
 va_start(ap, msg);
 retval=libswd_log_internal_va(libswdctx, loglevel, msg, ap);
 va_end(ap);
 return retval;
};

__attribute__ ((weak)) int libswd_log_flush(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...){
 int retval;
 va_list ap;
 va_start(ap, msg);
 retval=libswd_log_internal_va(libswdctx, loglevel, msg, ap);
 va_end(ap);
 fflush(stdout);
 return retval;
};

