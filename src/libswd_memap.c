/*
 * Serial Wire Debug Open Library.
 * MEM-AP Routines Body File.
 *
 * Copyright (C) 2010-2014, Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)
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
 * Written by Tomasz Boleslaw CEDRO <cederom@tlen.pl>, 2010-2014;
 *
 */

/** \file libswd_memap.c MEM-AP related routines. */

#include <libswd.h>

/*******************************************************************************
 * \defgroup libswd_memap High-level MEM-AP (Memory Access Port) operations.
 * These routines are based on DAP operations.
 * Return values: negative number on error, LIBSWD_OK on success.
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
            "LIBSWD_D: Executing libswd_memap_init(*libswdctx=%p, operation=%s)...\n",
            (void*)libswdctx, libswd_operation_string(operation) );

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;

 int res=0, *memapidr, *memapbase, *memapcswp, memapcsw;

 // Verify if DAP is already initialized, do so in necessary.
 if (!libswdctx->log.dp.initialized)
 {
  int *idcode;
  res=libswd_dap_init(libswdctx, operation, &idcode);
  if (res<0) goto libswd_memap_init_error;
 }

 // Select MEM-AP.
 //res=libswd_ap_select(libswdctx, operation, LIBSWD_MEMAP_APSEL_VAL);
 //if (res<0) goto libswd_memap_init_error;
 // TODO: DO WE NEED LIBSWD_AP_SELECT ???

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
 // Read back and cache CSW value.
 res=libswd_ap_read(libswdctx, operation, LIBSWD_MEMAP_CSW_ADDR, &memapcswp);
 if (res<0) goto libswd_memap_init_error;
 libswdctx->log.memap.csw=(*memapcswp);
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
            "LIBSWD_I: libswd_memap_init(): MEM-AP  CSW=0x%08X\n",
            libswdctx->log.memap.csw);

 // Mark MEM-AP as configured.
 libswdctx->log.memap.initialized=1;

 return LIBSWD_OK;

libswd_memap_init_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "LIBSWD_E: libswd_memap_init(): Cannot initialize MEM-AP (%s)!\n",
            libswd_error_string(res) );
 return res;
}

/** Setup the MEM-AP.
 * Use this function to setup CSW and TAR values for given MEM-AP operations.
 * This setup needs to be done before MEM-AP with different access size.
 * Function will try to compare agains chahed values to save bus traffic.
 * This function will set DBGSWENABLE and PROT bits in CSW by default.
 * \param *libswd LibSWD context to work on.
 * \param operation is the LIBSWD_OPERATION type.
 * \param csw is the CSW register value to be set.
 * \param tar is the TAR register value to be set.
 * \return LIBSWD_OK on success, LIBSWD_ERROR otherwise.
 */
int libswd_memap_setup(libswd_ctx_t *libswdctx, libswd_operation_t operation, int csw, int tar){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_setup(*libswdctx=%p, operation=%s, csw=0x%08X, tar=0x%08X)...\n",
            (void*)libswdctx, libswd_operation_string(operation), csw, tar );

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;

 int res, *memapcswp, memapcsw, *memaptarp;

 // Verify if MEM-AP is already initialized, do so in necessary.
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_setup_error;
 }

 // Remember to set these bits not to lock-out the Debug...
 memapcsw=csw|LIBSWD_MEMAP_CSW_DBGSWENABLE;
 memapcsw|=LIBSWD_MEMAP_CSW_PROT; // PROT ENABLES DEBUG!!

 // Update MEM-AP CSW register if necessary.
 if (memapcsw!=libswdctx->log.memap.csw)
 {
  // Write register value.
  res=libswd_ap_write(libswdctx, operation, LIBSWD_MEMAP_CSW_ADDR, &memapcsw);
  if (res<0) goto libswd_memap_setup_error;
  // Read-back and cache CSW value.
  res=libswd_ap_read(libswdctx, operation, LIBSWD_MEMAP_CSW_ADDR, &memapcswp);
  if (res<0) goto libswd_memap_setup_error;
  libswdctx->log.memap.csw=(*memapcswp);
 }

 // Update MEM-AP TAR register if necessary.
 if (tar!=libswdctx->log.memap.tar)
 {
  // Write register value.
  res=libswd_ap_write(libswdctx, operation, LIBSWD_MEMAP_TAR_ADDR, &tar);
  if (res<0) goto libswd_memap_setup_error;
  // Read-back and cache TAR value.
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


/** Generic read using MEM-AP into char array.
 * Data are stored into char array. Count shows CHAR elements.
 * Remember to setup MEM-AP first for valid access!
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to read with MEM-AP.
 * \param count is the number of bytes to read.
 * \param *data is the pointer to char array where result will be stored.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_read_char(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, char *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_read_char(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, *data=%p)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void*)data );

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int i, loc, res=0, accsize=0, *memapdrw;
 float tdeltam;
 struct timeval tstart, tstop;

 // Initialize MEM-AP if necessary.
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_read_char_error;
 }

 // Verify the count parameter to be access boundary.
 switch (libswdctx->log.memap.csw&LIBSWD_MEMAP_CSW_SIZE)
 {
  case LIBSWD_MEMAP_CSW_SIZE_8BIT:
   accsize=1;
   break;
  case LIBSWD_MEMAP_CSW_SIZE_16BIT:
   accsize=2;
   break;
  case LIBSWD_MEMAP_CSW_SIZE_32BIT:
   accsize=4;
   break;
  default:
   res=LIBSWD_ERROR_MEMAPACCSIZE;
   goto libswd_memap_read_char_error;
 }
 if (count%accsize) count=count-(count%accsize);

 // Check for alignment issues.
 if ((addr%accsize)!=0)
 {
  res=LIBSWD_ERROR_MEMAPALIGN;
  goto libswd_memap_read_char_error;
 }

 // Mark start time for transfer speed measurement.
 gettimeofday(&tstart, NULL);

 // Perform word-by-word read operation and implode result into char array.
 if (!(libswdctx->log.memap.csw&LIBSWD_MEMAP_CSW_ADDRINC))
 {
  // Use manual TAR incrementation (slower).
  for (i=0;i<count;i+=accsize)
  {
   int tmp;
   int drw_shift;
   loc=addr+i;
   // Calculate the offset in DRW where the data should be
   // see Data byte-laning in the ARM debug interface v5 documentation
   // note: this only works for little endian systems.
   drw_shift=8*(loc%4);
   // Measure transfer speed.
   gettimeofday(&tstop, NULL);
   tdeltam=fabsf((tstop.tv_sec-tstart.tv_sec)*1000+(tstop.tv_usec-tstart.tv_usec)/1000);
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
              "LIBSWD_I: libswd_memap_read_char() reading address 0x%08X (speed %fKB/s)\r",
              loc, count/tdeltam );
   fflush(0);
   // Pass address to TAR register.
   res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &loc);
   if (res<0) goto libswd_memap_read_char_error;
   libswdctx->log.memap.tar=loc;
   // Read data from DRW register.
   res=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &memapdrw);
   if (res<0) goto libswd_memap_read_char_error;
   libswdctx->log.memap.drw=*memapdrw;
   // apply the data byte-laning shift
   tmp=*memapdrw >>= drw_shift;
   memcpy((void*)data+i, &tmp, accsize);
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "\n");
 }
 else
 {
  // Use TAR Auto Increment (faster).
  // TAR auto increment is only guaranteed to work on the bottom 10 bits
  // of the TAR register. Above that it is implementation defined.
  // We use 1024 byte chunks as it will work on every platform
  // and one TAR write every 1024 bytes is not adding too much overhead.
  const unsigned int BOUNDARY = 1024;
  unsigned int i;

  // Check if packed transfer, if so use word access.
  if (libswdctx->log.memap.csw&LIBSWD_MEMAP_CSW_ADDRINC_PACKED) accsize=4;

  for (loc = addr, i = 0; loc < (addr + count); loc += accsize, i+= accsize)
  {
   int tmp;
   int drw_shift;

   // Calculate the offset in DRW where the data should be
   // see Data byte-laning in the ARM debug interface v5 documentation
   // note: this only works for little endian systems.
   drw_shift=8*(loc%4);

   // only write the TAR register, if this is the first time through the loop.
   // or if we've passed over the boundary where TAR auto inc isn't guaranteed
   // to work anymore.
   if (loc == addr ||
       (loc % BOUNDARY) == 0)
   {
    // Pass address to TAR register.
    res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &loc);
    if (res<0) goto libswd_memap_read_char_error;
    libswdctx->log.memap.tar=loc;
   }

   // Measure transfer speed.
   gettimeofday(&tstop, NULL);
   tdeltam=fabsf((tstop.tv_sec-tstart.tv_sec)*1000+(tstop.tv_usec-tstart.tv_usec)/1000);
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
              "LIBSWD_I: libswd_memap_read_char() reading address 0x%08X (speed %fKB/s)\r",
              loc, count/tdeltam);
   fflush(0);

   // Read data from the DRW register.
   res=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &memapdrw);
   if (res<0) goto libswd_memap_read_char_error;
   libswdctx->log.memap.drw=*memapdrw;

   // apply the data byte-laning shift
   tmp=*memapdrw >>= drw_shift;
   memcpy((void*)data + i, &tmp, accsize);
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "\n");
 }

 return LIBSWD_OK;

libswd_memap_read_char_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "\nLIBSWD_E: libswd_memap_read_char(): %s\n",
            libswd_error_string(res) );
 return res;
}


/** Generic read using MEM-AP to char array, with prior CSW setup.
 * Data are stored into char array.
 * Remember to setup CSW first for valid bus access!
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to read with MEM-AP.
 * \param count is the number of bytes to read.
 * \param *data is the pointer to char array where result will be stored.
 * \param csw is the value of csw register to write prior data write.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_read_char_csw(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, char *data, int csw){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_read_char_csw(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, *data=%p, csw=0x%X)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void**)data, csw);

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res=0, accsize=0;

 // Calculate required access size based on CSW value.
 switch (csw&LIBSWD_MEMAP_CSW_SIZE)
 {
  case LIBSWD_MEMAP_CSW_SIZE_8BIT:
   accsize=1;
   break;
  case LIBSWD_MEMAP_CSW_SIZE_16BIT:
   accsize=2;
   break;
  case LIBSWD_MEMAP_CSW_SIZE_32BIT:
   accsize=4;
   break;
  default:
   res=LIBSWD_ERROR_MEMAPACCSIZE;
   goto libswd_memap_read_char_csw_error;
 }
 // Verify the count parameter to be access boundary.
 if (count%accsize) count=count-(count%accsize);

 // Initialize MEM-AP if necessary.
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_read_char_csw_error;
 }

 // Setup MEM-AP CSW and TAR.
 res=libswd_memap_setup(libswdctx, operation, csw, addr);
 if (res<0) goto libswd_memap_read_char_csw_error;

 // Perform the read operation.
 res=libswd_memap_read_char(libswdctx, operation, addr, count, data);
 if (res<0) goto libswd_memap_read_char_csw_error;

 return LIBSWD_OK;

libswd_memap_read_char_csw_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "\nLIBSWD_E: libswd_memap_read_char_csw(): %s\n",
            libswd_error_string(res) );
 return res;
}


/** Generic read using MEM-AP to char array, using 32-bit data access.
 * Data are stored into char array.
 * Remember to setup CSW first for valid bus width!
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to read with MEM-AP.
 * \param count is the number of bytes to read.
 * \param *data is the pointer to char array where result will be stored.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_read_char_32(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, char *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_read_char_32(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, *data=%p)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void**)data );

 return libswd_memap_read_char_csw(libswdctx, operation, addr, count, data, LIBSWD_MEMAP_CSW_SIZE_32BIT|LIBSWD_MEMAP_CSW_ADDRINC_SINGLE);
}

/** Generic read using MEM-AP into int array.
 * Data are stored into int array. Count shows INT elements.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to read with MEM-AP.
 * \param count is the number of words to read.
 * \param *data is the pointer to int array where result will be stored.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_read_int(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, int *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_read_int(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, *data=%p)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void**)data);

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int i, loc, res, *memapdrw;
 float tdeltam;
 struct timeval tstart, tstop;

 // Initialize MEM-AP if necessary.
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_read_int_error;
 }

 // Mark start time for transfer speed measurement.
 gettimeofday(&tstart, NULL);

 // Perform word-by-word read operation and store result into int array.
 if (!(libswdctx->log.memap.csw&LIBSWD_MEMAP_CSW_ADDRINC))
 {
  // Use manual TAR incrementation (slower).
  for (i=0;i<count;i++)
  {
   loc=addr+i*4;
   // Measure transfer speed.
   gettimeofday(&tstop, NULL);
   tdeltam=fabsf((tstop.tv_sec-tstart.tv_sec)*1000+(tstop.tv_usec-tstart.tv_usec)/1000);
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
              "LIBSWD_I: libswd_memap_read_int() reading address 0x%08X (speed %fKB/s)\r",
              loc, count*4/tdeltam );
   fflush(0);
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
 }
 else
 {
  // Use TAR Auto Increment (faster).
  // TAR auto increment is only guaranteed to work on the bottom 10 bits
  // of the TAR register. Above that it is implementation defined.
  // We use 1024 byte chunks as it will work on every platform
  // and one TAR write every 1024 bytes is not adding too much overhead.
  const unsigned int BOUNDARY = 1024;
  unsigned int i;

  for (loc = addr, i = 0; loc < (addr + (count * 4)); loc += 4, i++)
  {
   // only write the TAR register, if this is the first time through the loop.
   // or if we've passed over the boundary where TAR auto inc isn't guaranteed
   // to work anymore.
   if (loc == addr ||
       (loc % BOUNDARY) == 0)
   {
    // Pass address to TAR register.
    res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &loc);
    if (res<0) goto libswd_memap_read_int_error;
    libswdctx->log.memap.tar=loc;
   }

   // Measure transfer speed.
   gettimeofday(&tstop, NULL);
   tdeltam=fabsf((tstop.tv_sec-tstart.tv_sec)*1000+(tstop.tv_usec-tstart.tv_usec)/1000);
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
              "LIBSWD_I: libswd_memap_read_int() reading address 0x%08X (speed %fKB/s)\r",
              loc, count*4/tdeltam );
   fflush(0);

   // Read data from the DRW register.
   res=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &memapdrw);
   if (res<0) goto libswd_memap_read_int_error;
   libswdctx->log.memap.drw=*memapdrw;
   data[i]=*memapdrw;
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "\n");
 }

 return LIBSWD_OK;

libswd_memap_read_int_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "\nLIBSWD_E: libswd_memap_read_int(): %s\n",
            libswd_error_string(res) );
 return res;
}


/** Generic read using MEM-AP, with prior CSW MEM-AP access setup.
 * Data are stored into int array.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to read with MEM-AP.
 * \param count is the number of words to read.
 * \param *data is the pointer to int array where result will be stored.
 * \param csw is the value of csw register to write prior data write.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_read_int_csw(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, int *data, int csw){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_read_int_csw(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, *data=%p, csw=0x%X)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void**)data, csw );

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res=0;

 // Calculate required access size based on CSW value.
 switch (csw&LIBSWD_MEMAP_CSW_SIZE)
 {
  case LIBSWD_MEMAP_CSW_SIZE_8BIT:
  case LIBSWD_MEMAP_CSW_SIZE_16BIT:
  case LIBSWD_MEMAP_CSW_SIZE_32BIT:
   break;
  default:
   res=LIBSWD_ERROR_MEMAPACCSIZE;
   goto libswd_memap_read_int_csw_error;
 }

 // Initialize MEM-AP if necessary.
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_read_int_csw_error;
 }

 // Setup MEM-AP CSW and TAR.
 res=libswd_memap_setup(libswdctx, operation, csw, addr);
 if (res<0) goto libswd_memap_read_int_csw_error;

 // Perform the read operation.
 res=libswd_memap_read_int(libswdctx, operation, addr, count, data);
 if (res<0) goto libswd_memap_read_int_csw_error;

 return LIBSWD_OK;

libswd_memap_read_int_csw_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "\nLIBSWD_E: libswd_memap_read_int_csw(): %s\n",
            libswd_error_string(res) );
 return res;
}


/** Generic read using MEM-AP, with prior 32-bit MEM-AP access setup.
 * Data are stored into int array.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to read with MEM-AP.
 * \param count is the number of words to read.
 * \param *data is the pointer to int array where result will be stored.
 * \param csw is the value of csw register to write prior data write.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_read_int_32(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, int *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_read_int_32(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, *data=%p)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void**)data);

 return libswd_memap_read_int_csw(libswdctx, operation, addr, count, data, LIBSWD_MEMAP_CSW_SIZE_32BIT|LIBSWD_MEMAP_CSW_ADDRINC_SINGLE);
}


/** Generic write using MEM-AP from char array.
 * Data are read from char array. Count shows CHAR elements.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to write with MEM-AP.
 * \param count is the number of bytes to write.
 * \param *data is the pointer to data to be written.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_write_char(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, char *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_write_char(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void*)data );

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int i, loc, res=0, accsize=0;
 float tdeltam;
 struct timeval tstart, tstop;

 // Initialize MEM-AP if neessary.
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_write_char_error;
 }

 // Verify the count parameter to be access boundary.
 switch (libswdctx->log.memap.csw&LIBSWD_MEMAP_CSW_SIZE)
 {
  case LIBSWD_MEMAP_CSW_SIZE_8BIT:
   accsize=1;
   break;
  case LIBSWD_MEMAP_CSW_SIZE_16BIT:
   accsize=2;
   break;
  case LIBSWD_MEMAP_CSW_SIZE_32BIT:
   accsize=4;
   break;
  default:
   res=LIBSWD_ERROR_MEMAPACCSIZE;
   goto libswd_memap_write_char_error;
 }
 if (count%accsize) count=count-(count%accsize);

 // check for alignment issues.
 if ((addr%accsize)!=0)
 {
  res=LIBSWD_ERROR_MEMAPALIGN;
  goto libswd_memap_write_char_error;
 }

 // Mark start time for transfer speed measurement.
 gettimeofday(&tstart, NULL);

 // Perform word-by-word write operation from char array.
 // Use write method that match the CSW AddrInc configuration.
 if (!(libswdctx->log.memap.csw&LIBSWD_MEMAP_CSW_ADDRINC))
 {
  // Use manual TAR incrementation (slower).
  for (i=0;i<count;i+=accsize)
  {
   int drw_shift;
   loc=addr+i;
   // Calculate the offset in DRW where the data should go
   // see Data byte-laning in the ARM debug interface v5 documentation
   // note: this only works for little endian systems.
   drw_shift=8*(loc%4);
   // Measure transfer speed.
   gettimeofday(&tstop, NULL);
   tdeltam=fabsf((tstop.tv_sec-tstart.tv_sec)*1000+(tstop.tv_usec-tstart.tv_usec)/1000);
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
              "LIBSWD_I: libswd_memap_write_char() writing address 0x%08X (speed %fKB/s)\r",
              loc, count/tdeltam );
   fflush(0);
   // Pass address to TAR register.
   res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &loc);
   if (res<0) goto libswd_memap_write_char_error;
   libswdctx->log.memap.tar=loc;
   // Implode and Write data to DRW register.
   memcpy((void*)&libswdctx->log.memap.drw, data+i, accsize);
   // apply the data byte-laning shift
   libswdctx->log.memap.drw <<= drw_shift;
   res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &libswdctx->log.memap.drw);
   if (res<0) goto libswd_memap_write_char_error;
  }
 }
 else
 {
  // Use TAR Auto Increment (faster).
  // TAR auto increment is only guaranteed to work on the bottom 10 bits
  // of the TAR register. Above that it is implementation defined.
  // We use 1024 byte chunks as it will work on every platform
  // and one TAR write every 1024 bytes is not adding too much overhead.
  const unsigned int BOUNDARY = 1024;
  unsigned int i;

  // Check if packed transfer, if so use word access.
  if (libswdctx->log.memap.csw&LIBSWD_MEMAP_CSW_ADDRINC_PACKED) accsize=4;

  for (loc = addr, i = 0; loc < (addr + count); loc += accsize, i+= accsize)
  {
   int drw_shift;

   // Calculate the offset in DRW where the data should go
   // see Data byte-laning in the ARM debug interface v5 documentation
   // note: this only works for little endian systems.
   drw_shift=8*(loc%4);

   // only write the TAR register, if this is the first time through the loop.
   // or if we've passed over the boundary where TAR auto inc isn't guaranteed
   // to work anymore.
   if (loc == addr ||
       (loc % BOUNDARY) == 0)
   {
    // Pass address to TAR register.
    res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &loc);
    if (res<0) goto libswd_memap_write_char_error;
    libswdctx->log.memap.tar=loc;
   }

   // Measure transfer speed.
   gettimeofday(&tstop, NULL);
   tdeltam=fabsf((tstop.tv_sec-tstart.tv_sec)*1000+(tstop.tv_usec-tstart.tv_usec)/1000);
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
              "LIBSWD_I: libswd_memap_write_char() writing address 0x%08X (speed %fKB/s)\r",
              loc, count/tdeltam );
   fflush(0);

   // apply the data byte-laning shift and write to the DRW register
   memcpy((void*)&libswdctx->log.memap.drw, data + i, accsize);
   libswdctx->log.memap.drw <<= drw_shift;
   res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &libswdctx->log.memap.drw);
   if (res<0) goto libswd_memap_write_char_error;
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "\n");
 }

 return LIBSWD_OK;

libswd_memap_write_char_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "LIBSWD_E: libswd_memap_write_char(): %s\n",
            libswd_error_string(res) );
 return res;
}


/** Generic write using MEM-AP from char array, with prior CSW setup.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to write with MEM-AP.
 * \param count is the number of bytes to write.
 * \param *data is the pointer to data to be written.
 * \param csw is the value of csw register to write prior data write.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_write_char_csw(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, char *data, int csw){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entring libswd_memap_write_char_csw(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p, csw=0x%X)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void**)data, csw );
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res=0, accsize=0;

 // Calculate required access size based on CSW value.
 switch (csw&LIBSWD_MEMAP_CSW_SIZE)
 {
  case LIBSWD_MEMAP_CSW_SIZE_8BIT:
   accsize=1;
   break;
  case LIBSWD_MEMAP_CSW_SIZE_16BIT:
   accsize=2;
   break;
  case LIBSWD_MEMAP_CSW_SIZE_32BIT:
   accsize=4;
   break;
  default:
   res=LIBSWD_ERROR_MEMAPACCSIZE;
   goto libswd_memap_write_char_csw_error;
 }
 // Verify the count parameter to be access boundary.
 if (count%accsize) count=count-(count%accsize);

 // Initialize MEM-AP if necessary.
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_write_char_csw_error;
 }

 // Setup MEM-AP CSW and TAR.
 res=libswd_memap_setup(libswdctx, operation, csw, addr);
 if (res<0) goto libswd_memap_write_char_csw_error;

 res=libswd_memap_write_char(libswdctx, operation, addr, count, data);
 if (res<0) goto libswd_memap_write_char_csw_error;

 return LIBSWD_OK;

libswd_memap_write_char_csw_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "LIBSWD_E: libswd_memap_write_char_csw(): %s\n",
            libswd_error_string(res) );
 return res;
}


/** Generic write using MEM-AP from char array, using 32-bit access.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to write with MEM-AP.
 * \param count is the number of bytes to write.
 * \param *data is the pointer to data to be written.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_write_char_32(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, char *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_write_char_32(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void**)data);

 return libswd_memap_write_char_csw(libswdctx, operation, addr, count, data, LIBSWD_MEMAP_CSW_SIZE_32BIT|LIBSWD_MEMAP_CSW_ADDRINC_SINGLE);
}


/** Generic write using MEM-AP from int array.
 * Data are stored into char array.
 * Remember to setup CSW first for valid bus access!
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to write with MEM-AP.
 * \param count is the number of words to write.
 * \param *data is the pointer to int data array to be written.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_write_int(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, int *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_write_int(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void*)data);

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int i, loc, res=0;
 float tdeltam;
 struct timeval tstart, tstop;

 // Initialize MEM-AP if necessary.
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_write_int_error;
 }

 // Mark start time for transfer speed measurement.
 gettimeofday(&tstart, NULL);

 // Perform word-by-word write operation from int array.
 if (!(libswdctx->log.memap.csw&LIBSWD_MEMAP_CSW_ADDRINC))
 {
  // Use manual TAR incrementation (slower).
  for (i=0;i<count;i++)
  {
   loc=addr+i*4;
   // Measure transfer speed.
   gettimeofday(&tstop, NULL);
   tdeltam=fabsf((tstop.tv_sec-tstart.tv_sec)*1000+(tstop.tv_usec-tstart.tv_usec)/1000);
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
              "LIBSWD_I: libswd_memap_write_int() writing address 0x%08X (speed %fKB/s)\r",
              loc, count*4/tdeltam );
   fflush(0);
   // Pass address to TAR register.
   res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &loc);
   if (res<0) goto libswd_memap_write_int_error;
   libswdctx->log.memap.tar=loc;
   // Implode and Write data to DRW register.
   res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, data+i);
   if (res<0) goto libswd_memap_write_int_error;
   libswdctx->log.memap.drw=data[i];
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "\n");
 }
 else
 {
  // Use TAR Auto Increment (faster).
  // TAR auto increment is only guaranteed to work on the bottom 10 bits
  // of the TAR register. Above that it is implementation defined.
  // We use 1024 byte chunks as it will work on every platform
  // and one TAR write every 1024 bytes is not adding too much overhead.
  const unsigned int BOUNDARY = 1024;
  unsigned int i;

  for (loc = addr, i = 0; loc < (addr + (count * 4)); loc += 4, i++)
  {
   // only write the TAR register, if this is the first time through the loop.
   // or if we've passed over the boundary where TAR auto inc isn't guaranteed
   // to work anymore.
   if (loc == addr ||
       (loc % BOUNDARY) == 0)
   {
    // Pass address to TAR register.
    res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_TAR_ADDR, &loc);
    if (res<0) goto libswd_memap_write_int_error;
    libswdctx->log.memap.tar=loc;
   }

   // Measure transfer speed.
   gettimeofday(&tstop, NULL);
   tdeltam=fabsf((tstop.tv_sec-tstart.tv_sec)*1000+(tstop.tv_usec-tstart.tv_usec)/1000);
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
              "LIBSWD_I: libswd_memap_write_int() writing address 0x%08X (speed %fKB/s)\r",
              loc, count*4/tdeltam );
   fflush(0);

   // Write data to DRW register.
   libswdctx->log.memap.drw=data[i];
   res=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE, LIBSWD_MEMAP_DRW_ADDR, &libswdctx->log.memap.drw);
   if (res<0) goto libswd_memap_write_int_error;
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "\n");
 }

 return LIBSWD_OK;

libswd_memap_write_int_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "\nLIBSWD_E: libswd_memap_write_int(): %s\n",
            libswd_error_string(res) );
 return res;
}

/** Generic write using MEM-AP from int array, with prior CSW MEM-AP setup.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to write with MEM-AP.
 * \param count is the number of words to write.
 * \param *data is the pointer to int data array to be written.
 * \param csw is the value of csw register to write prior data write.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_write_int_csw(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, int *data, int csw){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_write_int_csw(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p, csw=0x%X)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void**)data, csw );

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (operation!=LIBSWD_OPERATION_ENQUEUE && operation!=LIBSWD_OPERATION_EXECUTE)
  return LIBSWD_ERROR_BADOPCODE;

 int res=0;

 // Calculate required access size based on CSW value.
 switch (csw&LIBSWD_MEMAP_CSW_SIZE)
 {
  case LIBSWD_MEMAP_CSW_SIZE_8BIT:
  case LIBSWD_MEMAP_CSW_SIZE_16BIT:
  case LIBSWD_MEMAP_CSW_SIZE_32BIT:
   break;
  default:
   res=LIBSWD_ERROR_MEMAPACCSIZE;
   goto libswd_memap_write_int_csw_error;
 }

 // Initialize MEM-AP if necessary.
 if (!libswdctx->log.memap.initialized)
 {
  res=libswd_memap_init(libswdctx, operation);
  if (res<0) goto libswd_memap_write_int_csw_error;
 }

 // Setup MEM-AP CSW and TAR.
 res=libswd_memap_setup(libswdctx, operation, csw, addr);
 if (res<0) goto libswd_memap_write_int_csw_error;

 // Perform the write operation.
 res=libswd_memap_write_int(libswdctx, operation, addr, count, data);
 if (res<0) goto libswd_memap_write_int_csw_error;

 return LIBSWD_OK;

libswd_memap_write_int_csw_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "\nLIBSWD_E: libswd_memap_write_int_csw(): %s\n",
            libswd_error_string(res) );
 return res;
}

/** Generic write using MEM-AP from int array, with prior 32-bit MEM-AP access setup.
 * \param *libswdctx swd context to work on.
 * \param operation can be LIBSWD_OPERATION_ENQUEUE or LIBSWD_OPERATION_EXECUTE.
 * \param addr is the start address of the data to write with MEM-AP.
 * \param count is the number of words to write.
 * \param *data is the pointer to int data array to be written.
 * \return number of elements/words processed or LIBSWD_ERROR code on failure.
 */
int libswd_memap_write_int_32(libswd_ctx_t *libswdctx, libswd_operation_t operation, int addr, int count, int *data){
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: Entering libswd_memap_write_int_32(*libswdctx=%p, operation=%s, addr=0x%08X, count=0x%08X, **data=%p)...\n",
            (void*)libswdctx, libswd_operation_string(operation),
            addr, count, (void**)data);

 return libswd_memap_write_int_csw(libswdctx, operation, addr, count, data, LIBSWD_MEMAP_CSW_SIZE_32BIT|LIBSWD_MEMAP_CSW_ADDRINC_SINGLE);
}


/** @} */
