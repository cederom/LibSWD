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

/** Put a message into swd context log at specified verbosity level.
 * If specified message's log level is lower than actual context configuration,
 * message will be omitted. Verbosity level increases from 0 (silent) to 4 (debug).
 * \param *swdctx swd context.
 * \param loglevel at which to put selected message.
 * \param *msg message body with variable arguments as in "printf".
 * \return number of characters written or error code on failure.
 */
int swd_log(swd_ctx_t *swdctx, swd_loglevel_t loglevel, char *msg, ...){
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
 return SWD_OK;
}

/** @} */
