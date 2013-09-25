/*
 * Serial Wire Debug Open Library.
 * MEM-AP Routines Body File.
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

/** \file libswd_memap.c MEM-AP related routines. */

#include <libswd.h>

/*******************************************************************************
 * \defgroup libswd_memap High-level MEM-AP (Memory Access Port) operations.
 * These routines are based on DAP operations.
 * Return values: negative number on error, data on success.
 ******************************************************************************/

/** Initialize the MEM-AP.
 * This function will set DbgSwEnable, DeviceEn, 32-bit Size in CSW. 
 * This function will disable Tar Auto Increment.
 * Use libswd_memap_setup() to set specific CSW and TAR values.
 * \param *libswdctx swd context to work on.
 * \return LIBSWD_OK on success or LIBSWD_ERROR code on failure.
 */
int libswd_memap_init(libswd_ctx_t *libswdctx, libswd_operation_t operation){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_memap_init(*libswdctx=%p, operation=%s) entering function...\n",
            (void*)libswdctx, libswd_operation_string(operation) );
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int res=0, *memapidr, *memapbase, *memapcswp, memapcsw, dpabort, dpctrlstat, *dpctrlstatp; 
 if (!libswdctx->log.dp.initialized)
 {
  int *idcode;
  res=libswd_dap_init(libswdctx, operation, &idcode);
  if (res<0) goto libswd_memap_init_error;
 }
 // Select MEM-AP.
 res=libswd_ap_select(libswdctx, operation, LIBSWD_MEMAP_APSEL_VAL);  
 if (res<0) goto libswd_memap_init_error; 
 // Check IDentification Register, use cached value if possible.
 if (!libswdctx->log.memap.idr)
 {
  res=libswd_ap_read(libswdctx, operation, LIBSWD_MEMAP_IDR_ADDR, &memapidr);
  if (res<0) goto libswd_memap_init_error;
  libswdctx->log.memap.idr=*memapidr;
 }
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
            "LIBSWD_I: libswd_memap_init(): MEM-AP  IDR=0x%08X\n",
             libswdctx->log.memap.idr );
 // Check Debug BASE Address Register, use cached value if possible.
 if (!libswdctx->log.memap.base)
 {
  res=libswd_ap_read(libswdctx, operation, LIBSWD_MEMAP_BASE_ADDR, &memapbase);
  if (res<0) goto libswd_memap_init_error;
  libswdctx->log.memap.base=*memapbase;
 }
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
            "LIBSWD_I: libswd_memap_init(): MEM-AP BASE=0x%08X\n",
             libswdctx->log.memap.base );
 // Setup the CSW (MEM-AP Control and Status) register.
 memapcsw=0;
 // Check if DbgSwEnable bit is set, set if necessary.
 memapcsw|=LIBSWD_MEMAP_CSW_DBGSWENABLE;
 memapcsw&=~LIBSWD_MEMAP_CSW_ADDRINC;
 memapcsw&=~LIBSWD_MEMAP_CSW_SIZE;
 memapcsw|=LIBSWD_MEMAP_CSW_SIZE_32BIT;
 // Write new CSW value.
 res=libswd_ap_write(libswdctx, operation, LIBSWD_MEMAP_CSW_ADDR, &memapcsw);
 if (res<0) goto libswd_memap_init_error;
 res=libswd_ap_read(libswdctx, operation, LIBSWD_MEMAP_CSW_ADDR, &memapcswp);
 if (res<0) goto libswd_memap_init_error;
 libswdctx->log.memap.csw=(*memapcswp);
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
            "LIBSWD_I: libswd_memap_init(): MEM-AP  CSW=0x%08X\n",
            libswdctx->log.memap.csw);
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

/** Setup the MEM-AP.
 * This setup needs to be done before every MEM-AP access series,
 * because different operations may use different access size.
 * Function will try to compare agains chahed values.
 * \param *libswd LibSWD context to work on.
 * \param operation is the LIBSWD_OPERATION type.
 * \param csw is the CSW register value to be set.
 * \param tar is the TAR register value to be set.
 * \return LIBSWD_OK on success, LIBSWD_ERROR otherwise.
 */
int libswd_memap_setup(libswd_ctx_t *libswdctx, libswd_operation_t operation, int csw, int tar){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_memap_setup(*libswdctx=%p, operation=%s, csw=0x%08X, tar=0x%08X) entering function...\n",
            (void*)libswdctx, libswd_operation_string(operation), csw, tar );
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 int res, *memapcswp, memapcsw, *memaptarp; 
 memapcsw=csw|LIBSWD_MEMAP_CSW_DBGSWENABLE;
 memapcsw|=LIBSWD_MEMAP_CSW_PROT; //WTF IS PROT WHERE IS DOCUMENTED AND WHY THIS ENABLES DEBUG?! 
 if (memapcsw!=libswdctx->log.memap.csw)
 {
  // Write register value.
  res=libswd_ap_write(libswdctx, operation, LIBSWD_MEMAP_CSW_ADDR, &memapcsw);
  if (res<0) goto libswd_memap_setup_error;
  // Read-back and store register value.
  res=libswd_ap_read(libswdctx, operation, LIBSWD_MEMAP_CSW_ADDR, &memapcswp);
  if (res<0) goto libswd_memap_setup_error;
  libswdctx->log.memap.csw=(*memapcswp);
 }
 if (tar!=libswdctx->log.memap.tar)
 {
  // Write register value.
  res=libswd_ap_write(libswdctx, operation, LIBSWD_MEMAP_TAR_ADDR, &tar);
  if (res<0) goto libswd_memap_setup_error;
  // Read-back and cache the register value.
  res=libswd_ap_read(libswdctx, operation, LIBSWD_MEMAP_TAR_ADDR, &memaptarp);
  if (res<0) goto libswd_memap_setup_error;
  libswdctx->log.memap.tar=(*memaptarp);
 }
 return LIBSWD_OK;
libswd_memap_setup_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "LIBSWD_E: libswd_memap_setup(): Cannot setup MEM-AP (%s)!\n",
            libswd_error_string(res) );
 return res;
}


/** Macro function: Generic read of the memory and perihperals using MEM-AP.
 * Data are stored into char array.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to read with MEM-AP.
 * \param *data is the pointer to char array where result will be stored.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_read_char(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, char *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_memap_read_char(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, *data=%p) entering function...\n", (void*)libswdctx, libswd_operation_string(operation), addr, count, (void**)data); 
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT; 
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int i, loc, res=0, *memapcsw, *memaptar, *memapdrw;

 // Initialize MEM-AP if necessary.
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_read_char_error;
 }
 // Setup MEM-AP for 32-bit access.
 res=libswd_memap_setup(libswdctx, operation, LIBSWD_MEMAP_CSW_SIZE_32BIT, 0);
 if (res<0) goto libswd_memap_read_char_error;
 // Verify the count parameter to be 32-bit boundary.
 if (count%4) count=count-(count%4);

 // Perform word-by-word read operation.
 for (i=0;i<count/4;i++)
 {
  loc=addr+i*4;
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "LIBSWD_I: libswd_memap_read_char() reading address 0x%08X\r", addr+i*4);
  fflush();
  // Pass address to TAR register.
  res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &loc);
  if (res<0) goto libswd_memap_read_char_error;
  libswdctx->log.memap.tar=loc;
  // Read data from DRW register.
  res=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &memapdrw);
  if (res<0) goto libswd_memap_read_char_error;
  libswdctx->log.memap.drw=*memapdrw;
  data[4*i+0]=(unsigned char)(*memapdrw);
  data[4*i+1]=(unsigned char)(*memapdrw>>8);
  data[4*i+2]=(unsigned char)(*memapdrw>>16);
  data[4*i+3]=(unsigned char)(*memapdrw>>24);
 }
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "\n");

 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_memap_read_char(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, *data=%p) execution OK...\n", (void*)libswdctx, libswd_operation_string(operation), addr, count, (void**)data); 
 return LIBSWD_OK;

libswd_memap_read_char_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
            "\nLIBSWD_W: libswd_memap_read_char(): %s\n",
            libswd_error_string(res) );
 return res;
}


/** Macro function: Generic read of the memory and perihperals using MEM-AP.
 * Data are stored into int array.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to read with MEM-AP.
 * \param *data is the pointer to int array where result will be stored.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_read_int(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, int *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_memap_read_int(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, *data=%p) entering function...\n", (void*)libswdctx, libswd_operation_string(operation), addr, count, (void**)data); 
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT; 
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int i, loc, res, *memapcsw, *memaptar, *memapdrw;

 // Initialize the DAP (System and Debug powerup).
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_read_int_error;
 }
 // Setup MEM-AP for 32-bit access.
 res=libswd_memap_setup(libswdctx, operation, LIBSWD_MEMAP_CSW_SIZE_32BIT, 0);
 if (res<0) goto libswd_memap_read_int_error;

 for (i=0;i<count;i++)
 {
  loc=addr+i*4;
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "LIBSWD_I: libswd_memap_read_int() reading address 0x%08X\r", loc);
  fflush();
  // Pass address to TAR register.
  res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &loc);
  if (res<0) goto libswd_memap_read_int_error;
  libswdctx->log.memap.tar=loc;
  // Read data from DRW register.
  res=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &memapdrw);
  if (res<0) goto libswd_memap_read_int_error;
  libswdctx->log.memap.drw=*memapdrw;
  data[i]=*memapdrw;
 }
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "\n");

 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_memap_read_int(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p) execution OK...\n", (void*)libswdctx, libswd_operation_string(operation), addr, count, (void**)data); 
 return LIBSWD_OK;

libswd_memap_read_int_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
            "\nLIBSWD_W: libswd_memap_read_int(): %s\n",
            libswd_error_string(res) );
 return res;
}



/** Macro function: Generic write to the memory and perihperals using MEM-AP.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to write with MEM-AP.
 * \param *data is the pointer to data to be written.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_write_char(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, char *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_memap_write_char(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p) entering function...\n", (void*)libswdctx, libswd_operation_string(operation), addr, count, (void**)data); 
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT; 
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int i, loc, res=0, *memapcsw, *memaptar, *memapdrw;

 // Verify the count parameter to be 32-bit boundary.
 if (count%4) count=count-(count%4);

 // Initialize the DAP (System and Debug powerup).
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_write_char_error;
 }
 // Setup MEM-AP for 32-bit access.
 res=libswd_memap_setup(libswdctx, operation, LIBSWD_MEMAP_CSW_SIZE_32BIT, 0);
 if (res<0) goto libswd_memap_write_char_error;

 for (i=0;i<count/4;i++)
 {
  loc=addr+i*4;
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "LIBSWD_I: libswd_memap_write_char() writing address 0x%08X\r", addr+i*4);
  fflush();
  // Pass address to TAR register.
  res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &loc);
  if (res<0) goto libswd_memap_write_char_error;
  libswdctx->log.memap.tar=loc;
  // Implode and Write data to DRW register.
  memcpy((void*)&libswdctx->log.memap.drw, data+(i*4), 4);
  res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &libswdctx->log.memap.drw);
  if (res<0) goto libswd_memap_write_char_error;
 }

 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_memap_write_char(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p) execution OK...\n", (void*)libswdctx, libswd_operation_string(operation), addr, count, (void**)data); 
 return LIBSWD_OK;

libswd_memap_write_char_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
            "LIBSWD_W: libswd_memap_write_char(): %s\n",
            libswd_error_string(res) );
 return res;
}


/** Macro function: Generic write to the memory and perihperals using MEM-AP.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to write with MEM-AP.
 * \param *data is the pointer to int data array to be written.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_write_int(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, int *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_memap_write_int(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p) entering function...\n", (void*)libswdctx, libswd_operation_string(operation), addr, count, (void**)data); 
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT; 
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int i, loc, res=0, *memapcsw, *memaptar, *memapdrw;

 // Initialize the DAP (System and Debug powerup).
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_write_int_error;
 }
 // Setup MEM-AP for 32-bit access.
 res=libswd_memap_setup(libswdctx, operation, LIBSWD_MEMAP_CSW_SIZE_32BIT, 0);
 if (res<0) goto libswd_memap_write_int_error;

 for (i=0;i<count;i++)
 {
  loc=addr+i*4;
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "LIBSWD_I: libswd_memap_write_int() writing address 0x%08X\r", loc);
  fflush();
  // Pass address to TAR register.
  res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &loc);
  if (res<0) goto libswd_memap_write_int_error;
  libswdctx->log.memap.tar=loc;
  // Implode and Write data to DRW register.
  res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &data[i]);
  if (res<0) goto libswd_memap_write_int_error;
  libswdctx->log.memap.drw=data[i];
 }
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "\n");

 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_memap_write_int(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p) execution OK...\n", (void*)libswdctx, libswd_operation_string(operation), addr, count, (void**)data); 
 return LIBSWD_OK;

libswd_memap_write_int_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
            "\nLIBSWD_W: libswd_memap_write_int(): %s\n",
            libswd_error_string(res) );
 return res;
}


/** @} */
