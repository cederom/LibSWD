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

/** \file libswd_core.c */

#include <libswd.h>
#include <config.h>

/*******************************************************************************
 * \defgroup swd_core Library and Context (de)initialization routines.
 * @{
 ******************************************************************************/

/** LibSWD initialization routine.
 * It should be called prior any operation made with libswd. It initializes
 * command queue and basic parameters for context that is returned as pointer.
 * \return pointer to the initialized swd context.
 */
swd_ctx_t *swd_init(void){
 swd_ctx_t *swdctx;
 swdctx=(swd_ctx_t *)calloc(1,sizeof(swd_ctx_t));
 if (swdctx==NULL) return NULL;
 swdctx->driver=(swd_driver_t *)calloc(1,sizeof(swd_driver_t));
 if (swdctx->driver==NULL){
  free(swdctx);
  return NULL;
 }
 swdctx->cmdq=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (swdctx->cmdq==NULL) {
  swd_deinit_ctx(swdctx);
  return NULL;
 }
 swdctx->config.initialized=SWD_TRUE;
 swdctx->config.trnlen=SWD_TURNROUND_DEFAULT_VAL;
 swdctx->config.maxcmdqlen=SWD_CMDQLEN_DEFAULT;
 swdctx->config.loglevel=SWD_LOGLEVEL_NORMAL;
 swd_log(swdctx, SWD_LOGLEVEL_NORMAL, "SWD_N: Using " PACKAGE_STRING " (http://libswd.sf.net)\nSWD_N: (c) Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)\n");
 return swdctx;
}

/** De-initialize selected swd context and free its memory.
 * Note: This function will not free command queue for selected context!
 * \param *swdctx swd context pointer.
 * \return SWD_OK on success, SWD_ERROR_CODE on failure.
 */
int swd_deinit_ctx(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLPOINTER;
 free(swdctx);
 return SWD_OK;
}

/** De-initialize command queue and free its memory on selected swd context.
 * \param *swdctx swd context pointer.
 * \return number of commands freed, or SWD_ERROR_CODE on failure.
 */ 
int swd_deinit_cmdq(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLPOINTER;
 int res;
 res=swd_cmdq_free(swdctx->cmdq);
 if (res<0) return res;
 return res;
}

/** De-initialize selected swd context and its command queue.
 * \param *swdctx swd context pointer.
 * \return number of elements freed, or SWD_ERROR_CODE on failure.
 */ 
int swd_deinit(swd_ctx_t *swdctx){
 int res, cmdcnt=0;
 res=swd_deinit_cmdq(swdctx);
 if (res<0) return res;
 cmdcnt=res;
 res=swd_deinit_ctx(swdctx);
 if (res<0) return res;
 return cmdcnt+res;
}

/** @} */
