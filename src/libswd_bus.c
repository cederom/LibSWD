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

/** \file libswd_bus.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup libswd_bus SWD Bus Primitives: Request, ACK, Data+Parity, Direction.
 * These functions generate payloads and queue up all elements/commands
 * necessary to perform requested operations on the SWD bus. Depending
 * on "operation" type, elements can be only enqueued on the queue (operation ==
 * LIBSWD_OPERATION_ENQUEUE) or queued and then flushed into hardware driver
 * (operation == LIBSWD_OPERATION_EXECUTE) for immediate effect on the target.
 * Other operations are not allowed for these functions and will produce error.
 * This group of functions is intelligent, so they will react on errors, when
 * operation is LIBSWD_OPERATION_EXECUTE, otherwise simply queue up transaction.
 * These functions are primitives to use by high level functions operating
 * on the Debug Port (DP) or the Access Port (AP).
 * @{
 ******************************************************************************/

/** Append command queue with TRN WRITE/MOSI, if previous command was READ/MISO.
 * \param *libswdctx swd context pointer.
 * \return number of elements appended, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_bus_setdir_mosi(libswd_ctx_t *libswdctx){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int res, cmdcnt=0;
 libswd_cmd_t *cmdqtail=libswd_cmdq_find_tail(libswdctx->cmdq);
 if (cmdqtail==NULL) return LIBSWD_ERROR_QUEUE;
 if ( cmdqtail->prev==NULL || (cmdqtail->cmdtype*LIBSWD_CMDTYPE_MOSI<0) ) {
  res=libswd_cmd_enqueue_mosi_trn(libswdctx);
  if (res<1) return res;
  cmdcnt=+res;
 }
 return cmdcnt;
}

/** Append command queue with TRN READ/MISO, if previous command was WRITE/MOSI.
 * \param *libswdctx swd context pointer.
 * \return number of elements appended, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_bus_setdir_miso(libswd_ctx_t *libswdctx){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int res, cmdcnt=0;
 libswd_cmd_t *cmdqtail=libswd_cmdq_find_tail(libswdctx->cmdq);
 if (cmdqtail==NULL) return LIBSWD_ERROR_QUEUE;
 if (cmdqtail->prev==NULL || (cmdqtail->cmdtype*LIBSWD_CMDTYPE_MISO<0) ) {
  res=libswd_cmd_enqueue_miso_trn(libswdctx);
  if (res<0) return res;
  cmdcnt=+res;
 }
 return cmdcnt;
}

/** Perform Request (write provided raw byte).
 * \param *libswdctx swd context pointer.
 * \param operation type of action to perform with generated request.
 * \param *request request packet raw data
 * \return number of commands processed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_bus_write_request_raw
(libswd_ctx_t *libswdctx, libswd_operation_t operation, char *request){
 /* Verify function parameters.*/
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (request==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;

 /* Bus direction must be MOSI. */
 res=libswd_bus_setdir_mosi(libswdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 /* Append request command to the queue. */
 res=libswd_cmd_enqueue_mosi_request(libswdctx, request);
 if (res<0) return res;
 qcmdcnt+=res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  return qcmdcnt;
 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_cmdq_flush(libswdctx, &libswdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return qcmdcnt+tcmdcnt;
 } else return LIBSWD_ERROR_BADOPCODE;
}

/** Perform Request.
 * \param *libswdctx swd context pointer.
 * \param operation type of action to perform with generated request.
 * \param *APnDP AccessPort (high) or DebugPort (low) access value pointer.
 * \param *RnW Read (high) or Write (low) access value pointer.
 * \param *addr target register address value pointer.
 * \return number of commands processed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_bus_write_request
(libswd_ctx_t *libswdctx, libswd_operation_t operation, char *APnDP, char *RnW, char *addr){
 /* Verify function parameters.*/
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (*APnDP!=0 && *APnDP!=1) return LIBSWD_ERROR_APnDP;
 if (*RnW!=0 && *RnW!=1) return LIBSWD_ERROR_RnW;
 if (*addr<LIBSWD_ADDR_MINVAL || *addr>LIBSWD_ADDR_MAXVAL) return LIBSWD_ERROR_ADDR;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;
 char request;

 /* Generate request bitstream. */
 res=libswd_bitgen8_request(libswdctx, APnDP, RnW, addr, &request);
 if (res<0) return res;

 /* Bus direction must be MOSI. */
 res=libswd_bus_setdir_mosi(libswdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 /* Append request command to the queue. */
 res=libswd_cmd_enqueue_mosi_request(libswdctx, &request);
 if (res<0) return res;
 qcmdcnt+=res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  return qcmdcnt;
 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_cmdq_flush(libswdctx, &libswdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return qcmdcnt+tcmdcnt;
 } else return LIBSWD_ERROR_BADOPCODE;
}

/** Perform ACK read into *ack and verify received data.
 * \param *libswdctx swd context pointer.
 * \param operation type of action to perform with generated request.
 * \param *ack pointer to the result location.
 * \return number of commands processed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_bus_read_ack(libswd_ctx_t *libswdctx, libswd_operation_t operation, char **ack){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (ack==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;
 libswd_cmd_t *tmpcmdq, *cmdqtail;

 /* ACK can only show after REQ_MOSI,TRN_MISO sequence. */
 cmdqtail=libswd_cmdq_find_tail(libswdctx->cmdq);
 if (cmdqtail==NULL) return LIBSWD_ERROR_QUEUE;
 if (cmdqtail->prev==NULL) return LIBSWD_ERROR_ACKORDER;
 /* Check if there is REQ->TRN sequence at the command queue tail. */
 if (cmdqtail->prev->cmdtype!=LIBSWD_CMDTYPE_MOSI_REQUEST
  && cmdqtail->cmdtype!=LIBSWD_CMDTYPE_MISO_TRN){
   /* If not, there should be at least REQ. */
   if (cmdqtail->cmdtype!=LIBSWD_CMDTYPE_MOSI_REQUEST){
    return LIBSWD_ERROR_ACKORDER;
   } else {
    /* TRN was found at queue tail, so we need to append TRN_MISO command. */
    res=libswd_bus_setdir_miso(libswdctx);
    if (res<0) return res;
    qcmdcnt=+res;
   }
 }

 res=libswd_cmd_enqueue_miso_ack(libswdctx, ack);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  return qcmdcnt;
 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_cmdq_flush(libswdctx, &libswdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt+=res;
 } else return LIBSWD_ERROR_BADOPCODE;

 /* Now verify the read result and return/pass error code if necessary. */

 /* Use temporary queue pointer for context queue operations.*/
 tmpcmdq=libswdctx->cmdq;
 /* Search backward for ACK command on the queue (ack we have just appended). */
 while (tmpcmdq->cmdtype!=LIBSWD_CMDTYPE_MISO_ACK){
  if (tmpcmdq->prev==NULL) return LIBSWD_ERROR_ACKMISSING;
  tmpcmdq=tmpcmdq->prev;
 }
 /* If command was found and executed, read received ACK code, or error code. */
 if (tmpcmdq->cmdtype==LIBSWD_CMDTYPE_MISO_ACK && tmpcmdq->done){
  /* Verify data address found on the queue, with pointer selected before run.*/
  if (&tmpcmdq->ack!=*ack) return LIBSWD_ERROR_ACKMISMATCH;
  return qcmdcnt+tcmdcnt;
 } else return LIBSWD_ERROR_ACKNOTDONE;
}

/** Perform (MOSI) data write with provided parity value.
 * \param *libswdctx swd context pointer.
 * \param operation type of action to perform on generated command.
 * \param *data payload value pointer.
 * \param *parity payload parity value pointer.
 * \return number of elements processed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_bus_write_data_p
(libswd_ctx_t *libswdctx, libswd_operation_t operation, int *data, char *parity){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (data==NULL || parity==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;

 res=libswd_bus_setdir_mosi(libswdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=libswd_cmd_enqueue_mosi_data_p(libswdctx, data, parity);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  return qcmdcnt;
 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_cmdq_flush(libswdctx, &libswdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return qcmdcnt+tcmdcnt;
 } else return LIBSWD_ERROR_BADOPCODE;
}


/** Perform (MOSI) data write with automatic parity calculation.
 * \param *libswdctx swd context pointer.
 * \param operation type of action to perform on generated command.
 * \param *data payload value pointer.
 * \return number of elements processed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_bus_write_data_ap(libswd_ctx_t *libswdctx, libswd_operation_t operation, int *data){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;

 res=libswd_bus_setdir_mosi(libswdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=libswd_cmd_enqueue_mosi_data_ap(libswdctx, data);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  return qcmdcnt;
 } else if (operation==LIBSWD_OPERATION_EXECUTE) {
  res=libswd_cmdq_flush(libswdctx, &libswdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return qcmdcnt+tcmdcnt;
 } else return LIBSWD_ERROR_BADOPCODE;
}

/** Perform (MISO) data read.
 * \param *libswdctx swd context pointer.
 * \param operation type of action to perform on generated command.
 * \param *data payload value pointer.
 * \param *parity payload parity value pointer.
 * \return number of elements processed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_bus_read_data_p(libswd_ctx_t *libswdctx, libswd_operation_t operation, int **data, char **parity){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;
 libswd_cmd_t *tmpcmdq;

 res=libswd_bus_setdir_miso(libswdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=libswd_cmd_enqueue_miso_data_p(libswdctx, data, parity);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  return qcmdcnt;
 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_cmdq_flush(libswdctx, &libswdctx->cmdq, operation);
  if (res<2) return res;
  tcmdcnt+=res;
 } else return LIBSWD_ERROR_BADOPCODE;

 /* Now verify the read result and return error if necessary.
  * Maybe iterative approach should be applied, not only last elemnt found..? */

 /* Use temporary queue pointer for context queue operations.*/
 tmpcmdq=libswdctx->cmdq;
 /* Search backward for our MISO_DATA command on the queue. */
 while (tmpcmdq->cmdtype!=LIBSWD_CMDTYPE_MISO_DATA){
  if (tmpcmdq->prev==NULL) return LIBSWD_ERROR_NODATACMD;
  tmpcmdq=tmpcmdq->prev;
 }
 /* There should be parity bit (command) just after data (command). */
 if (tmpcmdq->next->cmdtype!=LIBSWD_CMDTYPE_MISO_PARITY)
  return LIBSWD_ERROR_NOPARITYCMD;
 /* If command found and executed, verify if data points to correct address. */
 if (tmpcmdq->cmdtype==LIBSWD_CMDTYPE_MISO_DATA && tmpcmdq->done){
  if (tmpcmdq->next->cmdtype==LIBSWD_CMDTYPE_MISO_PARITY && tmpcmdq->next->done){
   if (*data!=&tmpcmdq->misodata) return LIBSWD_ERROR_DATAPTR;
   if (*parity!=&tmpcmdq->next->parity) return LIBSWD_ERROR_PARITYPTR;
   return qcmdcnt+tcmdcnt;
  } else return LIBSWD_ERROR_NOTDONE;
 }
 return LIBSWD_OK;
}

/** Write CONTROL byte to the Target's DAP.
 * \param *libswdctx swd context.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param *ctlmsg byte/char array that contains control payload.
 * \param len number of bytes in the *ctlmsg to send.
 * \return number of bytes sent or LIBSWD_ERROR_CODE on failure.
 */
int libswd_bus_write_control(libswd_ctx_t *libswdctx, libswd_operation_t operation, char *ctlmsg, int len){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;
 if (ctlmsg==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (len<1 && len>libswdctx->config.maxcmdqlen) return LIBSWD_ERROR_PARAM;

 int res, qcmdcnt=0, tcmdcnt=0;

 /* Make sure that bus is in MOSI state. */
 res=libswd_bus_setdir_mosi(libswdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=libswd_cmd_enqueue_mosi_control(libswdctx, ctlmsg, len);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  return qcmdcnt;
 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_cmdq_flush(libswdctx, &libswdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt+=res;
 } else return LIBSWD_ERROR_BADOPCODE;
 return LIBSWD_OK;
}

/** @} */
