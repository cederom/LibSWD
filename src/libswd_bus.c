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

/** \file libswd_bus.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup swd_bus SWD Bus Primitives: Request, ACK, Data+Parity, Direction.
 * These functions generate payloads and queue up all elements/commands
 * necessary to perform requested operations on the SWD bus. Depending
 * on "operation" type, elements can be only enqueued on the queue (operation ==
 * SWD_OPERATION_ENQUEUE) or queued and then flushed into hardware driver 
 * (operation == SWD_OPERATION_EXECUTE) for immediate effect on the target.
 * Other operations are not allowed for these functions and will produce error.
 * This group of functions is intelligent, so they will react on errors, when
 * operation is SWD_OPERATION_EXECUTE, otherwise simply queue up transaction.
 * These functions are primitives to use by high level functions operating
 * on the Debug Port (DP) or the Access Port (AP).
 * @{
 ******************************************************************************/

/** Append command queue with TRN WRITE/MOSI, if previous command was READ/MISO.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_bus_setdir_mosi(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 int res, cmdcnt=0;
 swd_cmd_t *cmdqtail=swd_cmdq_find_tail(swdctx->cmdq);
 if (cmdqtail==NULL) return SWD_ERROR_QUEUE;
 if ( cmdqtail->prev==NULL || (cmdqtail->cmdtype*SWD_CMDTYPE_MOSI<0) ) {
  res=swd_cmd_enqueue_mosi_trn(swdctx);
  if (res<1) return res;
  cmdcnt=+res;
 }
 return cmdcnt;
}
 
/** Append command queue with TRN READ/MISO, if previous command was WRITE/MOSI.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_bus_setdir_miso(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 int res, cmdcnt=0;
 swd_cmd_t *cmdqtail=swd_cmdq_find_tail(swdctx->cmdq);
 if (cmdqtail==NULL) return SWD_ERROR_QUEUE;
 if (cmdqtail->prev==NULL || (cmdqtail->cmdtype*SWD_CMDTYPE_MISO<0) ) {
  res=swd_cmd_enqueue_miso_trn(swdctx);
  if (res<0) return res;
  cmdcnt=+res;
 }
 return cmdcnt;
}

/** Perform Request.
 * \param *swdctx swd context pointer.
 * \param operation type of action to perform with generated request.
 * \param *APnDP AccessPort (high) or DebugPort (low) access value pointer.
 * \param *RnW Read (high) or Write (low) access value pointer.
 * \param *addr target register address value pointer.
 * \return number of commands processed, or SWD_ERROR_CODE on failure.
 */
int swd_bus_write_request
(swd_ctx_t *swdctx, swd_operation_t operation, char *APnDP, char *RnW, char *addr){
 /* Verify function parameters.*/
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (*APnDP!=0 && *APnDP!=1) return SWD_ERROR_APnDP;
 if (*RnW!=0 && *RnW!=1) return SWD_ERROR_RnW;
 if (*addr<SWD_ADDR_MINVAL || *addr>SWD_ADDR_MAXVAL) return SWD_ERROR_ADDR;
 if (operation!=SWD_OPERATION_ENQUEUE && operation!=SWD_OPERATION_EXECUTE)
  return SWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;
 char request;

 /* Generate request bitstream. */ 
 res=swd_bitgen8_request(swdctx, APnDP, RnW, addr, &request);
 if (res<0) return res;

 /* Bus direction must be MOSI. */
 res=swd_bus_setdir_mosi(swdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 /* Append request command to the queue. */
 res=swd_cmd_enqueue_mosi_request(swdctx, &request);
 if (res<0) return res;
 qcmdcnt+=res;

 if (operation==SWD_OPERATION_ENQUEUE){
  return qcmdcnt; 
 } else if (operation==SWD_OPERATION_EXECUTE){
  res=swd_cmdq_flush(swdctx, &swdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return qcmdcnt+tcmdcnt;
 } else return SWD_ERROR_BADOPCODE;
}

/** Perform ACK read into *ack and verify received data.
 * \param *swdctx swd context pointer.
 * \param operation type of action to perform with generated request. 
 * \param *ack pointer to the result location.
 * \return number of commands processed, or SWD_ERROR_CODE on failure. 
 */
int swd_bus_read_ack(swd_ctx_t *swdctx, swd_operation_t operation, char **ack){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (ack==NULL) return SWD_ERROR_NULLPOINTER;
 if (operation!=SWD_OPERATION_ENQUEUE && operation!=SWD_OPERATION_EXECUTE)
  return SWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;
 swd_cmd_t *tmpcmdq;

 /* ACK can only show after REQ_MOSI and TRN_MISO. */
 if (swdctx->cmdq->prev==NULL) return SWD_ERROR_ACKORDER;
 /* Check if there is REQ->TRN sequence at the command queue tail. */
 if (swdctx->cmdq->prev->cmdtype!=SWD_CMDTYPE_MOSI_REQUEST
  && swdctx->cmdq->cmdtype!=SWD_CMDTYPE_MISO_TRN){
   /* If not, there should be at least REQ. */
   if (swdctx->cmdq->cmdtype!=SWD_CMDTYPE_MOSI_REQUEST){
    return SWD_ERROR_ACKORDER;
   } else {
    /* TRN was found at queue tail, so we need to append TRN_MISO command. */
    res=swd_bus_setdir_miso(swdctx);
    if (res<0) return res;
    qcmdcnt=+res;
   }
 }

 res=swd_cmd_enqueue_miso_ack(swdctx, ack);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==SWD_OPERATION_ENQUEUE){
  return qcmdcnt;
 } else if (operation==SWD_OPERATION_EXECUTE){
  res=swd_cmdq_flush(swdctx, &swdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt+=res;
 } else return SWD_ERROR_BADOPCODE;

 /* Now verify the read result and return/pass error code if necessary. */

 /* Use temporary queue pointer for context queue operations.*/
 tmpcmdq=swdctx->cmdq;
 /* Search backward for ACK command on the queue (ack we have just appended). */
 while (tmpcmdq->cmdtype!=SWD_CMDTYPE_MISO_ACK){
  if (tmpcmdq->prev==NULL) return SWD_ERROR_ACKMISSING;
  tmpcmdq=tmpcmdq->prev; 
 }
 /* If command was found and executed, read received ACK code, or error code. */
 if (tmpcmdq->cmdtype==SWD_CMDTYPE_MISO_ACK && tmpcmdq->done){
  /* Verify data address found on the queue, with pointer selected before run.*/
  if (&tmpcmdq->ack!=*ack) return SWD_ERROR_ACKMISMATCH;
  return qcmdcnt+tcmdcnt; 
 } else return SWD_ERROR_ACKNOTDONE;
}

/** Perform (MOSI) data write with provided parity value.
 * \param *swdctx swd context pointer.
 * \param operation type of action to perform on generated command.
 * \param *data payload value pointer.
 * \param *parity payload parity value pointer.
 * \return number of elements processed, or SWD_ERROR_CODE on failure.
 */
int swd_bus_write_data_p
(swd_ctx_t *swdctx, swd_operation_t operation, int *data, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL || parity==NULL) return SWD_ERROR_NULLPOINTER;
 if (operation!=SWD_OPERATION_ENQUEUE && operation!=SWD_OPERATION_EXECUTE)
  return SWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;

 res=swd_bus_setdir_mosi(swdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=swd_cmd_enqueue_mosi_data_p(swdctx, data, parity);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==SWD_OPERATION_ENQUEUE){
  return qcmdcnt;       
 } else if (operation==SWD_OPERATION_EXECUTE){
  res=swd_cmdq_flush(swdctx, &swdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return qcmdcnt+tcmdcnt;
 } else return SWD_ERROR_BADOPCODE;
}


/** Perform (MOSI) data write with automatic parity calculation.
 * \param *swdctx swd context pointer.
 * \param operation type of action to perform on generated command.
 * \param *data payload value pointer.
 * \return number of elements processed, or SWD_ERROR_CODE on failure.
 */
int swd_bus_write_data_ap(swd_ctx_t *swdctx, swd_operation_t operation, int *data){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (operation!=SWD_OPERATION_ENQUEUE && operation!=SWD_OPERATION_EXECUTE)
  return SWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;

 res=swd_bus_setdir_mosi(swdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=swd_cmd_enqueue_mosi_data_ap(swdctx, data);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==SWD_OPERATION_ENQUEUE){
  return qcmdcnt;       
 } else if (operation==SWD_OPERATION_EXECUTE) {
  res=swd_cmdq_flush(swdctx, &swdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return qcmdcnt+tcmdcnt;
 } else return SWD_ERROR_BADOPCODE;
}

/** Perform (MISO) data read.
 * \param *swdctx swd context pointer.
 * \param operation type of action to perform on generated command.
 * \param *data payload value pointer.
 * \param *parity payload parity value pointer.
 * \return number of elements processed, or SWD_ERROR_CODE on failure.
 */
int swd_bus_read_data_p(swd_ctx_t *swdctx, swd_operation_t operation, int **data, char **parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (operation!=SWD_OPERATION_ENQUEUE && operation!=SWD_OPERATION_EXECUTE)
  return SWD_ERROR_BADOPCODE;
 
 int res, qcmdcnt=0, tcmdcnt=0;
 swd_cmd_t *tmpcmdq;

 res=swd_bus_setdir_miso(swdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=swd_cmd_enqueue_miso_data_p(swdctx, data, parity);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==SWD_OPERATION_ENQUEUE){
  return qcmdcnt;        
 } else if (operation==SWD_OPERATION_EXECUTE){
  res=swd_cmdq_flush(swdctx, &swdctx->cmdq, operation);
  if (res<2) return res;
  tcmdcnt+=res;
 } else return SWD_ERROR_BADOPCODE;

 /* Now verify the read result and return error if necessary.
  * Maybe iterative approach should be applied, not only last elemnt found..? */

 /* Use temporary queue pointer for context queue operations.*/
 tmpcmdq=swdctx->cmdq;
 /* Search backward for our MISO_DATA command on the queue. */
 while (tmpcmdq->cmdtype!=SWD_CMDTYPE_MISO_DATA){
  if (tmpcmdq->prev==NULL) return SWD_ERROR_NODATACMD;
  tmpcmdq=tmpcmdq->prev; 
 }
 /* There should be parity bit (command) just after data (command). */
 if (tmpcmdq->next->cmdtype!=SWD_CMDTYPE_MISO_PARITY)
  return SWD_ERROR_NOPARITYCMD;
 /* If command found and executed, verify if data points to correct address. */
 if (tmpcmdq->cmdtype==SWD_CMDTYPE_MISO_DATA && tmpcmdq->done){
  if (tmpcmdq->next->cmdtype==SWD_CMDTYPE_MISO_PARITY && tmpcmdq->next->done){
   if (*data!=&tmpcmdq->misodata) return SWD_ERROR_DATAPTR;
   if (*parity!=&tmpcmdq->next->parity) return SWD_ERROR_PARITYPTR;
   return qcmdcnt+tcmdcnt; 
  } else return SWD_ERROR_NOTDONE;
 }
 return SWD_OK;
}

/** Write CONTROL byte to the Target's DAP.
 * \param *swdctx swd context.
 * \param operation can be SWD_OPERATION_ENQUEUE or SWD_OPERATION_EXECUTE.
 * \param *ctlmsg byte/char array that contains control payload.
 * \param len number of bytes in the *ctlmsg to send.
 * \return number of bytes sent or SWD_ERROR_CODE on failure. 
 */
int swd_bus_write_control(swd_ctx_t *swdctx, swd_operation_t operation, char *ctlmsg, int len){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (operation!=SWD_OPERATION_ENQUEUE && operation!=SWD_OPERATION_EXECUTE)
  return SWD_ERROR_BADOPCODE;
 if (ctlmsg==NULL) return SWD_ERROR_NULLPOINTER;
 if (len<1 && len>swdctx->config.maxcmdqlen) return SWD_ERROR_PARAM;

 int res, qcmdcnt=0, tcmdcnt=0;

 /* Make sure that bus is in MOSI state. */
 res=swd_bus_setdir_mosi(swdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=swd_cmd_enqueue_mosi_control(swdctx, ctlmsg, len);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==SWD_OPERATION_ENQUEUE){
  return qcmdcnt;        
 } else if (operation==SWD_OPERATION_EXECUTE){
  res=swd_cmdq_flush(swdctx, &swdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt+=res;
 } else return SWD_ERROR_BADOPCODE;
 return SWD_OK;
}

/** @} */
