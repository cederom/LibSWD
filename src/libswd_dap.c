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

/** \file libswd_dap.c DAP, DP, AP Operation Routines. */

#include <libswd.h>

/*******************************************************************************
 * \defgroup libswd_dap High-level SWD Debug Access Port operations.
 * High level functions in general call lower level functions that append
 * queue with specific commands and payload, but also react on received data.
 * They operate on data pointers where target data is being stored.
 * Operation can be LIBSWD_OPERATION_QUEUE_APPEND for queueing only the command
 * for later execution, or LIBSWD_OPERATION_EXECUTE to queue command, flush it
 * into the interface driver (target read/write) and react on its result before
 * function returns.
 * Return values: negative number on error, data on success.
 ******************************************************************************/

/** Debug Access Port Reset sends 50 CLK with TMS high that brings both
 * SW-DP and JTAG-DP into reset state.
 * \param *libswdctx swd context pointer.
 * \param operation type (LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE).
 * \return number of elements processed or LIBSWD_ERROR_CODE code on failure.
 */
int libswd_dap_reset(libswd_ctx_t *libswdctx, libswd_operation_t operation){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "LIBSWD_I: Executing libswd_dap_reset(*libswdctx=%p, operation=%s)\n", (void*)libswdctx, libswd_operation_string(operation));

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;  

 int res, qcmdcnt=0, tcmdcnt=0;
 res=libswd_bus_setdir_mosi(libswdctx);
 if (res<0) return res;
 res=libswd_cmd_enqueue_mosi_dap_reset(libswdctx);
 if (res<1) return res;
 qcmdcnt+=res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  return qcmdcnt;
 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_cmdq_flush(libswdctx, &libswdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt+=res;
  return qcmdcnt+tcmdcnt;
 } else return LIBSWD_ERROR_BADOPCODE;
}

/** Activate SW-DP by sending out RESET and JTAG-TO-SWD sequence on SWDIOTMS line.
 * \param *libswdctx swd context.
 * \return number of control bytes executed, or error code on failre.
 */
int libswd_dap_select(libswd_ctx_t *libswdctx, libswd_operation_t operation){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "LIBSWD_I: Executing libswd_dap_activate(*libswdctx=%p, operation=%s)\n", (void*)libswdctx, libswd_operation_string(operation));

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res, qcmdcnt=0, tcmdcnt=0;
 res=libswd_bus_setdir_mosi(libswdctx);
 if (res<0) return res;
 res=libswd_cmd_enqueue_mosi_jtag2swd(libswdctx);
 if (res<0) return res;
 qcmdcnt=res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  return qcmdcnt;       
 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_cmdq_flush(libswdctx, &libswdctx->cmdq, operation);
  if (res<0) return res;
  tcmdcnt=+res;
  return qcmdcnt+tcmdcnt;
 } else return LIBSWD_ERROR_BADOPCODE;
}

/** Read out the CTRL/STAT and clear apropriate flags in ABORT register.
 * This function is helpful for clearing sticky errors.
 * ABORT register flags are additionally bitmasked by a function parameter,
 * so called can control which bits can be set (0xFFFFFFFF allows all).
 * \param *libswdctx swd context pointer.
 * \param operation operation type.
 * \param *ctrlstat will hold the CTRL/STAT register value.
 * \param *abort bitmask of which ABORT flags can be set, also will hold the ABORT write.
 */
int libswd_dap_errors_handle(libswd_ctx_t *libswdctx, libswd_operation_t operation, int *ctrlstat, int *abort){
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_EXECUTE && operation!=LIBSWD_OPERATION_ENQUEUE) return LIBSWD_ERROR_BADOPCODE;

 int res, abortreg;
 char APnDP=0, R=1, W=0, ctrlstat_addr=LIBSWD_DP_CTRLSTAT_ADDR, abort_addr=LIBSWD_DP_ABORT_ADDR, *ack, *parity, cparity;
 if (ctrlstat!=NULL){
  res=libswd_bus_write_request(libswdctx, operation, &APnDP, &R, &ctrlstat_addr); 
  if (res<0) return res;
  res=libswd_bus_read_ack(libswdctx, operation, &ack);
  if (res<0) return res;
  res=libswd_bus_read_data_p(libswdctx, operation, &ctrlstat, &parity);
  if (res<0) return res;
  res=libswd_bin32_parity_even(ctrlstat, &cparity);
  if (res<0) return res;
  if (*parity!=cparity) return LIBSWD_ERROR_PARITY;
 }

 if (abort!=NULL) {
  *abort=*abort&(LIBSWD_DP_ABORT_STKCMPCLR|LIBSWD_DP_ABORT_STKERRCLR|LIBSWD_DP_ABORT_WDERRCLR|LIBSWD_DP_ABORT_ORUNERRCLR); 
  res=libswd_bus_write_request(libswdctx, operation, &APnDP, &W, &abort_addr);
  if (res<0) return res;
  res=libswd_bus_read_ack(libswdctx, operation, &ack);
  if (res<0) return res;
  res=libswd_bus_write_data_ap(libswdctx, operation, abort);
  if (res<0) return res;
 }
 return LIBSWD_OK;
}


/** Macro: Read out IDCODE register and return its value on function return.
 * \param *libswdctx swd context pointer.
 * \param operation operation type.
 * \return Number of elements processed or LIBSWD_ERROR code error on failure.
 */
int libswd_dp_read_idcode(libswd_ctx_t *libswdctx, libswd_operation_t operation, int **idcode){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_dp_read_idcode(*libswdctx=%p, operation=%s): entering function...\n", (void*)libswdctx, libswd_operation_string(operation));

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT; 
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
         return LIBSWD_ERROR_BADOPCODE;

 int res, cmdcnt=0;
 char APnDP, RnW, addr, cparity, *ack, *parity;

 APnDP=0;
 RnW=1;
 addr=LIBSWD_DP_IDCODE_ADDR;

 res=libswd_bus_write_request(libswdctx, LIBSWD_OPERATION_ENQUEUE, &APnDP, &RnW, &addr);
 if (res<1) return res;
 cmdcnt=+res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  res=libswd_bus_read_ack(libswdctx, operation, (char**)&libswdctx->qlog.read.ack);
  if (res<1) return res;
  cmdcnt=+res;
  res=libswd_bus_read_data_p(libswdctx, operation, (int**)&libswdctx->qlog.read.data, (char**)&libswdctx->qlog.read.parity);
  if (res<1) return res;
  cmdcnt=+res;
  return cmdcnt;

 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_bus_read_ack(libswdctx, operation, &ack);
  if (res<1) return res;
  cmdcnt+=res;
  res=libswd_bus_read_data_p(libswdctx, operation, idcode, &parity);
  if (res<0) return res;
  cmdcnt+=res;
  libswdctx->log.dp.idcode=**idcode;
  libswdctx->log.dp.parity=*parity;
  libswdctx->log.dp.ack   =*ack;
  res=libswd_bin32_parity_even(*idcode, &cparity); 
  if (res<0) return res;
  if (cparity!=*parity) return LIBSWD_ERROR_PARITY;
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "LIBSWD_I: libswd_dp_read_idcode(libswdctx=@%p, operation=%s, **idcode=0x%X/%s).\n", (void*)libswdctx, libswd_operation_string(operation), **idcode, libswd_bin32_string(*idcode));
  return cmdcnt;
 } else return LIBSWD_ERROR_BADOPCODE;
}

/** Macro: Reset target DAP, select SW-DP, read out IDCODE.
 * This is the proper SW-DP initialization as stated by ARM Information Center.
 * \param *libswdctx swd context pointer.
 * \param operation type (LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE).
 * \return Target's IDCODE, or LIBSWD_ERROR_CODE on failure.
 */ 
int libswd_dap_detect(libswd_ctx_t *libswdctx, libswd_operation_t operation, int **idcode){
 int res;
 res=libswd_dap_select(libswdctx, operation);
 if (res<1) return res;
 res=libswd_dap_reset(libswdctx, operation);
 if (res<1) return res;
 res=libswd_dp_read_idcode(libswdctx, operation, idcode);
 if (res<0) return res;
 return LIBSWD_OK;
}

/** Macro: Generic read of the DP register.
 * When operation is LIBSWD_OPERATION_EXECUTE it also caches register values.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the address of the DP register to read.
 * \param **data is the pointer to data where result will be stored.
 * \return number of elements processed or LIBSWD_ERROR_CODE on failure.
 */
int libswd_dp_read(libswd_ctx_t *libswdctx, libswd_operation_t operation, char addr, int **data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_dp_read(libswdctx=@%p, operation=%s, addr=0x%X, **data=%p) entering function...\n", (void*)libswdctx, libswd_operation_string(operation), addr, (void**)&data);
 
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT; 
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res, cmdcnt=0;
 char APnDP, RnW, cparity, *ack, *parity, request;

 APnDP=0;
 RnW=1;

 res=libswd_bitgen8_request(libswdctx, &APnDP, &RnW, &addr, &request);
 if (res<0) return res;
 res=libswd_bus_write_request_raw(libswdctx, LIBSWD_OPERATION_ENQUEUE, &request);
 if (res<1) return res;
 cmdcnt=+res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  res=libswd_bus_read_ack(libswdctx, operation, (char**)&libswdctx->qlog.read.ack);
  if (res<1) return res;
  cmdcnt=+res;
  res=libswd_bus_read_data_p(libswdctx, operation, (int**)&data, (char**)&libswdctx->qlog.read.parity);
  if (res<1) return res;
  cmdcnt=+res;
  return cmdcnt;

 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_bus_read_ack(libswdctx, operation, &ack);
  if (res>=0) {
   res=libswd_bus_read_data_p(libswdctx, operation, data, &parity);
   if (res<0) return res;
   res=libswd_bin32_parity_even(*data, &cparity); 
   if (res<0) return res;
   if (cparity!=*parity) return LIBSWD_ERROR_PARITY;
   cmdcnt=+res;
  } else if (res==LIBSWD_ERROR_ACK_WAIT) {
   //We got ACK==WAIT, retry last transfer until success or failure.
   int retry, ctrlstat, abort;
   for (retry=LIBSWD_RETRY_COUNT_DEFAULT; retry>0; retry--){
    abort=0xFFFFFFFE;
    res=libswd_dap_errors_handle(libswdctx, LIBSWD_OPERATION_EXECUTE, &ctrlstat, &abort); 
    if (res<0) continue;
    res=libswd_bus_write_request_raw(libswdctx, LIBSWD_OPERATION_ENQUEUE, &request);
    if (res<0) continue; 
    res=libswd_bus_read_ack(libswdctx, LIBSWD_OPERATION_EXECUTE, &ack);
    if (res<0) continue;
    res=libswd_bus_read_data_p(libswdctx, LIBSWD_OPERATION_EXECUTE, data, &parity);
    if (res<0) continue;
    res=libswd_dp_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_DP_RDBUFF_ADDR, data);
    if (res<0) continue;
    break;
   }
   if (retry==0) return LIBSWD_ERROR_MAXRETRY;
  }
  if (res<0) {
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR, "LIBSWD_E: libswd_dp_read(libswdctx=@%p, operation=%s, addr=0x%X, **data=0x%X/%s) failed: %s.\n", (void*)libswdctx, libswd_operation_string(operation), addr, **data, libswd_bin32_string(*data), libswd_error_string(res));
   return res;
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_dp_read(libswdctx=@%p, operation=%s, addr=0x%X, **data=0x%X/%s) execution OK.\n", (void*)libswdctx, libswd_operation_string(operation), addr, **data, libswd_bin32_string(*data));
  // Here we also can cache DP register values into libswdctx log.
  switch(addr){
   case LIBSWD_DP_IDCODE_ADDR: libswdctx->log.dp.idcode=**data; break;
   case LIBSWD_DP_RDBUFF_ADDR: libswdctx->log.dp.rdbuff=**data; break;
   case LIBSWD_DP_RESEND_ADDR: libswdctx->log.dp.resend=**data; break;
   case LIBSWD_DP_CTRLSTAT_ADDR: // which is also LIBSWD_DP_WCR_ADDR
    if (libswdctx->log.dp.select&1){
     libswdctx->log.dp.wcr=**data;
    } else libswdctx->log.dp.ctrlstat=**data;
    break;
  }
  return cmdcnt;
 } else return LIBSWD_ERROR_BADOPCODE;
} 

/** Macro function: Generic write of the DP register.
 * When operation is LIBSWD_OPERATION_EXECUTE it also caches register values.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the address of the DP register to write.
 * \param *data is the pointer to data to be written.
 * \return number of elements processed or LIBSWD_ERROR code on failure.
 */
int libswd_dp_write(libswd_ctx_t *libswdctx, libswd_operation_t operation, char addr, int *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_dp_write(*libswdctx=%p, operation=%s, addr=0x%X, *data=%p) entering function...\n", (void*)libswdctx, libswd_operation_string(operation), addr, (void*)data);

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT; 
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res, cmdcnt=0;
 char APnDP, RnW, cparity, *ack, *parity, request;

 APnDP=0;
 RnW=0;

 res=libswd_bitgen8_request(libswdctx, &APnDP, &RnW, &addr, &request);
 if (res<0) return res; 
 res=libswd_bus_write_request_raw(libswdctx, LIBSWD_OPERATION_ENQUEUE, &request);
 if (res<1) return res;
 cmdcnt=+res;

 libswd_bin32_parity_even(data, (char *)&libswdctx->qlog.write.parity);

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  res=libswd_bus_read_ack(libswdctx, operation, (char**)&libswdctx->qlog.write.ack);
  if (res<1) return res;
  cmdcnt=+res;
  res=libswd_bus_write_data_ap(libswdctx, operation, data);
  if (res<1) return res;
  cmdcnt=+res;
  return cmdcnt;

 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_bus_read_ack(libswdctx, operation, &ack);
  if (res>=0) {
   res=libswd_bus_write_data_ap(libswdctx, operation, data);
  } else if (res==LIBSWD_ERROR_ACK_WAIT) {
   //We got ACK==WAIT, retry last transfer until success or failure.
   int retry, ctrlstat, abort;
   for (retry=LIBSWD_RETRY_COUNT_DEFAULT; retry>0; retry--){
    abort=0xFFFFFFFF;
    res=libswd_dap_errors_handle(libswdctx, LIBSWD_OPERATION_EXECUTE, &ctrlstat, &abort); 
    if (res<0) continue;
    res=libswd_bus_write_request_raw(libswdctx, LIBSWD_OPERATION_ENQUEUE, &request);
    if (res<0) continue; 
    res=libswd_bus_read_ack(libswdctx, LIBSWD_OPERATION_EXECUTE, &ack);
    if (res<0) continue;
    res=libswd_bus_write_data_ap(libswdctx, LIBSWD_OPERATION_EXECUTE, data);
    if (res<0) continue;
    break;
   }
   if (retry==0) return LIBSWD_ERROR_MAXRETRY;
  }
  if (res<0) {
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR, "LIBSWD_E: libswd_dp_write(libswdctx=@%p, operation=%s, addr=0x%X, *data=0x%X/%s) failed: %s.\n", (void*)libswdctx, libswd_operation_string(operation), addr, *data, libswd_bin32_string(data), libswd_error_string(res));
   return res;
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_dp_write(libswdctx=@%p, operation=%s, addr=0x%X, *data=0x%X/%s) execution OK.\n", (void*)libswdctx, libswd_operation_string(operation), addr, *data, libswd_bin32_string(data));
  // Here we also can cache DP register values into libswdctx log.
  switch(addr){
   case LIBSWD_DP_ABORT_ADDR: libswdctx->log.dp.abort=*data; break;
   case LIBSWD_DP_SELECT_ADDR: libswdctx->log.dp.select=*data; break;
   case LIBSWD_DP_ROUTESEL_ADDR: libswdctx->log.dp.routesel=*data; break;
   case LIBSWD_DP_CTRLSTAT_ADDR: // which is also LIBSWD_DP_WCR_ADDR
    if (libswdctx->log.dp.select&1){
     libswdctx->log.dp.wcr=*data;
    } else libswdctx->log.dp.ctrlstat=*data;
    break;
  }
  return cmdcnt;
 } else return LIBSWD_ERROR_BADOPCODE;
} 

/** Macro function: Selects APBANK based on provided address value.
 * Bank number is calculated from [7..4] bits of the address.
 * It is also possible to provide direct value of APBANKSEL register on bits [7..4].
 * Remember not to enqueue any other SELECT writes after this function and before queue flush.
 * \param *libswdctx swd context to work on.
 * \param addr is the target AP register address, bank will be calculated based on this value.
 * \return number of cmdq operations on success, or LIBSWD_ERROR code on failure.
 */
int libswd_ap_bank_select(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_ap_bank_select(*libswdctx=%p, operation=%s, addr=0x%02X) entering function...\n", (void*)libswdctx, libswd_operation_string(operation), addr);
 // If the correct AP bank is already selected no need to change it.
 // Verify against cached DP SELECT register value.
 // Unfortunately SELECT register is read only so we need to work on cached values...
 if ( (libswdctx->log.dp.select&LIBSWD_DP_SELECT_APBANKSEL)==(addr&LIBSWD_DP_SELECT_APBANKSEL) ) return LIBSWD_OK;
 // If the cached value of APBANKSEL is different from addr, set it up.
 int retval;
 int new_select=libswdctx->log.dp.select;
 new_select&= ~LIBSWD_DP_SELECT_APBANKSEL;
 new_select|= addr&0x00F0; 
 retval=libswd_dp_write(libswdctx, operation, LIBSWD_DP_SELECT_ADDR, &new_select);
 if (retval<0){
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING, "libswd_ap_bank_select(%p, %0x02X): cannot update DP SELECT register (%s)\n", (void*)libswdctx, addr, libswd_error_string(retval));
 } else libswdctx->log.dp.select=new_select;
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_ap_bank_select(*libswdctx=%p, operation=%s, addr=0x%02X) execution OK.\n", (void*)libswdctx, libswd_operation_string(operation), addr);
 return retval;
}


/** Macro function: Select Access Port (updates APSEL field in DP SELECT register).
 * Remember not to enqueue any other SELECT writes after this function and before queue flush.
 * \param *libswdctx swd context to work on.
 * \param addr is the target AP register address, bank will be calculated based on this value.
 * \return number of cmdq operations on success, or LIBSWD_ERROR code on failure.
 */
int libswd_ap_select(libswd_ctx_t *libswdctx, libswd_operation_t operation, int ap){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_ap_select(*libswdctx=%p, operation=%s, ap=0x%02X) entering function...\n", (void*)libswdctx, libswd_operation_string(operation), ap);

 // If the correct AP is already selected no need to change it.
 // Verify against cached DP SELECT register value.
 // Unfortunately SELECT register is write only so we need to work on cached values...
 int retval;
 int new_select=libswdctx->log.dp.select;
 new_select&= ~LIBSWD_DP_SELECT_APSEL;
 new_select|= ap<<LIBSWD_DP_SELECT_APSEL_BITNUM;
 retval=libswd_dp_write(libswdctx, operation, LIBSWD_DP_SELECT_ADDR, &new_select);
 if (retval<0){
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING, "LIBSWD_W: libswd_ap_select(%p, %0x02X): cannot update DP SELECT register with 0x%08X (%s).\n", (void*)libswdctx, ap, new_select, libswd_error_string(retval));
 } else libswdctx->log.dp.select=new_select;
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_ap_select(*libswdctx=%p, operation=%s, ap=0x%02X) execution OK.\n", (void*)libswdctx, libswd_operation_string(operation), ap);
 return retval;
}


/** Macro function: Generic read of the AP register.
 * Address field should contain AP BANK on bits [4..7].
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the address of the AP register to read plus AP BANK on bits [4..7].
 * \param **data is the pointer to data where result will be stored.
 * \return number of elements processed or LIBSWD_ERROR code on failure.
 */
int libswd_ap_read(libswd_ctx_t *libswdctx, libswd_operation_t operation, char addr, int **data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_ap_read(*libswdctx=%p, command=%s, addr=0x%X, *data=%p) entering function...\n", (void*)libswdctx, libswd_operation_string(operation), (unsigned char)addr, (void**)data);

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT; 
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res, cmdcnt=0;
 char APnDP, RnW, cparity, *ack, *parity, request;

 res=libswd_ap_bank_select(libswdctx, operation, addr);
 if (res<0) return res;

 APnDP=1;
 RnW=1;

 res=libswd_bitgen8_request(libswdctx, &APnDP, &RnW, &addr, &request);
 if (res<0) return res;
 res=libswd_bus_write_request_raw(libswdctx, LIBSWD_OPERATION_ENQUEUE, &request);
 if (res<1) return res;
 cmdcnt=+res;

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  res=libswd_bus_read_ack(libswdctx, operation, (char**)&libswdctx->qlog.read.ack);
  if (res<1) return res;
  cmdcnt=+res;
  res=libswd_bus_read_data_p(libswdctx, operation, (int**)&libswdctx->qlog.read.data, (char**)&libswdctx->qlog.read.parity);
  if (res<1) return res;
  cmdcnt=+res;
  return cmdcnt;

 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_bus_read_ack(libswdctx, operation, &ack);
  if (res>=0) {
   res=libswd_bus_read_data_p(libswdctx, operation, data, &parity);
   if (res<0) return res;
   res=libswd_bin32_parity_even(*data, &cparity); 
   if (res<0) return res;
   if (cparity!=*parity) return LIBSWD_ERROR_PARITY;
  } else if (res==LIBSWD_ERROR_ACK_WAIT) {
   //We got ACK==WAIT, retry last transfer until success or failure.
   int retry, ctrlstat, abort;
   for (retry=LIBSWD_RETRY_COUNT_DEFAULT; retry>0; retry--){
    abort=0xFFFFFFFF;
    res=libswd_dap_errors_handle(libswdctx, LIBSWD_OPERATION_EXECUTE, &ctrlstat, &abort); 
    if (res<0) continue;
    res=libswd_bus_write_request_raw(libswdctx, LIBSWD_OPERATION_ENQUEUE, &request);
    if (res<0) continue; 
    res=libswd_bus_read_ack(libswdctx, LIBSWD_OPERATION_EXECUTE, &ack);
    if (res<0) continue;
    res=libswd_bus_read_data_p(libswdctx, LIBSWD_OPERATION_EXECUTE, data, &parity);
    if (res<0) continue;
    break;
   }
   if (retry==0) return LIBSWD_ERROR_MAXRETRY;
  }
  res=libswd_dp_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_DP_RDBUFF_ADDR, data);
  if (res<0) {
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR, "LIBSWD_E: libswd_ap_read(libswdctx=@%p, operation=%s, addr=0x%X, **data=0x%X/%s) failed: %s.\n", (void*)libswdctx, libswd_operation_string(operation), addr, **data, libswd_bin32_string(*data), libswd_error_string(res));
   return res;
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_ap_read(libswdctx=@%p, command=%s, addr=0x%X, *data=0x%X/%s) execution OK.\n", (void*)libswdctx, libswd_operation_string(operation), addr, **data, libswd_bin32_string(*data));
  return cmdcnt;
 } else return LIBSWD_ERROR_BADOPCODE;
} 

/** Macro function: Generic write of the AP register.
 * Address field should contain AP BANK on bits [4..7].
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the address of the AP register to write plus AP BANK on bits[4..7].
 * \param *data is the pointer to data to be written.
 * \return number of elements processed or LIBSWD_ERROR code on failure.
 */
int libswd_ap_write(libswd_ctx_t *libswdctx, libswd_operation_t operation, char addr, int *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_ap_write(libswdctx=@%p, operation=%s, addr=0x%X, *data=0x%X).\n", (void*)libswdctx, libswd_operation_string(operation), addr, (void**)data);
 
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT; 
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res, cmdcnt=0;
 char APnDP, RnW, cparity, *ack, *parity, request;

 res=libswd_ap_bank_select(libswdctx, operation, addr);
 if (res<0) return res;

 APnDP=1;
 RnW=0;

 res=libswd_bitgen8_request(libswdctx, &APnDP, &RnW, &addr, &request);
 if (res<0) return res;
 res=libswd_bus_write_request_raw(libswdctx, LIBSWD_OPERATION_ENQUEUE, &request);
 if (res<1) return res;
 cmdcnt=+res;
 libswd_bin32_parity_even(data, (char *)&libswdctx->qlog.write.parity);

 if (operation==LIBSWD_OPERATION_ENQUEUE){
  res=libswd_bus_read_ack(libswdctx, operation, (char**)&libswdctx->qlog.write.ack);
  if (res<1) return res;
  cmdcnt=+res;
  libswdctx->qlog.write.data=*data;
  res=libswd_bus_write_data_ap(libswdctx, operation, &libswdctx->qlog.write.data);
  if (res<1) return res;
  cmdcnt=+res;
  return cmdcnt;

 } else if (operation==LIBSWD_OPERATION_EXECUTE){
  res=libswd_bus_read_ack(libswdctx, operation, &ack);
  if (res>=0) {
   res=libswd_bus_write_data_ap(libswdctx, operation, data);
   if (res<0) return res;
   cmdcnt+=res;
  } else if (res==LIBSWD_ERROR_ACK_WAIT) {
   //We got ACK==WAIT, retry last transfer until success or failure.
   int retry, ctrlstat, abort;
   for (retry=LIBSWD_RETRY_COUNT_DEFAULT; retry>0; retry--){
    abort=0xFFFFFFFF;
    res=libswd_dap_errors_handle(libswdctx, LIBSWD_OPERATION_EXECUTE, &ctrlstat, &abort); 
    if (res<0) continue;
    res=libswd_bus_write_request_raw(libswdctx, LIBSWD_OPERATION_ENQUEUE, &request);
    if (res<0) continue; 
    res=libswd_bus_read_ack(libswdctx, LIBSWD_OPERATION_EXECUTE, &ack);
    if (res<0) continue;
    res=libswd_bus_write_data_ap(libswdctx, LIBSWD_OPERATION_EXECUTE, data);
    if (res<0) continue;
    break;
   }
   if (retry==0) return LIBSWD_ERROR_MAXRETRY;
  }
  if (res<0) {
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR, "LIBSWD_E: libswd_ap_write(libswdctx=@%p, operation=%s, addr=0x%X, *data=0x%X/%s) failed: %s.\n", (void*)libswdctx, libswd_operation_string(operation), addr, *data, libswd_bin32_string(data), libswd_error_string(res));
   return res;
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_ap_write(libswdctx=@%p, operation=%s, addr=0x%X, *data=0x%X/%s) execution OK.\n", (void*)libswdctx, libswd_operation_string(operation), addr, *data, libswd_bin32_string(data));
  return cmdcnt;
 } else return LIBSWD_ERROR_BADOPCODE;
} 


/** Initialize the DAP.
 * \param *libswdctx swd context to work on.
 * \return LIBSWD_OK on success or LIBSWD_ERROR code on failure.
 */
int libswd_dap_init(libswd_ctx_t *libswdctx){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_dap_init(*libswdctx=%p) entring function...\n",
            (void*)libswdctx );
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int i, res, dpctrlstat, *dpctrlstatp, dpabort;
 // Check if there are any errors to handle first.
 dpabort=0xFFFFFFFE;
 res=libswd_dap_errors_handle(libswdctx, LIBSWD_OPERATION_EXECUTE, &dpctrlstat, &dpabort);
 libswdctx->log.dp.ctrlstat=dpctrlstat;
 if (res<0) goto libswd_dap_init_error;
 // Select MEM-AP.
 res=libswd_ap_select(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_APSEL_VAL);
 if (res<0) goto libswd_dap_init_error;
 // Activate System and Debug Unit power domains if not already active.
 if ( !(dpctrlstat&LIBSWD_DP_CTRLSTAT_CDBGPWRUPACK && dpctrlstat|LIBSWD_DP_CTRLSTAT_CSYSPWRUPACK) )
 {
  dpctrlstat=libswdctx->log.dp.ctrlstat|LIBSWD_DP_CTRLSTAT_CDBGPWRUPREQ|LIBSWD_DP_CTRLSTAT_CSYSPWRUPREQ;
  res=libswd_dp_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_DP_CTRLSTAT_ADDR, &dpctrlstat);
  if (res<0) goto libswd_dap_init_error;
  // Wait for System and Debug Unit powerup.
  for (i=0;i<LIBSWD_RETRY_COUNT_DEFAULT;i++)
  {
   res=libswd_dp_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_DP_CTRLSTAT_ADDR, &dpctrlstatp); 
   if (res<0) goto libswd_dap_init_error;
   if (*dpctrlstatp&(LIBSWD_DP_CTRLSTAT_CDBGPWRUPACK|LIBSWD_DP_CTRLSTAT_CSYSPWRUPACK)) break;
   usleep(100);
  }
  if (!(*dpctrlstatp&LIBSWD_DP_CTRLSTAT_CDBGPWRUPACK))
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
              "LIBSWD_W: libswd_dap_init(): DP CTRL/STAT CDBGPWRUPACK not set!\n");
  if (!(*dpctrlstatp&LIBSWD_DP_CTRLSTAT_CSYSPWRUPACK))
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
              "LIBSWD_W: libswd_dap_init(): DP CTRL/STAT CSYSPWRUPACK not set!\n");
 } else libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
                   "LIBSWD_I: libswd_dap_init(): CDBGPWRUP and CSYSPWRUP already set!\n");
 // Enable Overrun detection and set valid TRNMODE.
 dpctrlstat=libswdctx->log.dp.ctrlstat;
 dpctrlstat|=LIBSWD_DP_CTRLSTAT_ORUNDETECT;
 dpctrlstat&=~LIBSWD_DP_CTRLSTAT_TRNMODE;
 res=libswd_dp_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_DP_CTRLSTAT_ADDR, &dpctrlstat);
 if (res<0) goto libswd_dap_init_error;
 res=libswd_dp_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_DP_CTRLSTAT_ADDR, &dpctrlstatp); 
 if (res<0) goto libswd_dap_init_error;
 libswdctx->log.dp.ctrlstat=*dpctrlstatp;
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
           "LIBSWD_I:   libswd_dap_init(): DP CTRL/STAT=0x%08X\n", libswdctx->log.dp.ctrlstat);
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_dap_init(*libswdctx=%p) execution OK.\n",
            (void*)libswdctx );
 return LIBSWD_OK;
libswd_dap_init_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "LIBSWD_E: libswd_dap_init(): Cannot setup SW-DAP (%s)!\n",
             libswd_error_string(res) );
 return res;
}


/** Setup the MEM-AP.
 * \param *libswdctx swd context to work on.
 * \return LIBSWD_OK on success or LIBSWD_ERROR code on failure.
 */
int libswd_memap_init(libswd_ctx_t *libswdctx){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_memap_init(*libswdctx=%p) entering function...\n",
            (void*)libswdctx );
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int res, *memapidr, *memapbase, *memapcsw; 
 // Setup Debug Access Port first.
 res=libswd_dap_init(libswdctx);
 if (res<0) goto libswd_memap_init_error;
 // Check IDentification Register, use cached value if possible.
 if (!libswdctx->log.memap.idr)
 {
  res=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_IDR_ADDR, &memapidr);
  if (res<0) goto libswd_memap_init_error;
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
             "LIBSWD_I: libswd_memap_init(): MEM-AP   IDR=0x%08X\n",
             *memapidr );
  libswdctx->log.memap.idr=*memapidr;
 }
 // Check Debug BASE Address Register, use chached value if possible.
 if (!libswdctx->log.memap.base)
 {
  res=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_BASE_ADDR, &memapbase);
  if (res<0) goto libswd_memap_init_error;
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
             "LIBSWD_I: libswd_memap_init(): MEM-AP  BASE=0x%08X\n",
             *memapbase );
  libswdctx->log.memap.base=*memapbase;
 }
 // Check CSW Register, use chaced value if possible. Verify if DeviceEN=1 (must have).
 // TODO: Verify DbgSwEnable, if 0 no debug is implemented, should be set to 1?
 // Read the CSW register if we don't have it yet.
 if (!libswdctx->log.memap.csw)
 {
  res=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_CSW_ADDR, &memapcsw); 
  if (res<0) goto libswd_memap_init_error;
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
             "LIBSWD_I: libswd_memap_init(): MEM-AP   CSW=0x%08X\n",
             *memapcsw);
  libswdctx->log.memap.csw=*memapcsw; 
 }
 // Check if DeviceEN bit in CSW is set. 
 if (!libswdctx->log.memap.csw&LIBSWD_MEMAP_CSW_DEVICEEN)
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
             "LIBSWD_W: libswd_memap_init(): MEM-AP:CSW[DeviceEN] bit is not set! Using MEM-AP may fail!\n",
             *memapcsw);
 // Check if DbgSwEnable bit is set, set if necessary.
 if (!libswdctx->log.memap.csw&LIBSWD_MEMAP_CSW_DBGSWENABLE)
 {
  *memapcsw=libswdctx->log.memap.csw|LIBSWD_MEMAP_CSW_DBGSWENABLE;
  res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_CSW_ADDR, memapcsw);
  if (res<0) goto libswd_memap_init_error;
  libswdctx->log.memap.csw=*memapcsw;
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
             "LIBSWD_I: libswd_memap_init(): Read MEM-AP CSW=0x%08X\n",
             *memapcsw);
 }
 // Mark MEM-AP as configured.
 libswdctx->log.memap.initialized=1;
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_memap_init(*libswdctx=%p) execution OK.\n",
            (void*)libswdctx );
 return LIBSWD_OK;
libswd_memap_init_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "LIBSWD_E: libswd_memap_init(): Cannot setup MEM-AP (%s)!\n",
            libswd_error_string(res) );
 return res;
}

/** Macro function: Generic read of the memory and perihperals using MEM-AP.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the address of the data to read with MEM-AP.
 * \param **data is the pointer to data where result will be stored.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_read(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, char **data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_memap_read(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p) entering function...\n", (void*)libswdctx, libswd_operation_string(operation), addr, count, (void**)data); 
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT; 
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int i, res, *memapcsw, *memaptar, *memapdrw;

 // Initialize the DAP (System and Debug powerup).
 if (!libswdctx->log.memap.initialized) res=libswd_memap_init(libswdctx);
 if (res<0) goto libswd_memap_read_error;

 // Pass address to TAR register.
 res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &addr);
 if (res<0) goto libswd_memap_read_error;
 libswdctx->log.memap.tar=addr;
 // Read data from DRW register.
 // DOES NOT RETURN VALID DATA! WHY?
 res=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &memapdrw);
 if (res<0) goto libswd_memap_read_error;
 res=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &memapdrw);
 if (res<0) goto libswd_memap_read_error;
printf("MEM-AP DRW: 0x%08X\n", *memapdrw);

 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_memap_read(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p) execution OK...\n", (void*)libswdctx, libswd_operation_string(operation), addr, count, (void**)data); 
 return LIBSWD_OK;

libswd_memap_read_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
            "LIBSWD_W: libswd_memap_read(): %s\n",
            libswd_error_string(res) );
 return res;
}


/** @} */
