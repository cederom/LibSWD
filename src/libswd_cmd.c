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

/** \file libswd_cmd.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup swd_cmd_enqueue SWD Command Genration and Enqueueing routines.
 * Command quants are first generated/created in memory, then enqueued into
 * command queue for execution by swd_cmd_flush(). Queue elements are created
 * in memory and filled with payload, then appended to the queue. If enqueue
 * fails, memory for newly created elements is set free before return.
 * All functions here start with "swd_cmd_queue_append_" prefix.
 * Functions here are NOT intelligent, they only create payload in memory,
 * so treat them rather as blocks for high-level functions.
 * @{
 ******************************************************************************/

/** Append selected command to a context's command queue.
 * \param *swdctx swd context pointer containing the command queue.
 * \param *cmd command to be appended to the context's command queue.
 * \return number of elements appended or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue(swd_ctx_t *swdctx, swd_cmd_t *cmd){
 if (swdctx==NULL || cmd==NULL) return SWD_ERROR_NULLPOINTER;
 int res;
 res=swd_cmdq_append(swdctx->cmdq, cmd);
 if (res>0) swdctx->cmdq=cmd;
 return res;
}

/** Appends command queue with SWD Request packet header.
 * Note that contents is not validated, so bad request can be sent as well.
 * \param *swdctx swd context pointer.
 * \param *request pointer to the 8-bit request payload.
 * \return return number elements appended (1), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_request(swd_ctx_t *swdctx, char *request){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (request==NULL) return SWD_ERROR_NULLPOINTER;
 int res;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 cmd->request=*request;
 cmd->bits=SWD_REQUEST_BITLEN;
 cmd->cmdtype=SWD_CMDTYPE_MOSI_REQUEST;
 res=swd_cmd_enqueue(swdctx, cmd);
 if (res<1) free(cmd);
 return res;
} 

/** Append command queue with Turnaround activating MOSI mode.
 * \param *swdctx swd context pointer.
 * \return return number elements appended (1), or SWD_ERROR_CODE on failure.
 */ 
int swd_cmd_enqueue_mosi_trn(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 int res;
  swd_cmd_t *cmd;
  cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
  if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
  cmd->TRNnMOSI=0;
  cmd->bits=swdctx->config.trnlen;
  cmd->cmdtype=SWD_CMDTYPE_MOSI_TRN;
  res=swd_cmd_enqueue(swdctx, cmd);
  if (res<1) free(cmd);
  return res;
}

/** Append command queue with Turnaround activating MISO mode.
 * \param *swdctx swd context pointer.
 * \return return number of elements appended (1), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_miso_trn(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 int res;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 cmd->TRNnMOSI=1;
 cmd->bits=swdctx->config.trnlen;
 cmd->cmdtype=SWD_CMDTYPE_MISO_TRN;
 res=swd_cmd_enqueue(swdctx, cmd);
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with bus binary read bit-by-bit operation.
 * This function will append command to the queue for each bit, and store
 * one bit into single char array element, so read is not constrained to 8 bits.
 * On error memory is released and apropriate error code is returned.
 * Important: Memory pointed by *data must be allocated prior call!
 * \param *swdctx swd context pointer.
 * \param **data allocated data array to write result into.
 * \param count number of bits to read (also the **data size).
 * \return number of elements processed, or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_miso_nbit(swd_ctx_t *swdctx, char **data, int count){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (count<=0) return SWD_ERROR_PARAM;
 int res, res2;
 swd_cmd_t *cmd, *oldcmdq=swdctx->cmdq;
 int i,cmdcnt=0;
 for (i=0;i<count;i++){
  cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
  if (cmd==NULL) {
   res=SWD_ERROR_OUTOFMEM;
   break;
  }
  cmd->bits=1;
  if (data!=NULL) *data=&cmd->misobit;
  cmd->cmdtype=SWD_CMDTYPE_MISO_BITBANG;
  res=swd_cmd_enqueue(swdctx, cmd);
  if (res<1) break;
  cmdcnt+=res;
 }
 //If there was problem enqueueing elements, rollback changes on queue.
 if (res<1) {
  res2=swd_cmdq_free_tail(oldcmdq);
  if (res2<0) return res2;
  return res;
 } else return cmdcnt;
}

/** Append command queue with bus binary write bit-by-bit operation.
 * This function will append command to the queue for each bit and store
 * one bit into single char array element, so read is not constrained to 8 bits.
 * On error memory is released and apropriate error code is returned.
 * Important: Memory pointed by *data must be allocated prior call!
 * \param *swdctx swd context pointer.
 * \param **data allocated data array to write result into.
 * \param count number of bits to read (also the **data size).
 * \return number of elements processed, or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_nbit(swd_ctx_t *swdctx, char *data, int count){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (count<=0) return SWD_ERROR_PARAM;
 int res, res2, cmdcnt=0;
 swd_cmd_t *cmd, *oldcmdq=swdctx->cmdq;
int i;
 for (i=0;i<count;i++){
  cmd=(swd_cmd_t *)calloc(1, sizeof(swd_cmd_t));
  if (cmd==NULL) {
   res=SWD_ERROR_OUTOFMEM;
   break;
  }
  cmd->mosibit=data[i];
  cmd->bits=1;
  cmd->cmdtype=SWD_CMDTYPE_MOSI_BITBANG;
  res=swd_cmd_enqueue(swdctx, cmd);
  if (res<1) break;
  cmdcnt+=res;
 }
 //If there was problem enqueueing elements, rollback changes on queue.
 if (res<1){
  res2=swd_cmdq_free_tail(oldcmdq);
  if (res2<0) return res2;
  swdctx->cmdq=oldcmdq;
  return res;
 } else return cmdcnt;
}

/** Append command queue with parity bit write.
 * \param *swdctx swd context pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (1), or SWD_ERROR_CODE on failure.
 */ 
int swd_cmd_enqueue_mosi_parity(swd_ctx_t *swdctx, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (*parity!=0 && *parity!=1) return SWD_ERROR_PARAM;
 int res;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 cmd->parity=*parity;
 cmd->bits=1;
 cmd->cmdtype=SWD_CMDTYPE_MOSI_PARITY;
 res=swd_cmd_enqueue(swdctx, cmd);
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with parity bit read.
 * \param *swdctx swd context pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (1), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_miso_parity(swd_ctx_t *swdctx, char **parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 int res;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 if (parity!=NULL) *parity=&cmd->parity;
 cmd->bits=1;
 cmd->cmdtype=SWD_CMDTYPE_MISO_PARITY;
 res=swd_cmd_enqueue(swdctx, cmd);
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with data read.
 * \param *swdctx swd context pointer.
 * \param *data data pointer.
 * \return of elements appended (1), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_miso_data(swd_ctx_t *swdctx, int **data){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 int res;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 if (data!=NULL) *data=&cmd->misodata;
 cmd->bits=32;
 cmd->cmdtype=SWD_CMDTYPE_MISO_DATA;
 res=swd_cmd_enqueue(swdctx, cmd); // should be 1 on success
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with data and parity read.
 * \param *swdctx swd context pointer.
 * \param *data data value pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (2), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_miso_data_p(swd_ctx_t *swdctx, int **data, char **parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 int res, cmdcnt=0;
 res=swd_cmd_enqueue_miso_data(swdctx, data);
 if (res<1) return res;
 cmdcnt+=res;
 res=swd_cmd_enqueue_miso_parity(swdctx, parity);
 if (res<1) return res;
 cmdcnt+=res;
 return cmdcnt; // should be 2 or 3(+trn) on success
}

/** Append command queue with series of data and parity read.
 * \param *swdctx swd context pointer.
 * \param **data data value array pointer.
 * \param **parity parity value array pointer.
 * \param count number of (data+parity) elements to read.
 * \return number of elements appended (2*count), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_miso_n_data_p(swd_ctx_t *swdctx, int **data, char **parity, int count){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (count<=0) return SWD_ERROR_PARAM;
 int i,res, cmdcnt=0;
 for (i=0;i<=count;i++){
  res=swd_cmd_enqueue_miso_data_p(swdctx, &data[i], &parity[i]);
  if (res<2) return SWD_ERROR_RESULT;
  cmdcnt=+res;
 }
 return cmdcnt;
}

/** Append command queue with data and parity write.
 * \param *swdctx swd context pointer.
 * \param *data data value pointer.
 * \return number of elements appended (1), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_data(swd_ctx_t *swdctx, int *data){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 int res;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 cmd->mosidata=*data;
 cmd->bits=32;
 cmd->cmdtype=SWD_CMDTYPE_MOSI_DATA;
 res=swd_cmd_enqueue(swdctx, cmd); // should be 1 or 2 on success
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with data and automatic parity write.
 * \param *swdctx swd context pointer.
 * \param *data data value pointer.
 * \return number of elements appended (2), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_data_ap(swd_ctx_t *swdctx, int *data){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 int res, cmdcnt=0;
 char parity;
 res=swd_cmd_enqueue_mosi_data(swdctx, data);
 if (res<1) return res;
 cmdcnt=+res;
 res=swd_bin32_parity_even(data, &parity);
 if (res<0) return res;
 res=swd_cmd_enqueue_mosi_parity(swdctx, &parity);
 if (res<1) return res;
 cmdcnt=+res;
 return cmdcnt; // should be 2 or 3 on success
}

/** Append command queue with data and provided parity write.
 * \param *swdctx swd context pointer.
 * \param *data data value pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (2), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_data_p(swd_ctx_t *swdctx, int *data, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL || parity==NULL) return SWD_ERROR_NULLPOINTER;
 int res, cmdcnt=0;
 res=swd_cmd_enqueue_mosi_data(swdctx, data);
 if (res<1) return res;
 cmdcnt=+res;
 res=swd_cmd_enqueue_mosi_parity(swdctx, parity);
 if (res<1) return res;
 cmdcnt=+res;
 return cmdcnt; // should be 2 or 3 on success
}

/** Append command queue with series of data and automatic parity writes.
 * \param *swdctx swd context pointer.
 * \param **data data value array pointer.
 * \param count number of (data+parity) elements to read.
 * \return number of elements appended (2*count), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_n_data_ap(swd_ctx_t *swdctx, int **data, int count){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (count<1) return SWD_ERROR_PARAM;
 int i, res, cmdcnt=0;
 for (i=0;i<count;i++){
  res=swd_cmd_enqueue_mosi_data(swdctx, data[i]);
  if (res<1) return res;
  cmdcnt=+res;
 }
 return cmdcnt;
}

/** Append command queue with series of data and provided parity writes.
 * \param *swdctx swd context pointer.
 * \param **data data value array pointer.
 * \param **parity parity value array pointer.
 * \param count number of (data+parity) elements to read.
 * \return number of elements appended (2*count), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_n_data_p(swd_ctx_t *swdctx, int **data, char **parity, int count){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (count<1) return SWD_ERROR_PARAM;
 int i, res, cmdcnt=0;
 for (i=0;i<count;i++){
  res=swd_cmd_enqueue_mosi_data_p(swdctx, data[i], parity[i]);
  if (res<1) return res;
  cmdcnt=+res;
 }
 return cmdcnt;
}

/** Append queue with ACK read.
 * \param *swdctx swd context pointer.
 * \param *ack packet value pointer.
 * \return number of elements appended (1), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_miso_ack(swd_ctx_t *swdctx, char **ack){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 int res;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 if (ack!=NULL) *ack=&cmd->ack;
 cmd->bits=SWD_ACK_BITLEN;
 cmd->cmdtype=SWD_CMDTYPE_MISO_ACK;
 res=swd_cmd_enqueue(swdctx, cmd); //should be 1 on success
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with len-octet size control seruence.
 * This control sequence can be used for instance to send payload of packets
 * switching DAP between JTAG and SWD mode.
 * \param *swdctx swd context pointer.
 * \param *ctlmsg control message array pointer.
 * \param len number of elements to send from *ctlmsg.
 * \return number of elements appended (len), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_control(swd_ctx_t *swdctx, char *ctlmsg, int len){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (ctlmsg==NULL) return SWD_ERROR_NULLPOINTER;
 if (len<=0) return SWD_ERROR_PARAM;
 int elm, res, res2, cmdcnt=0;
 swd_cmd_t *cmd=NULL, *oldcmdq=swdctx->cmdq;
 for (elm=0;elm<len;elm++){
  cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
  if (cmd==NULL){
   res=SWD_ERROR_OUTOFMEM;
   break;
  }
  cmd->control=ctlmsg[elm];
  cmd->cmdtype=SWD_CMDTYPE_MOSI_CONTROL;
  cmd->bits=sizeof(ctlmsg[elm])*SWD_DATA_BYTESIZE;
  res=swd_cmd_enqueue(swdctx, cmd); 
  if (res<1) break;
  cmdcnt=+res;
 }
 //If there was problem enqueueing elements, rollback changes on queue.
 if (res<1){
  res2=swd_cmdq_free_tail(oldcmdq);
  if (res2<0) return res2;
  swdctx->cmdq=oldcmdq;
  return res;
 } return cmdcnt;
}

/** Append command queue with SW-DP-RESET sequence.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_dap_reset(swd_ctx_t *swdctx){
 return swd_cmd_enqueue_mosi_control(swdctx, (char *)SWD_CMD_SWDPRESET, sizeof(SWD_CMD_SWDPRESET));
}

/** Append command queue with idle sequence.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_idle(swd_ctx_t *swdctx){
 return swd_cmd_enqueue_mosi_control(swdctx, (char *)SWD_CMD_IDLE, sizeof(SWD_CMD_IDLE));
}

/** Append command queue with JTAG-TO-SWD DAP-switch sequence.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_jtag2swd(swd_ctx_t *swdctx){
 return swd_cmd_enqueue_mosi_control(swdctx, (char *)SWD_CMD_JTAG2SWD, sizeof(SWD_CMD_JTAG2SWD));
}

/** Append command queue with SWD-TO-JTAG DAP-switch sequence.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_cmd_enqueue_mosi_swd2jtag(swd_ctx_t *swdctx){
 return swd_cmd_enqueue_mosi_control(swdctx, (char *)SWD_CMD_SWD2JTAG, sizeof(SWD_CMD_SWD2JTAG));
}

/** Return human readable command type string of *cmd.
 * \param *cmd command the name is to be printed.
 * \return string containing human readable command name, or NULL on failure.
 */
char *swd_cmd_string_cmdtype(swd_cmd_t *cmd){
 if (cmd==NULL) return NULL;
 switch (cmd->cmdtype){
  case SWD_CMDTYPE_MOSI_DATA:    return "MOSI_DATA";
  case SWD_CMDTYPE_MOSI_REQUEST: return "MOSI_REQUEST";
  case SWD_CMDTYPE_MOSI_TRN:     return "MOSI_TRN";
  case SWD_CMDTYPE_MOSI_PARITY:  return "MOSI_PARITY";
  case SWD_CMDTYPE_MOSI_BITBANG: return "MOSI_BITBANG";
  case SWD_CMDTYPE_MOSI_CONTROL: return "MOSI_CONTROL";
  case SWD_CMDTYPE_MOSI:         return "MOSI";
  case SWD_CMDTYPE_UNDEFINED:    return "UNDEFINED";
  case SWD_CMDTYPE_MISO:         return "MISO";
  case SWD_CMDTYPE_MISO_ACK:     return "MISO_ACK";
  case SWD_CMDTYPE_MISO_BITBANG: return "MISO_BITBANG";
  case SWD_CMDTYPE_MISO_PARITY:  return "MISO_PARITY";
  case SWD_CMDTYPE_MISO_TRN:     return "MISO_TRN";
  case SWD_CMDTYPE_MISO_DATA:    return "MISO_DATA";
  default: return "Unknown command type!";
 }
}


/** @} */
