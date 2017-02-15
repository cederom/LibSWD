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

/** \file libswd_cmdq.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup libswd_cmdq Command Queue helper functions
 * @{
 ******************************************************************************/

/** Initialize new queue element in memory that becomes a queue root.
 * \param *cmdq pointer to the command queue element of type libswd_cmd_t
 * \return LIBSWD_OK on success, LIBSWD_ERROR_CODE code on failure
 */
int libswd_cmdq_init(libswd_cmd_t *cmdq){
 cmdq=(libswd_cmd_t *)calloc(1,sizeof(libswd_cmd_t));
 if (cmdq==NULL) return LIBSWD_ERROR_OUTOFMEM;
 cmdq->prev=NULL;
 cmdq->next=NULL;
 return LIBSWD_OK;
}

/** Find queue root (first element).
 * \param *cmdq pointer to any queue element
 * \return libswd_cmd_t* pointer to the first element (root), NULL on failure
 */
libswd_cmd_t* libswd_cmdq_find_head(libswd_cmd_t *cmdq){
 if (cmdq==NULL) return NULL;
 libswd_cmd_t *cmd=cmdq;
 while (cmd->prev!=NULL) cmd=cmd->prev;
 return cmd;
}

/** Find queue tail (last element).
 * \param  *cmdq pointer to any queue element
 * \return libswd_cmd_t* pointer to the last element (tail), NULL on failure
 */
libswd_cmd_t* libswd_cmdq_find_tail(libswd_cmd_t *cmdq){
 if (cmdq==NULL) return NULL;
 libswd_cmd_t *cmd=cmdq;
 while (cmd->next!=NULL) cmd=cmd->next;
 return cmd;
}

/** Find last executed element from the *cmdq.
 * Start search at *cmdq head, return element pointer or NULL if not found.
 * \param *cmdq queue that contains elements.
 * \return libswd_cmd_t* pointer to the last executed element or NULL on error.
 */
libswd_cmd_t* libswd_cmdq_find_exectail(libswd_cmd_t *cmdq){
 if (cmdq==NULL) return NULL;
 libswd_cmd_t *cmd=libswd_cmdq_find_head(cmdq);
 if (cmd==NULL) return NULL;
 for (cmd=libswd_cmdq_find_head(cmdq);cmd;cmd=cmd->next){
  if (cmd->done) {
   if (cmd->next){
    if (!cmd->next->done) return cmd;
   } else return cmd;
  }
 }
 return NULL;
}

/** Append element pointed by *cmd at the end of the quque pointed by *cmdq.
 * After this operation queue will be pointed by appended element (ie. last
 * element added becomes actual quque pointer to show what was added recently).
 * \param *cmdq pointer to any element on command queue
 * \param *cmd pointer to the command to be appended
 * \return number of appended elements (one), LIBSWD_ERROR_CODE on failure
 */
int libswd_cmdq_append(libswd_cmd_t *cmdq, libswd_cmd_t *cmd){
 if (cmdq==NULL) return LIBSWD_ERROR_NULLQUEUE;
 if (cmd==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (cmdq->next != NULL){
  libswd_cmd_t *lastcmd;
  lastcmd=libswd_cmdq_find_tail(cmdq);
  lastcmd->next=cmd;
  cmd->prev=lastcmd;
 } else {
  cmdq->next=cmd;
  cmd->prev=cmdq;
 }
 return 1;
}

/** Free queue pointed by *cmdq element.
 * \param *cmdq pointer to any element on command queue
 * \return number of elements destroyed, LIBSWD_ERROR_CODE on failure
 */
int libswd_cmdq_free(libswd_cmd_t *cmdq){
 if (cmdq==NULL) return LIBSWD_ERROR_NULLQUEUE;
 int cmdcnt=0;
 libswd_cmd_t *cmd, *nextcmd;
 cmd=libswd_cmdq_find_head(cmdq);
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
 * \return number of elements destroyed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmdq_free_head(libswd_cmd_t *cmdq){
 if (cmdq==NULL) return LIBSWD_ERROR_NULLQUEUE;
 int cmdcnt=0;
 libswd_cmd_t *cmdqroot, *nextcmd;
 cmdqroot=libswd_cmdq_find_head(cmdq);
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
 * \return number of elements destroyed, or LIBSWD_ERROR_CODE on failure.
 */
int libswd_cmdq_free_tail(libswd_cmd_t *cmdq){
 if (cmdq==NULL) return LIBSWD_ERROR_NULLQUEUE;
 int cmdcnt=0;
 libswd_cmd_t *cmdqend;
 if (cmdq->next==NULL) return 0;
 cmdqend=libswd_cmdq_find_tail(cmdq->next);
 if (cmdqend==NULL) return LIBSWD_ERROR_QUEUE;
 while(cmdqend!=cmdq){
  cmdqend=cmdqend->prev;
  free(cmdqend->next);
  cmdcnt++;
 }
 cmdq->next=NULL;
 return cmdcnt;
}

/** Flush command queue contents into interface driver and update **cmdq.
 * Operation is specified by LIBSWD_OPERATION and can be used to select
 * how to flush the queue, ie. head-only, tail-only, one, all, etc.
 * This function is not only used to flush libswdctx->cmdq but also other
 * queues (i.e. error handling) so the parameter is **cmdq not libswdctx itself.
 * This is the only place where **cmdq is updated to the last executed element.
 * Double pointer is used because we update pointer element not its data.
 * \param *cmdq pointer to queue to be flushed.
 * \param operation tells how to flush the queue.
 * \return number of commands transmitted, or LIBSWD_ERROR_CODE on failure.
 * !TODO: HOW WE WANT TO UPDATE CMDQ ELEMENT AFTER PROCESSING WITHOUT DOUBLE POINTER?
 */
int libswd_cmdq_flush(libswd_ctx_t *libswdctx, libswd_cmd_t **cmdq, libswd_operation_t operation){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (*cmdq==NULL||cmdq==NULL) return LIBSWD_ERROR_NULLQUEUE;
 if (operation<LIBSWD_OPERATION_FIRST || operation>LIBSWD_OPERATION_LAST)
  return LIBSWD_ERROR_BADOPCODE;

 int res, cmdcnt=0;
 libswd_cmd_t *cmd, *firstcmd, *lastcmd;

 switch (operation){
  case LIBSWD_OPERATION_TRANSMIT_HEAD:
   firstcmd=libswd_cmdq_find_head(*cmdq);
   lastcmd=*cmdq;
   break;
  case LIBSWD_OPERATION_TRANSMIT_TAIL:
   firstcmd=*cmdq;
   lastcmd=libswd_cmdq_find_tail(*cmdq);
   break;
  case LIBSWD_OPERATION_EXECUTE:
  case LIBSWD_OPERATION_TRANSMIT_ALL:
   firstcmd=libswd_cmdq_find_head(*cmdq);
   lastcmd=libswd_cmdq_find_tail(*cmdq);
   break;
  case LIBSWD_OPERATION_TRANSMIT_ONE:
   firstcmd=*cmdq;
   lastcmd=*cmdq;
   break;
  case LIBSWD_OPERATION_TRANSMIT_LAST:
   firstcmd=libswd_cmdq_find_tail(*cmdq);
   lastcmd=firstcmd;
   break;
  default:
   return LIBSWD_ERROR_BADOPCODE;
 }

 if (firstcmd==NULL) return LIBSWD_ERROR_QUEUEROOT;
 if (lastcmd==NULL) return LIBSWD_ERROR_QUEUETAIL;

 if (firstcmd==lastcmd){
  if (!firstcmd->done) {
   res=libswd_drv_transmit(libswdctx, firstcmd);
   if (res<0) return res;
   *cmdq=firstcmd;
  }
  return 1;
 }

 for (cmd=firstcmd;;cmd=cmd->next){
  if (cmd->done){
   if (cmd->next){
    continue;
   } else break;
  }
  res=libswd_drv_transmit(libswdctx, cmd);
  if (res<0) return res;
  cmdcnt=+res;
  if (cmd==lastcmd) break;
 }
 *cmdq=cmd;
 return cmdcnt;
}

/** @} */
