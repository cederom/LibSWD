/*
 * Serial Wire Debug Open Library.
 * Debug Functionalities Body File.
 *
 * Copyright (C) 2013, Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)
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
 * Written by Tomasz Boleslaw CEDRO <cederom@tlen.pl>, 2013;
 *
 */

/** \file libswd_debug.c Debug Related Routines. */

#include <libswd.h>

/*******************************************************************************
 * \defgroup libswd_debug High-level Debug operations using SWD DAP.
 ******************************************************************************/

/** Detect Debug Unit.
 * This is the full initialization and setup of the SW-DP.
 * It may come handy to bring DAP to a known state on error/stall etc.
 * \param *libswdctx swd context pointer.
 * \param operation type (LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE).
 * \return LIBSWD_OK if Debug Unit is supported, LIBSWD_ERROR_UNSUPPORTED otherwise.
 */
int libswd_debug_detect(libswd_ctx_t *libswdctx, libswd_operation_t operation)
{
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_I: Executing libswd_debug_detect(*libswdctx=%p, operation=%s)\n", (void*)libswdctx, libswd_operation_string(operation));

 if (!libswdctx) return LIBSWD_ERROR_NULLCONTEXT;
 int retval=0, cpuid;
 unsigned int i;

 if (!libswdctx->log.memap.initialized)
 {
  retval=libswd_memap_init(libswdctx, operation);
  if (retval<0) return retval;
 }

 retval=libswd_memap_read_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_ARM_DEBUG_CPUID_ADDR, 1, &cpuid);
 if (retval<0) return retval;

 for (i=0;i<LIBSWD_NUM_SUPPORTED_CPUIDS;i++)
 {
  if (cpuid==libswd_arm_debug_CPUID[i].default_value)
  {
   libswdctx->log.debug.cpuid=libswd_arm_debug_CPUID[i];
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
              "LIBSWD_I: libswd_debug_detect(): Found supported CPUID=0x%08X (%s).\n",
              libswd_arm_debug_CPUID[i].default_value, libswd_arm_debug_CPUID[i].name );
   break;
  }
 }
 if (i==LIBSWD_NUM_SUPPORTED_CPUIDS) {
	 libswdctx->log.debug.cpuid.default_value=cpuid;
	 return LIBSWD_ERROR_UNSUPPORTED;
 }
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_I: libswd_debug_detect(*libswdctx=%p, operation=%s) execution OK...\n", (void*)libswdctx, libswd_operation_string(operation));
 return LIBSWD_OK;
}


int libswd_debug_init(libswd_ctx_t *libswdctx, libswd_operation_t operation)
{
 if (!libswdctx) return LIBSWD_ERROR_NULLCONTEXT;
 if ( operation!=LIBSWD_OPERATION_EXECUTE && operation!=LIBSWD_OPERATION_ENQUEUE) return LIBSWD_ERROR_BADOPCODE;

 int retval;
 retval=libswd_debug_detect(libswdctx, operation);
 if (retval<0) return retval;
 libswdctx->log.debug.initialized=1;
 return LIBSWD_OK;
}


int libswd_debug_halt(libswd_ctx_t *libswdctx, libswd_operation_t operation)
{
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_EXECUTE && operation!=LIBSWD_OPERATION_ENQUEUE) return LIBSWD_ERROR_PARAM;

 int retval, i, dbgdhcsr;

 if (!libswdctx->log.debug.initialized)
 {
  retval=libswd_debug_init(libswdctx, operation);
  if (retval<0) return retval;
 }
 // Halt the CPU.
 retval=libswd_memap_read_int_32(libswdctx, operation, LIBSWD_ARM_DEBUG_DHCSR_ADDR, 1, &dbgdhcsr);
 if (retval<0) return retval;
 for (i=LIBSWD_RETRY_COUNT_DEFAULT;i;i--)
 {
  dbgdhcsr=LIBSWD_ARM_DEBUG_DHCSR_DBGKEY;
  dbgdhcsr|=LIBSWD_ARM_DEBUG_DHCSR_CDEBUGEN;
  dbgdhcsr|=LIBSWD_ARM_DEBUG_DHCSR_CHALT;
  dbgdhcsr&=~LIBSWD_ARM_DEBUG_DHCSR_CMASKINTS;
  retval=libswd_memap_write_int_32(libswdctx, operation, LIBSWD_ARM_DEBUG_DHCSR_ADDR, 1, &dbgdhcsr);
  if (retval<0) return retval;
  retval=libswd_memap_read_int_32(libswdctx, operation, LIBSWD_ARM_DEBUG_DHCSR_ADDR, 1, &dbgdhcsr);
  if (retval<0) return retval;
  if (dbgdhcsr&LIBSWD_ARM_DEBUG_DHCSR_SHALT)
  {
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "LIBSWD_I: libswd_debug_halt(): DHCSR=0x%08X\n", dbgdhcsr);
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N: libswd_debug_halt(): TARGET HALT OK!\n");
   libswdctx->log.debug.dhcsr=dbgdhcsr;
   return LIBSWD_OK;
  }
 }
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR, "LIBSWD_E: libswd_debug_halt(): TARGET HALT ERROR!\n");
 return LIBSWD_ERROR_MAXRETRY;
}

int libswd_debug_run(libswd_ctx_t *libswdctx, libswd_operation_t operation)
{
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_EXECUTE && operation!=LIBSWD_OPERATION_ENQUEUE) return LIBSWD_ERROR_PARAM;

 int retval, i, dbgdhcsr;

 if (!libswdctx->log.debug.initialized)
 {
  retval=libswd_debug_init(libswdctx, operation);
  if (retval<0) return retval;
 }
 // UnHalt the CPU.
 retval=libswd_memap_read_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_ARM_DEBUG_DHCSR_ADDR, 1, &dbgdhcsr);
 if (retval<0) return retval;
 for (i=LIBSWD_RETRY_COUNT_DEFAULT;i;i--)
 {
  dbgdhcsr=LIBSWD_ARM_DEBUG_DHCSR_DBGKEY;
  dbgdhcsr|=LIBSWD_ARM_DEBUG_DHCSR_CDEBUGEN;
  dbgdhcsr&=~LIBSWD_ARM_DEBUG_DHCSR_CHALT;
  retval=libswd_memap_write_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_ARM_DEBUG_DHCSR_ADDR, 1, &dbgdhcsr);
  if (retval<0) return retval;
  retval=libswd_memap_read_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_ARM_DEBUG_DHCSR_ADDR, 1, &dbgdhcsr);
  if (retval<0) return retval;
  if (!(dbgdhcsr&LIBSWD_ARM_DEBUG_DHCSR_SHALT))
  {
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N: libswd_debug_run(): TARGET RUN OK!\n");
   libswdctx->log.debug.dhcsr=dbgdhcsr;
   return LIBSWD_OK;
  }

 }
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR, "LIBSWD_E: libswd_debug_run(): TARGET RUN ERROR!\n");
 return LIBSWD_ERROR_MAXRETRY;
}

int libswd_debug_is_halted(libswd_ctx_t *libswdctx, libswd_operation_t operation)
{
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_EXECUTE && operation!=LIBSWD_OPERATION_ENQUEUE) return LIBSWD_ERROR_PARAM;

 return (libswdctx->log.debug.dhcsr&LIBSWD_ARM_DEBUG_DHCSR_SHALT)?1:0;
}


/** @} */
