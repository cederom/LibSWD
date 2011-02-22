/*
 * $Id$
 *
 * Serial Wire Debug Open Library.
 * Library Body File.
 *
 * Copyright (C) 2010, Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)
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
 * Written by Tomasz Boleslaw CEDRO <tomek.cedro@gmail.com>, 2010;
 *
 */

/** \file libswd.c */

#include <libswd.h>
#include <urjtag/libswd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

/*******************************************************************************
 * \defgroup swd_bin Binary operations helper functions.
 * @{
 ******************************************************************************/

/**
 * Data parity calculator, calculates even parity on char type.
 * \param *data source data pointer.
 * \param *parity resulting data pointer.
 * \return negative value on error, 0 or 1 as parity result.
 */
int swd_bin8_parity_even(char *data, char *parity){
 char i;
 unsigned char test=*data;
 *parity=0;
 for (i=0;i<=8;i++) *parity ^= ((test>>i)&1);
 if (*parity<0 || *parity>1) return SWD_ERROR_PARITY;
 return (int)*parity;
}

/**
 * Data parity calculator, calculates even parity on integer type.
 * \param *data source data pointer.
 * \param *parity resulting data pointer.
 * \return negative value on error, 0 or 1 as parity result.
 */
int swd_bin32_parity_even(int *data, char *parity){
 int i;
 unsigned int test=*data;
 *parity=0;
 for (i=0;i<32;i++) *parity ^= (test>>i)&1; 
 if (*parity<0 || *parity>1) return SWD_ERROR_PARITY;
 return (int)*parity;
}

/**
 * Prints binary data of a char value on the screen.
 * \param *data source data pointer.
 * \return number of characters printed.
 */
int swd_bin8_print(char *data){
 unsigned char i, bits=*data;
 for (i=0;i<8;i++) putchar(((bits<<i)&0x80)?'1':'0'); 
 return i;
}

/**
 * Prints binary data of an integer value on the screen.
 * \param *data source data pointer.
 * \return number of characters printed.
 */
int swd_bin32_print(int *data){
 unsigned int i, bits=*data;
 for (i=0;i<32;i++) putchar(((bits<<i)&0x80000000)?'1':'0');
 return i;
}

/**
 * Generates string containing binary data of a char value.
 * \param *data source data pointer.
 * \return pointer to the resulting string.
 */
char *swd_bin8_string(char *data){
 static char string[9]; string[8]=0;
 unsigned char i, bits=*data;
 for (i=0;i<8;i++) string[7-i]=(bits&(1<<i))?'1':'0'; 
 return string;
}

/**
 * Generates string containing binary data of an integer value.
 * \param *data source data pointer.
 * \return pointer to the resulting string.
 */
char *swd_bin32_string(int *data){
 static char string[33]; string[32]=0;
 unsigned int i, bits=*data;
 for (i=0;i<32;i++) string[31-i]=(bits&(1<<i))?'1':'0';
 return string;
}

/**
 * Bit swap helper function that reverse bit order in char *buffer. 
 * Most Significant Bit becomes Least Significant Bit.
 * It is possible to  swap only n-bits from char (8-bit) *buffer.
 * \param *buffer unsigned char (8-bit) data pointer.
 * \param bitcount how many bits to swap.
 * \return swapped bit count (positive) or error code (negative). 
 */
int swd_bin8_bitswap(unsigned char *buffer, int bitcount){
 if (buffer==NULL) return SWD_ERROR_NULLPOINTER;
 if (bitcount>8) return SWD_ERROR_PARAM;
 unsigned char bit, result=0; //res must be unsigned for proper shifting result
 #ifdef __SWDDEBUG__
 printf("|SWD_DEBUG: swd_bin8_bitswap(%02X, %d);\n", *buffer, bitcount);
 #endif
 for (bit=0;bit<bitcount;bit++) {
  result=(result<<1)|(((*buffer>>bit)&1)?1:0); 
  #ifdef __SWDDEBUG__
  printf("|SWD_DEBUG: swd_bin8_bitswap: in=%02X out=%02X bit=%d\n", *buffer, result, bit);
  #endif
 }
 *buffer=result;
 return bit;
}

/**
 * Bit swap helper function that reverse bit order in int *buffer. 
 * Most Significant Bit becomes Least Significant Bit.
 * It is possible to  swap only n-bits from int (32-bit) *buffer.
 * \param *buffer unsigned char (32-bit) data pointer.
 * \param bitcount how many bits to swap.
 * \return swapped bit count (positive) or error code (negative). 
 */
int swd_bin32_bitswap(unsigned int *buffer, int bitcount){
 if (buffer==NULL) return SWD_ERROR_NULLPOINTER;
 if (bitcount>32) return SWD_ERROR_PARAM;
 unsigned int bit, result=0; //res must be unsigned for proper shifting result
 #ifdef __SWDDEBUG__
 printf("|SWD_DEBUG: swd_bin32_bitswap(%08X, %d);\n", *buffer, bitcount);
 #endif
 for (bit=0;bit<bitcount;bit++) {
  result=(result<<1)|(((*buffer>>bit)&1)?1:0); 
  #ifdef __SWDDEBUG__
  printf("|SWD_DEBUG: swd_bin32_bitswap: in=%08X out=%08X bit=%d\n", *buffer, result, bit);
  #endif
 }
 *buffer=result;
 return bit;
}

/** @} */


/*******************************************************************************
 * \defgroup swd_cmd_queue Command Queue helper functions
 * @{
 ******************************************************************************/

/** Initialize new queue element in memory that becomes a queue root.
 * \param *cmdq pointer to the command queue element of type swd_cmd_t
 * \return SWD_OK on success, SWD_ERROR_CODE code on failure
 */
int swd_cmdq_init(swd_cmd_t *cmdq){
 if (cmdq==NULL) return SWD_ERROR_NULLPOINTER;
 cmdq=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmdq==NULL) return SWD_ERROR_OUTOFMEM;
 cmdq->prev=NULL;
 cmdq->next=NULL;
 return SWD_OK;
}

/** Find queue root (first element).
 * \param *cmdq pointer to any queue element
 * \return swd_cmd_t* pointer to the first element (root), NULL on failure
 */
swd_cmd_t* swd_cmdq_find_root(swd_cmd_t *cmdq){
 if (cmdq==NULL) return NULL;
 swd_cmd_t *cmd=cmdq;
 while (cmd->prev!=NULL) cmd=cmd->prev;
 return cmd;
}

/** Find queue tail (last element).
 * \param  *cmdq pointer to any queue element
 * \return swd_cmd_t* pointer to the last element (tail), NULL on failure
 */
swd_cmd_t* swd_cmdq_find_tail(swd_cmd_t *cmdq){
 if (cmdq==NULL) return NULL;
 swd_cmd_t *cmd=cmdq;
 while (cmd->next!=NULL) cmd=cmd->next;
 return cmd;
}

/** Append element pointed by *cmd at the end of the quque pointed by *cmdq.
 * After this operation queue will be pointed by appended element (ie. last
 * element added becomes actual quque pointer to show what was added recently).
 * \param *cmdq pointer to any element on command queue 
 * \param *cmd pointer to the command to be appended
 * \return number of appended elements (one), SWD_ERROR_CODE on failure
 */
int swd_cmdq_append(swd_cmd_t *cmdq, swd_cmd_t *cmd){
 if (cmdq==NULL) return SWD_ERROR_NULLQUEUE;
 if (cmd==NULL) return SWD_ERROR_NULLPOINTER;
 if (cmdq->next != NULL){
  swd_cmd_t *lastcmd;
  lastcmd=swd_cmdq_find_tail(cmdq);
  lastcmd->next=cmd;
  cmd->prev=lastcmd;
 } else {
  cmdq->next=cmd;
  cmd->prev=cmdq;
 }
 cmdq=cmd; 
 return 1;
}

/** Free queue pointed by *cmdq element.
 * \param *cmdq pointer to any element on command queue
 * \return number of elements destroyed, SWD_ERROR_CODE on failure
 */
int swd_cmdq_free(swd_cmd_t *cmdq){
 if (cmdq==NULL) return SWD_ERROR_NULLQUEUE;
 int cmdcnt=0;
 swd_cmd_t *cmd, *nextcmd;
 cmd=swd_cmdq_find_root(cmdq);
 while (cmd!=NULL) {
  nextcmd=cmd->next;
  free(cmd);
  cmd=nextcmd;
  cmdcnt++;
 }
 return cmdcnt;
}

/** Free queue head up to *cmdq element.
 * \param *cmdq pointer to the element that becomes new queue root.
 * \return number of elements destroyed, or SWD_ERROR_CODE on failure.
 */
int swd_cmdq_free_head(swd_cmd_t *cmdq){
 if (cmdq==NULL) return SWD_ERROR_NULLQUEUE;
 int cmdcnt=0;
 swd_cmd_t *cmdqroot, *nextcmd;
 cmdqroot=swd_cmdq_find_root(cmdq);
 while(cmdqroot!=cmdq){
  nextcmd=cmdqroot->next;
  free(cmdqroot);
  cmdqroot=nextcmd;
  cmdcnt++;
 }
 cmdqroot->prev=NULL;
 return cmdcnt;
}

/** Free queue tail starting after *cmdq element.
 * \param *cmdq pointer to the last element on the new queue.
 * \return number of elements destroyed, or SWD_ERROR_CODE on failure.
 */
int swd_cmdq_free_tail(swd_cmd_t *cmdq){
 if (cmdq==NULL) return SWD_ERROR_NULLQUEUE;
 int cmdcnt=0;
 swd_cmd_t *cmdqend, *nextcmd;
 nextcmd=cmdq;
 cmdqend=swd_cmdq_find_tail(cmdq);
 if (cmdqend==NULL) return SWD_ERROR_QUEUE; 
 while(nextcmd!=cmdq){
  nextcmd=cmdqend->prev;
  free(cmdqend);
  cmdqend=nextcmd;
  cmdcnt++;
 }
 cmdqend->next=NULL;
 return cmdcnt;
}
/** @} */


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
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (count<=0) return SWD_ERROR_PARAM;
 int res;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(count,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 int i,cmdcnt=0;
 for (i=0;i<count;i++){
  data[i]=&cmd[i].misobit;
  cmd[i].bits=1;
  cmd[i].cmdtype=SWD_CMDTYPE_MISO_BITBANG;
  res=swd_cmd_enqueue(swdctx, &cmd[i]);
  if (res<1) break;
  cmdcnt+=res;
 }
 if (res<1){
  free(cmd);
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
 int res, cmdcnt=0;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(count, sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 int i;
 for (i=0;i<count;i++){
  cmd[i].mosibit=data[i];
  cmd[i].bits=1;
  cmd[i].cmdtype=SWD_CMDTYPE_MOSI_BITBANG;
  res=swd_cmd_enqueue(swdctx, &cmd[i]);
  if (res<1) break;
  cmdcnt+=res;
 }
 if (res<1){
  free(cmd);
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
 if (*parity!=0 || *parity!=1) return SWD_ERROR_PARAM;
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
int swd_cmd_enqueue_miso_parity(swd_ctx_t *swdctx, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (parity==NULL) return SWD_ERROR_NULLPOINTER;
 int res;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 parity=&cmd->parity;
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
int swd_cmd_enqueue_miso_data(swd_ctx_t *swdctx, int *data){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 int res;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 data=&cmd->misodata;
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
int swd_cmd_enqueue_miso_data_p(swd_ctx_t *swdctx, int *data, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (parity==NULL) return SWD_ERROR_NULLPOINTER;
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
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (count<=0) return SWD_ERROR_PARAM;
 int i,res, cmdcnt=0;
 for (i=0;i<=count;i++){
  res=swd_cmd_enqueue_miso_data_p(swdctx, data[i], parity[i]);
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
int swd_cmd_enqueue_miso_ack(swd_ctx_t *swdctx, char *ack){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (ack==NULL) return SWD_ERROR_NULLPOINTER;
 int res;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 ack=&cmd->ack;
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
 int elm, res, cmdcnt=0;
 swd_cmd_t *cmd=NULL;
 cmd=(swd_cmd_t *)calloc(len,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 for (elm=0;elm<len;elm++){
  cmd[elm].control=ctlmsg[elm];
  cmd[elm].cmdtype=SWD_CMDTYPE_MOSI_CONTROL;
  cmd[elm].bits=sizeof(ctlmsg[elm])*SWD_DATA_BYTESIZE;
  res=swd_cmd_enqueue(swdctx, &cmd[elm]); 
  if (res<1) break;
  cmdcnt=+res;
 }
 if (res<1){
  free(cmd);
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

/** Append command queue with TRN WRITE/MOSI, if previous command was READ/MISO.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_bus_setdir_mosi(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 int res, cmdcnt=0;
 if ( swdctx->cmdq->prev==NULL || (swdctx->cmdq->cmdtype*SWD_CMDTYPE_MOSI<0) ) {
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
 if ( swdctx->cmdq->prev==NULL || (swdctx->cmdq->cmdtype*SWD_CMDTYPE_MISO<0) ) {
  res=swd_cmd_enqueue_miso_trn(swdctx);
  if (res<0) return res;
  cmdcnt=+res;
 }
 return cmdcnt;
}
/** @} */

/*******************************************************************************
 * \defgroup swd_cmd_string SWD command element to string converters.
 * @{
 */

char *swd_cmd_string_cmdtype(swd_cmd_t *cmd){
 if (cmd==NULL) return NULL;
 switch (cmd->cmdtype){
  case SWD_CMDTYPE_MOSI_DATA: return "MOSI_DATA";
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


/*******************************************************************************
 * \defgroup swd_bitgen SWD Bitstream / Packet Payload generation routines.
 * @{
 ******************************************************************************/

/** Generate 8-bit SWD-REQUEST packet contents with provided parameters.
 * Note that parity bit value is calculated automatically.
 * \param *swdctx swd context pointer.
 * \param *APnDP AccessPort (high) or DebugPort (low) access type pointer.
 * \param *RnW Read (high) or Write (low) operation type pointer.
 * \param *addr target register address value pointer.
 * \param *request pointer where to store resulting packet.
 * \return number of generated packets (1), or SWD_ERROR_CODE on failure.
 */
int swd_bitgen8_request(swd_ctx_t *swdctx, char *APnDP, char *RnW, char *addr, char *request){
 /* Verify function parameters.*/
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (*APnDP!=0 && *APnDP!=1) return SWD_ERROR_APnDP;
 if (*RnW!=0 && *RnW!=1) return SWD_ERROR_RnW;
 if (*addr<SWD_ADDR_MINVAL && *addr>SWD_ADDR_MAXVAL) return SWD_ERROR_ADDR;

 /* Build request header content. */
 unsigned char reqhdr=0;
 char parity, req;
 int res;
 reqhdr|=(((*addr&(1<<2))?1:0)<<SWD_REQUEST_A2_BITNUM);
 reqhdr|=(((*addr&(1<<3))?1:0)<<SWD_REQUEST_A3_BITNUM);
 reqhdr|=((*APnDP?1:0)<<SWD_REQUEST_APnDP_BITNUM);
 reqhdr|=(((*RnW?1:0)<<SWD_REQUEST_RnW_BITNUM));
 req=reqhdr;
 res=swd_bin8_parity_even(&req, &parity);
 if (res<0) return res;
 if (parity<0 || parity>1) return SWD_ERROR_PARITY;
 reqhdr|=(res<<SWD_REQUEST_PARITY_BITNUM);
 reqhdr|=(SWD_REQUEST_START_VAL<<SWD_REQUEST_START_BITNUM);
 reqhdr|=(SWD_REQUEST_STOP_VAL<<SWD_REQUEST_STOP_BITNUM);
 reqhdr|=(SWD_REQUEST_PARK_VAL<<SWD_REQUEST_PARK_BITNUM);

 *request=reqhdr;
 return 1;
}
/** @} */


/*******************************************************************************
 * \defgroup swd_drv SWD Bus and Interface Driver Transfer Functions that
 * executes command queue.
 * @{
 ******************************************************************************/

extern int swd_drv_mosi_8(swd_ctx_t *swdctx, char *data, int bits, int nLSBfirst);
extern int swd_drv_mosi_32(swd_ctx_t *swdctx, int *data, int bits, int nLSBfirst);
extern int swd_drv_miso_8(swd_ctx_t *swdctx, char *data, int bits, int nLSBfirst);
extern int swd_drv_miso_32(swd_ctx_t *swdctx, int *data, int bits, int nLSBfirst);
extern int swd_drv_mosi_trn(swd_ctx_t *swdctx, int bits);
extern int swd_drv_miso_trn(swd_ctx_t *swdctx, int bits);

/** Transmit selected command from the command queue to the interface driver.
 * \param *swdctx swd context pointer.
 * \param *cmd pointer to the command to be sent.
 * \return number of commands transmitted (1), or SWD_ERROR_CODE on failure.
 */ 
int swd_bus_transmit(swd_ctx_t *swdctx, swd_cmd_t *cmd){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (cmd==NULL) return SWD_ERROR_NULLPOINTER;

 int res=SWD_ERROR_BADCMDTYPE;

 switch (cmd->cmdtype){
  case SWD_CMDTYPE_MOSI:
  case SWD_CMDTYPE_MISO:
   swd_log(swdctx, SWD_LOGLEVEL_WARNING, "This command does not contain payload.");
   break;

  case SWD_CMDTYPE_MOSI_CONTROL:
   // 8 clock cycles.
   if (cmd->bits!=8) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, &cmd->control, 8, SWD_DIR_MSBFIRST);
   break;

  case SWD_CMDTYPE_MOSI_BITBANG:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, &cmd->mosibit, 1, SWD_DIR_LSBFIRST);
   break;

  case SWD_CMDTYPE_MOSI_PARITY:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, &cmd->parity, 1, SWD_DIR_LSBFIRST);
   break;

  case SWD_CMDTYPE_MOSI_TRN:
   // 1..4-bit clock cycle.
   if (cmd->bits<SWD_TURNROUND_MIN && cmd->bits>SWD_TURNROUND_MAX)
    return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_trn(swdctx, cmd->bits);
   break;

  case SWD_CMDTYPE_MOSI_REQUEST:
   // 8 clock cycles.
   if (cmd->bits!=SWD_REQUEST_BITLEN) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_mosi_8(swdctx, &cmd->request, 8, SWD_DIR_MSBFIRST);
   break;

  case SWD_CMDTYPE_MOSI_DATA:
   // 32 clock cycles.
   if (cmd->bits!=SWD_DATA_BITLEN) return SWD_ERROR_BADCMDDATA; 
   res=swd_drv_mosi_32(swdctx, &cmd->mosidata, 32, SWD_DIR_MSBFIRST);
   break;

  case SWD_CMDTYPE_MISO_ACK:
   // 3 clock cycles.
   if (cmd->bits!=SWD_ACK_BITLEN) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_8(swdctx, &cmd->ack, cmd->bits, SWD_DIR_LSBFIRST);
   break;

  case SWD_CMDTYPE_MISO_BITBANG:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_8(swdctx, &cmd->misobit, 1, SWD_DIR_LSBFIRST);
   break;

  case SWD_CMDTYPE_MISO_PARITY:
   // 1 clock cycle.
   if (cmd->bits!=1) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_8(swdctx, &cmd->parity, 1, SWD_DIR_LSBFIRST);
   break;

  case SWD_CMDTYPE_MISO_TRN:
   // 1..4 clock cycles
   if (cmd->bits<SWD_TURNROUND_MIN && cmd->bits>SWD_TURNROUND_MAX)
    return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_trn(swdctx, cmd->bits);
   break;

  case SWD_CMDTYPE_MISO_DATA:
   // 32 clock cycles
   if (cmd->bits!=SWD_DATA_BITLEN) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_32(swdctx, &cmd->misodata, cmd->bits, SWD_DIR_MSBFIRST);
   break;

  case SWD_CMDTYPE_UNDEFINED:
   res=0;
   break; 

  default:
   return SWD_ERROR_BADCMDTYPE;
 } 

 swd_log(swdctx, SWD_LOGLEVEL_DEBUG,
  "|swd_bus_transmit() bits=%-2d cmdtype=%-12s returns=%-3d payload=0x%08x (%s)\n",
  cmd->bits, swd_cmd_string_cmdtype(cmd), res,
  (cmd->bits>8)?cmd->data32:cmd->data8,
  (cmd->bits<=8)?swd_bin8_string(&cmd->data8):swd_bin32_string(&cmd->data32));

 if (res<0) return res;
 cmd->done=1;
 return res;
}

/** Flush command queue contents into interface driver.
 * Operation is specified by SWD_OPERATION and can be used to select
 * how to flush the queue, ie. head-only, tail-only, one, all, etc.
 * \param *swdctx swd context pointer.
 * \param operation tells how to flush the queue.
 * \return number of commands transmitted, or SWD_ERROR_CODE on failure.
 */
int swd_cmdq_flush(swd_ctx_t *swdctx, swd_operation_t operation){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (operation<SWD_OPERATION_FIRST || operation>SWD_OPERATION_LAST)
  return SWD_ERROR_BADOPCODE;

 int res, cmdcnt=0;
 swd_cmd_t *cmd, *firstcmd, *lastcmd;

 switch (operation){
  case SWD_OPERATION_TRANSMIT_HEAD:
   firstcmd=swd_cmdq_find_root(swdctx->cmdq);
   lastcmd=swdctx->cmdq;
   break;
  case SWD_OPERATION_TRANSMIT_TAIL:
   firstcmd=swdctx->cmdq;
   lastcmd=swd_cmdq_find_tail(swdctx->cmdq);
   break;
  case SWD_OPERATION_EXECUTE:
  case SWD_OPERATION_TRANSMIT_ALL:
   firstcmd=swd_cmdq_find_root(swdctx->cmdq);
   lastcmd=swd_cmdq_find_tail(swdctx->cmdq);
   break;
  case SWD_OPERATION_TRANSMIT_ONE:
   firstcmd=swdctx->cmdq;
   lastcmd=swdctx->cmdq;
   break;
  case SWD_OPERATION_TRANSMIT_LAST:
   firstcmd=swd_cmdq_find_tail(swdctx->cmdq);
   lastcmd=firstcmd;
   break;
  default:
   return SWD_ERROR_BADOPCODE;
 }

 if (firstcmd==NULL) return SWD_ERROR_QUEUEROOT;
 if (lastcmd==NULL) return SWD_ERROR_QUEUETAIL;

 if (firstcmd==lastcmd)
  return swd_bus_transmit(swdctx, firstcmd); 

 for (cmd=firstcmd;;cmd=cmd->next){
  if (cmd->done) continue;
  res=swd_bus_transmit(swdctx, cmd); 
  if (res<0) return res;
  cmdcnt=+res;
  if (cmd==lastcmd) break;
 } 

 return cmdcnt;
}

/** @} */


/*******************************************************************************
 * \defgroup swd_packet SWD Operations Generators: Request, ACK, Data.
 * These functions generate payloads and queue up all elements/commands
 * necessary to perform requested operations on the SWD bus. Depending
 * on "operation" type, elements can be only enqueued on the queue (operation ==
 * SWD_OPERATION_ENQUEUE) or queued and then flushed into hardware driver 
 * (operation == SWD_OPERATION_EXECUTE) for immediate effect on the target.
 * Other operations are not allowed for these functions and will produce error.
 * This group of functions if intelligent, so they will react on errors.
 * Functions here are named "mosi" instead of "write" and "miso" instead of
 * "read", because read/write can cause confusion, where Master/Slave does not.
 * @{
 ******************************************************************************/

/** Perform Request.
 * \param *swdctx swd context pointer.
 * \param operation type of action to perform with generated request.
 * \param *APnDP AccessPort (high) or DebugPort (low) access value pointer.
 * \param *RnW Read (high) or Write (low) access value pointer.
 * \param *addr target register address value pointer.
 * \return number of commands processed, or SWD_ERROR_CODE on failure.
 */
int swd_mosi_request
(swd_ctx_t *swdctx, swd_operation_t operation, char *APnDP, char *RnW, char *addr){
 /* Verify function parameters.*/
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (*APnDP!=0 && *APnDP!=1) return SWD_ERROR_APnDP;
 if (*RnW!=0 && *RnW!=1) return SWD_ERROR_RnW;
 if (*addr<SWD_ADDR_MINVAL && *addr>SWD_ADDR_MAXVAL) return SWD_ERROR_ADDR;
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
  res=swd_cmdq_flush(swdctx, operation);
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
int swd_miso_ack(swd_ctx_t *swdctx, swd_operation_t operation, char *ack){
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
  res=swd_cmdq_flush(swdctx, operation);
  if (res<0) return res;
  tcmdcnt+=res;
  return qcmdcnt+tcmdcnt;
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
  if (&tmpcmdq->ack!=ack) return SWD_ERROR_ACKMISMATCH;
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
int swd_mosi_data_p
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
  res=swd_cmdq_flush(swdctx, operation);
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
int swd_mosi_data_ap(swd_ctx_t *swdctx, swd_operation_t operation, int *data){
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
  res=swd_cmdq_flush(swdctx, operation);
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
int swd_miso_data_p(swd_ctx_t *swdctx, swd_operation_t operation, int *data, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL || parity==NULL) return SWD_ERROR_NULLPOINTER;
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
  res=swd_cmdq_flush(swdctx, operation);
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
   if (data!=&tmpcmdq->misodata) return SWD_ERROR_DATAPTR;
   if (parity!=&tmpcmdq->next->parity) return SWD_ERROR_PARITYPTR;
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
int swd_mosi_control(swd_ctx_t *swdctx, swd_operation_t operation, char *ctlmsg, int len){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (operation!=SWD_OPERATION_ENQUEUE && operation!=SWD_OPERATION_EXECUTE)
  return SWD_ERROR_BADOPCODE;
 if (ctlmsg==NULL) return SWD_ERROR_NULLPOINTER;
 if (len<1 && len>swdctx->config.maxcmdqlen) return SWD_ERROR_PARAM;

 int res, qcmdcnt=0, tcmdcnt=0;

 /* Make sure that bus is in MOSI state. */
 res=swd_bus_setdir_miso(swdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=swd_cmd_enqueue_mosi_control(swdctx, ctlmsg, len);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==SWD_OPERATION_ENQUEUE){
  return qcmdcnt;        
 } else if (operation==SWD_OPERATION_EXECUTE){
  res=swd_cmdq_flush(swdctx, operation);
  if (res<0) return res;
  tcmdcnt+=res;
 } else return SWD_ERROR_BADOPCODE;
 return SWD_OK;
}
 
/** Switch DAP into SW-DP. According to ARM documentation target's DAP use JTAG
 * transport by default and so JTAG-DP is active after power up. To use SWD
 * user must perform predefined sequence on SWDIO/TMS lines, then read out the
 * IDCODE to ensure proper SW-DP operation.
 */
int swd_mosi_jtag2swd(swd_ctx_t *swdctx, swd_operation_t operation){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (operation!=SWD_OPERATION_ENQUEUE && operation!=SWD_OPERATION_EXECUTE)
  return SWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;

 res=swd_cmd_enqueue_mosi_jtag2swd(swdctx);
 if (res<0) return res;
 qcmdcnt=res;

 if (operation==SWD_OPERATION_ENQUEUE){
  return qcmdcnt;       
 } else if (operation==SWD_OPERATION_EXECUTE){
  res=swd_cmdq_flush(swdctx, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return qcmdcnt+tcmdcnt;
 } else return SWD_ERROR_BADOPCODE;
}

/** Read target's IDCODE register value.
 * \param *swdctx swd context pointer.
 * \param operation type of action to perform (queue or execute).
 * \param *idcode resulting register value pointer.
 * \param *ack resulting acknowledge response value pointer.
 * \param *parity resulting data parity value pointer.
 * \return number of elements processed on the queue, or SWD_ERROR_CODE on failure.
 */
int swd_miso_idcode
(swd_ctx_t *swdctx, swd_operation_t operation, int *idcode, char *ack, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT; 
 if (idcode==NULL || ack==NULL || parity==NULL) return SWD_ERROR_NULLPOINTER;
 if (operation!=SWD_OPERATION_ENQUEUE && operation!=SWD_OPERATION_EXECUTE)
         return SWD_ERROR_BADOPCODE;

 int res, cmdcnt=0;
 char APnDP, RnW, addr, cparity;

 APnDP=1;
 RnW=1;
 addr=SWD_DP_ADDR_IDCODE;

 res=swd_mosi_request(swdctx, SWD_OPERATION_ENQUEUE, &APnDP, &RnW, &addr);
 if (res<1) return res;
 cmdcnt=+res;

 if (operation==SWD_OPERATION_ENQUEUE){
  res=swd_miso_ack(swdctx, SWD_OPERATION_ENQUEUE, ack);
  if (res<1) return res;
  cmdcnt=+res;
  res=swd_miso_data_p(swdctx, SWD_OPERATION_ENQUEUE, idcode, parity);
  if (res<1) return res;
  cmdcnt=+res;
  return cmdcnt;

 } else if (operation==SWD_OPERATION_EXECUTE){
  res=swd_miso_ack(swdctx, operation, ack);
  if (res<1) return res;
//  if (*ack!=SWD_ACK_OK_VAL){
//   if (*ack==SWD_ACK_WAIT_VAL) return SWD_ERROR_ACK_WAIT;
//   if (*ack==SWD_ACK_FAULT_VAL) return SWD_ERROR_ACK_FAULT; 
//   return SWD_ERROR_ACK;
//  }
  res=swd_miso_data_p(swdctx, operation, idcode, parity);
  if (res<0) return res;
  res=swd_bin32_parity_even(idcode, &cparity); 
  if (res<0) return res;
  if (cparity!=*parity) return SWD_ERROR_PARITY;
  return cmdcnt;
 } else return SWD_ERROR_BADOPCODE;
}

/** @} */

/*******************************************************************************
 * \defgroup swd_highlevel High-level SWD operation functions.
 * High level functions in general call lower level functions that append
 * queue with specific commands and payload, but also react on received data.
 * They operate on data pointers where target data is being stored.
 * Operation can be SWD_OPERATION_QUEUE_APPEND for queueing only the command
 * for later execution, or SWD_OPERATION_EXECUTE to queue command, flush it
 * into the interface driver (target read/write) and react on its result before
 * function returns.
 * Return values: negative number on error, data on success.
 ******************************************************************************/

/** Debug Access Port Reset sends 50 CLK with TMS high that brings both
 * SW-DP and JTAG-DP into reset state.
 * \param *swdctx swd context pointer.
 * \param operation type (SWD_OPERATION_ENQUEUE or SWD_OPERATION_EXECUTE).
 * \return number of elements processed or SWD_ERROR_CODE code on failure.
 */
int swd_dap_reset(swd_ctx_t *swdctx, swd_operation_t operation){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (operation!=SWD_OPERATION_ENQUEUE && operation!=SWD_OPERATION_EXECUTE)
  return SWD_ERROR_BADOPCODE;  

 int res, qcmdcnt=0, tcmdcnt=0;

 res=swd_cmd_enqueue_mosi_trn(swdctx);
 if (res<0) return res;
 res=swd_cmd_enqueue_mosi_dap_reset(swdctx);
 if (res<1) return res;
 qcmdcnt+=res;

 if (operation==SWD_OPERATION_ENQUEUE){
  return qcmdcnt;
 } else if (operation==SWD_OPERATION_EXECUTE){
  res=swd_cmdq_flush(swdctx, operation);
  if (res<0) return res;
  tcmdcnt+=res;
  return qcmdcnt+tcmdcnt;
 } else return SWD_ERROR_BADOPCODE;
}

/** Activate SW-DP by sending out JTAG-TO-SWD sequence on SWDIOTMS line.
 * \param *swdctx swd context.
 * \return number of control bytes executed, or error code on failre.
 */
int swd_dap_select_swj(swd_ctx_t *swdctx, swd_operation_t operation){
 return swd_mosi_jtag2swd(swdctx, operation);
}

/** Macro: Read out IDCODE register and return its value on function return.
 * \param *swdctx swd context pointer.
 * \param operation operation type.
 * \return Target's IDCODE value or code error on failure.
 */
int swd_dap_idcode(swd_ctx_t *swdctx, swd_operation_t operation){
 int res;
 res=swd_miso_idcode(swdctx, operation, &swdctx->misoswdp.idcode, &swdctx->misoswdp.ack, &swdctx->misoswdp.parity);
 if (res<1)
  return res;
 else return swdctx->misoswdp.idcode;
}

/** Macro: Reset target DAP, select SW-DP, read out IDCODE.
 * This is the proper SW-DP initialization as stated by ARM Information Center.
 * \param *swdctx swd context pointer.
 * \param operation type (SWD_OPERATION_ENQUEUE or SWD_OPERATION_EXECUTE).
 * \return Target's IDCODE, or error code on failure.
 */ 
int swd_dap_reset_select_idcode(swd_ctx_t *swdctx, swd_operation_t operation){
 int res;
 res=swd_dap_reset(swdctx, operation);
 if (res<1) return res;
 res=swd_dap_select_swj(swdctx, operation);
 if (res<1) return res;
 res=swd_dap_idcode(swdctx, operation);
 return res;
}

/** @} */

/*******************************************************************************
 * \defgroup swd_log Miscelanous logging functionalities.
 * @{
 ******************************************************************************/

/** Put a message into swd context log at specified verbosity level.
 * If specified message's log level is lower than actual context configuration,
 * message will be omitted. Verbosity level increases from 0 (silent) to 4 (debug).
 * \param *swdctx swd context.
 * \param loglevel at which to put selected message.
 * \param *msg message body with variable arguments as in "printf".
 * \return number of characters written or error code on failure.
 */
int swd_log(swd_ctx_t *swdctx, swd_loglevel_t loglevel, char *msg, ...){
 if (loglevel<SWD_LOGLEVEL_MIN && loglevel>SWD_LOGLEVEL_MAX)
  return SWD_ERROR_LOGLEVEL;
 if (loglevel < swdctx->config.loglevel) return 0;
 int res;
 va_list ap;
 va_start(ap, msg);
 res=vprintf(msg, ap);  
 va_end(ap);
 return res;
}


/** @} */

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
  case SWD_ERROR_ACKMISMATCH:  return "[SWD_ERROR_ACKMISMATCH] different result address expected";
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
  default:                     return "undefined error";
 }
 return "undefined error";
}

/** @} */

/*******************************************************************************
 * \defgroup swd_init Library and Context (de)initialization routines.
 * @{
 ******************************************************************************/

/** LibSWD initialization routine.
 * It should be called prior any operation made with libswd. It initializes
 * command queue and basic parameters for context that is returned as pointer.
 * \return pointer to the initialized swd context.
 */
swd_ctx_t *swd_init(void){
 swd_ctx_t *swdctx;
 swdctx=(swd_ctx_t *)calloc(1,sizeof(swd_ctx_t));
 if (swdctx==NULL) return NULL;
 swdctx->driver=(swd_driver_t *)calloc(1,sizeof(swd_driver_t));
 if (swdctx->driver==NULL){
  free(swdctx);
  return NULL;
 }
 swdctx->cmdq=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (swdctx->cmdq==NULL) {
  swd_deinit_ctx(swdctx);
  return NULL;
 }
 swdctx->config.initialized=SWD_TRUE;
 swdctx->config.trnlen=SWD_TURNROUND_DEFAULT;
 swdctx->config.maxcmdqlen=SWD_CMDQLEN_DEFAULT;
 swdctx->config.loglevel=SWD_LOGLEVEL_DEBUG;
 return swdctx;
}

/** De-initialize selected swd context and free its memory.
 * Note: This function will not free command queue for selected context!
 * \param *swdctx swd context pointer.
 * \return SWD_OK on success, SWD_ERROR_CODE on failure.
 */
int swd_deinit_ctx(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLPOINTER;
 free(swdctx);
 return SWD_OK;
}

/** De-initialize command queue and free its memory on selected swd context.
 * \param *swdctx swd context pointer.
 * \return number of commands freed, or SWD_ERROR_CODE on failure.
 */ 
int swd_deinit_cmdq(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLPOINTER;
 int res;
 res=swd_cmdq_free(swdctx->cmdq);
 if (res<0) return res;
 return res;
}

/** De-initialize selected swd context and its command queue.
 * \param *swdctx swd context pointer.
 * \return number of elements freed, or SWD_ERROR_CODE on failure.
 */ 
int swd_deinit(swd_ctx_t *swdctx){
 int res, cmdcnt=0;
 res=swd_deinit_cmdq(swdctx);
 if (res<0) return res;
 cmdcnt=res;
 res=swd_deinit_ctx(swdctx);
 if (res<0) return res;
 return cmdcnt+res;
}


