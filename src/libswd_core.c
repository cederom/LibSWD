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

/** \file libswd_core.c */

#include <libswd.h>
#include <config.h>

/*******************************************************************************
 * \defgroup libswd_core Library and Context (de)initialization routines.
 * @{
 ******************************************************************************/

/** LibSWD initialization routine.
 * It should be called prior any operation made with libswd. It initializes
 * command queue and basic parameters for context that is returned as pointer.
 * \return pointer to the initialized swd context.
 */
libswd_ctx_t *libswd_init(void){
 int res;
 libswd_ctx_t *libswdctx;
 libswdctx=(libswd_ctx_t *)calloc(1,sizeof(libswd_ctx_t));
 if (libswdctx==NULL) return NULL;
 libswdctx->driver=(libswd_driver_t *)calloc(1,sizeof(libswd_driver_t));
 if (libswdctx->driver==NULL){
  free(libswdctx);
  return NULL;
 }
 res=libswd_cmdq_init(libswdctx);
 if (res<0) {
	  libswd_deinit_ctx(libswdctx);
	  return NULL;
 }
 libswdctx->config.initialized=LIBSWD_TRUE;
 libswdctx->config.trnlen=LIBSWD_TURNROUND_DEFAULT_VAL;
 libswdctx->config.maxcmdqlen=LIBSWD_CMDQLEN_DEFAULT;
 libswdctx->config.loglevel=LIBSWD_LOGLEVEL_DEFAULT;
 libswdctx->config.autofixerrors=LIBSWD_AUTOFIX_DEFAULT;
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N: Using " PACKAGE_STRING " (http://libswd.sf.net)\nLIBSWD_N: (c) Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)\n");
 return libswdctx;
}

/** De-initialize selected swd context and free its memory.
 * Note: This function will not free command queue for selected context!
 * \param *libswdctx swd context pointer.
 * \return LIBSWD_OK on success, LIBSWD_ERROR_CODE on failure.
 */
int libswd_deinit_ctx(libswd_ctx_t *libswdctx){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLPOINTER;
 free(libswdctx);
 return LIBSWD_OK;
}

/** De-initialize command queue and free its memory on selected swd context.
 * OBSOLETE FUNCTION, please use libswd_cmdq_free directly
 * \param *libswdctx swd context pointer.
 * \return number of commands freed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_deinit_cmdq(libswd_ctx_t *libswdctx){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLPOINTER;
 int res;
 res=libswd_cmdq_free(libswdctx);
 if (res<0) return res;
 return res;
}

/** De-initialize selected swd context and its command queue.
 * \param *libswdctx swd context pointer.
 * \return number of elements freed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_deinit(libswd_ctx_t *libswdctx){
 int res, cmdcnt=0;
 if (libswdctx->membuf.data) free(libswdctx->membuf.data);
 res=libswd_cmdq_free(libswdctx);
 if (res<0) return res;
 cmdcnt=res;
 //TODO what about libswdctx->driver? should be free-ed to!
 res=libswd_deinit_ctx(libswdctx);
 if (res<0) return res;
 return cmdcnt+res;
}

/** @} */
