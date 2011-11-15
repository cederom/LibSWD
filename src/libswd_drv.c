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

/** \file libswd_drv.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup swd_drv SWD Bus and Interface Driver Transfer Functions that
 * executes command queue.
 * @{
 ******************************************************************************/

extern int swd_drv_mosi_8(swd_ctx_t *swdctx, swd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
extern int swd_drv_mosi_32(swd_ctx_t *swdctx, swd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
extern int swd_drv_miso_8(swd_ctx_t *swdctx, swd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
extern int swd_drv_miso_32(swd_ctx_t *swdctx, swd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
extern int swd_drv_mosi_trn(swd_ctx_t *swdctx, int bits);
extern int swd_drv_miso_trn(swd_ctx_t *swdctx, int bits);

/** Transmit selected command from the command queue to the interface driver.
 * Also update the swdctx->log structure (this should be done only here!).
 * Because commands that were queued does not get ack/parity data anymore,
 * we need to verify ACK/PARITY that was just read and return error if necesary.
 * When ACK/PARITY error is detected queue tail is removed as it is invalid.
 * When CTRL/STAT:STICKYORUN=1 ACK={WAIT,FAULT] requires additional data phase.
 * \param *swdctx swd context pointer.
 * \param *cmd pointer to the command to be sent.
 * \return number of commands transmitted (1), or SWD_ERROR_CODE on failure.
 */ 
int swd_drv_transmit(swd_ctx_t *swdctx, swd_cmd_t *cmd){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (cmd==NULL) return SWD_ERROR_NULLPOINTER;
  
 int res=SWD_ERROR_BADCMDTYPE, errcode=SWD_ERROR_RESULT;

 switch (cmd->cmdtype){
  case SWD_CMDTYPE_MOSI:
  case SWD_CMDTYPE_MISO:
   swd_log(swdctx, SWD_LOGLEVEL_WARNING, "This command does not contain payload.");
   break;

  case SWD_CMDTYPE_MOSI_CONTROL:
   // 8 clock cycles.
   if (cmd->bits!=8) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, cmd, &cmd->control, 8, SWD_DIR_LSBFIRST);
   swdctx->log.write.control=cmd->control;
   break;

  case SWD_CMDTYPE_MOSI_BITBANG:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, cmd, &cmd->mosibit, 1, SWD_DIR_LSBFIRST);
   swdctx->log.write.bitbang=cmd->mosibit;
   break;

  case SWD_CMDTYPE_MOSI_PARITY:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, cmd, &cmd->parity, 1, SWD_DIR_LSBFIRST);
   swdctx->log.write.parity=cmd->parity;
   break;

  case SWD_CMDTYPE_MOSI_TRN:
   // 1..4-bit clock cycle.
   if (cmd->bits<SWD_TURNROUND_MIN_VAL && cmd->bits>SWD_TURNROUND_MAX_VAL)
    return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_trn(swdctx, cmd->bits);
   break;

  case SWD_CMDTYPE_MOSI_REQUEST:
   // 8 clock cycles.
   if (cmd->bits!=SWD_REQUEST_BITLEN) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, cmd, &cmd->request, 8, SWD_DIR_LSBFIRST);
   swdctx->log.write.request=cmd->request;
   // Log human-readable request fields for easier transmission debug.
   unsigned int addr,rnw;
   addr=((cmd->request&(1<<SWD_REQUEST_A3_BITNUM))?1:0)<<1;
   addr|=(cmd->request&(1<<SWD_REQUEST_A2_BITNUM))?1:0;
   rnw=(cmd->request&(1<<SWD_REQUEST_RnW_BITNUM));
   swd_log(swdctx, SWD_LOGLEVEL_DEBUG, \
    "SWD_D: Sending Request: %s %s Addr=%d%d/%s Parity=%d \n",\
    rnw?"R":"W", \
    (cmd->request&(1<<SWD_REQUEST_APnDP_BITNUM))?"AP":"DP", \
    (cmd->request&(1<<SWD_REQUEST_A3_BITNUM))?1:0, \
    (cmd->request&(1<<SWD_REQUEST_A2_BITNUM))?1:0, \
    swd_dap_register_string(swdctx, rnw, addr), \
    (cmd->request&(1<<SWD_REQUEST_PARITY_BITNUM))?1:0);
   break;

  case SWD_CMDTYPE_MOSI_DATA:
   // 32 clock cycles.
   if (cmd->bits!=SWD_DATA_BITLEN) return SWD_ERROR_BADCMDDATA; 
   res=swd_drv_mosi_32(swdctx, cmd, &cmd->mosidata, 32, SWD_DIR_LSBFIRST);
   swdctx->log.write.data=cmd->mosidata;
   break;

  case SWD_CMDTYPE_MISO_ACK:
   // 3 clock cycles.
   if (cmd->bits!=SWD_ACK_BITLEN) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_8(swdctx, cmd, &cmd->ack, cmd->bits, SWD_DIR_LSBFIRST);
   swdctx->log.read.ack=cmd->ack;
   break;

  case SWD_CMDTYPE_MISO_BITBANG:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_8(swdctx, cmd, &cmd->misobit, 1, SWD_DIR_LSBFIRST);
   swdctx->log.read.bitbang=cmd->misobit;
   break;

  case SWD_CMDTYPE_MISO_PARITY:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_8(swdctx, cmd, &cmd->parity, 1, SWD_DIR_LSBFIRST);
   swdctx->log.read.parity=cmd->parity;
   break;

  case SWD_CMDTYPE_MISO_TRN:
   // 1..4 clock cycles
   if (cmd->bits<SWD_TURNROUND_MIN_VAL && cmd->bits>SWD_TURNROUND_MAX_VAL)
    return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_trn(swdctx, cmd->bits);
   break;

  case SWD_CMDTYPE_MISO_DATA:
   // 32 clock cycles
   if (cmd->bits!=SWD_DATA_BITLEN) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_32(swdctx, cmd, &cmd->misodata, cmd->bits, SWD_DIR_LSBFIRST);
   swdctx->log.read.data=cmd->misodata;
   break;

  case SWD_CMDTYPE_UNDEFINED:
   res=0;
   break; 

  default:
   return SWD_ERROR_BADCMDTYPE;
 } 

 swd_log(swdctx, SWD_LOGLEVEL_PAYLOAD,
  "SWD_P: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p) bits=%-2d cmdtype=%-12s returns=%-3d payload=0x%08x (%s)\n",
  swdctx, cmd, cmd->bits, swd_cmd_string_cmdtype(cmd), res,
  (cmd->bits>8)?cmd->data32:cmd->data8,
  (cmd->bits<=8)?swd_bin8_string(&cmd->data8):swd_bin32_string(&cmd->data32));

 if (res<0) return res;
 cmd->done=1;

 /* Now verify the ACK value and notify caller about possible errors.
  * If error was detected, delete trailing queue elements, maybe perform data phase.
  * Accodring to ADIv5.0 specification (ARM IHI 0031A, section 5.4.5) data phase is required when STICKYORUN=1.
  * Unfortunately at this point we cannot read the CTRL/STAT flag, so we will write zeros to avoid random Request.
  */
 if (cmd->cmdtype==SWD_CMDTYPE_MISO_ACK){
  switch(cmd->ack){
   // If the ACK was OK then simply return to the caller.
   case SWD_ACK_OK_VAL: return res;
   // For other ACK codes produce a warning and remember the code.
   case SWD_ACK_FAULT_VAL:
    swd_log(swdctx, SWD_LOGLEVEL_WARNING,
      "SWD_W: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): SWD_ACK_FAULT detected!\n",
      (void*)swdctx, (void*)cmd );
    errcode=SWD_ERROR_ACK_FAULT;
    break;
   case SWD_ACK_WAIT_VAL:
    swd_log(swdctx, SWD_LOGLEVEL_WARNING,
      "SWD_W: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): SWD_ACK_WAIT detectd!\n",
      (void*)swdctx, (void*)cmd );
    errcode=SWD_ERROR_ACK_WAIT;
    break;
   default:
    swd_log(swdctx, SWD_LOGLEVEL_ERROR,
      "SWD_E: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): Unknown ACK detected / Protocol Error Sequence! DAP Stalled or Target Power Off?\n",
      (void*)swdctx, (void*)cmd );
    errcode=SWD_ERROR_ACKUNKNOWN;
  }
  // Now log error, clean cmdq tail (as it contains invalid operations).
  swd_log(swdctx, SWD_LOGLEVEL_INFO,
    "SWD_I: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): ACK!=OK, clearing cmdq tail to preserve synchronization...\n",
    (void*)swdctx, (void*)cmd );
  if (swd_cmdq_free_tail(cmd)<0) {
   swd_log(swdctx, SWD_LOGLEVEL_ERROR,
     "SWD_E: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): Cannot free cmdq tail in ACK error hanlig routine, Protocol Error Sequence imminent...\n",
     (void*)swdctx, (void*)cmd );
   return SWD_ERROR_QUEUENOTFREE;
  }
  // If ACK={WAIT,FAULT} then append data phase and again flush the queue to maintain sync.
  // MOSI_TRN + 33 zero data cycles should be universal for STICKYORUN={0,1} ???
  if (errcode==SWD_ERROR_ACK_WAIT || errcode==SWD_ERROR_ACK_FAULT){
   swd_log(swdctx, SWD_LOGLEVEL_DEBUG, "SWD_D: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): Performing data phase after ACK={WAIT,FAULT}...\n");
   int data=0, parity=0;
   res=swd_bus_write_data_p(swdctx, SWD_OPERATION_EXECUTE, &data, &parity);
   if (res<0){
    swd_log(swdctx, SWD_LOGLEVEL_ERROR,
      "SWD_E: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): Cannot perform data phase after ACK=WAIT/FAIL, Protocol Error Sequence imminent...\n",
      (void*)swdctx, (void*)cmd );
   }
   // Caller now should read CTRL/STAT and clear STICKY Error Flags.
  }
  // If ACK={UNKNOWN} then we probably have Protocol Error Sequence. We need to reset+detect DAP.
  if (errcode==SWD_ERROR_ACKUNKNOWN){
   swd_log(swdctx, SWD_LOGLEVEL_DEBUG, "SWD_D: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): Trying to detect Target...\n", (void*)swdctx, (void*)cmd);
   int *idcode;
   res=swd_dap_detect(swdctx, SWD_OPERATION_EXECUTE, &idcode);
   if (res<0){
    swd_log(swdctx, SWD_LOGLEVEL_ERROR,
      "SWD_E: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): Error Detecting DAP after Protocol Error Sequence!\n",
      (void*)swdctx, (void*)cmd );
   }
  }
  return errcode;
 }


 /* Verify the PARITY value and notify caller about possible errors.
  * If error was detected, delete trailing queue elements.
  */
 if (cmd->cmdtype==SWD_CMDTYPE_MISO_PARITY){
  // Parity must be preceded with data, look for that data and verify parity.
  if (cmd->prev->cmdtype==SWD_CMDTYPE_MISO_DATA){
   char testparity;
   // Calculate parity based on data value or give warning it cannot be performed.
   if (swd_bin32_parity_even(&cmd->prev->misodata, &testparity)<0)
    swd_log(swdctx, SWD_LOGLEVEL_WARNING,
      "SWD_W: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): Cannot perform parity check (calculation error).\n",
      (void*)swdctx, (void*)cmd );
   // Verify calculated data parity with value received from target.
   if (cmd->parity!=testparity){
    // Give error message.
    swd_log(swdctx, SWD_LOGLEVEL_ERROR,
      "SWD_E: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): Parity mismatch detected (%s/%d)!\n",
      (void*)swdctx, (void*)cmd, swd_bin32_string(&cmd->prev->misodata), cmd->parity );
    // Clean the cmdq tail (as it contains invalid operations).
    swd_log(swdctx, SWD_LOGLEVEL_WARNING,
      "SWD_W: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): Bad PARITY, clearing cmdq tail to preserve synchronization...\n",
      (void*)swdctx, (void*)cmd );
    if (swd_cmdq_free_tail(cmd)<0) {
     swd_log(swdctx, SWD_LOGLEVEL_ERROR,
       "SWD_E: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): Cannot free cmdq tail in PARITY error hanlig routine!\n",
       (void*)swdctx, (void*)cmd);
     return SWD_ERROR_QUEUENOTFREE;
    }
    // Return parity error.
    return SWD_ERROR_PARITY;
   }
  } else {
   // If data element was not found then parity cannot be calculated.
   // Give warning about that but does not return an error, as queue might be cleaned just before.
   swd_log(swdctx, SWD_LOGLEVEL_WARNING,
     "SWD_W: swd_drv_transmit(swdctx=@0x%p, cmd=@0x%p): Cannot perform parity check (data missing).\n",
     (void*)swdctx, (void*)cmd );
   return res;
  }
 }

 /* Everyting went fine, return number of elements processed. */
 return res;
}

/** @} */
