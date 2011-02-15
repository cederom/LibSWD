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

/*******************************************************************************
 * \defgroup swd_bin Binary helper functions.
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
int swd_cmd_queue_init(swd_cmd_t *cmdq){
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
swd_cmd_t* swd_cmd_queue_find_root(swd_cmd_t *cmdq){
 if (cmdq==NULL) return NULL;
 swd_cmd_t *cmd=cmdq;
 while (cmd->prev!=NULL) cmd=cmd->prev;
 return cmd;
}

/** Find queue tail (last element).
 * \param  *cmdq pointer to any queue element
 * \return swd_cmd_t* pointer to the last element (tail), NULL on failure
 */
swd_cmd_t* swd_cmd_queue_find_tail(swd_cmd_t *cmdq){
 if (cmdq==NULL) return NULL;
 swd_cmd_t *cmd=cmdq;
 while (cmd->next!=NULL) cmd=cmd->next;
 return cmd;
}

/** Append element pointed by *cmd at the end of the quque pointed by *cmdq.
 * \param *cmdq pointer to any element on command queue 
 * \param *cmd pointer to the command to be appended
 * \return number of appended elements (one), SWD_ERROR_CODE on failure
 */
int swd_cmd_queue_append(swd_cmd_t *cmdq, swd_cmd_t *cmd){
 if (cmdq==NULL) return SWD_ERROR_NULLQUEUE;
 if (cmd==NULL) return SWD_ERROR_NULLPOINTER;
 if (cmdq->next != NULL){
  swd_cmd_t *lastcmd;
  lastcmd=swd_cmd_queue_find_tail(cmdq);
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
int swd_cmd_queue_free(swd_cmd_t *cmdq){
 if (cmdq==NULL) return SWD_ERROR_NULLQUEUE;
 int cmdcnt=0;
 swd_cmd_t *cmd, *nextcmd;
 cmd=swd_cmd_queue_find_root(cmdq);
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
int swd_cmd_queue_free_head(swd_cmd_t *cmdq){
 if (cmdq==NULL) return SWD_ERROR_NULLQUEUE;
 int cmdcnt=0;
 swd_cmd_t *cmdqroot, *nextcmd;
 cmdqroot=swd_cmd_queue_find_root(cmdq);
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
int swd_cmd_queue_free_tail(swd_cmd_t *cmdq){
 if (cmdq==NULL) return SWD_ERROR_NULLQUEUE;
 int cmdcnt=0;
 swd_cmd_t *cmdqend, *nextcmd;
 nextcmd=cmdq;
 cmdqend=swd_cmd_queue_find_tail(cmdq);
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
 * \defgroup SWD Command queue elements generation routines.
 * These command quants are created in memory and can be easily appended
 * to the end of existing queue pointed by *cmdq element.
 * All functions here start with "swd_cmd_queue_append_" prefix.
 * @{
 ******************************************************************************/

/** Appends command queue with SWD Request packet header.
 * Note that contents is not validated, so bad request can be sent as well.
 * \param *swdctx swd context pointer.
 * \param *request pointer to the 8-bit request payload.
 * \return return number elements appended (1), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_mosi_request(swd_ctx_t *swdctx, char *request){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (request==NULL) return SWD_ERROR_NULLPOINTER;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 cmd->request=*request;
 cmd->bits=SWD_REQUEST_BITLEN;
 cmd->cmdtype=SWD_CMDTYPE_MOSI_REQUEST;
 return swd_cmd_queue_append(swdctx->cmdq, cmd);
} 

/** Append command queue with Turnaround activating MOSI mode.
 * \param *swdctx swd context pointer.
 * \return return number elements appended (1), or SWD_ERROR_CODE on failure.
 */ 
int swd_cmd_queue_append_mosi_trn(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
  swd_cmd_t *cmd;
  cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
  if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
  cmd->TRNnMOSI=0;
  cmd->bits=swdctx->config.trnlen;
  cmd->cmdtype=SWD_CMDTYPE_MOSI_TRN;
  return swd_cmd_queue_append(swdctx->cmdq, cmd);
}

/** Append command queue with Turnaround activating MISO mode.
 * \param *swdctx swd context pointer.
 * \return return number of elements appended (1), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_miso_trn(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
  swd_cmd_t *cmd;
  cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
  if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
  cmd->TRNnMOSI=1;
  cmd->bits=swdctx->config.trnlen;
  cmd->cmdtype=SWD_CMDTYPE_MISO_TRN;
  return swd_cmd_queue_append(swdctx->cmdq, cmd);
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
int swd_cmd_queue_append_miso_nbit(swd_ctx_t *swdctx, char **data, int count){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (count<=0) return SWD_ERROR_PARAM;
 swd_cmd_t **cmd;
 cmd=(swd_cmd_t **)calloc(count,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 int i;
 for (i=0;i<count;i++){
  data[i]=&cmd[i]->misobit;
  cmd[i]->bits=1;
  cmd[i]->cmdtype=SWD_CMDTYPE_MISO_BITBANG;
  if (swd_cmd_queue_append(swdctx->cmdq, cmd[i])!=1){
   swd_cmd_queue_free_tail(cmd[0]); // if that fails, free memory, return error
   for (i=0;i<count;i++) free(cmd[i]);
   return SWD_ERROR_QUEUE;
  }
 }
 return i;
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
int swd_cmd_queue_append_mosi_nbit(swd_ctx_t *swdctx, char *data, int count){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (count<=0) return SWD_ERROR_PARAM;
 swd_cmd_t **cmd;
 cmd=(swd_cmd_t **)calloc(count, sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 int i;
 for (i=0;i<count;i++){
  cmd[i]->mosibit=data[i];
  cmd[i]->bits=1;
	 cmd[i]->cmdtype=SWD_CMDTYPE_MOSI_BITBANG;
  if (swd_cmd_queue_append(swdctx->cmdq, cmd[i])!=1){
   swd_cmd_queue_free_tail(cmd[0]); // if that fails, clean memory, return error
   for (i=0;i<count;i++) free(cmd[i]);
   return SWD_ERROR_QUEUE;
  }
 }
 return count;
}

/** Append command queue with parity bit write.
 * \param *swdctx swd context pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (1), or SWD_ERROR_CODE on failure.
 */ 
int swd_cmd_queue_append_mosi_parity(swd_ctx_t *swdctx, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (*parity!=0 || *parity!=1) return SWD_ERROR_PARAM;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 cmd->parity=*parity;
 cmd->bits=1;
 cmd->cmdtype=SWD_CMDTYPE_MOSI_PARITY;
 return swd_cmd_queue_append(swdctx->cmdq, cmd);
}

/** Append command queue with parity bit read.
 * \param *swdctx swd context pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (1), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_miso_parity(swd_ctx_t *swdctx, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (parity==NULL) return SWD_ERROR_NULLPOINTER;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 parity=&cmd->parity;
 cmd->bits=1;
 cmd->cmdtype=SWD_CMDTYPE_MISO_PARITY;
 return swd_cmd_queue_append(swdctx->cmdq, cmd);
}

/** Append command queue with data read.
 * \param *swdctx swd context pointer.
 * \param *data data pointer.
 * \return of elements appended (1), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_miso_data(swd_ctx_t *swdctx, int *data){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 data=&cmd->misodata;
 cmd->bits=32;
 cmd->cmdtype=SWD_CMDTYPE_MISO_DATA;
 return swd_cmd_queue_append(swdctx->cmdq, cmd); // should be 1 on success
}

/** Append command queue with data and parity read.
 * \param *swdctx swd context pointer.
 * \param *data data value pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (2), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_miso_data_p(swd_ctx_t *swdctx, int *data, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (parity==NULL) return SWD_ERROR_NULLPOINTER;
 int res, cmdcnt=0;
 res=swd_cmd_queue_append_miso_data(swdctx, data);
 if (res<0) return res;
 cmdcnt=+res;
 res=swd_cmd_queue_append_miso_parity(swdctx, parity);
 if (res<0) return res;
 cmdcnt=+res;
 return cmdcnt; // should be 2 on success
}

/** Append command queue with series of data and parity read.
 * \param *swdctx swd context pointer.
 * \param **data data value array pointer.
 * \param **parity parity value array pointer.
 * \param count number of (data+parity) elements to read.
 * \return number of elements appended (2*count), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_miso_n_data_p(swd_ctx_t *swdctx, int **data, char **parity, int count){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (count<=0) return SWD_ERROR_PARAM;
 int i,res, cmdcnt=0;
 for (i=0;i<=count;i++){
  res=swd_cmd_queue_append_miso_data_p(swdctx, data[i], parity[i]);
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
int swd_cmd_queue_append_mosi_data(swd_ctx_t *swdctx, int *data){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 cmd->mosidata=*data;
 cmd->bits=32;
 cmd->cmdtype=SWD_CMDTYPE_MOSI_DATA;
 return swd_cmd_queue_append(swdctx->cmdq, cmd); // should be 1 on success
}

/** Append command queue with data and automatic parity write.
 * \param *swdctx swd context pointer.
 * \param *data data value pointer.
 * \return number of elements appended (2), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_mosi_data_ap(swd_ctx_t *swdctx, int *data){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 int res, cmdcnt=0;
 char parity;
 res=swd_cmd_queue_append_mosi_data(swdctx, data);
 if (res<1) return res;
 cmdcnt=+res;
 res=swd_bin32_parity_even(data, &parity);
 if (res<0) return res;
 res=swd_cmd_queue_append_mosi_parity(swdctx, &parity);
 if (res<1) return res;
 cmdcnt=+res;
 return cmdcnt; // should be 2 on success
}

/** Append command queue with data and provided parity write.
 * \param *swdctx swd context pointer.
 * \param *data data value pointer.
 * \param *parity parity value pointer.
 * \return number of elements appended (2), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_mosi_data_p(swd_ctx_t *swdctx, int *data, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL || parity==NULL) return SWD_ERROR_NULLPOINTER;
 int res, cmdcnt=0;
 res=swd_cmd_queue_append_mosi_data(swdctx, data);
 if (res<1) return res;
 cmdcnt=+res;
 res=swd_cmd_queue_append_mosi_parity(swdctx, parity);
 if (res<1) return res;
 cmdcnt=+res;
 return cmdcnt; // should be 2 on success
}

/** Append command queue with series of data and automatic parity writes.
 * \param *swdctx swd context pointer.
 * \param **data data value array pointer.
 * \param count number of (data+parity) elements to read.
 * \return number of elements appended (2*count), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_append_mosi_n_data_ap(swd_ctx_t *swdctx, int **data, int count){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (count<1) return SWD_ERROR_PARAM;
 int i, res, cmdcnt=0;
 for (i=0;i<count;i++){
  res=swd_cmd_queue_append_mosi_data(swdctx, data[i]);
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
int swd_cmd_append_mosi_n_data_p(swd_ctx_t *swdctx, int **data, char **parity, int count){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL) return SWD_ERROR_NULLPOINTER;
 if (count<1) return SWD_ERROR_PARAM;
 int i, res, cmdcnt=0;
 for (i=0;i<count;i++){
  res=swd_cmd_queue_append_mosi_data_p(swdctx, data[i], parity[i]);
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
int swd_cmd_queue_append_miso_ack(swd_ctx_t *swdctx, char *ack){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (ack==NULL) return SWD_ERROR_NULLPOINTER;
 swd_cmd_t *cmd;
 cmd=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 ack=&cmd->ack;
 cmd->bits=SWD_ACK_BITLEN;
 cmd->cmdtype=SWD_CMDTYPE_MISO_ACK;
 return swd_cmd_queue_append(swdctx->cmdq, cmd); //should be 1 on success
}

/** Append command queue with len-octet size control seruence.
 * This control sequence can be used for instance to send payload of packets
 * switching DAP between JTAG and SWD mode.
 * \param *swdctx swd context pointer.
 * \param *ctlmsg control message array pointer.
 * \param len number of elements to send from *ctlmsg.
 * \return number of elements appended (len), or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_mosi_control(swd_ctx_t *swdctx, char *ctlmsg, int len){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (ctlmsg==NULL) return SWD_ERROR_NULLPOINTER;
 if (len<=0) return SWD_ERROR_PARAM;
 int elm, res, cmdcnt=0;
 swd_cmd_t **cmd;
 cmd=(swd_cmd_t **)calloc(len,sizeof(swd_cmd_t));
 if (cmd==NULL) return SWD_ERROR_OUTOFMEM;
 for (elm=0;elm<len;elm++){
  cmd[elm]->control=ctlmsg[elm];
  cmd[elm]->cmdtype=SWD_CMDTYPE_MOSI_CONTROL;
  cmd[elm]->bits=sizeof(ctlmsg[elm]);
  res=swd_cmd_queue_append(swdctx->cmdq, cmd[elm]); 
  if (res<=1) return res;
  cmdcnt=+res;
 }
 return cmdcnt;
}

/** Append command queue with SW-DP-RESET sequence.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_swdpreset(swd_ctx_t *swdctx){
 return swd_cmd_queue_append_mosi_control(swdctx, (char *)SWD_CMD_SWDPRESET, sizeof(SWD_CMD_SWDPRESET));
}

/** Append command queue with JTAG-TO-SWD DAP-switch sequence.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_jtag2swd(swd_ctx_t *swdctx){
 return swd_cmd_queue_append_mosi_control(swdctx, (char *)SWD_CMD_JTAG2SWD, sizeof(SWD_CMD_JTAG2SWD));
}

/** Append command queue with SWD-TO-JTAG DAP-switch sequence.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_append_swd2jtag(swd_ctx_t *swdctx){
 return swd_cmd_queue_append_mosi_control(swdctx, (char *)SWD_CMD_SWD2JTAG, sizeof(SWD_CMD_SWD2JTAG));
}

/** Append command queue with TRN WRITE/MOSI.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_bus_setdir_mosi(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 int res, cmdcnt=0;
 if ( swdctx->cmdq->prev==NULL || (swdctx->cmdq->cmdtype*SWD_CMDTYPE_MOSI<0) ) {
  res=swd_cmd_queue_append_mosi_trn(swdctx);
  if (res<1) return res;
  cmdcnt=+res;
 }
 return cmdcnt;
}
 
/** Append command queue with TRN READ/MISO.
 * \param *swdctx swd context pointer.
 * \return number of elements appended, or SWD_ERROR_CODE on failure.
 */
int swd_bus_setdir_miso(swd_ctx_t *swdctx){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 int res, cmdcnt=0;
 if ( swdctx->cmdq->prev==NULL || (swdctx->cmdq->cmdtype*SWD_CMDTYPE_MISO<0) ) {
  res=swd_cmd_queue_append_miso_trn(swdctx);
  if (res<0) return res;
  cmdcnt=+res;
 }
 return cmdcnt;
}

/** @} */

/*******************************************************************************
 * \defgroup swd_bit_gen Packet generation helper routines.
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
int swd_bit8_gen_request(swd_ctx_t *swdctx, char *APnDP, char *RnW, char *addr, char *request){
 /* Verify function parameters.*/
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (*APnDP!=0 || *APnDP!=1) return SWD_ERROR_APnDP;
 if (*RnW!=0 || *RnW!=1) return SWD_ERROR_RnW;
 if (*addr<SWD_ADDR_MINVAL || *addr>SWD_ADDR_MAXVAL) return SWD_ERROR_ADDR;

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

/*
extern int swd_drv_mosi_8(swd_ctx_t *swdctx, char *data, int bits, int nLSBfirst);
extern int swd_drv_mosi_32(int *data, int bits, int nLSBfirst);
extern int swd_drv_miso_8(char *data, int bits, int nLSBfirst);
extern int swd_drv_miso_32(int *data, int bits, int nLSBfirst);
*/

/** Transmit selected command to the interface driver.
 * \param *swdctx swd context pointer.
 * \param *cmd pointer to the command to be sent.
 * \return number of commands transmitted (1), or SWD_ERROR_CODE on failure.
 */ 
int swd_transmit(swd_ctx_t *swdctx, swd_cmd_t *cmd){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (cmd==NULL) return SWD_ERROR_NULLPOINTER;

 int res=SWD_ERROR_BADCMDTYPE;

 switch (cmd->cmdtype){
  case SWD_CMDTYPE_MOSI:
  case SWD_CMDTYPE_MISO:
   swd_log(SWD_LOGLEVEL_WARNING, "This command does not contain payload.");
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
   if (cmd->bits<SWD_TURNROUND_MIN || cmd->bits>SWD_TURNROUND_MAX)
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
   swd_drv_miso_8(swdctx, &cmd->ack, cmd->bits, SWD_DIR_LSBFIRST);
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
   if (cmd->bits<SWD_TURNROUND_MIN || cmd->bits>SWD_TURNROUND_MAX)
    return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_trn(swdctx, cmd->bits);
   break;

  case SWD_CMDTYPE_MISO_DATA:
   // 32 clock cycles
   if (cmd->bits!=SWD_DATA_BITLEN) return SWD_ERROR_BADCMDDATA;
   res=swd_drv_miso_32(swdctx, &cmd->misodata, cmd->bits, SWD_DIR_MSBFIRST);
   break;

  default:
   res=SWD_ERROR_BADCMDTYPE;
   return res;
 } 

 if (res<0) return res;
 cmd->done=1;
 return 1;
}

/** Flush command queue contents into interface driver.
 * Operation is specified by SWD_OPERATION and can be used to select
 * how to flush the queue, ie. head-only, tail-only, one, all, etc.
 * \param *swdctx swd context pointer.
 * \param operation tells how to flush the queue.
 * \return number of commands transmitted, or SWD_ERROR_CODE on failure.
 */
int swd_cmd_queue_flush(swd_ctx_t *swdctx, swd_operation_t operation){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (operation<SWD_OPERATION_FIRST || operation>SWD_OPERATION_LAST)
  return SWD_ERROR_BADOPCODE;

 int res, cmdcnt=0;
 swd_cmd_t *cmd, *firstcmd, *lastcmd;

 switch (operation){
  case SWD_OPERATION_TRANSMIT_HEAD:
   firstcmd=swd_cmd_queue_find_root(swdctx->cmdq);
   lastcmd=swdctx->cmdq;
   break;
  case SWD_OPERATION_TRANSMIT_TAIL:
   firstcmd=swdctx->cmdq;
   lastcmd=swd_cmd_queue_find_tail(swdctx->cmdq);
   break;
  case SWD_OPERATION_TRANSMIT_ALL:
   firstcmd=swd_cmd_queue_find_root(swdctx->cmdq);
   lastcmd=swd_cmd_queue_find_tail(swdctx->cmdq);
   break;
  case SWD_OPERATION_TRANSMIT_ONE:
   firstcmd=swdctx->cmdq;
   lastcmd=swdctx->cmdq;
   break;
  case SWD_OPERATION_TRANSMIT_LAST:
   firstcmd=swd_cmd_queue_find_tail(swdctx->cmdq);
   lastcmd=firstcmd;
   break;
  default:
   return SWD_ERROR_BADOPCODE;
 }

 if (firstcmd==NULL) return SWD_ERROR_QUEUEROOT;
 if (lastcmd==NULL) return SWD_ERROR_QUEUE;

 if (firstcmd==lastcmd)
  return swd_transmit(swdctx, firstcmd); 

 for (cmd=firstcmd;cmd!=lastcmd;cmd=cmd->next){
  res=swd_transmit(swdctx, cmd); 
  if (res<0) return res;
  cmdcnt=+res;
 } 

 return cmdcnt;
}

/** @} */


/*******************************************************************************
 * \defgroup swd_packet SWD Operations Generators: Request, ACK, Data.
 * These functions generate payloads and queue up all elements/commands
 * necessary to perform requested operations on the SWD bus. Depending
 * on operation type, elements can be executed or queued up for future transfer.
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
 if (operation<SWD_OPERATION_FIRST && operation>SWD_OPERATION_LAST)
  return SWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;
 char request;

 /* Generate request bitstream. */ 
 res=swd_bit8_gen_request(swdctx, APnDP, RnW, addr, &request);
 if (res<0) return res;

 /* Bus direction must be MOSI. */
 res=swd_bus_setdir_mosi(swdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 /* Append request command to the queue. */
 res=swd_cmd_queue_append_mosi_request(swdctx, &request);
 if (res<0) return res;
 qcmdcnt+=res;

 if (operation==SWD_OPERATION_QUEUE_APPEND){
  return qcmdcnt; 
 } else {
  res=swd_cmd_queue_flush(swdctx, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return qcmdcnt+tcmdcnt;
 }
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
 if (operation<SWD_OPERATION_FIRST || operation>SWD_OPERATION_LAST) return SWD_ERROR_BADOPCODE;

 if (swdctx->cmdq->prev->cmdtype!=SWD_CMDTYPE_MOSI_REQUEST
  && swdctx->cmdq->prev->cmdtype!=SWD_CMDTYPE_MISO_TRN)  return SWD_ERROR_ACKORDER;

 int res, qcmdcnt=0, tcmdcnt=0;
 swd_cmd_t *tmpcmdq;

 /* Bus direction must be MISO. */ 
 res=swd_bus_setdir_miso(swdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=swd_cmd_queue_append_miso_ack(swdctx, ack);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==SWD_OPERATION_QUEUE_APPEND){
  return qcmdcnt;
 } else {
  res=swd_cmd_queue_flush(swdctx, operation);
  if (res<0) return res;
  return res;
 }

 /* Now verify the read result and return error if necessary. */

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
int swd_mosi_data_p(swd_ctx_t *swdctx, swd_operation_t operation, int *data, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (data==NULL || parity==NULL) return SWD_ERROR_NULLPOINTER;
 if (operation<SWD_OPERATION_FIRST || operation>SWD_OPERATION_LAST) return SWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;

 res=swd_bus_setdir_mosi(swdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=swd_cmd_queue_append_mosi_data_p(swdctx, data, parity);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==SWD_OPERATION_QUEUE_APPEND){
  return qcmdcnt;       
 } else {
  res=swd_cmd_queue_flush(swdctx, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return tcmdcnt;
 }
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
 if (operation<SWD_OPERATION_FIRST || operation>SWD_OPERATION_LAST) return SWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;

 res=swd_bus_setdir_mosi(swdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=swd_cmd_queue_append_mosi_data_ap(swdctx, data);
 if (res<0) return res;
 qcmdcnt=+res;

 if (operation==SWD_OPERATION_QUEUE_APPEND){
  return qcmdcnt;       
 } else {
  res=swd_cmd_queue_flush(swdctx, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return tcmdcnt;
 }
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
 if (operation<SWD_OPERATION_FIRST || operation>SWD_OPERATION_LAST) return SWD_ERROR_BADOPCODE;
 
 int res, qcmdcnt=0, tcmdcnt=0;
 swd_cmd_t *tmpcmdq;

 res=swd_bus_setdir_miso(swdctx);
 if (res<0) return res;
 qcmdcnt=+res;

 res=swd_cmd_queue_append_miso_data_p(swdctx, data, parity);
 if (res<0) return res;
 qcmdcnt=+res;

 res=swd_cmd_queue_flush(swdctx, operation);
 if (res<0) return res;

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
   if (data!=&tmpcmdq->misodata) return SWD_ERROR_DATAADDR;
   if (parity!=&tmpcmdq->next->parity) return SWD_ERROR_PARITYADDR;
   return qcmdcnt+tcmdcnt; 
  } else return SWD_ERROR_NOTDONE;
 }
 return SWD_OK;
}

/** Switch DAP into SW-DP. According to ARM documentation target's DAP use JTAG
 * transport by default and so JTAG-DP is active after power up. To use SWD
 * user must perform predefined sequence on SWDIO/TMS lines, then read out the
 * IDCODE to ensure proper SW-DP operation.
 */
int swd_mosi_jtag2swd(swd_ctx_t *swdctx, swd_operation_t operation){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT;
 if (operation<SWD_OPERATION_FIRST || operation>SWD_OPERATION_LAST) return SWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;

 res=swd_cmd_queue_append_jtag2swd(swdctx);
 if (res<0) return res;
 qcmdcnt=res;

 if (operation==SWD_OPERATION_QUEUE_APPEND){
  return qcmdcnt;       
 } else {
  res=swd_cmd_queue_flush(swdctx, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return tcmdcnt;
 }
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

/** Activate SW-DP and deactivate JTAG-DP by sending out JTAG-TO-SWD sequence.
 * \param *swdctx swd context.
 * \return number of control bytes executed, or error code on failre.
 */
int swd_jtag2swd(swd_ctx_t *swdctx, swd_operation_t operation){
 return swd_mosi_jtag2swd(swdctx, operation);
}

/** Read target's IDCODE register value.
 * \param *swdctx swd context pointer.
 * \param operation type of action to perform (queue or execute).
 * \param *idcode resulting register value pointer.
 * \param *ack resulting acknowledge response value pointer.
 * \param *parity resulting data parity value pointer.
 * \return number of elements processed on the queue, or SWD_ERROR_CODE on failure.
 */
int swd_idcode(swd_ctx_t *swdctx, swd_operation_t operation, int *idcode, char *ack, char *parity){
 if (swdctx==NULL) return SWD_ERROR_NULLCONTEXT; 
 if (idcode==NULL || ack==NULL || parity==NULL) return SWD_ERROR_NULLPOINTER;
 if (operation!=SWD_OPERATION_QUEUE_APPEND && operation!=SWD_OPERATION_EXECUTE) return SWD_ERROR_BADOPCODE;

 int res, cmdcnt=0;
 char APnDP, RnW, addr, cparity;

 APnDP=0;
 RnW=0;
 addr=SWD_DP_ADDR_IDCODE;

 res=swd_mosi_request(swdctx, SWD_OPERATION_QUEUE_APPEND, &APnDP, &RnW, &addr);
 if (res<1) return res;
 cmdcnt=+res;

 if (operation==SWD_OPERATION_QUEUE_APPEND){
  res=swd_miso_ack(swdctx, SWD_OPERATION_QUEUE_APPEND, ack);
  if (res<1) return res;
  cmdcnt=+res;
  res=swd_miso_data_p(swdctx, SWD_OPERATION_QUEUE_APPEND, idcode, parity);
  if (res<1) return res;
  cmdcnt=+res;
  return cmdcnt;

 } else if (operation==SWD_OPERATION_EXECUTE){
  res=swd_miso_ack(swdctx, SWD_OPERATION_TRANSMIT_HEAD, ack);
  if (res<0) return res;
  if (*ack!=SWD_ACK_OK_VAL){
   if (*ack==SWD_ACK_WAIT_VAL) return SWD_ERROR_ACK_WAIT;
   if (*ack==SWD_ACK_FAULT_VAL) return SWD_ERROR_ACK_FAULT; 
   return SWD_ERROR_ACK;
  }
  res=swd_miso_data_p(swdctx, SWD_OPERATION_TRANSMIT_HEAD, idcode, parity);
  if (res<0) return res;
  res=swd_bin32_parity_even(idcode, &cparity); 
  if (res<0) return res;
  if (cparity!=*parity) return SWD_ERROR_PARITY;
  return cmdcnt;
 } else return SWD_ERROR_BADOPCODE;
}

/** @} */

/*******************************************************************************
 * \defgroup swd_log Miscelanous logging functionalities.
 * @{
 ******************************************************************************/

int swd_log(swd_loglevel_t loglevel, char *msg){
 return fputs(msg, stderr);  
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
  case SWD_ERROR_ACKORDER:     return "[SWD_ERROR_ACKORDER] not in sequence REQ->TRN->ACK";
  case SWD_ERROR_BADOPCODE:    return "[SWD_ERROR_BADOPCODE] unsupported operation requested";
  case SWD_ERROR_NODATACMD:    return "[SWD_ERROR_NODATACMD] command not found on the queue";
  case SWD_ERROR_DATAADDR:     return "[SWD_ERROR_DATAADDR] bad data address";
  case SWD_ERROR_NOPARITYCMD:  return "[SWD_ERROR_NOPARITYCMD] parity command missing or misplaced";
  case SWD_ERROR_PARITYADDR:   return "[SWD_ERROR_PARITYADDR] bad parity command result address";
  case SWD_ERROR_NOTDONE:      return "[SWD_ERROR_NOTDONE] could not end selected task";
  case SWD_ERROR_QUEUEROOT:    return "[SWD_ERROR_QUEUEROOT] queue root not found or null";
  case SWD_ERROR_BADCMDTYPE:   return "[SWD_ERROR_BADCMDTYPE] unknown command detected";
  case SWD_ERROR_BADCMDDATA:   return "[SWD_ERROR_BADCMDDATA] command contains bad data (out of range, etc)";
  case SWD_ERROR_ACK_WAIT:     return "[SWD_ERROR_ACK_WAIT] got ACK_WAIT response";
  case SWD_ERROR_ACK_FAULT:    return "[SWD_ERROR_ACK_FAULT] got ACK_FAULT response";
  case SWD_ERROR_QUEUENOTFREE: return "[SWD_ERROR_QUEUENOTFREE] cannot free resources, queue not empty";
  case SWD_ERROR_TRANSPORT:    return "[SWD_ERROR_TRANSPORT] transport error or undefined";
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
 swdctx->cmdq=(swd_cmd_t *)calloc(1,sizeof(swd_cmd_t));
 if (swdctx->cmdq==NULL) {
  swd_deinit_ctx(swdctx);
  return NULL;
 }
 swdctx->config.initialized=SWD_TRUE;
 swdctx->config.trnlen=SWD_TURNROUND_DEFAULT;
 swdctx->config.maxcmdqlen=SWD_CMDQLEN_DEFAULT;
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
 res=swd_cmd_queue_free(swdctx->cmdq);
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


