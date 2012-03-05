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

/** \file libswd_error.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup swd_error Error handling and information routines.
 * @{
 ******************************************************************************/

char *swd_error_string(swd_error_code_t error){
 switch (error){
  case SWD_OK:                 return "[SWD_OK] hmm, there was no error";
  case SWD_ERROR_GENERAL:      return "[SWD_ERROR_GENERAL] general error";
  case SWD_ERROR_NULLPOINTER:  return "[SWD_ERROR_NULLPOINTER] null pointer";
  case SWD_ERROR_NULLQUEUE:    return "[SWD_ERROR_NULLQUEUE] null queue";
  case SWD_ERROR_NULLTRN:      return "[SWD_ERROR_NULLTRN] null turnaround";
  case SWD_ERROR_PARAM:        return "[SWD_ERROR_PARAM] bad parameter";
  case SWD_ERROR_OUTOFMEM:     return "[SWD_ERROR_OUTOFMEM] out of memory";
  case SWD_ERROR_RESULT:       return "[SWD_ERROR_RESULT] bad result";
  case SWD_ERROR_RANGE:        return "[SWD_ERROR_RANGE] out of range";
  case SWD_ERROR_DEFINITION:   return "[SWD_ERROR_DEFINITION] definition error";
  case SWD_ERROR_NULLCONTEXT:  return "[SWD_ERROR_NULLCONTEXT] null context";
  case SWD_ERROR_QUEUE:        return "[SWD_ERROR_QUEUE] queue error";
  case SWD_ERROR_ADDR:         return "[SWD_ERROR_ADDR] addressing error";
  case SWD_ERROR_APnDP:        return "[SWD_ERROR_APnDP] bad APnDP value";
  case SWD_ERROR_RnW:          return "[SWD_ERROR_RnW] bad RnW value";
  case SWD_ERROR_PARITY:       return "[SWD_ERROR_PARITY] parity error";
  case SWD_ERROR_ACK:          return "[SWD_ERROR_ACK] acknowledge error";
  case SWD_ERROR_ACKUNKNOWN:   return "[SWD_ERROR_ACKUNKNOWN] got unknown acknowledge";
  case SWD_ERROR_ACKNOTDONE:   return "[SWD_ERROR_ACKNOTDONE] not yet executed on target";
  case SWD_ERROR_ACKMISSING:   return "[SWD_ERROR_ACKMISSING] command not found on the queue";
  case SWD_ERROR_ACKMISMATCH:  return "[SWD_ERROR_ACKMISMATCH] different result address/value expected";
  case SWD_ERROR_ACKORDER:     return "[SWD_ERROR_ACKORDER] cmdq not in sequence REQ->TRN->ACK";
  case SWD_ERROR_BADOPCODE:    return "[SWD_ERROR_BADOPCODE] unsupported operation requested";
  case SWD_ERROR_NODATACMD:    return "[SWD_ERROR_NODATACMD] command not found on the queue";
  case SWD_ERROR_DATAPTR:      return "[SWD_ERROR_DATAPTR] bad data pointer address";
  case SWD_ERROR_NOPARITYCMD:  return "[SWD_ERROR_NOPARITYCMD] parity command missing or misplaced";
  case SWD_ERROR_PARITYPTR:    return "[SWD_ERROR_PARITYPTR] bad parity pointer address";
  case SWD_ERROR_NOTDONE:      return "[SWD_ERROR_NOTDONE] could not end selected task";
  case SWD_ERROR_QUEUEROOT:    return "[SWD_ERROR_QUEUEROOT] queue root not found or null";
  case SWD_ERROR_QUEUETAIL:    return "[SWD_ERROR_QUEUETAIL] queue tail not found or null";
  case SWD_ERROR_BADCMDTYPE:   return "[SWD_ERROR_BADCMDTYPE] unknown command detected";
  case SWD_ERROR_BADCMDDATA:   return "[SWD_ERROR_BADCMDDATA] command contains bad data (out of range, etc)";
  case SWD_ERROR_ACK_WAIT:     return "[SWD_ERROR_ACK_WAIT] got ACK_WAIT response";
  case SWD_ERROR_ACK_FAULT:    return "[SWD_ERROR_ACK_FAULT] got ACK_FAULT response";
  case SWD_ERROR_QUEUENOTFREE: return "[SWD_ERROR_QUEUENOTFREE] cannot free resources, queue not empty";
  case SWD_ERROR_TRANSPORT:    return "[SWD_ERROR_TRANSPORT] transport error or undefined";
  case SWD_ERROR_DIRECTION:    return "[SWD_ERROR_DIRECTION] MSb/LSb direction error";
  case SWD_ERROR_UNHANDLED:    return "[SWD_ERROR_UNHANDLED] cannot handle that error automatically";
  case SWD_ERROR_MAXRETRY:     return "[SWD_ERROR_MAXRETRY] maximum retry count exceeded";
  default:                     return "undefined error";
 }
 return "undefined error";
}


int swd_error_handle(swd_ctx_t *swdctx){
 // At this point we got negative return code from swd_cmd_flush() so we need to handle errors accordingly here.
 // swdctx->cmdq should point to the last element executed that produced error.
 int retval;
 swd_cmd_t *exectail;

 // Verify if swdctx->cmdq contains last executed element, correct if necessary.
 exectail=swd_cmdq_find_exectail(swdctx->cmdq);
 if (exectail==NULL) {
  swd_log(swdctx, SWD_LOGLEVEL_ERROR, "SWD_E: swd_error_handle(swdctx=@%p): Cannot find last executed element on the queue!\n", (void*)swdctx);
  return SWD_ERROR_QUEUE;
 }
 if (exectail!=swdctx->cmdq){
  swd_log(swdctx, SWD_LOGLEVEL_WARNING, "SWD_W: swd_error_handle(swdctx=@%p): Correcting swdctx->cmdq to match last executed element...\n", (void*)swdctx);
  swdctx->cmdq=exectail;
 } 

 switch (swdctx->cmdq->cmdtype){
  case SWD_CMDTYPE_MISO_ACK:
   retval=swd_error_handle_ack(swdctx);
   break;
  default:
   return SWD_ERROR_UNHANDLED;
 }

 if (retval<0){
  swd_log(swdctx, SWD_LOGLEVEL_ERROR, "swd_error_handle(@%p) failed! on cmdq=@%p", (void*)swdctx, (void*)swdctx->cmdq); 
 }
 return retval;
}

int swd_error_handle_ack(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 // Make sure we are working on the ACK cmdq element.
 if (swdctx->cmdq->cmdtype!=SWD_CMDTYPE_MISO_ACK){
  swd_log(swdctx, SWD_LOGLEVEL_ERROR, "swd_error_handle_ack(@%p):swdctx->cmdq does not point to ACK!", (void*)swdctx);
  return SWD_ERROR_UNHANDLED; //do we want to handle this kind of error here?
 }

 switch (swdctx->cmdq->ack) {
  case SWD_ACK_OK_VAL:
   // Uhm, there was no error.
   return SWD_OK;
  case SWD_ACK_WAIT_VAL:
   return swd_error_handle_ack_wait(swdctx);
  case SWD_ACK_FAULT_VAL:
   // TODO: Handle ACK=FAULT accordingly.
   return SWD_ERROR_UNHANDLED;
  default:
   // TODO: By default we assume lost synchronization, handle accordingly.
   return SWD_ERROR_UNHANDLED;
 }
}

int swd_error_handle_ack_wait(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 // Make sure we are working on the ACK cmdq element.
 if (swdctx->cmdq->cmdtype!=SWD_CMDTYPE_MISO_ACK){
  swd_log(swdctx, SWD_LOGLEVEL_ERROR, "swd_error_handle_ack_wait(@%p):swdctx->cmdq does not point to ACK!", (void*)swdctx);
  return SWD_ERROR_UNHANDLED; //do we want to handle this kind of error here?
 }
 // Make sure the ACK contains WAIT response.
 if (swdctx->cmdq->ack!=SWD_ACK_WAIT_VAL){
  swd_log(swdctx, SWD_LOGLEVEL_ERROR, "swd_error_handle_ack_wait(@%p):swdctx->cmdq->ack does not contain WAIT response!", (void*)swdctx);
  return SWD_ERROR_ACKMISMATCH;
 }

 // Remember original cmdq, restore on return.
 swd_cmd_t mastercmdq = *swdctx->cmdq;

 // Create new error handling queue, fix sticky flags and retry operation.
 int retval, *ctrlstat, *rdata, abort;
 retval=swd_cmdq_init(swdctx->cmdq->errors);
 if (retval<0) goto swd_error_handle_ack_wait_end;
 swdctx->cmdq=swdctx->cmdq->errors; // From now, this becomes out main cmdq for use with standard functions.

 // ctrlstat clenup / read retry body below 
 int retrycnt;
 for (retrycnt=SWD_RETRY_COUNT_DEFAULT; retrycnt>0; retrycnt--){
  retval=swd_dp_read(swdctx, SWD_OPERATION_EXECUTE, SWD_DP_CTRLSTAT_ADDR, &ctrlstat);
  if (retval<0) goto swd_error_handle_ack_wait_end;
  abort=0x00000014;
  retval=swd_dp_write(swdctx, SWD_OPERATION_EXECUTE, SWD_DP_ABORT_ADDR, &abort);
  if (retval<0) goto swd_error_handle_ack_wait_end;
  retval=swd_dp_read(swdctx, SWD_OPERATION_EXECUTE, SWD_DP_CTRLSTAT_ADDR, &ctrlstat);
  if (retval<0) goto swd_error_handle_ack_wait_end;
  if (*ctrlstat&SWD_DP_CTRLSTAT_READOK){
   retval=swd_dp_read(swdctx, SWD_OPERATION_EXECUTE, SWD_DP_RDBUFF_ADDR, &rdata);
   if (retval<0) goto swd_error_handle_ack_wait_end;
   break;
  }
 }
 if (retrycnt==0){
  retval=SWD_ERROR_MAXRETRY;
  goto swd_error_handle_ack_wait_end;
 }
  
 // At this point we should have the read result from RDBUFF ready for MEM-AP read fix. 


swd_error_handle_ack_wait_end:
 // Exit ACK WAIT handling routine, verify retval before return.
 if (retval<0){
  swd_log(swdctx, SWD_LOGLEVEL_ERROR, "SWD_E: swd_error_handle_ack_wait(swdctx=@%p): %s\n", (void*)swdctx, swd_error_string(retval));
 }

 *swdctx->cmdq=mastercmdq;
 while (1) {printf("ACK WAIT HANDLER\n");sleep(1);}
 return retval;
}
/** @} */
