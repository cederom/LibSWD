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

/** \file libswd_error.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup libswd_error Error handling and information routines.
 * @{
 ******************************************************************************/

char *libswd_error_string(libswd_error_code_t error){
 switch (error){
  case LIBSWD_OK:                 return "[LIBSWD_OK] hmm, there was no error";
  case LIBSWD_ERROR_GENERAL:      return "[LIBSWD_ERROR_GENERAL] general error";
  case LIBSWD_ERROR_NULLPOINTER:  return "[LIBSWD_ERROR_NULLPOINTER] null pointer";
  case LIBSWD_ERROR_NULLQUEUE:    return "[LIBSWD_ERROR_NULLQUEUE] null queue";
  case LIBSWD_ERROR_NULLTRN:      return "[LIBSWD_ERROR_NULLTRN] null turnaround";
  case LIBSWD_ERROR_PARAM:        return "[LIBSWD_ERROR_PARAM] bad parameter";
  case LIBSWD_ERROR_OUTOFMEM:     return "[LIBSWD_ERROR_OUTOFMEM] out of memory";
  case LIBSWD_ERROR_RESULT:       return "[LIBSWD_ERROR_RESULT] bad result";
  case LIBSWD_ERROR_RANGE:        return "[LIBSWD_ERROR_RANGE] out of range";
  case LIBSWD_ERROR_DEFINITION:   return "[LIBSWD_ERROR_DEFINITION] definition error";
  case LIBSWD_ERROR_NULLCONTEXT:  return "[LIBSWD_ERROR_NULLCONTEXT] null context";
  case LIBSWD_ERROR_QUEUE:        return "[LIBSWD_ERROR_QUEUE] queue error";
  case LIBSWD_ERROR_ADDR:         return "[LIBSWD_ERROR_ADDR] addressing error";
  case LIBSWD_ERROR_APnDP:        return "[LIBSWD_ERROR_APnDP] bad APnDP value";
  case LIBSWD_ERROR_RnW:          return "[LIBSWD_ERROR_RnW] bad RnW value";
  case LIBSWD_ERROR_PARITY:       return "[LIBSWD_ERROR_PARITY] parity error";
  case LIBSWD_ERROR_ACK:          return "[LIBSWD_ERROR_ACK] acknowledge error";
  case LIBSWD_ERROR_ACKUNKNOWN:   return "[LIBSWD_ERROR_ACKUNKNOWN] got unknown acknowledge";
  case LIBSWD_ERROR_ACKNOTDONE:   return "[LIBSWD_ERROR_ACKNOTDONE] not yet executed on target";
  case LIBSWD_ERROR_ACKMISSING:   return "[LIBSWD_ERROR_ACKMISSING] command not found on the queue";
  case LIBSWD_ERROR_ACKMISMATCH:  return "[LIBSWD_ERROR_ACKMISMATCH] different result address/value expected";
  case LIBSWD_ERROR_ACKORDER:     return "[LIBSWD_ERROR_ACKORDER] cmdq not in sequence REQ->TRN->ACK";
  case LIBSWD_ERROR_BADOPCODE:    return "[LIBSWD_ERROR_BADOPCODE] unsupported operation requested";
  case LIBSWD_ERROR_NODATACMD:    return "[LIBSWD_ERROR_NODATACMD] command not found on the queue";
  case LIBSWD_ERROR_DATAPTR:      return "[LIBSWD_ERROR_DATAPTR] bad data pointer address";
  case LIBSWD_ERROR_NOPARITYCMD:  return "[LIBSWD_ERROR_NOPARITYCMD] parity command missing or misplaced";
  case LIBSWD_ERROR_PARITYPTR:    return "[LIBSWD_ERROR_PARITYPTR] bad parity pointer address";
  case LIBSWD_ERROR_NOTDONE:      return "[LIBSWD_ERROR_NOTDONE] could not end selected task";
  case LIBSWD_ERROR_QUEUEROOT:    return "[LIBSWD_ERROR_QUEUEROOT] queue root not found or null";
  case LIBSWD_ERROR_QUEUETAIL:    return "[LIBSWD_ERROR_QUEUETAIL] queue tail not found or null";
  case LIBSWD_ERROR_BADCMDTYPE:   return "[LIBSWD_ERROR_BADCMDTYPE] unknown command detected";
  case LIBSWD_ERROR_BADCMDDATA:   return "[LIBSWD_ERROR_BADCMDDATA] command contains bad data (out of range, etc)";
  case LIBSWD_ERROR_ACK_WAIT:     return "[LIBSWD_ERROR_ACK_WAIT] got ACK_WAIT response";
  case LIBSWD_ERROR_ACK_FAULT:    return "[LIBSWD_ERROR_ACK_FAULT] got ACK_FAULT response";
  case LIBSWD_ERROR_QUEUENOTFREE: return "[LIBSWD_ERROR_QUEUENOTFREE] cannot free resources, queue not empty";
  case LIBSWD_ERROR_TRANSPORT:    return "[LIBSWD_ERROR_TRANSPORT] transport error or undefined";
  case LIBSWD_ERROR_DIRECTION:    return "[LIBSWD_ERROR_DIRECTION] MSb/LSb direction error";
  case LIBSWD_ERROR_LOGLEVEL:     return "[LIBSWD_ERROR_LOGLEVEL] invalid loglevel value";
  case LIBSWD_ERROR_UNHANDLED:    return "[LIBSWD_ERROR_UNHANDLED] cannot handle that error automatically";
  case LIBSWD_ERROR_MAXRETRY:     return "[LIBSWD_ERROR_MAXRETRY] maximum retry count exceeded";
  case LIBSWD_ERROR_CLISYNTAX:    return "[LIBSWD_ERROR_CLISYNTAX] CLI syntax error, see '?' for help";
  case LIBSWD_ERROR_FILE:         return "[LIBSWD_ERROR_FILE] file I/O related problem";
  case LIBSWD_ERROR_UNSUPPORTED:  return "[LIBSWD_ERROR_UNSUPPORTED] Target not supported";
  case LIBSWD_ERROR_MEMAPACCSIZE: return "[LIBSWD_ERROR_MEMAPACCSIZE] Invalid MEM-AP access size";
  case LIBSWD_ERROR_MEMAPALIGN:   return "[LIBSWD_ERROR_MEMAPALIGN] Invalid address alignment for access size";
  default:                        return "undefined error";
 }
 return "undefined error";
}


int libswd_error_handle(libswd_ctx_t *libswdctx){
 // At this point we got negative return code from libswd_cmd_flush() so we need to handle errors accordingly here.
 // libswdctx->cmdq should point to the last element executed that produced error.
 int retval;
 libswd_cmd_t *exectail;

 // Verify if libswdctx->cmdq contains last executed element, correct if necessary.
 exectail=libswd_cmdq_find_exectail(libswdctx->cmdq);
 if (exectail==NULL) {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR, "LIBSWD_E: libswd_error_handle(libswdctx=@%p): Cannot find last executed element on the queue!\n", (void*)libswdctx);
  return LIBSWD_ERROR_QUEUE;
 }
 if (exectail!=libswdctx->cmdq){
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "LIBSWD_I: libswd_error_handle(libswdctx=@%p): Correcting libswdctx->cmdq to match last executed element...\n", (void*)libswdctx);
  libswdctx->cmdq=exectail;
 }

 switch (libswdctx->cmdq->cmdtype){
  case LIBSWD_CMDTYPE_MISO_ACK:
   retval=libswd_error_handle_ack(libswdctx);
   break;
  default:
   return LIBSWD_ERROR_UNHANDLED;
 }

 if (retval<0){
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING, "LIBSWD_W: libswd_error_handle(@%p) failed! on cmdq=@%p", (void*)libswdctx, (void*)libswdctx->cmdq);
 }
 return retval;
}

int libswd_error_handle_ack(libswd_ctx_t *libswdctx){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 // Make sure we are working on the ACK cmdq element.
 if (libswdctx->cmdq->cmdtype!=LIBSWD_CMDTYPE_MISO_ACK){
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR, "LIBSWD_E: libswd_error_handle_ack(@%p):libswdctx->cmdq does not point to ACK!", (void*)libswdctx);
  return LIBSWD_ERROR_UNHANDLED; //do we want to handle this kind of error here?
 }

 switch (libswdctx->cmdq->ack) {
  case LIBSWD_ACK_OK_VAL:
   // Uhm, there was no error.
   // Should we return OK or search for next ACK recursively?
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING, "LIBSWD_W: libswd_error_handle_ack(libswdctx=@%p): ACK=OK, handling wrong element?\n", (void*)libswdctx);
   return LIBSWD_OK;
  case LIBSWD_ACK_WAIT_VAL:
   return libswd_error_handle_ack_wait(libswdctx);
  case LIBSWD_ACK_FAULT_VAL:
   // TODO: Handle ACK=FAULT accordingly.
   return LIBSWD_ERROR_UNHANDLED;
  default:
   // TODO: By default we assume lost synchronization, handle accordingly.
   return LIBSWD_ERROR_UNHANDLED;
 }
}

int libswd_error_handle_ack_wait(libswd_ctx_t *libswdctx){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 // Make sure we are working on the ACK cmdq element.
 if (libswdctx->cmdq->cmdtype!=LIBSWD_CMDTYPE_MISO_ACK){
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING, "LIBSWD_W: libswd_error_handle_ack_wait(libswdctx=@%p):libswdctx->cmdq does not point to ACK!", (void*)libswdctx);
  return LIBSWD_ERROR_UNHANDLED; //do we want to handle this kind of error here?
 }
 // Make sure the ACK contains WAIT response.
 if (libswdctx->cmdq->ack!=LIBSWD_ACK_WAIT_VAL){
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING, "LIBSWD_W: libswd_error_handle_ack_wait(libswdctx=@%p):libswdctx->cmdq->ack does not contain WAIT response!", (void*)libswdctx);
  return LIBSWD_ERROR_ACKMISMATCH;
 }

 //TODO: NOW DECIDE IF AN OPERATION WAS READ OR WRITE AND PERFORM RETRY ACCORDINGLY
 // READ AND WRITE WILL HAVE DIFFERENT RETRY SEQUENCES

 char request = libswdctx->cmdq->prev->prev->request;
 char *ack, *rparity;
 char parity=0;

 // Remember original cmdq and cmdqlen, restore on return.
 libswd_cmd_t *mastercmdq = libswdctx->cmdq;
 unsigned int mastercmdqlen = libswdctx->stats.cmdqlen;

 // Append dummy data phase, fix sticky flags and retry operation.
 int retval=0, *ctrlstat, *rdata, abort, retrycnt=50;
// retval=libswd_cmdq_init(errors);
 libswdctx->cmdq->errors=(libswd_cmd_t*)calloc(1,sizeof(libswd_cmd_t));
 //retval = LIBSWD_ERROR_OUTOFMEM;
 if (libswdctx->cmdq->errors==NULL) goto libswd_error_handle_ack_wait_end;
 libswdctx->cmdq=libswdctx->cmdq->errors; // From now, this becomes out main cmdq for use with standard functions.
 libswdctx->stats.cmdqlen=1; //Do not forget about memory management
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_error_handle_ack_wait(libswdctx=@%p): Performing data phase after ACK={WAIT,FAULT}...\n", (void*)libswdctx);
 int data=0;
 retval=libswd_bus_write_data_p(libswdctx, LIBSWD_OPERATION_EXECUTE, &data, &parity);
 if (retval<0) goto libswd_error_handle_ack_wait_end;

 // NOW WE CAN HANDLE MEM-AP READ RETRY:
 // 1. READ STICKY FLAGS FROM CTRL/STAT
 // 2. CLEAR STICKY FLAGS IN ABORT - this will discard AP transaction
 // 3. RETRY MEM-AP DRW READ - now it must be ACK=OK (it will return last mem-ap read result).
 // 4. READ DP RDBUFF TO OBTAIN READ DATA

 for (retrycnt=50/*LIBSWD_RETRY_COUNT_DEFAULT*/; retrycnt>0; retrycnt--){
  retval=libswd_dp_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_DP_CTRLSTAT_ADDR, &ctrlstat);
  if (retval<0) goto libswd_error_handle_ack_wait_end;
  abort=0x00000014;
  retval=libswd_dp_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_DP_ABORT_ADDR, &abort);
  if (retval<0) goto libswd_error_handle_ack_wait_end;
  retval=libswd_bus_write_request_raw(libswdctx, LIBSWD_OPERATION_ENQUEUE, &request);
  retval=libswd_bus_read_ack(libswdctx, LIBSWD_OPERATION_EXECUTE, &ack);
  if (retval<0 || *ack!=LIBSWD_ACK_OK_VAL) goto libswd_error_handle_ack_wait_end;
  retval=libswd_bus_read_data_p(libswdctx, LIBSWD_OPERATION_EXECUTE, &rdata, &rparity);
  if (retval<0) goto libswd_error_handle_ack_wait_end;

  retval=libswd_dp_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_DP_CTRLSTAT_ADDR, &ctrlstat);
  if (retval<0) goto libswd_error_handle_ack_wait_end;


  if (*ctrlstat&LIBSWD_DP_CTRLSTAT_READOK){
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "=========================GOT RESPONSE===========================\n\n\n");
   retval=libswd_dp_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_DP_RDBUFF_ADDR, &rdata);
   if (retval<0) goto libswd_error_handle_ack_wait_end;
   break;
  }
 }
 if (retrycnt==0){
  retval=LIBSWD_ERROR_MAXRETRY;
  goto libswd_error_handle_ack_wait_end;
 }

 //Make sure we have RDATA and PARITY elements after libswdctx->cmdq.
 //Should we check for this at the procedure start???
 libswd_cmdq_free(libswdctx); //Well, as a first thing, release the memory
 libswdctx->cmdq=mastercmdq; //Revert origal cmdq
 libswdctx->cmdq->errors=NULL; //Delete a pointer which is probably no more valid
 libswdctx->stats.cmdqlen=mastercmdqlen; //Do not forget about original cmdqlen
 if (libswdctx->cmdq->cmdtype==LIBSWD_CMDTYPE_MISO_ACK && libswdctx->cmdq->next->cmdtype==LIBSWD_CMDTYPE_MISO_DATA && libswdctx->cmdq->next->next->cmdtype==LIBSWD_CMDTYPE_MISO_PARITY){
  libswdctx->cmdq->ack=LIBSWD_ACK_OK_VAL;
  libswdctx->cmdq=libswdctx->cmdq->next;
  libswdctx->cmdq->misodata=*rdata;
  libswdctx->cmdq->done=1;
  libswdctx->cmdq=libswdctx->cmdq->next;
  //libswd_bin8_parity_even(rdata, &parity);
  libswdctx->cmdq->parity=*rparity;
  libswdctx->cmdq->done=1;
  return LIBSWD_OK;
 } else libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR, "LIBSWD_E: UNSUPPORTED COMMAND SEQUENCE ON CMDQ (NOT ACK->RDATA->PARITY)\n");


 // At this point we should have the read result from RDBUFF ready for MEM-AP read fix.
 return LIBSWD_ERROR_BADOPCODE;

libswd_error_handle_ack_wait_end:
 // Exit ACK WAIT handling routine, verify retval before return.
 if (retval<0||retrycnt==0){
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR, "LIBSWD_E: libswd_error_handle_ack_wait(libswdctx=@%p) ejecting: %s\n", (void*)libswdctx, libswd_error_string(retval));
 }

 libswd_cmdq_free(libswdctx); //Well, as a first thing, release the memory
 libswdctx->cmdq=mastercmdq; //Revert origal cmdq
 libswdctx->cmdq->errors=NULL; //Delete a pointer which is probably no more valid
 libswdctx->stats.cmdqlen=mastercmdqlen; //Do not forget about original cmdqlen
 while (1) {printf("ACK WAIT HANDLER\n");usleep(1000);} //TODO this is unacceptable!
 return retval;
}
/** @} */
