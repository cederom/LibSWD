/*
 * Serial Wire Debug Open Library.
 * Library Body File.
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

/** \file libswd_log.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup libswd_log Miscelanous logging functionalities.
 * @{
 ******************************************************************************/

/** Logging functionality can be external or internal, by default external
 * function can be defined to use target program logging mechanism.
 * To use internal logging mechanism simply wrap libswd_log_internal() around
 * this function in application specific driver bridge file,
 * see liblibswd_externs.c for examples. When you want to use variable argument
 * (printf style) invocation you can use libswd_log_internal_va() as vprintf().
 */
extern int libswd_log(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...);

/** Put a message into swd context log at specified verbosity level.
 * If specified message's log level is lower than actual context configuration,
 * message will be omitted. Verbosity level increases from 0 (silent) to 6 (bitstream).
 * This function does not put '\n' at the end of line so you need to put them by hand.
 * \param *libswdctx swd context.
 * \param loglevel at which to put selected message.
 * \param *msg message body with variable arguments as in "printf".
 * \return number of characters written or error code on failure.
 */
int libswd_log_internal(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...){
 if (loglevel<LIBSWD_LOGLEVEL_MIN || loglevel>LIBSWD_LOGLEVEL_MAX)
  return LIBSWD_ERROR_LOGLEVEL;
 if (loglevel > libswdctx->config.loglevel) return LIBSWD_OK;
 int res;
 va_list ap;
 va_start(ap, msg);
 res=vprintf(msg, ap);
 va_end(ap);
 return res;
}

/** Put a fmt+va_list message into swd context log at specified verbosity level.
 * It works just as libswd_log_internal() but is intended for use instead
 * vprintf() between va_start() and va_end()...
 * \param *libswdctx swd context.
 * \param loglevel at which to put selected message.
 * \param *msg message body with variable arguments as in "printf".
 * \return number of characters written or error code on failure.
 */
int libswd_log_internal_va(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *fmt, va_list ap){
 if (loglevel<LIBSWD_LOGLEVEL_MIN || loglevel>LIBSWD_LOGLEVEL_MAX)
  return LIBSWD_ERROR_LOGLEVEL;
 if (loglevel > libswdctx->config.loglevel) return LIBSWD_OK;
 int res;
 res=vprintf(fmt, ap);
 return res;
}

/** Change log level to increase or decrease verbosity level.
 * \param *libswdctx swd context.
 * \param loglevel is the target verbosity level to be set.
 * \return LIBSWD_OK on success or error code.
 */
int libswd_log_level_set(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (loglevel<LIBSWD_LOGLEVEL_MIN || loglevel>LIBSWD_LOGLEVEL_MAX)
  return LIBSWD_ERROR_LOGLEVEL;

 libswdctx->config.loglevel=loglevel;
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_log_level_set(libswdctx=0x%p, loglevel[%d..%d]=%d/%s)\n", (void*)libswdctx, LIBSWD_LOGLEVEL_MIN, LIBSWD_LOGLEVEL_MAX, loglevel, libswd_log_level_string(loglevel));
 return LIBSWD_OK;
}

/** Return integer log level value.
 * \param *libswdctx swd context.
 * \return integer log level value or LIBSWD_ERROR code on failure.
 */
int libswd_log_level_get(libswd_ctx_t *libswdctx){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 return libswdctx->config.loglevel;
}

/** Helper function that returns loglevel name string for logging purposes.
 * \param loglevel is the libswd_loglevel_t code to produce a string.
 * \return char* loglevel name sring array.
 */
const char *libswd_log_level_string(libswd_loglevel_t loglevel){
 switch (loglevel){
  case LIBSWD_LOGLEVEL_SILENT:  return "LIBSWD_LOGLEVEL_SILENT";
  case LIBSWD_LOGLEVEL_ERROR:   return "LIBSWD_LOGLEVEL_ERROR";
  case LIBSWD_LOGLEVEL_WARNING: return "LIBSWD_LOGLEVEL_WARNING";
  case LIBSWD_LOGLEVEL_NORMAL:  return "LIBSWD_LOGLEVEL_NORMAL";
  case LIBSWD_LOGLEVEL_INFO:    return "LIBSWD_LOGLEVEL_INFO";
  case LIBSWD_LOGLEVEL_DEBUG:   return "LIBSWD_LOGLEVEL_DEBUG";
  case LIBSWD_LOGLEVEL_PAYLOAD: return "LIBSWD_LOGLEVEL_PAYLOAD";
 }
 return "UNKNOWN_LOGLEVEL";
};

/** Helper function to produce operation name string for logging purposes.
 * \param operation is the libswd_operation_t code to return as string.
 * \return char* array with operation name string.
 */
const char *libswd_operation_string(libswd_operation_t operation){
 switch(operation){
  case LIBSWD_OPERATION_ENQUEUE:       return "LIBSWD_OPERATION_ENQUEUE";
  case LIBSWD_OPERATION_EXECUTE:       return "LIBSWD_OPERATION_EXECUTE";
  case LIBSWD_OPERATION_TRANSMIT_HEAD: return "LIBSWD_OPERATION_TRANSMIT_HEAD";
  case LIBSWD_OPERATION_TRANSMIT_TAIL: return "LIBSWD_OPERATION_TRANSMIT_TAIL";
  case LIBSWD_OPERATION_TRANSMIT_ALL:  return "LIBSWD_OPERATION_TRANSMIT_ALL";
  case LIBSWD_OPERATION_TRANSMIT_ONE:  return "LIBSWD_OPERATION_TRANSMIT_ONE";
  case LIBSWD_OPERATION_TRANSMIT_LAST: return "LIBSWD_OPERATION_TRANSMIT_LAST";
 }
 return "UNKNOWN_LIBSWD_OPERATION";
}

/** Helper function that can print name of the request fields.
 * DP SELECT APBANKSEL fields are also taken into account here for APACC.
 * \param libswdctx points to the swd context and its necessary to know
    DP SELECT register value as it determines CTRL/STAT or WCR access.
 * \param RnW is the read/write bit of the request packet.
 * \param addr is the address of the register.
 * \return char* array with the register name string.
 */
const char *libswd_request_string(libswd_ctx_t *libswdctx, char request){
 static char string[100], tmp[8]; string[0]=0;
 int apndp=request&LIBSWD_REQUEST_APnDP;
 int addr=0;
 addr|=((request&LIBSWD_REQUEST_A3)?1<<3:0);
 addr|=((request&LIBSWD_REQUEST_A2)?1<<2:0);
 if (apndp) addr|=(libswdctx->log.dp.select&LIBSWD_DP_SELECT_APBANKSEL);
 int rnw=request&LIBSWD_REQUEST_RnW;
 int parity=request&LIBSWD_REQUEST_PARITY;

 strcat(string, apndp?"AccessPort ":"DebugPort ");
 strcat(string, rnw?"Read ":"Write ");
 strcat(string, "Addr="); sprintf(tmp, "0x%02X", addr); strcat(string, tmp);

 if (apndp){
  // APnDP=1 so we print out the AHB-AP registers
  addr|=libswdctx->log.dp.select&LIBSWD_DP_SELECT_APBANKSEL;
  switch (addr){
   case 0x00: strcat(string, "(R/W: Control/Status Word, CSW (reset value: 0x43800042)) "); break;
   case 0x04: strcat(string, "(R/W: Transfer Address, TAR (reset value: 0x00000000)) "); break;
   case 0x08: strcat(string, "(Reserved SBZ) "); break;
   case 0x0c: strcat(string, "(R/W, Data Read/Write, DRW) "); break;
   case 0x10: strcat(string, "(R/W, Banked Data 0, BD0) "); break;
   case 0x14: strcat(string, "(R/W, Banked Data 1, BD1) "); break;
   case 0x18: strcat(string, "(R/W, Banked Data 2, BD2 )"); break;
   case 0x1c: strcat(string, "(R/W, Banked Data 3, BD3) "); break;
   case 0xf8: strcat(string, "(RO, Debug ROM table (reset value: 0xE00FF000)) "); break;
   case 0xfc: strcat(string, "(RO, Identification Register, IDR (reset value: 0x24770001)) "); break;
   default:   strcat(string, "(UNKNOWN) ");
  }
 } else {
  // APnDP=0 so we print out the SW-DP registers
  if (rnw) {
   switch (addr){
    case LIBSWD_DP_IDCODE_ADDR: strcat(string, "(IDCODE)"); break;
    case LIBSWD_DP_CTRLSTAT_ADDR: strcat(string, (libswdctx->log.dp.select&1<<LIBSWD_DP_SELECT_CTRLSEL_BITNUM)?"(CTRL/STAT or [WCR])":"([CTRL/STAT] or WCR)"); break;
    case LIBSWD_DP_RESEND_ADDR: strcat(string ,"(RESEND) "); break;
    case LIBSWD_DP_RDBUFF_ADDR: strcat(string, "(RDBUFF) "); break;
    default: strcat(string, "(UNKNOWN) ");
   }
  } else {
   switch (addr) {
    case LIBSWD_DP_ABORT_ADDR: strcat(string, "(ABORT) "); break;
    case LIBSWD_DP_CTRLSTAT_ADDR: strcat(string, (libswdctx->log.dp.select&1<<LIBSWD_DP_SELECT_CTRLSEL_BITNUM)?"(CTRL/STAT or [WCR]) ":"([CTRL/STAT] or WCR) "); break;
    case LIBSWD_DP_SELECT_ADDR: strcat(string, "(SELECT) "); break;
    case LIBSWD_DP_ROUTESEL_ADDR: strcat(string, "(ROUTESEL)"); break;
    default: strcat(string, "(UNKNOWN) ");
   }
  }
 }
 strcat(string, "Parity="); strcat(string, parity?"1":"0");
 return string;
}


/** @} */
