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

/** \file libswd_cmd.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup libswd_cmd_enqueue SWD Command Genration and Enqueueing routines.
 * Command quants are first generated/created in memory, then enqueued into
 * command queue for execution by libswd_cmd_flush(). Queue elements are created
 * in memory and filled with payload, then appended to the queue. If enqueue
 * fails, memory for newly created elements is set free before return.
 * All functions here start with "libswd_cmd_queue_append_" prefix.
 * Functions here are NOT intelligent, they only create payload in memory,
 * so treat them rather as blocks for high-level functions.
 * @{
 ******************************************************************************/

/** Append selected command to a context's command queue (libswdctx->cmdq).
 * This function does not update the libswdctx->cmdq pointer (its updated on flush).
 * Limitation to CMD queue len is handled here.
 * \param *libswdctx swd context pointer containing the command queue.
 * \param *cmd command to be appended to the context's command queue.
 * \return number of elements appended or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd){
 if (libswdctx==NULL || cmd==NULL) return LIBSWD_ERROR_NULLPOINTER;
 int res;
 if (libswdctx->config.maxcmdqlen!=0){
	 while (libswdctx->stats.cmdqlen >= libswdctx->config.maxcmdqlen){
		 libswd_cmd_t *cmd;
		 cmd=libswd_cmdq_find_head(libswdctx->cmdq);
		 if ((cmd->done!=1) || (cmd==libswdctx->cmdq)){
			 //Need to release memory, but this CMD in queue is not yet handled, so we can`t remove it
			 return LIBSWD_ERROR_OUTOFMEM;
		 } else {
			 res=libswd_cmdq_free_one_element(libswdctx, cmd);
			 if (res<1) return LIBSWD_ERROR_QUEUE;
		 }
	 }
 }
 res=libswd_cmdq_append(libswdctx, cmd);
 return res;
}

/** Appends command queue with SWD Request packet header.
 * Note that contents is not validated, so bad request can be sent as well.
 * \param *libswdctx swd context pointer.
 * \param *request pointer to the 8-bit request payload.
 * \return return number elements appended (1), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_request(libswd_ctx_t *libswdctx, char *request){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (request==NULL) return LIBSWD_ERROR_NULLPOINTER;
 int res;
 libswd_cmd_t *cmd;
 cmd=(libswd_cmd_t *)calloc(1,sizeof(libswd_cmd_t));
 if (cmd==NULL) return LIBSWD_ERROR_OUTOFMEM;
 cmd->request=*request;
 cmd->bits=LIBSWD_REQUEST_BITLEN;
 cmd->cmdtype=LIBSWD_CMDTYPE_MOSI_REQUEST;
 res=libswd_cmd_enqueue(libswdctx, cmd);
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with Turnaround activating MOSI mode.
 * \param *libswdctx swd context pointer.
 * \return return number elements appended (1), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_trn(libswd_ctx_t *libswdctx){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int res;
  libswd_cmd_t *cmd;
  cmd=(libswd_cmd_t *)calloc(1,sizeof(libswd_cmd_t));
  if (cmd==NULL) return LIBSWD_ERROR_OUTOFMEM;
  cmd->TRNnMOSI=0;
  cmd->bits=libswdctx->config.trnlen;
  cmd->cmdtype=LIBSWD_CMDTYPE_MOSI_TRN;
  res=libswd_cmd_enqueue(libswdctx, cmd);
  if (res<1) free(cmd);
  return res;
}

/** Append command queue with Turnaround activating MISO mode.
 * \param *libswdctx swd context pointer.
 * \return return number of elements appended (1), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_miso_trn(libswd_ctx_t *libswdctx){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int res;
 libswd_cmd_t *cmd;
 cmd=(libswd_cmd_t *)calloc(1,sizeof(libswd_cmd_t));
 if (cmd==NULL) return LIBSWD_ERROR_OUTOFMEM;
 cmd->TRNnMOSI=1;
 cmd->bits=libswdctx->config.trnlen;
 cmd->cmdtype=LIBSWD_CMDTYPE_MISO_TRN;
 res=libswd_cmd_enqueue(libswdctx, cmd);
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with bus binary read bit-by-bit operation.
 * This function will append command to the queue for each bit, and store
 * one bit into single char array element, so read is not constrained to 8 bits.
 * On error memory is released and apropriate error code is returned.
 * Important: Memory pointed by *data must be allocated prior call!
 * \param *libswdctx swd context pointer.
 * \param **data allocated data array to write result into.
 * \param count number of bits to read (also the **data size).
 * \return number of elements processed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_miso_nbit(libswd_ctx_t *libswdctx, char **data, int count){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (count<=0) return LIBSWD_ERROR_PARAM;
 int res, res2;
 libswd_cmd_t *cmd, *oldcmdq=libswdctx->cmdq;
 int i,cmdcnt=0;
 for (i=0;i<count;i++){
  cmd=(libswd_cmd_t *)calloc(1,sizeof(libswd_cmd_t));
  if (cmd==NULL) {
   res=LIBSWD_ERROR_OUTOFMEM;
   break;
  }
  cmd->bits=1;
  if (data!=NULL) *data=&cmd->misobit;
  cmd->cmdtype=LIBSWD_CMDTYPE_MISO_BITBANG;
  res=libswd_cmd_enqueue(libswdctx, cmd);
  if (res<1) break;
  cmdcnt+=res;
 }
 //If there was problem enqueueing elements, rollback changes on queue.
 if (res<1) {
  res2=libswd_cmdq_free_tail(libswdctx, oldcmdq);
  if (res2<0) return res2;
  return res;
 } else return cmdcnt;
}

/** Append command queue with bus binary write bit-by-bit operation.
 * This function will append command to the queue for each bit and store
 * one bit into single char array element, so read is not constrained to 8 bits.
 * On error memory is released and apropriate error code is returned.
 * Important: Memory pointed by *data must be allocated prior call!
 * \param *libswdctx swd context pointer.
 * \param **data allocated data array to write result into.
 * \param count number of bits to read (also the **data size).
 * \return number of elements processed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_nbit(libswd_ctx_t *libswdctx, char *data, int count){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (count<=0) return LIBSWD_ERROR_PARAM;
 int res, res2, cmdcnt=0;
 libswd_cmd_t *cmd, *oldcmdq=libswdctx->cmdq;
int i;
 for (i=0;i<count;i++){
  cmd=(libswd_cmd_t *)calloc(1, sizeof(libswd_cmd_t));
  if (cmd==NULL) {
   res=LIBSWD_ERROR_OUTOFMEM;
   break;
  }
  cmd->mosibit=data[i];
  cmd->bits=1;
  cmd->cmdtype=LIBSWD_CMDTYPE_MOSI_BITBANG;
  res=libswd_cmd_enqueue(libswdctx, cmd);
  if (res<1) break;
  cmdcnt+=res;
 }
 //If there was problem enqueueing elements, rollback changes on queue.
 if (res<1){
  res2=libswd_cmdq_free_tail(libswdctx, oldcmdq);
  if (res2<0) return res2;
  libswdctx->cmdq=oldcmdq;
  return res;
 } else return cmdcnt;
}

/** Append command queue with parity bit write.
 * \param *libswdctx swd context pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (1), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_parity(libswd_ctx_t *libswdctx, char *parity){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (*parity!=0 && *parity!=1) return LIBSWD_ERROR_PARAM;
 int res;
 libswd_cmd_t *cmd;
 cmd=(libswd_cmd_t *)calloc(1,sizeof(libswd_cmd_t));
 if (cmd==NULL) return LIBSWD_ERROR_OUTOFMEM;
 cmd->parity=*parity;
 cmd->bits=1;
 cmd->cmdtype=LIBSWD_CMDTYPE_MOSI_PARITY;
 res=libswd_cmd_enqueue(libswdctx, cmd);
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with parity bit read.
 * \param *libswdctx swd context pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (1), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_miso_parity(libswd_ctx_t *libswdctx, char **parity){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int res;
 libswd_cmd_t *cmd;
 cmd=(libswd_cmd_t *)calloc(1,sizeof(libswd_cmd_t));
 if (cmd==NULL) return LIBSWD_ERROR_OUTOFMEM;
 if (parity!=NULL) *parity=&cmd->parity;
 cmd->bits=1;
 cmd->cmdtype=LIBSWD_CMDTYPE_MISO_PARITY;
 res=libswd_cmd_enqueue(libswdctx, cmd);
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with data read.
 * \param *libswdctx swd context pointer.
 * \param *data data pointer.
 * \return of elements appended (1), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_miso_data(libswd_ctx_t *libswdctx, int **data){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int res;
 libswd_cmd_t *cmd;
 cmd=(libswd_cmd_t *)calloc(1,sizeof(libswd_cmd_t));
 if (cmd==NULL) return LIBSWD_ERROR_OUTOFMEM;
 if (data!=NULL) *data=&cmd->misodata;
 cmd->bits=32;
 cmd->cmdtype=LIBSWD_CMDTYPE_MISO_DATA;
 res=libswd_cmd_enqueue(libswdctx, cmd); // should be 1 on success
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with data and parity read.
 * \param *libswdctx swd context pointer.
 * \param *data data value pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (2), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_miso_data_p(libswd_ctx_t *libswdctx, int **data, char **parity){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int res, cmdcnt=0;
 res=libswd_cmd_enqueue_miso_data(libswdctx, data);
 if (res<1) return res;
 cmdcnt+=res;
 res=libswd_cmd_enqueue_miso_parity(libswdctx, parity);
 if (res<1) return res;
 cmdcnt+=res;
 return cmdcnt; // should be 2 or 3(+trn) on success
}

/** Append command queue with series of data and parity read.
 * \param *libswdctx swd context pointer.
 * \param **data data value array pointer.
 * \param **parity parity value array pointer.
 * \param count number of (data+parity) elements to read.
 * \return number of elements appended (2*count), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_miso_n_data_p(libswd_ctx_t *libswdctx, int **data, char **parity, int count){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (count<=0) return LIBSWD_ERROR_PARAM;
 int i,res, cmdcnt=0;
 for (i=0;i<=count;i++){
  res=libswd_cmd_enqueue_miso_data_p(libswdctx, &data[i], &parity[i]);
  if (res<2) return LIBSWD_ERROR_RESULT;
  cmdcnt+=res;
 }
 return cmdcnt;
}

/** Append command queue with data and parity write.
 * \param *libswdctx swd context pointer.
 * \param *data data value pointer.
 * \return number of elements appended (1), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_data(libswd_ctx_t *libswdctx, int *data){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 int res;
 libswd_cmd_t *cmd;
 cmd=(libswd_cmd_t *)calloc(1,sizeof(libswd_cmd_t));
 if (cmd==NULL) return LIBSWD_ERROR_OUTOFMEM;
 cmd->mosidata=*data;
 cmd->bits=32;
 cmd->cmdtype=LIBSWD_CMDTYPE_MOSI_DATA;
 res=libswd_cmd_enqueue(libswdctx, cmd); // should be 1 or 2 on success
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with data and automatic parity write.
 * \param *libswdctx swd context pointer.
 * \param *data data value pointer.
 * \return number of elements appended (2), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_data_ap(libswd_ctx_t *libswdctx, int *data){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 int res, cmdcnt=0;
 char parity;
 res=libswd_cmd_enqueue_mosi_data(libswdctx, data);
 if (res<1) return res;
 cmdcnt+=res;
 res=libswd_bin32_parity_even(data, &parity);
 if (res<0) return res;
 res=libswd_cmd_enqueue_mosi_parity(libswdctx, &parity);
 if (res<1) return res;
 cmdcnt+=res;
 return cmdcnt; // should be 2 or 3 on success
}

/** Append command queue with data and provided parity write.
 * \param *libswdctx swd context pointer.
 * \param *data data value pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (2), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_data_p(libswd_ctx_t *libswdctx, int *data, char *parity){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (data==NULL || parity==NULL) return LIBSWD_ERROR_NULLPOINTER;
 int res, cmdcnt=0;
 res=libswd_cmd_enqueue_mosi_data(libswdctx, data);
 if (res<1) return res;
 cmdcnt+=res;
 res=libswd_cmd_enqueue_mosi_parity(libswdctx, parity);
 if (res<1) return res;
 cmdcnt+=res;
 return cmdcnt; // should be 2 or 3 on success
}

/** Append command queue with series of data and automatic parity writes.
 * \param *libswdctx swd context pointer.
 * \param **data data value array pointer.
 * \param count number of (data+parity) elements to read.
 * \return number of elements appended (2*count), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_n_data_ap(libswd_ctx_t *libswdctx, int **data, int count){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (count<1) return LIBSWD_ERROR_PARAM;
 int i, res, cmdcnt=0;
 for (i=0;i<count;i++){
  res=libswd_cmd_enqueue_mosi_data(libswdctx, data[i]);
  if (res<1) return res;
  cmdcnt+=res;
 }
 return cmdcnt;
}

/** Append command queue with series of data and provided parity writes.
 * \param *libswdctx swd context pointer.
 * \param **data data value array pointer.
 * \param **parity parity value array pointer.
 * \param count number of (data+parity) elements to read.
 * \return number of elements appended (2*count), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_n_data_p(libswd_ctx_t *libswdctx, int **data, char **parity, int count){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (count<1) return LIBSWD_ERROR_PARAM;
 int i, res, cmdcnt=0;
 for (i=0;i<count;i++){
  res=libswd_cmd_enqueue_mosi_data_p(libswdctx, data[i], parity[i]);
  if (res<1) return res;
  cmdcnt+=res;
 }
 return cmdcnt;
}

/** Append queue with ACK read.
 * \param *libswdctx swd context pointer.
 * \param *ack packet value pointer.
 * \return number of elements appended (1), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_miso_ack(libswd_ctx_t *libswdctx, char **ack){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int res;
 libswd_cmd_t *cmd;
 cmd=(libswd_cmd_t *)calloc(1,sizeof(libswd_cmd_t));
 if (cmd==NULL) return LIBSWD_ERROR_OUTOFMEM;
 if (ack!=NULL) *ack=&cmd->ack;
 cmd->bits=LIBSWD_ACK_BITLEN;
 cmd->cmdtype=LIBSWD_CMDTYPE_MISO_ACK;
 res=libswd_cmd_enqueue(libswdctx, cmd); //should be 1 on success
 if (res<1) free(cmd);
 return res;
}

/** Append command queue with len-octet size control seruence.
 * This control sequence can be used for instance to send payload of packets
 * switching DAP between JTAG and SWD mode.
 * \param *libswdctx swd context pointer.
 * \param *ctlmsg control message array pointer.
 * \param len number of elements to send from *ctlmsg.
 * \return number of elements appended (len), or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_control(libswd_ctx_t *libswdctx, char *ctlmsg, int len){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (ctlmsg==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (len<=0) return LIBSWD_ERROR_PARAM;
 int elm, res, res2, cmdcnt=0;
 libswd_cmd_t *cmd=NULL, *oldcmdq=libswdctx->cmdq;
 for (elm=0;elm<len;elm++){
  cmd=(libswd_cmd_t *)calloc(1,sizeof(libswd_cmd_t));
  if (cmd==NULL){
   res=LIBSWD_ERROR_OUTOFMEM;
   break;
  }
  cmd->control=ctlmsg[elm];
  cmd->cmdtype=LIBSWD_CMDTYPE_MOSI_CONTROL;
  cmd->bits=sizeof(ctlmsg[elm])*LIBSWD_DATA_BYTESIZE;
  res=libswd_cmd_enqueue(libswdctx, cmd);
  if (res<1) break;
  cmdcnt+=res;
 }
 //If there was problem enqueueing elements, rollback changes on queue.
 if (res<1){
  res2=libswd_cmdq_free_tail(libswdctx, oldcmdq);
  if (res2<0) return res2;
  libswdctx->cmdq=oldcmdq;
  return res;
 } return cmdcnt;
}

/** Append command queue with SW-DP-RESET sequence.
 * \param *libswdctx swd context pointer.
 * \return number of elements appended, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_dap_reset(libswd_ctx_t *libswdctx){
 return libswd_cmd_enqueue_mosi_control(libswdctx, (char *)LIBSWD_CMD_SWDPRESET, sizeof(LIBSWD_CMD_SWDPRESET));
}

/** Append command queue with idle sequence.
 * \param *libswdctx swd context pointer.
 * \return number of elements appended, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_idle(libswd_ctx_t *libswdctx){
 return libswd_cmd_enqueue_mosi_control(libswdctx, (char *)LIBSWD_CMD_IDLE, sizeof(LIBSWD_CMD_IDLE));
}

/** Append command queue with JTAG-TO-SWD DAP-switch sequence.
 * \param *libswdctx swd context pointer.
 * \return number of elements appended, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_jtag2swd(libswd_ctx_t *libswdctx){
 return libswd_cmd_enqueue_mosi_control(libswdctx, (char *)LIBSWD_CMD_JTAG2SWD, sizeof(LIBSWD_CMD_JTAG2SWD));
}

/** Append command queue with SWD-TO-JTAG DAP-switch sequence.
 * \param *libswdctx swd context pointer.
 * \return number of elements appended, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmd_enqueue_mosi_swd2jtag(libswd_ctx_t *libswdctx){
 return libswd_cmd_enqueue_mosi_control(libswdctx, (char *)LIBSWD_CMD_SWD2JTAG, sizeof(LIBSWD_CMD_SWD2JTAG));
}

/** Return human readable command type string of *cmd.
 * \param *cmd command the name is to be printed.
 * \return string containing human readable command name, or NULL on failure.
 */
char *libswd_cmd_string_cmdtype(libswd_cmd_t *cmd){
 if (cmd==NULL) return NULL;
 switch (cmd->cmdtype){
  case LIBSWD_CMDTYPE_MOSI_DATA:    return "MOSI_DATA";
  case LIBSWD_CMDTYPE_MOSI_REQUEST: return "MOSI_REQUEST";
  case LIBSWD_CMDTYPE_MOSI_TRN:     return "MOSI_TRN";
  case LIBSWD_CMDTYPE_MOSI_PARITY:  return "MOSI_PARITY";
  case LIBSWD_CMDTYPE_MOSI_BITBANG: return "MOSI_BITBANG";
  case LIBSWD_CMDTYPE_MOSI_CONTROL: return "MOSI_CONTROL";
  case LIBSWD_CMDTYPE_MOSI:         return "MOSI";
  case LIBSWD_CMDTYPE_UNDEFINED:    return "UNDEFINED";
  case LIBSWD_CMDTYPE_MISO:         return "MISO";
  case LIBSWD_CMDTYPE_MISO_ACK:     return "MISO_ACK";
  case LIBSWD_CMDTYPE_MISO_BITBANG: return "MISO_BITBANG";
  case LIBSWD_CMDTYPE_MISO_PARITY:  return "MISO_PARITY";
  case LIBSWD_CMDTYPE_MISO_TRN:     return "MISO_TRN";
  case LIBSWD_CMDTYPE_MISO_DATA:    return "MISO_DATA";
  default: return "Unknown command type!";
 }
}


/** @} */
