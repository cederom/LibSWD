/*
 * $Id$
 *
 * Serial Wire Debug Open Library.
 * Library Body File.
 *
 * Copyright (C) 2010-2012, Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)
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
 * Written by Tomasz Boleslaw CEDRO <cederom@tlen.pl>, 2010-2012;
 *
 */

/** \file liblibswd_drv.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup libswd_drv SWD Bus and Interface Driver Transfer Functions that
 * executes command queue.
 * @{
 ******************************************************************************/

extern int libswd_drv_mosi_8(libswd_ctx_t *swdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
extern int libswd_drv_mosi_32(libswd_ctx_t *swdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
extern int libswd_drv_miso_8(libswd_ctx_t *swdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
extern int libswd_drv_miso_32(libswd_ctx_t *swdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
extern int libswd_drv_mosi_trn(libswd_ctx_t *swdctx, int bits);
extern int libswd_drv_miso_trn(libswd_ctx_t *swdctx, int bits);

/** Transmit selected command from the *cmdq to the interface driver.
 * Also update the swdctx->log structure (this should be done only here!).
 * Because commands that were queued does not get ack/parity data anymore,
 * we need to verify ACK/PARITY that was just read and return error if necesary.
 * When ACK/PARITY error is detected queue tail is removed as it is invalid.
 * When CTRL/STAT:STICKYORUN=1 ACK={WAIT,FAULT] requires additional data phase.
 * \param *swdctx swd context pointer.
 * \param *cmd pointer to the command to be sent.
 * \return number of commands transmitted (1), or LIBSWD_ERROR_CODE on failure.
 */ 
int libswd_drv_transmit(libswd_ctx_t *swdctx, libswd_cmd_t *cmd){
 if (swdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (cmd==NULL) return LIBSWD_ERROR_NULLPOINTER;
  
 int res=LIBSWD_ERROR_BADCMDTYPE, errcode=LIBSWD_ERROR_RESULT;

 switch (cmd->cmdtype){
  case LIBSWD_CMDTYPE_MOSI:
  case LIBSWD_CMDTYPE_MISO:
   libswd_log(swdctx, LIBSWD_LOGLEVEL_WARNING, "This command does not contain payload.");
   break;

  case LIBSWD_CMDTYPE_MOSI_CONTROL:
   // 8 clock cycles.
   if (cmd->bits!=8) return LIBSWD_ERROR_BADCMDDATA;
   res=libswd_drv_mosi_8(swdctx, cmd, &cmd->control, 8, LIBSWD_DIR_LSBFIRST);
   if (res>=0) swdctx->log.write.control=cmd->control;
   break;

  case LIBSWD_CMDTYPE_MOSI_BITBANG:
   // 1 clock cycle.
   if (cmd->bits!=1) return LIBSWD_ERROR_BADCMDDATA;
   res=libswd_drv_mosi_8(swdctx, cmd, &cmd->mosibit, 1, LIBSWD_DIR_LSBFIRST);
   if (res>=0) swdctx->log.write.bitbang=cmd->mosibit;
   break;

  case LIBSWD_CMDTYPE_MOSI_PARITY:
   // 1 clock cycle.
   if (cmd->bits!=1) return LIBSWD_ERROR_BADCMDDATA;
   res=libswd_drv_mosi_8(swdctx, cmd, &cmd->parity, 1, LIBSWD_DIR_LSBFIRST);
   if (res>=0) swdctx->log.write.parity=cmd->parity;
   break;

  case LIBSWD_CMDTYPE_MOSI_TRN:
   // 1..4-bit clock cycle.
   if (cmd->bits<LIBSWD_TURNROUND_MIN_VAL && cmd->bits>LIBSWD_TURNROUND_MAX_VAL)
    return LIBSWD_ERROR_BADCMDDATA;
   res=libswd_drv_mosi_trn(swdctx, cmd->bits);
   break;

  case LIBSWD_CMDTYPE_MOSI_REQUEST:
   // 8 clock cycles.
   if (cmd->bits!=LIBSWD_REQUEST_BITLEN) return LIBSWD_ERROR_BADCMDDATA;
   res=libswd_drv_mosi_8(swdctx, cmd, &cmd->request, 8, LIBSWD_DIR_LSBFIRST);
   if (res>=0){
    swdctx->log.write.request=cmd->request;
    // Log human-readable request fields for easier transmission debug.
    libswd_log(swdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: Sending Request: %s\n", \
     libswd_request_string(swdctx, cmd->request));
   }
   break;

  case LIBSWD_CMDTYPE_MOSI_DATA:
   // 32 clock cycles.
   if (cmd->bits!=LIBSWD_DATA_BITLEN) return LIBSWD_ERROR_BADCMDDATA; 
   res=libswd_drv_mosi_32(swdctx, cmd, &cmd->mosidata, 32, LIBSWD_DIR_LSBFIRST);
   if (res>=0) swdctx->log.write.data=cmd->mosidata;
   break;

  case LIBSWD_CMDTYPE_MISO_ACK:
   // 3 clock cycles.
   if (cmd->bits!=LIBSWD_ACK_BITLEN) return LIBSWD_ERROR_BADCMDDATA;
   res=libswd_drv_miso_8(swdctx, cmd, &cmd->ack, cmd->bits, LIBSWD_DIR_LSBFIRST);
   if (res>=0) swdctx->log.read.ack=cmd->ack;
   break;

  case LIBSWD_CMDTYPE_MISO_BITBANG:
   // 1 clock cycle.
   if (cmd->bits!=1) return LIBSWD_ERROR_BADCMDDATA;
   res=libswd_drv_miso_8(swdctx, cmd, &cmd->misobit, 1, LIBSWD_DIR_LSBFIRST);
   if (res>=0) swdctx->log.read.bitbang=cmd->misobit;
   break;

  case LIBSWD_CMDTYPE_MISO_PARITY:
   // 1 clock cycle.
   if (cmd->bits!=1) return LIBSWD_ERROR_BADCMDDATA;
   res=libswd_drv_miso_8(swdctx, cmd, &cmd->parity, 1, LIBSWD_DIR_LSBFIRST);
   if (res>=0) swdctx->log.read.parity=cmd->parity;
   break;

  case LIBSWD_CMDTYPE_MISO_TRN:
   // 1..4 clock cycles
   if (cmd->bits<LIBSWD_TURNROUND_MIN_VAL && cmd->bits>LIBSWD_TURNROUND_MAX_VAL)
    return LIBSWD_ERROR_BADCMDDATA;
   res=libswd_drv_miso_trn(swdctx, cmd->bits);
   break;

  case LIBSWD_CMDTYPE_MISO_DATA:
   // 32 clock cycles
   if (cmd->bits!=LIBSWD_DATA_BITLEN) return LIBSWD_ERROR_BADCMDDATA;
   res=libswd_drv_miso_32(swdctx, cmd, &cmd->misodata, cmd->bits, LIBSWD_DIR_LSBFIRST);
   if (res>=0) swdctx->log.read.data=cmd->misodata;
   break;

  case LIBSWD_CMDTYPE_UNDEFINED:
   res=0;
   break; 

  default:
   return LIBSWD_ERROR_BADCMDTYPE;
 } 

 libswd_log(swdctx, LIBSWD_LOGLEVEL_PAYLOAD,
  "LIBSWD_P: libswd_drv_transmit(swdctx=@%p, cmd=@%p) bits=%-2d cmdtype=%-12s returns=%-3d payload=0x%08x (%s)\n",
  swdctx, cmd, cmd->bits, libswd_cmd_string_cmdtype(cmd), res,
  (cmd->bits>8)?cmd->data32:cmd->data8,
  (cmd->bits<=8)?libswd_bin8_string(&cmd->data8):libswd_bin32_string(&cmd->data32));

 if (res<0) return res;
 cmd->done=1;

 /* Now verify the ACK value, notify caller about possible errors, truncate cmdq if swdctx.config.autofixerrors is not set.
  * Accodring to ADIv5.0 specification (ARM IHI 0031A, section 5.4.5) data phase is required when STICKYORUN=1.
  * Unfortunately at this point we cannot read the CTRL/STAT flag, so we will write zeros to avoid random Request.
  */
 if (cmd->cmdtype==LIBSWD_CMDTYPE_MISO_ACK){
  switch(cmd->ack){
   // If the ACK was OK then simply return to the caller.
   case LIBSWD_ACK_OK_VAL: return res;
   // For other ACK codes produce a warning and remember the code.
   case LIBSWD_ACK_FAULT_VAL:
    libswd_log(swdctx, LIBSWD_LOGLEVEL_WARNING,
      "LIBSWD_W: libswd_drv_transmit(swdctx=@%p, cmd=@%p): LIBSWD_ACK_FAULT detected!\n",
      (void*)swdctx, (void*)cmd );
    errcode=LIBSWD_ERROR_ACK_FAULT;
    break;
   case LIBSWD_ACK_WAIT_VAL:
    libswd_log(swdctx, LIBSWD_LOGLEVEL_WARNING,
      "LIBSWD_W: libswd_drv_transmit(swdctx=@%p, cmd=@%p): LIBSWD_ACK_WAIT detectd!\n",
      (void*)swdctx, (void*)cmd );
    errcode=LIBSWD_ERROR_ACK_WAIT;
    break;
   default:
    libswd_log(swdctx, LIBSWD_LOGLEVEL_ERROR,
      "LIBSWD_E: libswd_drv_transmit(swdctx=@%p, cmd=@%p): Unknown ACK detected / Protocol Error Sequence! DAP Stalled or Target Power Off?\n",
      (void*)swdctx, (void*)cmd );
    errcode=LIBSWD_ERROR_ACKUNKNOWN;
  }
  // If swdctx.config.autofixerrors is not set, on error truncate cmdq, append+execute dummy data phase, then let caller handle situation.
  // The reason for clearing out the queue is to preserve synchronization with Target.
  if (!swdctx->config.autofixerrors){
   libswd_log(swdctx, LIBSWD_LOGLEVEL_DEBUG,
     "LIBSWD_D: libswd_drv_transmit(swdctx=@%p, cmd=@%p): ACK!=OK, clearing cmdq tail to preserve synchronization...\n",
     (void*)swdctx, (void*)cmd );
   if (libswd_cmdq_free_tail(cmd)<0) {
    libswd_log(swdctx, LIBSWD_LOGLEVEL_ERROR,
      "LIBSWD_E: libswd_drv_transmit(swdctx=@%p, cmd=@%p): Cannot free cmdq tail in ACK error handling routine, Protocol Error Sequence imminent...\n",
      (void*)swdctx, (void*)cmd );
    return LIBSWD_ERROR_QUEUENOTFREE;
   }
   // TODO: MOVE THIS INTO SEPARATE ERROR HANDLING ROUTINE
   // If ACK={WAIT,FAULT} then append data phase and again flush the queue to maintain sync.
   // MOSI_TRN + 33 zero data cycles should be universal for STICKYORUN={0,1} ???
   if (errcode==LIBSWD_ERROR_ACK_WAIT || errcode==LIBSWD_ERROR_ACK_FAULT){
    libswd_log(swdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_drv_transmit(swdctx=@%p, cmd=@%p): Performing data phase after ACK={WAIT,FAULT}...\n", (void*)swdctx, (void*)cmd);
    int data=0, parity=0;
    res=libswd_bus_write_data_p(swdctx, LIBSWD_OPERATION_EXECUTE, &data, &parity);
    if (res<0){
     libswd_log(swdctx, LIBSWD_LOGLEVEL_ERROR,
       "LIBSWD_E: libswd_drv_transmit(swdctx=@%p, cmd=@%p): Cannot perform data phase after ACK=WAIT/FAIL, Protocol Error Sequence imminent...\n",
       (void*)swdctx, (void*)cmd );
    }
    // Caller now should read CTRL/STAT and clear STICKY Error Flags at this point.
   }
  } else {
   libswd_log(swdctx, LIBSWD_LOGLEVEL_DEBUG,
     "LIBSWD_D: libswd_drv_transmit(swdctx=@%p, cmd=@%p): swdctx->config.autofixerrors is set, applying error handling...\n", (void*)swdctx, (void*)cmd );
   res=libswd_error_handle(swdctx);
   if (res<0){
    libswd_log(swdctx, LIBSWD_LOGLEVEL_ERROR, "libswd_drv_transmit(swdctx=@%p, @%p): error handling failed, %s\n", (void*)swdctx, (void*)cmd, libswd_error_string(res));
    return res;
   }
   errcode=LIBSWD_OK;
  }
  return errcode;
 }


 /* Verify the PARITY value and notify caller about possible errors.
  * If error was detected, delete trailing queue elements.
  */
 if (cmd->cmdtype==LIBSWD_CMDTYPE_MISO_PARITY){
  // Parity must be preceded with data, look for that data and verify parity.
  if (cmd->prev->cmdtype==LIBSWD_CMDTYPE_MISO_DATA){
   char testparity;
   // Calculate parity based on data value or give warning it cannot be performed.
   if (libswd_bin32_parity_even(&cmd->prev->misodata, &testparity)<0)
    libswd_log(swdctx, LIBSWD_LOGLEVEL_WARNING,
      "LIBSWD_W: libswd_drv_transmit(swdctx=@%p, cmd=@%p): Cannot perform parity check (calculation error).\n",
      (void*)swdctx, (void*)cmd );
   // Verify calculated data parity with value received from target.
   if (cmd->parity!=testparity){
    // Give error message.
    libswd_log(swdctx, LIBSWD_LOGLEVEL_ERROR,
      "LIBSWD_E: libswd_drv_transmit(swdctx=@%p, cmd=@%p): Parity mismatch detected (%s/%d)!\n",
      (void*)swdctx, (void*)cmd, libswd_bin32_string(&cmd->prev->misodata), cmd->parity );
    // Clean the cmdq tail (as it contains invalid operations).
    libswd_log(swdctx, LIBSWD_LOGLEVEL_WARNING,
      "LIBSWD_W: libswd_drv_transmit(swdctx=@%p, cmd=@%p): Bad PARITY, clearing cmdq tail to preserve synchronization...\n",
      (void*)swdctx, (void*)cmd );
    if (libswd_cmdq_free_tail(cmd)<0) {
     libswd_log(swdctx, LIBSWD_LOGLEVEL_ERROR,
       "LIBSWD_E: libswd_drv_transmit(swdctx=@%p, cmd=@%p): Cannot free cmdq tail in PARITY error hanlig routine!\n",
       (void*)swdctx, (void*)cmd);
     return LIBSWD_ERROR_QUEUENOTFREE;
    }
    // Return parity error.
    return LIBSWD_ERROR_PARITY;
   }
  } else {
   // If data element was not found then parity cannot be calculated.
   // Give warning about that but does not return an error, as queue might be cleaned just before.
   libswd_log(swdctx, LIBSWD_LOGLEVEL_WARNING,
     "LIBSWD_W: libswd_drv_transmit(swdctx=@%p, cmd=@%p): Cannot perform parity check (data missing).\n",
     (void*)swdctx, (void*)cmd );
   return res;
  }
 }

 /* Everyting went fine, return number of elements processed. */
 return res;
}

/** @} */
