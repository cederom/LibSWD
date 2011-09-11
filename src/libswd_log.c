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

/** \file libswd_log.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup swd_log Miscelanous logging functionalities.
 * @{
 ******************************************************************************/

/** Logging functionality can be external or internal, by default external
 * function can be defined to use target program logging mechanism.
 * To use internal logging mechanism simply wrap swd_log_internal() around
 * this function in application specific driver bridge file,
 * see libswd_externs.c for examples.
 */
extern int swd_log(swd_ctx_t *swdctx, swd_loglevel_t loglevel, char *msg, ...);

/** Put a message into swd context log at specified verbosity level.
 * If specified message's log level is lower than actual context configuration,
 * message will be omitted. Verbosity level increases from 0 (silent) to 4 (debug).
 * \param *swdctx swd context.
 * \param loglevel at which to put selected message.
 * \param *msg message body with variable arguments as in "printf".
 * \return number of characters written or error code on failure.
 */
int swd_log_internal(swd_ctx_t *swdctx, swd_loglevel_t loglevel, char *msg, ...){
 if (loglevel<SWD_LOGLEVEL_MIN && loglevel>SWD_LOGLEVEL_MAX)
  return SWD_ERROR_LOGLEVEL;
 if (loglevel > swdctx->config.loglevel) return SWD_OK;
 int res;
 va_list ap;
 va_start(ap, msg);
 res=vprintf(msg, ap);  
 va_end(ap);
 return res;
}

/** Change log level to increase or decrease verbosity level.
 * \param *swdctx swd context.
 * \param loglevel is the target verbosity level to be set.
 * \return SWD_OK on success or error code.
 */
int swd_log_level_set(swd_ctx_t *swdctx, swd_loglevel_t loglevel){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (loglevel<SWD_LOGLEVEL_MIN && loglevel>SWD_LOGLEVEL_MAX)
  return SWD_ERROR_LOGLEVEL;

 swdctx->config.loglevel=loglevel;
 swd_log(swdctx, SWD_LOGLEVEL_DEBUG, "SWD_D: swd_log_level_set(swdctx=0x%p, loglevel[%d..%d]=%d/%s)", (void*)swdctx, SWD_LOGLEVEL_MIN, SWD_LOGLEVEL_MAX, loglevel, swd_log_level_string(loglevel));
 return SWD_OK;
}

/** Helper function that returns loglevel name string for logging purposes.
 * \param loglevel is the swd_loglevel_t code to produce a string.
 * \return char* loglevel name sring array.
 */
const char *swd_log_level_string(swd_loglevel_t loglevel){
 switch (loglevel){
  case SWD_LOGLEVEL_SILENT:  return "SWD_LOGLEVEL_SILENT";
  case SWD_LOGLEVEL_ERROR:   return "SWD_LOGLEVEL_ERROR";
  case SWD_LOGLEVEL_WARNING: return "SWD_LOGLEVEL_WARNING";
  case SWD_LOGLEVEL_NORMAL:  return "SWD_LOGLEVEL_NORMAL";
  case SWD_LOGLEVEL_INFO:    return "SWD_LOGLEVEL_INFO";
  case SWD_LOGLEVEL_DEBUG:   return "SWD_LOGLEVEL_DEBUG";
 }
 return "UNKNOWN LOGLEVEL!";
};

/** Helper function to produce operation name string for logging purposes.
 * \param operation is the swd_operation_t code to return as string.
 * \return char* array with operation name string.
 */
const char *swd_operation_string(swd_operation_t operation){
 switch(operation){
  case SWD_OPERATION_ENQUEUE:       return "SWD_OPERATION_ENQUEUE";
  case SWD_OPERATION_EXECUTE:       return "SWD_OPERATION_EXECUTE";
  case SWD_OPERATION_TRANSMIT_HEAD: return "SWD_OPERATION_TRANSMIT_HEAD";
  case SWD_OPERATION_TRANSMIT_TAIL: return "SWD_OPERATION_TRANSMIT_TAIL";
  case SWD_OPERATION_TRANSMIT_ALL:  return "SWD_OPERATION_TRANSMIT_ALL";
  case SWD_OPERATION_TRANSMIT_ONE:  return "SWD_OPERATION_TRANSMIT_ONE";
  case SWD_OPERATION_TRANSMIT_LAST: return "SWD_OPERATION_TRANSMIT_LAST"; 
 }
 return "UNKNOWN_SWD_OPERATION";
} 

/** @} */
