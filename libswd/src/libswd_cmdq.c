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

/** \file libswd_cmdq.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup swd_cmdq Command Queue helper functions
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
  return swd_drv_transmit(swdctx, firstcmd); 

 for (cmd=firstcmd;;cmd=cmd->next){
  if (cmd->done) continue;
  res=swd_drv_transmit(swdctx, cmd); 
  if (res<0) return res;
  cmdcnt=+res;
  if (cmd==lastcmd) break;
 } 

 return cmdcnt;
}

/** @} */
