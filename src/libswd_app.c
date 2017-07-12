/*
 * Serial Wire Debug Open Library.
 * Application Body File.
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

/** \file libswd_app.c */

#include <libswd_app.h>
#include <libswd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#if defined(__MINGW32__) || (defined(__APPLE__) && defined(__MACH__))
#include <libftdi1/ftdi.h>
#else
#include <ftdi.h>
#endif
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include <signal.h>
#include <config.h>

/*******************************************************************************
 * \defgroup libswd_app LibSWD Application functions.
 * @{
 ******************************************************************************/

libswdapp_context_t *appctx;
char history_filename[128];


void libswdapp_shutdown(int sig)
{
 int retval=appctx->retval;
 printf("\nShutting down application...\n");
 if (appctx->interface->signal)
  libswdapp_interface_signal_del(appctx,"*");
 if (appctx->interface->ctx!=NULL) appctx->interface->deinit(appctx);
 if (appctx->interface!=NULL) free(appctx->interface);
 if (appctx->libswdctx!=NULL) libswd_deinit(appctx->libswdctx);
 if (appctx!=NULL) free(appctx);
 stifle_history(LIBSWDAPP_CLI_HISTORY_MAXLEN);
 if ( (retval=write_history(history_filename)) )
  printf("WARNING: Cannot save CLI history to file (error %d)!\n", retval);
 printf("Exit OK\n");
 exit(retval);
}

int libswdapp_print_banner(void){
 printf("*******************************************************************\n");
 printf("* Welcome to LibSWD CLI Application! Type '?' or 'help' for help. *\n");
 printf("* See project website http://libswd.sf.net for more information.. *\n");
 printf("* (C) CeDeROM (http://www.tomek.cedro.info)  Version: %11s *\n", VERSION);
 printf("*******************************************************************\n\n");
 return LIBSWD_OK;
}

int libswdapp_print_usage(void){
 printf(" LibSWD Application available options ('*' also available via cli): \n");
 printf("  * -l : Use this log level (min=0..6=max)\n");
 printf("    -q : Quiet mode, no verbose output (equals '-l 0')\n");
 printf("    -i : Interface Driver selection (by name)\n");
 printf("    -v : Interface VID (default 0x0403 if not specified)\n");
 printf("    -p : Interface PID (default 0xbbe2 if not specified)\n");
 printf("  * -f : Flash Memory related operations\n");
 printf("  * -h : Display this help\n\n");
 // List available interface drivers.
 int i;
 printf(" Available Interface Drivers:\n");
 for (i=0;libswdapp_interface_configs[i].name[0];i++)
  printf("  %s (%s)\n",
         libswdapp_interface_configs[i].name,
         libswdapp_interface_configs[i].description);
 printf("\n");
 libswdapp_handle_command_signal_usage();
 libswdapp_handle_command_flash_usage();
 printf(" Note: Parameters marked with '< >' are optional.\n");
 printf(" Press Ctrl+C or type [q]uit on prompt to exit LibSWD Application.\n\n");
 return LIBSWD_OK;
}


/** LibSWD Application
 * \param *argc program parameter count.
 * \param **argv program invocation parameter values.
 * \return ERROR_OK on success, negative error code otherwise.
 */
int main(int argc, char **argv){
 char *cmd, *command;
 int i, retval=0;
 libswdapp_context_t *libswdappctx;

 // Initialize application context in memory.
 libswdappctx=(libswdapp_context_t*)calloc(1,sizeof(libswdapp_context_t));
 if (libswdappctx==NULL)
 {
  printf("ERROR: Cannot initialize LibSWD Application Context, ejecting!\n");
  retval=LIBSWD_ERROR_OUTOFMEM;
  goto quit;
 }
 libswdappctx->interface=(libswdapp_interface_t*)calloc(1,sizeof(libswdapp_interface_t));
 if (libswdappctx->interface==NULL)
 {
  printf("ERROR: Cannot initialize Interface structure, ejecting!\n");
  retval=LIBSWD_ERROR_OUTOFMEM;
  goto quit;
 }
 libswdappctx->loglevel=LIBSWD_LOGLEVEL_DEFAULT;

 // Catch SIGINT signal and call shutdown.
 appctx=libswdappctx;
 signal(SIGINT, libswdapp_shutdown);

 // Handle program commandline execution arguments.
 while ( (i=getopt(argc,argv,"hqi:l:p:v:"))!=-1 )
 {
  switch (i)
  {
   case 'q':
    libswdappctx->loglevel=LIBSWD_LOGLEVEL_SILENT;
    break;
   case 'l':
    libswdappctx->loglevel=atol(optarg);
    break;
   case 'v':
    libswdappctx->interface->vid_forced=strtol(optarg, (char**)NULL, 16);
    break;
   case 'p':
    libswdappctx->interface->pid_forced=strtol(optarg, (char**)NULL, 16);
    break;
   case 'i':
    strncpy(libswdappctx->interface->name, optarg, LIBSWDAPP_INTERFACE_NAME_MAXLEN);
    break;
   case 'h':
   default:
    libswdapp_print_banner();
    libswdapp_print_usage();
    if (optopt=='h')
    {
     return LIBSWD_OK;
    } else return LIBSWD_ERROR_PARAM;
  }
 }

 // Print application banner.
 libswdapp_print_banner();

 // Initialize LibSWD.
 libswdappctx->libswdctx=libswd_init();
 if (libswdappctx->libswdctx==NULL)
 {
  if (libswdappctx->loglevel) printf("ERROR: Cannot initialize libswd, ejecting!\n");
  retval=LIBSWD_ERROR_NULLCONTEXT;
  goto quit;
 }
 // Set default LogLevel.
 if (LIBSWD_OK!=libswd_log_level_set(libswdappctx->libswdctx,libswdappctx->loglevel))
 {
  if (libswdappctx->loglevel)
   libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
              "Cannot set %s loglevel for LibSWD!\n",\
              libswd_log_level_string(libswdappctx->loglevel) );
  goto quit;
 }
 // We don't want the automatic error fix.
 libswdappctx->libswdctx->config.autofixerrors=0;

 // Initialize the Interface
 retval=libswdapp_handle_command_interface_init(libswdappctx, NULL);
 if (retval!=LIBSWD_OK) return retval;
 // Store program Context for use with interface drivers.
 libswdappctx->libswdctx->driver->ctx=libswdappctx;
 // Store interface structure in libswd driver.
 libswdappctx->libswdctx->driver->interface=libswdappctx->interface;

 // Setup the Readline
 rl_completion_append_character='\0'; //disable auto-complete
 sprintf(history_filename, "%s%s", getenv("HOME"),LIBSWDAPP_CLI_HISTORY_FILENAME);
 if ( (retval=read_history(history_filename)) )
  libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_WARNING,
             "WARNING: Cannot load CLI history from file (error %d)!\n",
             retval );

 /* Run the Command Line Interpreter loop. */
 while ((command=readline("libswd>")) != NULL){
  while ( (cmd=strsep(&command, "\n;")) )
  {
   if (cmd[0]!=0) add_history(cmd);
   if (!strncmp(cmd,"q",1) || !strncmp(cmd,"quit",4)) goto quit;
   if (!strncmp(cmd,"s",1) || !strncmp(cmd,"signal",5))
   {
    libswdapp_handle_command_signal(libswdappctx, cmd);
    continue;
   }
   if (!strncmp(cmd,"f",1) || !strncmp(cmd,"flash",5))
   {
    libswdapp_handle_command_flash(libswdappctx, cmd);
    continue;
   }
   if (!strncmp(cmd,"h",1) || !strncmp(cmd,"help",4) || !strncmp(cmd,"?",1))
   {
    libswdapp_print_banner();
    libswdapp_print_usage();
   }
   retval=libswd_cli(libswdappctx->libswdctx, cmd);
   if (retval!=LIBSWD_OK) if (retval!=LIBSWD_ERROR_CLISYNTAX) goto quit;
  }
 }

 /* Cleanup and quit. */
quit:
 libswdapp_shutdown(2);
 return LIBSWD_OK;
}


/******************************************************************************
 * LibSWD APPLICATION COMMAND HANDLERS                                        *
 ******************************************************************************/

/** Print out the Flash command usage.
 * \return Always LIBSWD_OK.
 */
int libswdapp_handle_command_flash_usage(void){
 printf(" LibSWD Application Flash ('[f]lash') usage:\n");
 printf("  [r]ead <filename> <count>\n");
 printf("  [w]rite filename\n");
 printf("  [m]ass[e]rase\n");
 printf("  Note: Target will be detected automaticaly to verify flash support.\n");
 printf("\n");
 return LIBSWD_OK;
}

/** Handle Flash command invocation.
 * \param *libswdappctx LibSWD Application Context to work on.
 * \param *command to be parsed and executed.
 * \return LIBSWD_OK on success or LIBSWD_ERROR code otherwise.
 */

int libswdapp_handle_command_flash(libswdapp_context_t *libswdappctx, char *command)
{
 if (!libswdappctx) return LIBSWD_ERROR_NULLCONTEXT;
 int i, j, retval, *idcode, flashdrvidx=0, dbgdhcsr, data, *datap, count, addr, addrstart;
 char buf[4], *cmd, *filename;
 libswd_ctx_t *libswdctx=(libswd_ctx_t*)libswdappctx->libswdctx;
 libswdapp_flash_stm32f1_memmap_t flash_memmap;

 // Initialize the MEM-AP and DAP if not yet initialized...
 if (!libswdctx->log.memap.initialized)
 {
  retval=libswd_memap_init(libswdctx, LIBSWD_OPERATION_EXECUTE);
  if (retval<0) goto libswdapp_handle_command_flash_error;
 }

 // Setup Flash configuration, routines, memmap etc.
 // At the moment only STM32F10x is supported/hardcoded.
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "Using STM32F1 configuration...\n");
 flash_memmap=libswdapp_flash_stm321f_mediumdensity;
 addrstart=flash_memmap.page_start;
 count=(flash_memmap.page_end-flash_memmap.page_start);

 // Check if target is halted, halt if necessary.
 if (!libswd_debug_is_halted(libswdctx, LIBSWD_OPERATION_EXECUTE))
 {
  retval=libswd_debug_halt(libswdctx, LIBSWD_OPERATION_EXECUTE);
  if (retval<0) goto libswdapp_handle_command_flash_error;
 }

 // Target is ready to perform Flash operations.
 if (command) cmd=strsep(&command," ");
 if (command) cmd=strsep(&command," ");

 // Check for READ invocation.
 if ( strncmp(cmd,"r",1)==0 || strncmp(cmd,"read",4)==0 )
 {
  // Parse filename parameter.
  filename=NULL;
  if (command)
  {
   cmd=strsep(&command," ");
   if (cmd && cmd!='\0' && !isspace(*cmd)) filename=cmd;
  }
  // Parse optional count parameter.
  if (command)
  {
   cmd=strsep(&command," ");
   errno=LIBSWD_OK;
   i=strtol(cmd, (char**)NULL, 0);
   if (errno==LIBSWD_OK)
   {
    count=i-(i%4);
   }
   else
   {
    retval=LIBSWD_ERROR_CLISYNTAX;
    goto libswdapp_handle_command_flash_error;
   }
  }
  // Take care of proper memory (re)allocation.
  if (libswdctx->membuf.data) free(libswdctx->membuf.data);
  libswdctx->membuf.data=(unsigned char*)malloc(count*sizeof(char));
  if (!libswdctx->membuf.data)
  {
   libswdctx->membuf.size=0;
   libswd_log(libswdctx, LIBSWD_ERROR_OUTOFMEM,
              "FLASH ERROR: Cannot (re)allocate memory buffer!\n");
   return LIBSWD_ERROR_OUTOFMEM;
  } else memset((void*)libswdctx->membuf.data, 0xFF, count);
  libswdctx->membuf.size=count*sizeof(char);
  retval=libswd_memap_read_char_32(libswdctx, LIBSWD_OPERATION_EXECUTE,
                           addrstart, count,
                           (char *)libswdctx->membuf.data);
  if (retval<0) goto libswdapp_handle_command_flash_error;
  // Store result to a file if requested.
  if (filename)
  {
   FILE *fp;
   fp=fopen(filename,"w");
   if (!fp)
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
               "FLASH ERROR: Cannot open '%s' data file!\n",
               filename, strerror(errno) );
    retval=LIBSWD_ERROR_FILE;
    goto libswdapp_handle_command_flash_error;
   }
   retval=fwrite(libswdctx->membuf.data, sizeof(char), count, fp);
   if (!retval)
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
               "FLASH ERROR: Cannot write to data file '%s' (%s)!\n",
               filename, strerror(errno) );
    goto libswdapp_handle_command_flash_error;
   }
   i=fclose(fp);
   if (i)
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
               "FLASH ERROR: Cannot close data file '%s' (%s)!\n",
               filename, strerror(errno) );
    goto libswdapp_handle_command_flash_error;
   }
  }
  // Print out the result.
  for (i=0; i<libswdctx->membuf.size; i=i+16)
  {
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "\n%08X: ", i+addrstart);
   for (j=0;j<16&&i+j<libswdctx->membuf.size;j++)
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "%02X ",
               (unsigned char)libswdctx->membuf.data[i+j] );
   if (j<16) for(;j<16;j++)
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "   ");
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, " ");
   for (j=0;j<16&&i+j<libswdctx->membuf.size;j++)
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "%c",
               isprint(libswdctx->membuf.data[i+j])?libswdctx->membuf.data[i+j]:'.');
   if (j<16)
    for (;j<16;j++)
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, ".");
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
             "\nFLASH READ OK!\n");
  return LIBSWD_OK;
 }

 // Check for ERASE invocation.
 else if ( strncmp(cmd,"me",2)==0 || strncmp(cmd,"masserase",9)==0 )
 {
  // Unlock the Flash Controller.
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "FLASH: Unlocking STM32 FPEC...\n");
  data=LIBSWDAPP_FLASH_STM32F1_FLASH_KEYR_KEY1_VAL;
  retval=libswd_memap_write_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_KEYR_ADDR, 1, &data);
  if (retval<0) goto libswdapp_handle_command_flash_error;
  data=LIBSWDAPP_FLASH_STM32F1_FLASH_KEYR_KEY2_VAL;
  retval=libswd_memap_write_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_KEYR_ADDR, 1, &data);
  if (retval<0) goto libswdapp_handle_command_flash_error;
  // Perform Mass-Erase operation.
  //Wait for BSY flag clearance.
  for (i=LIBSWD_RETRY_COUNT_DEFAULT;i;i--)
  {
   retval=libswd_memap_read_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_SR_ADDR, 1, &data);
   if (!(data&LIBSWDAPP_FLASH_STM32F1_FLASH_SR_BSY)) break;
   usleep(100);
  }
  if (!i)
  {
   retval=LIBSWD_ERROR_MAXRETRY;
   goto libswdapp_handle_command_flash_error;
  }
  //Set MER bit in FLASH_CR
  retval=libswd_memap_read_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_CR_ADDR, 1, &data);
  if (retval<0) goto libswdapp_handle_command_flash_error;
  data|=LIBSWDAPP_FLASH_STM32F1_FLASH_CR_MER;
  retval=libswd_memap_write_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_CR_ADDR, 1, &data);
  if (retval<0) goto libswdapp_handle_command_flash_error;
  data|=LIBSWDAPP_FLASH_STM32F1_FLASH_CR_STRT;
  retval=libswd_memap_write_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_CR_ADDR, 1, &data);
  if (retval<0) goto libswdapp_handle_command_flash_error;
  //Wait for BSY flag clearance.
  for (i=LIBSWD_RETRY_COUNT_DEFAULT;i;i--)
  {
   retval=libswd_memap_read_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_SR_ADDR, 1, &data);
   if (!(data&LIBSWDAPP_FLASH_STM32F1_FLASH_SR_BSY)) break;
   usleep(100);
  }
  if (!i)
  {
   retval=LIBSWD_ERROR_MAXRETRY;
   goto libswdapp_handle_command_flash_error;
  }
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "FLASH MASS-ERASE OK!\n");
 }

  // Check for WRITE invocation.
  else if ( strncmp(cmd,"w",1)==0 || strncmp(cmd,"write",5)==0 )
  {
   if (command)
   {
    cmd=strsep(&command," ");
    filename=cmd;
   }
   else
   {
    retval=LIBSWD_ERROR_CLISYNTAX;
    goto libswdapp_handle_command_flash_error;
   }
   retval=LIBSWD_OK;
   FILE *fp;
   fp=fopen(filename,"r");
   if (!fp)
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
               "FLASH ERROR: Cannot open '%s' data file (%s)!\n",
               filename, strerror(errno));
    retval=LIBSWD_ERROR_FILE;
    goto libswdapp_handle_command_flash_error;
   }
   // Take care of (re)allocating memory for membuf based on a file size.
   if (libswdctx->membuf.size) free(libswdctx->membuf.data);
   fseek(fp, 0, SEEK_END);
   // Verify if filesize fits in Flash meory.
   if (ftell(fp)>count)
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
               "FLASH ERROR: File (0x%X) won't fit in memory (0x%X)!\n",
               ftell(fp), count);
    retval=LIBSWD_ERROR_OUTOFMEM;
    goto libswdapp_handle_command_flash_error;
   }
   // Allocate memory for file content.
   libswdctx->membuf.size=ftell(fp);
   libswdctx->membuf.data=(unsigned char*)malloc(libswdctx->membuf.size*sizeof(char));
   fseek(fp, 0, SEEK_SET);
   if (!libswdctx->membuf.data)
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
               "FLASH ERROR: Cannot allocate memory to load '%s' file, aborting!\n",
               filename );
    retval=LIBSWD_ERROR_OUTOFMEM;
    goto libswdapp_handle_command_flash_file_load_error;
   }
   // Load file content into memory.
   retval=fread(libswdctx->membuf.data, sizeof(char), libswdctx->membuf.size, fp);
   if (!retval)
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
               "FLASH ERROR: Cannot load data from '%s' file (%s)!\n",
               filename, strerror(errno) );
    retval=LIBSWD_ERROR_FILE;
   }
libswdapp_handle_command_flash_file_load_error:
   retval=fclose(fp);
   if (retval)
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
               "FLASH ERROR: libswd_cli(): Cannot close data file '%s' (%s)!\n",
               filename, strerror(errno) );
    retval=LIBSWD_ERROR_FILE;
    goto libswdapp_handle_command_flash_file_load_error;
   }
libswdapp_handle_command_flash_file_load_ok:
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
              "FLASH: %d bytes of data from '%s' file loaded!\n",
              libswdctx->membuf.size, filename );
   count=libswdctx->membuf.size;
   // At this point data are in membuf, sent them to MEM-AP.

   // FLASH WRITE
   // Unlock the Flash Controller.
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "FLASH: Unlocking STM32 FPEC...\n");
   data=LIBSWDAPP_FLASH_STM32F1_FLASH_KEYR_KEY1_VAL;
   retval=libswd_memap_write_int_32(libswdctx, LIBSWD_OPERATION_ENQUEUE, flash_memmap.FLASH_KEYR_ADDR, 1, &data);
   if (retval<0) goto libswdapp_handle_command_flash_error;
   data=LIBSWDAPP_FLASH_STM32F1_FLASH_KEYR_KEY2_VAL;
   retval=libswd_memap_write_int_32(libswdctx, LIBSWD_OPERATION_ENQUEUE, flash_memmap.FLASH_KEYR_ADDR, 1, &data);
   if (retval<0) goto libswdapp_handle_command_flash_error;
   // Perform Mass-Erase operation.
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "FLASH: Performing Flash Mass-Erase...\n");
   //Wait for BSY flag clearance.
   for (i=LIBSWD_RETRY_COUNT_DEFAULT;i;i--)
   {
    retval=libswd_memap_read_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_SR_ADDR, 1, &data);
    if (!(data&LIBSWDAPP_FLASH_STM32F1_FLASH_SR_BSY)) break;
    usleep(LIBSWD_RETRY_DELAY_DEFAULT);
   }
   if (!i)
   {
    retval=LIBSWD_ERROR_MAXRETRY;
    goto libswdapp_handle_command_flash_error;
   }
   //Set MER bit in FLASH_CR
   retval=libswd_memap_read_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_CR_ADDR, 1, &data);
   if (retval<0) goto libswdapp_handle_command_flash_error;
   data|=LIBSWDAPP_FLASH_STM32F1_FLASH_CR_MER;
   retval=libswd_memap_write_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_CR_ADDR, 1, &data);
   if (retval<0) goto libswdapp_handle_command_flash_error;
   data|=LIBSWDAPP_FLASH_STM32F1_FLASH_CR_STRT;
   retval=libswd_memap_write_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_CR_ADDR, 1, &data);
   if (retval<0) goto libswdapp_handle_command_flash_error;
   //Wait for BSY flag clearance.
   for (i=LIBSWD_RETRY_COUNT_DEFAULT;i;i--)
   {
    retval=libswd_memap_read_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_SR_ADDR, 1, &data);
    if (!(data&LIBSWDAPP_FLASH_STM32F1_FLASH_SR_BSY)) break;
    usleep(100);
   }
   if (!i)
   {
    retval=LIBSWD_ERROR_MAXRETRY;
    goto libswdapp_handle_command_flash_error;
   }
   // Perform Flash write.
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "FLASH: Performing Flash Write...\n");
   retval=libswd_memap_read_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_CR_ADDR, 1, &data);
   if (retval<0) goto libswdapp_handle_command_flash_error;
   data=LIBSWDAPP_FLASH_STM32F1_FLASH_CR_PG;
   retval=libswd_memap_write_int_32(libswdctx, LIBSWD_OPERATION_EXECUTE, flash_memmap.FLASH_CR_ADDR, 1, &data);
   if (retval<0) goto libswdapp_handle_command_flash_error;
   // Perform the data write phase using MEM-AP.
   int accsize=4;
   addr=flash_memmap.page_start;
   retval=libswd_memap_write_char_csw(libswdctx, LIBSWD_OPERATION_EXECUTE, addr, count, (char *)libswdctx->membuf.data, LIBSWD_MEMAP_CSW_SIZE_16BIT|LIBSWD_MEMAP_CSW_ADDRINC_PACKED);
    if (retval<0) goto libswdapp_handle_command_flash_error;
   // Print out the data.
   for (i=0; i<libswdctx->membuf.size; i=i+16)
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "\n%08X: ", i+addrstart);
    for (j=0;j<16&&i+j<libswdctx->membuf.size;j++)
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "%02X ",
                (unsigned char)libswdctx->membuf.data[i+j] );
    if (j<16) for(;j<16;j++)
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "   ");
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, " ");
    for (j=0;j<16&&i+j<libswdctx->membuf.size;j++)
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "%c",
                isprint(libswdctx->membuf.data[i+j])?libswdctx->membuf.data[i+j]:'.' );
    if (j<16) for (;j<16;j++)
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, ".");
   }
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "\nFLASH: WRITE OK!\n");
  }

 return LIBSWD_OK;

libswdapp_handle_command_flash_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR, "FLASH ERROR: %s.\n", libswd_error_string(retval));
 return retval;
}


/** Print out the Interface Signal command usage.
 * \return Always LIBSWD_OK.
 */
int libswdapp_handle_command_signal_usage(void){
 printf(" LibSWD Application Interface Signal ('[s]ignal') usage:\n");
 printf("  list            lists available signals and cached values\n");
 printf("  add:name=mask   adds signal with given <name>\n");
 printf("  del:name        removes signal with given <name>\n");
 printf("  name=value      write hex <value> to given <name> signal mask\n");
 printf("  name            reads the <name> signal value\n");
 printf("\n");
 return LIBSWD_OK;
}


/** Handle signal command (cli and commandline parameter).
 * Multiple signal operations are allowed separated by a space in a single line.
 * \param *libswdappctx context to work on.
 * \param *cmd is the signal command argument.
 * \return LIBSWD_OK on success, negative value LIBSWD_ERROR code otehrwise.
 */
int libswdapp_handle_command_signal(libswdapp_context_t *libswdappctx, char *cmd){
 int retval,sigmask;
 unsigned int sigval;
 char *command, *signamep, *sigmaskp, *sigvalp, *cmdp, *cmdip, *cmdap, *cmdvp;
 libswdapp_interface_signal_t *sig;
 libswd_ctx_t *libswdctx=(libswd_ctx_t*)libswdappctx->libswdctx;

 if (cmd==0) return LIBSWD_OK;

 command=(char*)calloc(strlen(cmd),sizeof(char));
 if (!command)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Cannot allocate memory for command anaysis!\n" );
  return LIBSWD_ERROR_OUTOFMEM;
 }
 strncpy(command, cmd, strlen(cmd));

 cmdp=strsep(&command," ");
 if (!cmd)
  return libswdapp_handle_command_signal_usage();

 while ( (cmdp=strsep(&command," ")) )
 {
  // Check if list command, handle if necessary.
  if (!strncasecmp(cmdp,"list",4))
  {
   sig = libswdappctx->interface->signal;
   while (sig)
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "%s[0x%0X]=0x%0X\n",
               sig->name, sig->mask, sig->value );
    sig = sig->next;
   }
   continue;
  }
  // Check if add/delete command, handle if necessary.
  if (strchr(cmdp,':'))
  {
   cmdip=strsep(&cmdp,":");
   if (!strncmp(cmdip,"del",3))
   {
    signamep=cmdp;
    if (!*signamep)
    {
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                "WARNING: Syntax Error, see '?' or '[s]ignal' for more information...\n" );
     continue;
    }
    retval=libswdapp_interface_signal_del(libswdappctx, signamep);
    if (retval!=LIBSWD_OK)
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                "WARNING: Cannot remove signal '%s'!\n", signamep );
    continue;
   }
   else if (!strncmp(cmdip,"add",3))
   {
    signamep=strsep(&cmdp,"=");
    sigmaskp=cmdp;
    if (!signamep || !sigmaskp)
    {
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                "WARNING: Syntax Error, see '?' or '[s]ignal' for more information...\n" );
     continue;
    }
    errno=LIBSWD_OK;
    sigmask=strtol(sigmaskp,NULL,16);
    if (errno!=LIBSWD_OK)
    {
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                "WARNING: Syntax Error, see '?' or '[s]ignal' for more information...\n" );
     continue;
    }
    retval=libswdapp_interface_signal_add(libswdappctx, signamep, sigmask);
    if (retval!=LIBSWD_OK)
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                "WARNING: Cannot add '%s' signal with '%x' mask!\n", signamep, sigmaskp, sigmask );
    continue;
   }
  }
  // Check if signal read/write, handle if necessary.
  signamep=strsep(&cmdp,"=");
  sigvalp=cmdp;
  if (sigvalp)
  {
   if (!strncmp(sigvalp,"hi",2))
    sigval=~0;
   else if (!strncmp(sigvalp,"lo",2))
    sigval=0;
   else
   {
    errno=LIBSWD_OK;
    sigval=strtol(sigvalp,NULL,16);
    if (errno!=LIBSWD_OK)
    {
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                "WARNING: Syntax Error, see '?' or '[s]ignal' for more information...\n" );
     continue;
    }
   }
  }
  sig=libswdapp_interface_signal_find(libswdappctx, signamep);
  if (!sig)
  {
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
              "WARNING: Signal '%s' not found!\n", signamep );
   continue;
  }
  if (signamep && sigvalp)
  {
   retval=libswdappctx->interface->bitbang(libswdappctx, sig->mask, 0, &sigval);
   if (retval==LIBSWD_OK)
   {
    sig->value=sigval;
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "%s[0x%X]=0x%X\n",
                signamep, sig->mask, sig->value );
   } else libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                     "WARNING: Cannot set signal '%s' value!\n", signamep);
   continue;
  }
  else if (signamep)
  {
   retval=libswdappctx->interface->bitbang(libswdappctx, sig->mask, 1, &sigval);
   if (retval==LIBSWD_OK)
   {
    sig->value=sigval;
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
               "%s[0x%X]=0x%X\n", signamep, sig->mask, sig->value );
   } else libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                     "WARNING: Cannot read signal '%s' value!\n", signamep );
   continue;
  }
 }
 free(command);
 return LIBSWD_OK;

libswdapp_handle_command_signal_error:
  free(command);
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Syntax Error, see '?' or '[s]ignal' for more information...\n" );
  return LIBSWD_ERROR_CLISYNTAX;
}


/** It will prepare interface for use or fail.
 * If an interface is already configured, it will check if requested interface
 * is available and reinitialize driver (remove old driver and load new one).
 * cmd==NULL means that we need to load default interface (defined in header file)
 * or we need to (re)initialize an interface that is already set in interface->name.
 */
int libswdapp_handle_command_interface_init(libswdapp_context_t *libswdappctx, char *cmd){
 int retval, i, interface_number;
 char *param, buf[8];
 libswd_ctx_t *libswdctx=(libswd_ctx_t*)libswdappctx->libswdctx;

 // Verify the working context.
 if (!libswdappctx)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: libswdapp_hanle_command_interface() NULL libswdappctx context!\n");
  return LIBSWD_ERROR_NULLCONTEXT;
 }

 // Verify selected interface name or try the dedault.
 if (!libswdappctx->interface->name[0])
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
             "Trying the default interface '%s'...\n",
             LIBSWDAPP_INTERFACE_NAME_DEFAULT );
  strncpy(
          libswdappctx->interface->name,
          LIBSWDAPP_INTERFACE_NAME_DEFAULT,
          LIBSWDAPP_INTERFACE_NAME_MAXLEN
         );
 } else libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
                   "Selected interface: %s\n", libswdappctx->interface->name );

 // At this point we have the interface name stored in the
 // libswdappctx->interface->name. See if its supported.
 interface_number=-1;
 for (i=0;libswdapp_interface_configs[i].name[0];i++)
  if (!strncasecmp(libswdappctx->interface->name,libswdapp_interface_configs[i].name,LIBSWDAPP_INTERFACE_NAME_MAXLEN) )
  {
   interface_number=i;
   break;
  }
 if (interface_number==-1)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Interface '%s' is not supported!\n",
             libswdappctx->interface->name );
  return LIBSWD_ERROR_PARAM;
 }
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
            "Loading '%s' interface...",
             libswdapp_interface_configs[interface_number].name );

 // Unload existing interface settings if necessary.
 if (!libswdappctx->interface->name) return LIBSWD_ERROR_DRIVER;
 if (libswdappctx->interface->deinit)
  libswdappctx->interface->deinit(libswdappctx);
 if (libswdappctx->interface->ctx!=NULL && libswdappctx->interface->deinit)
  libswdappctx->interface->deinit(libswdappctx);
 if (libswdappctx->interface->signal)
  libswdapp_interface_signal_del(libswdappctx, "*");
 libswdappctx->interface->description[0]=0;
 libswdappctx->interface->init=NULL;
 libswdappctx->interface->deinit=NULL;
 libswdappctx->interface->set_freq=NULL;
 libswdappctx->interface->bitbang=NULL;
 libswdappctx->interface->transfer_bits=NULL;
 libswdappctx->interface->transfer_bytes=NULL;
 libswdappctx->interface->latency=0;
 libswdappctx->interface->maxfrequency=0;
 libswdappctx->interface->frequency=-1;
 libswdappctx->interface->chunksize=0;
 libswdappctx->interface->initialized=0;

 // Load selected interface configuration.
 strncpy(libswdappctx->interface->name,libswdapp_interface_configs[interface_number].name,LIBSWDAPP_INTERFACE_NAME_MAXLEN);
 strncpy(libswdappctx->interface->description,libswdapp_interface_configs[interface_number].description,LIBSWDAPP_INTERFACE_NAME_MAXLEN);
 libswdappctx->interface->init           = libswdapp_interface_configs[interface_number].init;
 libswdappctx->interface->deinit         = libswdapp_interface_configs[interface_number].deinit;
 libswdappctx->interface->set_freq       = libswdapp_interface_configs[interface_number].set_freq;
 libswdappctx->interface->bitbang        = libswdapp_interface_configs[interface_number].bitbang;
 libswdappctx->interface->transfer_bits  = libswdapp_interface_configs[interface_number].transfer_bits;
 libswdappctx->interface->transfer_bytes = libswdapp_interface_configs[interface_number].transfer_bytes;
 libswdappctx->interface->vid            = libswdapp_interface_configs[interface_number].vid;
 libswdappctx->interface->pid            = libswdapp_interface_configs[interface_number].pid;
 libswdappctx->interface->latency        = libswdapp_interface_configs[interface_number].latency;
 libswdappctx->interface->maxfrequency   = libswdapp_interface_configs[interface_number].maxfrequency;
 libswdappctx->interface->frequency      = libswdapp_interface_configs[interface_number].frequency;
 libswdappctx->interface->chunksize      = libswdapp_interface_configs[interface_number].chunksize;
 libswdappctx->interface->sigsetupstr    = libswdapp_interface_configs[interface_number].sigsetupstr;
 if (libswdappctx->interface->vid_forced)
  libswdappctx->interface->vid           = libswdappctx->interface->vid_forced;
 if (libswdappctx->interface->pid_forced)
  libswdappctx->interface->pid           = libswdappctx->interface->pid_forced;
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "OK\n");

 // Call the interface specific binary initialization routine.
 retval=libswdappctx->interface->init(libswdappctx);
 if (retval!=LIBSWD_OK)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Interface specific intialization routine failed!\n" );
  return retval;
 }

 libswdappctx->interface->initialized=1;
 return retval;
}


/******************************************************************************
 * INTERFACE SIGNAL INFRASTRUCTURE AND OPERATIONS                             *
 ******************************************************************************/

/** Check if specified signal is already defined (case insensitive).
 * Return pointer to the signal structure if found.
 * \param *name signal name to check
 * \return pointer to signal structure in memory if found, NULL otherwise.
 */
libswdapp_interface_signal_t *libswdapp_interface_signal_find(libswdapp_context_t *libswdappctx, char *name)
{
 // Check if interface signal to already exists
 if (!libswdappctx->interface->signal) return NULL;
 //Check if signal name is correct
 if (!name || *name==' ')
 {
  printf("ERROR: Interface signal name cannot be empty!\n");
  return NULL;
 }
 // Check if signal name already exist
 libswdapp_interface_signal_t *sig;
 sig = libswdappctx->interface->signal;
 while (sig)
 {
  if (!strncasecmp(sig->name, name, LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN))
   return sig;
  sig = sig->next;
 }
 // If signal is not found return null pointer.
 return NULL;
}

/** Add new signal to the interface.
 * Signal will be allocated in memory with provided name and pin mask.
 * Note: Signal definition may take place before interface is ready to operate,
 * therefore value will be not assigned at time of signal creation.
 * Signal value can be set with appropriate 'bitbang' call.
 * The default value for new signal equals provided mask to maintain Hi-Z.
 *
 * \param *name is the signal name (max 32 char).
 * \param mask is the signal mask (unsigned int).
 * \param value is the initial value for signal to set.
 * \return ERROR_OK on success or ERROR_FAIL on failure.
 */
int libswdapp_interface_signal_add(libswdapp_context_t *libswdappctx, char *name, unsigned int mask)
{
 libswdapp_interface_signal_t *newsignal, *lastsignal;
 int snlen;

 libswd_ctx_t *libswdctx=(libswd_ctx_t*)libswdappctx->libswdctx;

 // Check if name is correct string.
 if (!name || *name==' ')
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Interface signal name cannot be empty!\n" );
  return LIBSWD_ERROR_PARAM;
 }
 // Verify signal name length.
 snlen = strnlen(name, 2*LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN);
 if (snlen < LIBSWDAPP_INTERFACE_SIGNAL_NAME_MINLEN || snlen > LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Interface signal name '%s' too short or too long!\n",
             name );
  return LIBSWD_ERROR_PARAM;
 }

 // Check if signal name already exist and return error if so.
 if (libswdapp_interface_signal_find(libswdappctx, name))
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Interface signal '%s' already exist!\n", name);
  return LIBSWD_ERROR_PARAM;
 }

 // Allocate memory for new signal structure.
 newsignal = (libswdapp_interface_signal_t*)calloc(1,sizeof(libswdapp_interface_signal_t));
 if (!newsignal)
  goto libswdapp_interface_signal_add_end;

 newsignal->name = (char*)calloc(snlen+1,sizeof(char)); //Remember about trailing '\0'.
 if (!newsignal->name)
   goto libswdapp_interface_signal_add_end;

 // Initialize structure data and return or break on error.
 if (!strncpy(newsignal->name, name, snlen))
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
             "WARNING: Interface signal cannot copy '%s' name!", name );
  goto libswdapp_interface_signal_add_end;
 }

 newsignal->mask = mask;
 newsignal->value = mask;

 if (!libswdappctx->interface->signal)
 {
  libswdappctx->interface->signal = newsignal;
 }
 else
 {
  lastsignal = libswdappctx->interface->signal;
  while (lastsignal->next) lastsignal=lastsignal->next;
  lastsignal->next=newsignal;
 }
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
            "INFO: Interface signal '%s' added.\n", name );
 return LIBSWD_OK;

 // If there was an error free up resources and return error.
libswdapp_interface_signal_add_end:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "ERROR: Cannot add signal '%s'!\n", name );
 if (newsignal->name) free(newsignal->name);
 if (newsignal) free(newsignal);
 return LIBSWD_ERROR_DRIVER;
}

/** Delete interface signal.
 * Removes signal from singly linked list of interface signals and free memory.
 * \param name is the name of the signal to remove.
 * \return ERROR_OK on success, ERROR_FAIL on failure.
 */
int libswdapp_interface_signal_del(libswdapp_context_t *libswdappctx, char *name)
{
 libswdapp_interface_signal_t *delsig, *prevsig;
 libswd_ctx_t *libswdctx=(libswd_ctx_t*)libswdappctx->libswdctx;

 // Check if interface any signal exist
 if (!libswdappctx->interface->signal)
  return LIBSWD_ERROR_NULLPOINTER;
 // Check if signal name is correct.
 if (!name || *name==' ')
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Interface signal name cannot be empty!\n" );
  return LIBSWD_ERROR_DRIVER;
 }
 // See if we want to remove all signals ('*' name).
 if (strchr(name,'*'))
 {
  for (delsig=libswdappctx->interface->signal;delsig;delsig=delsig->next)
  {
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
              "INFO: Removing Interface Signal '%s'...", delsig->name );
   free(delsig->name);
   free(delsig);
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "OK\n");
  }
  libswdappctx->interface->signal=NULL;
  return LIBSWD_OK;
 }
 // look for the signal name on the list.
 delsig = libswdapp_interface_signal_find(libswdappctx, name);
 // return error if signal is not on the list.
 if (!delsig)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Interface signal '%s' not found!", name );
  return LIBSWD_ERROR_DRIVER;
 }
 // detach signal to be removed from the list.
 prevsig = libswdappctx->interface->signal;
 if (prevsig == delsig)
 {
  // we need to detach first signal on the list.
  if (prevsig->next)
   libswdappctx->interface->signal = prevsig->next;
   else libswdappctx->interface->signal=NULL;
 }
 else
 {
  for (; prevsig->next; prevsig = prevsig->next)
  {
   if (prevsig->next == delsig)
   {
    if (prevsig->next->next)
     prevsig->next = prevsig->next->next;
    else prevsig->next=NULL;
    break;
   }
  }
 }
 // now free memory of detached element.
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
            "INFO: Removing Interface Signal '%s'...", name );
 free(delsig->name);
 free(delsig);
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO, "OK\n");
 return LIBSWD_OK;
}






/******************************************************************************
 * INTERFACE DATA TRANSFER INFRASTRUCTURE AND OPERATIONS                      *
 ******************************************************************************/

/** Generic IO BITBANG Port Manipulation Routine.
 * It can read and write port state using signal names. Each interface have its
 * own specific signal names and fields. This function works on those fields
 * and based on their values talks to the FT*232 chip on the interface device.
 * ft2232 drivers use {low,high}_{output,direction} global variables to remember
 * port direction and value, so we need to work on them as well not to change
 * any other pin with our bit-baning performed only on selected pins.
 * The function name 'bitbang' reflects ability to change selected pin states.
 *
 * @Note: FT2232 has special mechanism called MPSSE for serial communications
 * that is far more efficient than pure 'bitbang' mode on this device family.
 * Although our function is named 'bitbang' it does not use bitbang mode.
 * MPSSE command send value and port bytes on port write, but does not on read.
 * This happens every time we want to change pin value, so we need to use cache.
 * On write we want to OR direction mask already set by init() procedure
 * to mark bit-mask output. On read we want to clear bits given by mask
 * to mark them input. To read we need to write/update port state first.
 * Long story short: to read data we first need to set pins to input.
 *
 * @Warning: reading and writing will set pin direction input or output,
 * so it is possible to disable basic data output pins with bad masking,
 * but also gives chance to create and manage full TCL signal description,
 * that can be used to take advantage of some additional interface hardware
 * features installed on some devices (i.e. ADC, power supply, etc).
 * This gives new way of signal handling that is still backward-compatible.
 *
 * \param *device void pointer to pass additional driver information to the routine.
 * \param signal is the string representation of the signal mask stored in layout structure.
 * \param GETnSET if zero then perform read operation, write otherwise.
 * \param *value is the pointer that holds the value.
 * \return ERROR_OK on success, or ERROR_FAIL on failure.
 * TODO: Bitbang Read must also update other bits status, otherwise cached values are invalid and abiguous!!!
 */
int libswdapp_interface_ftdi_bitbang(libswdapp_context_t *libswdappctx, unsigned int bitmask, int GETnSET, unsigned int *value)
{
 unsigned char  buf[3];
 int retval, retry;
 int bytes_written, bytes_read;
 unsigned int vall=0, valh=0, gpioval=0, gpiodir=0;
 struct ftdi_context *ftdictx=(struct ftdi_context*)libswdappctx->interface->ctx;

 if (!GETnSET) {
  // We will SET port pin values for selected bitmask.
  // Modify our pins value, but remember about other pins and their previous value.
  gpioval = (libswdappctx->interface->gpioval & ~bitmask) | (*value & bitmask);
  // Modify our pins direction, but remember about other pins and their previous direction.
  gpiodir = libswdappctx->interface->gpiodir | bitmask;
  // Now send those settings to the interface chip.
  buf[0] = 0x80;  // Set Data Bits LowByte.
  buf[1] = gpioval&0x00ff;
  buf[2] = gpiodir&0x00ff;
  bytes_written = ftdi_write_data(ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3) return bytes_written;
  buf[0] = 0x82;   // Set Data Bits HighByte.
  buf[1] = (gpioval>>8)&0x00ff;
  buf[2] = (gpiodir>>8)&0x00ff;
  bytes_written = ftdi_write_data(ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3)
  {
   libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
              "ERROR: Interface bitbang error!\n" );
   return bytes_written;
  }
  libswdappctx->interface->gpioval=gpioval;
  libswdappctx->interface->gpiodir=gpiodir;
  *value = gpioval&bitmask;
 } else {
  // Modify our pins value, but remember about other pins and their previous value.
  // DO WE REALLY NEED TO PULL-UP PINS TO READ THEIR STATE OR SIMPLY LEAVE AS IS?.
  // low_output  = (low_output & ~sigmask) | (sigmask & 0x0ff);
  // high_output = (high_output & ~sigmask) | (sigmask>>8) & 0x0ff);
  // Modify our pins direction to input, but remember about other pins and their previous direction.
  gpiodir = libswdappctx->interface->gpiodir & ~bitmask;
  gpioval = libswdappctx->interface->gpioval;
  // Now send those settings to the interface chip.
  // First change desired pins to input.
  buf[0] = 0x80;  // Set Data Bits LowByte.
  buf[1] = gpioval&0x00ff;
  buf[2] = gpiodir&0x00ff;
  bytes_written = ftdi_write_data(ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3)
  {
   libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
              "ERROR: libswdapp_interface_bitbang(): ftdi_write_data() returns invalid bytes count: %d\n",
              bytes_written );
   return LIBSWD_ERROR_DRIVER;
  }
  buf[0] = 0x82;   // Set Data Bits HighByte.
  buf[1] = (gpioval>>8)&0x00ff;
  buf[2] = (gpiodir>>8)&0x00ff;
  bytes_written = ftdi_write_data(ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3)
  {
   libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
              "ERROR: libswdapp_interface_bitbang(): ftdi_write_data() returns invalid bytes count: %d\n",
              bytes_written);
   return LIBSWD_ERROR_DRIVER;
  }
  // Then read pins designated by a signal mask.
  buf[0] = 0x81;    // Read Data Bits LowByte.
  bytes_written = ftdi_write_data(ftdictx, buf, 1);
  if (bytes_written<0 || bytes_written!=1)
  {
   libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
              "ERROR: libswdapp_interface_bitbang(): ftdi_write_data() returns invalid bytes count: %d\n",
              bytes_written);
   return LIBSWD_ERROR_DRIVER;
  }
  for (retry=0;retry<LIBSWD_RETRY_COUNT_DEFAULT;retry++)
  {
   bytes_read = ftdi_read_data(ftdictx, (unsigned char*)&vall, 1);
   if (bytes_read>0) break;
  }
  if (bytes_read<0 || bytes_read!=1)
  {
   libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
              "ERROR: libswdapp_interface_bitbang(): ftdi_read_data() returns invalid bytes count: %d\n",
              bytes_read );
   return LIBSWD_ERROR_DRIVER;
  }
  buf[0] = 0x83;    // Read Data Bits HighByte.
  bytes_written = ftdi_write_data(ftdictx, buf, 1);
  if (bytes_written<0 || bytes_written!=1)
  {
   libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
              "ERROR: libswdapp_interface_bitbang(): ftdi_write_data() returns invalid bytes count: %d\n",
              bytes_written );
   return LIBSWD_ERROR_DRIVER;
  }
  for (retry=0;retry<LIBSWD_RETRY_COUNT_DEFAULT;retry++)
  {
   bytes_read = ftdi_read_data(ftdictx, (unsigned char*)&valh, 1);
   if (bytes_read>0) break;
  }
  if (bytes_read<0 || bytes_read!=1)
  {
   libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
              "ERROR: libswdapp_interface_bitbang(): ftdi_read_data() returns invalid bytes count: %d\n",
              bytes_read );
   return LIBSWD_ERROR_DRIVER;
  }
  *value = ((valh << 8) | vall) & bitmask; // Join result bytes and apply signal bitmask.
  libswdappctx->interface->gpioval = (libswdappctx->interface->gpioval & ~bitmask) | (*value & bitmask);
  libswdappctx->interface->gpiodir = gpiodir;
 }
 return LIBSWD_OK;
}

/** Transfer bits in/out stored in char array each bit starting from LSB first
 * or MSB first, alternatively if you want to make MSB-first shift on
 * LSB-first mode put data in reverse order into input/output array.
 * \param *device void pointer to pass driver details to the function.
 * \param bits is the number of bits (char array elements) to transfer.
 * \param *mosidata pointer to char array with data to be send.
 * \param *misodata pointer to char array with data to be received.
 * \param nLSBfirst if zero shift data LSB-first, otherwise MSB-first.
 * \return number of bits sent on success, or ERROR_FAIL on failure.
 */
int libswdapp_interface_ftdi_transfer_bits(libswdapp_context_t *libswdappctx, int bits, char *mosidata, char *misodata, int nLSBfirst)
{
 static unsigned char buf[65539], databuf;
 int i, retval, bit=0, byte=0, bytes=0, retry;
 int bytes_written, bytes_read;
 struct ftdi_context *ftdictx=(struct ftdi_context*)libswdappctx->interface->ctx;

 if (bits>65535)
 {
  libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Cannot transfer more than 65536 bits at once!\n");
  return LIBSWD_ERROR_DRIVER;
 }

 if (bits>=8)
 {
  // Try to pack as many bits into bytes for better performance.
  bytes=bits/8;
  bytes--;                        // MPSSE starts counting bytes from 0.
  buf[0] = (nLSBfirst)?0x31:0x39; // Clock Bytes In and Out LSb or MSb first.
  buf[1] = (char)bytes&0x0ff;
  buf[2] = (char)((bytes>>8)&0x0ff);
  bytes++;
  // Fill in the data buffer.
  for (byte=0;byte*8<bits;byte++)
  {
   databuf = 0;
   for (i=0;i<8;i++) databuf|=mosidata[byte*8+i]?(1<<i):0;
   buf[byte+3]=databuf;
  }
  bytes_written = ftdi_write_data(ftdictx, buf, bytes+3);
  if (bytes_written<0 || bytes_written!=(bytes+3))
  {
   // TODO: LibFTDI transfer failed, try to know why!
   libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
              "ERROR: libswdapp_interface_transfer_bits(): ft2232_write() returns %d not %d!\n",
              bytes_written, bytes+3 );
   return LIBSWD_ERROR_DRIVER;
  }
  // This retry is necessary because sometimes FTDI Chip returns 0 bytes.
  for (retry=0;retry<LIBSWD_RETRY_COUNT_DEFAULT;retry++)
  {
   bytes_read=ftdi_read_data(ftdictx, (unsigned char*)buf, bytes);
   if (bytes_read>0) break;
  }
  if (bytes_read<0 || bytes_read!=bytes)
  {
   libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
              "ERROR: libswdapp_interface_transfer_bits(): ft2232_read() returns %d instead %d!\n",
              bytes_read, bytes+3 );
   return LIBSWD_ERROR_DRIVER;
  }
  // Explode read bytes into bit array.
  for (byte=0;byte*8<bits;byte++)
   for (bit=0;bit<8;bit++)
    misodata[byte*8+bit]=buf[byte]&(1<<bit)?1:0;
 }

 // Now send remaining bits that cannot be packed as bytes.
 // Because "Clock Data Bits In and Out LSB/MSB" of FTDI is a mess, pack single
 // bit read/writes into buffer and then flush it using single USB transfer.
 for (bit=bytes*8;bit<bits;bit++)
 {
  buf[3*bit+0] = (nLSBfirst)?0x33:0x3b; // Clock Bits In and Out LSb or MSb first.
  buf[3*bit+1] = 0;                     // One bit per element.
  buf[3*bit+2] = mosidata[bit]?0xff:0;  // Take data from supplied array.
 }
 bytes_written = ftdi_write_data(ftdictx,buf,3*(bits-(bytes*8)));
 if (bytes_written<0)
 {
  libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: libswdapp_interface_transfer_bits(): ft2232_write() returns invalid bytes count: %d\n",
             bytes_written );
  return LIBSWD_ERROR_DRIVER;
 }
 // This retry is necessary because sometimes FTDI Chip returns 0 bytes.
 for (retry=0;retry<LIBSWD_RETRY_COUNT_DEFAULT;retry++)
 {
  bytes_read=ftdi_read_data(ftdictx, (unsigned char*)buf, bits-(bytes*8));
  if (bytes_read>0) break;
 }
 if (bytes_read<0 || bytes_read!=(bits-(bytes*8)))
 {
  libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: libswdapp_interface_transfer_bits(): ft2232_read() returns invalid bytes count: %d\n",
             bytes_read );
  return LIBSWD_ERROR_DRIVER;
 }
 // FTDI MPSSE returns shift register value, our bit is MSb.
 for (bit=bytes*8;bit<bits;bit++)
 {
  misodata[bit]=(buf[bit]&(nLSBfirst?0x01:0x80))?1:0;
  // USE THIS FOR WIRE-LEVEL DEBUG */
  //printf("\n===TRANSFER: Bit %d read 0x%02X written 0x%02X\n", bit, misodata[bit], mosidata[bit]);
 }
 return bit;
}

/** Transfer bytes in/out stored in char array starting from LSB first
 * or MSB first, alternatively if you want to make MSB-first shift on
 * LSB-first mode put data in reverse order into input/output array.
 * \param *device void pointer to pass driver details to the function.
 * \param bits is the number of bits (char array elements) to transfer.
 * \param *mosidata pointer to char array with data to be send.
 * \param *misodata pointer to char array with data to be received.
 * \param nLSBfirst if zero shift data LSB-first, otherwise MSB-first.
 * \return number of bits sent on success, or ERROR_FAIL on failure.
 */
int libswdapp_interface_ftdi_transfer_bytes(libswdapp_context_t *libswdappctx, int bytes, char *mosidata, char *misodata, int nLSBfirst)
{
 static unsigned char buf[65539], databuf;
 int i, retval, byte=0, retry;
 int bytes_written, bytes_read;
 struct ftdi_context *ftdictx=(struct ftdi_context*)libswdappctx->interface->ctx;

 if (bytes>65535)
 {
  libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Cannot transfer more than 65536 bits at once!\n");
  return LIBSWD_ERROR_DRIVER;
 }

 bytes--;                        // MPSSE starts counting bytes from 0.
 buf[0] = (nLSBfirst)?0x31:0x39; // Clock Bytes In and Out MSb or LSb first.
 buf[1] = (char)bytes&0x0ff;
 buf[2] = (char)((bytes>>8)&0x0ff);
 bytes++;
 // Fill in the data buffer.
 for (byte=0;byte<bytes;byte++) buf[byte+3]=*mosidata;
 bytes_written = ftdi_write_data(ftdictx, buf, bytes+3);
 if (bytes_written<0 || bytes_written!=(bytes+3))
 {
  libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: libswdapp_interface_transfer_bytes(): ft2232_write() returns %d\n",
             bytes_written );
  return LIBSWD_ERROR_DRIVER;
 }
 // This retry is necessary because sometimes FTDI Chip returns 0 bytes.
 for (retry=0;retry<LIBSWD_RETRY_COUNT_DEFAULT;retry++)
 {
  bytes_read=ftdi_read_data(ftdictx, buf, bytes);
  if (bytes_read>0) break;
 }
 if (bytes_read<0 || bytes_read!=bytes)
 {
  libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: libswdapp_interface_transfer_bytes(): ft2232_read() returns %d\n",
             bytes_read );
  return LIBSWD_ERROR_DRIVER;
 }
 // Return incoming data.
 for (byte=0;byte<bytes;byte++) misodata[byte]=buf[byte];
 return byte;
}

int libswdapp_interface_ftdi_init(libswdapp_context_t *libswdappctx)
{
 int retval;
 int ftdi_channel=INTERFACE_ANY;
 unsigned char latency_timer;
 static unsigned int port_direction, port_value;
 libswd_ctx_t *libswdctx=(libswd_ctx_t*)libswdappctx->libswdctx;
 struct ftdi_context *ftdictx=(struct ftdi_context*)libswdappctx->interface->ctx;

 // Initialize LibFTDI Context.
 if (libswdappctx->loglevel)
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "Initializing LibFTDI..." );
 ftdictx=ftdi_new();
 if (ftdictx==NULL)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Cannot initialize LibFTDI!\n" );
  return LIBSWD_ERROR_DRIVER;
 } else if (libswdappctx->loglevel) libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "OK\n");
 libswdappctx->interface->ctx=(void*)ftdictx;

 // Open FTDI interface with given VID:PID pair.
 if (libswdappctx->loglevel)
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
             "Opening FTDI interface USB[%04X:%04X]...",
             libswdappctx->interface->vid,
             libswdappctx->interface->pid );
 retval=ftdi_usb_open(ftdictx,
                      libswdappctx->interface->vid,
                      libswdappctx->interface->pid
                     );
 if (retval<0)
 {
  if (libswdappctx->loglevel)
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "FAILED (%s)\n",
           ftdi_get_error_string(ftdictx) );
  return LIBSWD_ERROR_DRIVER;
 }
 else
 {
  if (ftdictx->type==TYPE_R)
  {
   unsigned int chipid;
   retval=ftdi_read_chipid(ftdictx, &chipid);
   if (retval<0)
   {
    if (libswdappctx->loglevel)
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "OK\n");
   }
   else if (libswdappctx->loglevel)
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "OK (ChipId=%X)\n", chipid);
  } else if (libswdappctx->loglevel)
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "OK\n");
 }
 // Reeset the FTDI device.
 if (ftdi_usb_reset(ftdictx)<0)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Unable to reset ftdi device!\n" );
  return LIBSWD_ERROR_DRIVER;
 }
 // Reset Command Controller
 if(ftdi_set_bitmode(ftdictx, 0, BITMODE_RESET)<0)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Cannot (re)set bitmode for FTDI interface!\n" );
  return LIBSWD_ERROR_DRIVER;
 }
 // Set interface channel to use.
 if (ftdi_channel == INTERFACE_ANY) ftdi_channel = INTERFACE_A;
 if (ftdi_set_interface(ftdictx, ftdi_channel)<0)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Unable to select FT2232 channel A: %s\n",
             ftdictx->error_str );
  return LIBSWD_ERROR_DRIVER;
 }
 // Set the Latency Timer.
 if (ftdi_set_latency_timer(ftdictx,libswdappctx->interface->latency)<0)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Unable to set latency timer!\n" );
  return LIBSWD_ERROR_DRIVER;
 }
 if (ftdi_get_latency_timer(ftdictx,&latency_timer)<0)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Unable to get latency timer!\n" );
  return LIBSWD_ERROR_DRIVER;
 } else libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,"FTDI latency timer is: %i\n", latency_timer);
 // Set MPSSE mode for FT2232 chip.
 retval=ftdi_set_bitmode(ftdictx, 0, BITMODE_MPSSE);
 if (retval<0)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Cannot set bitmode BITMODE_MPSSE for '%s' interface!\n",
             libswdappctx->interface->name );
  return LIBSWD_ERROR_DRIVER;
 }
 // Set large chunksize for faster transfers of large data.
 retval=ftdi_write_data_set_chunksize(ftdictx, libswdappctx->interface->chunksize);
  if (retval<0)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Cannot set %d write chunksize for '%s' interface!\n",
             libswdappctx->interface->chunksize,
             libswdappctx->interface->name );
  return LIBSWD_ERROR_DRIVER;
 }
 retval=ftdi_read_data_set_chunksize(ftdictx, libswdappctx->interface->chunksize);
  if (retval<0)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Cannot set %d read chunksize for '%s' interface!\n",
             libswdappctx->interface->chunksize,
             libswdappctx->interface->name );
  return LIBSWD_ERROR_DRIVER;
 }
 // Set default interface speed/frequency
 libswdapp_interface_ftdi_set_freq(libswdappctx, libswdappctx->interface->frequency);

 // Call the interface specific signal configuration command string.
 retval=libswdapp_handle_command_signal(libswdappctx, libswdappctx->interface->sigsetupstr);
 if (retval!=LIBSWD_OK)
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Cannot setup Interface specific signals!\n" );
  return retval;
 }

 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
            "FTDI MPSSE initialization complete!\n");
 return LIBSWD_OK;
}



/** Set interface frequency in Hz.
 * Warning: Passing zero as freq parameter means maximum available frequency.
 * \param *libswdappctx pointer to the LibSWD Application Context.
 * \param freq desired frequency in Hz (0 means maximum frequency).
 * \return LIBSWD_OK on success, negative error code otherwise.
 */
int libswdapp_interface_ftdi_set_freq(libswdapp_context_t *libswdappctx, int freq)
{
 unsigned int reg, maxfreq;
 unsigned char buf[3];
 char bytes_written;
 struct ftdi_context *ftdictx=(struct ftdi_context*)libswdappctx->interface->ctx;
 if (!libswdappctx || !libswdappctx->interface || !libswdappctx->interface->ctx)
  return LIBSWD_ERROR_NULLPOINTER;
 if (freq<0)
 {
  libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Invalid interface frequency value '%d'!\n", freq);
  return LIBSWD_ERROR_PARAM;
 }
 maxfreq=libswdappctx->interface->maxfrequency;
 if (!maxfreq) maxfreq=6000000;
 if (freq!=0)
 {
  reg=(((maxfreq*2)/freq)-1)/2;
 } else reg=0;
 buf[0] = 0x86;
 buf[1] = reg&0x0ff;
 buf[2] = (reg>>8)&0xff;
 bytes_written = ftdi_write_data(ftdictx, buf, 3);
 if (bytes_written<0 || bytes_written!=3) return bytes_written;
 libswdappctx->interface->frequency=freq;
 libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_INFO,
            "INFO: Interface frequency set to %d\n", freq);
 return LIBSWD_OK;
}


//KT-LINK Interface Init
int libswdapp_interface_ftdi_init_ktlink(libswdapp_context_t *libswdappctx)
{
 int retval;
 unsigned char buf[4];
 struct ftdi_context *ftdictx;

 retval=libswdapp_interface_ftdi_init(libswdappctx);
 if (retval<0) return retval;
 ftdictx=(struct ftdi_context*)libswdappctx->interface->ctx;

 libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_INFO,
            "INFO: Disabling CLK/5 (set max CLK=30MHz)...");
 buf[0]=0x8A;
 retval=ftdi_write_data(ftdictx, buf, 1);
 if (retval<0 || retval!=1)
 {
  libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_INFO,
             "FAILED!\n");
  libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "ERROR: Cannot switch off clock divisor!\n");
  return retval;
 }
 libswdappctx->interface->maxfrequency=30000000;
 libswd_log(libswdappctx->libswdctx, LIBSWD_LOGLEVEL_INFO, "OK\n");

 return LIBSWD_OK;
}

// Generic FTDI interface deinit routine (all GPIO=Input=HI-Z)
// Free LibFTDI context.
int libswdapp_interface_ftdi_deinit(libswdapp_context_t *libswdappctx)
{
 unsigned int dir=0,val;
 struct ftdi_context *ftdictx=(struct ftdi_context*)libswdappctx->interface->ctx;
 libswdappctx->interface->bitbang(libswdappctx, dir, 1, &val);
 ftdi_deinit(ftdictx);
 return LIBSWD_OK;
}



/******************************************************************************
 * BRIDGE FUNCTIONS BETWEEN LIBSWD AND INTERFACE OPERATIONS                   *
 ******************************************************************************/

/**
 * Driver code to write 8-bit data (char type).
 * MOSI (Master Output Slave Input) is a SWD Write Operation.
 *
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char data.
 * \bits tells how many bits to send (at most 8).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
ar)*/
int libswd_drv_mosi_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst)
{
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_drv_mosi_8(libswdctx=@%p, cmd=@%p, data=@%p, bits=%d, nLSBfirst=0x%02X)\n",
            (void *)libswdctx, (void *)cmd, *data, bits, nLSBfirst
           );

 if (data == NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 || bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[8], mosidata[8];
 libswdapp_interface_t *interface=(libswdapp_interface_t*)libswdctx->driver->interface;

 // Split output data into char array.
 for (i=0;i<8;i++)
  mosidata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?i:(bits-1-i)]=((1<<i)&(*data))?1:0;
 // Then send that array into interface hardware.
 res=interface->transfer_bits(libswdctx->driver->ctx,bits,mosidata,misodata,0);
 if (res<0) return LIBSWD_ERROR_DRIVER;

 return res;
}

/**
 * Driver code to write 32-bit data (int type).
 * MOSI (Master Output Slave Input) is a SWD Write Operation.
 *
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char buffer array.
 * \bits tells how many bits to send (at most 32).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_mosi_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst)
{
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_drv_mosi_32(libswdctx=@%p, cmd=@%p, data=0x%08X, bits=%d, nLSBfirst=0x%02X)\n",
            (void *)libswdctx, (void *)cmd, *data, bits, nLSBfirst
           );

 if (data == NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 || bits>32) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 static unsigned int i;
 static signed int res;
 static char misodata[32], mosidata[32];
 libswdapp_interface_t *interface=(libswdapp_interface_t*)libswdctx->driver->interface;

 // UrJTAG drivers shift data LSB-First.
 for (i=0;i<32;i++)
  mosidata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?i:(bits-1-i)]=((1<<i)&(*data))?1:0;
 res=interface->transfer_bits(libswdctx->driver->ctx,bits,mosidata,misodata,0);
 if (res<0) return LIBSWD_ERROR_DRIVER;

 return res;
}

/**
 * MISO (Master Input Slave Output) is a SWD Read Operation.
 *
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char buffer array.
 * \bits tells how many bits to send (at most 8).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_miso_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst)
{
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 || bits>8) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 static int i;
 static signed int res;
 static char misodata[8], mosidata[8];
 libswdapp_interface_t *interface=(libswdapp_interface_t*)libswdctx->driver->interface;

 res=interface->transfer_bits(libswdctx->driver->ctx,bits,mosidata,misodata,LIBSWD_DIR_LSBFIRST);
 if (res<0) return LIBSWD_ERROR_DRIVER;
 // Now we need to reconstruct the received LSb data byte into  byte array.
 *data = 0;
 for (i=0;i<bits;i++)
  *data|=misodata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?i:(bits-1-i)]?(1<<i):0;
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_drv_miso_8(libswdctx=@%p, cmd=@%p, data=@%p, bits=%d, nLSBfirst=0x%02X) reads: 0x%02X\n",
           (void *)libswdctx, (void *)cmd, (void *)data, bits, nLSBfirst, *data
           );

 return res;
}

/**
 * Driver code to read 32-bit data (int type).
 * MISO (Master Input Slave Output) is a SWD Read Operation.
 *
 * \param *libswdctx swd context to work on.
 * \param *cmd point to the actual command being sent.
 * \param *data points to the char buffer array.
 * \bits tells how many bits to send (at most 32).
 * \bits nLSBfirst tells the shift direction: 0 = LSB first, other MSB first.
 * \return data count transferred, or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_miso_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst)
{
 if (data==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bits<0 || bits>32) return LIBSWD_ERROR_PARAM;
 if (nLSBfirst!=0 && nLSBfirst!=1) return LIBSWD_ERROR_PARAM;

 static int i;
 static signed int res;
 static char misodata[32], mosidata[32];
 libswdapp_interface_t *interface=(libswdapp_interface_t*)libswdctx->driver->interface;

 res = interface->transfer_bits(libswdctx->driver->ctx, bits, mosidata, misodata, LIBSWD_DIR_LSBFIRST);
 if (res<0) return LIBSWD_ERROR_DRIVER;
 // Now we need to reconstruct the data byte from shifted in LSBfirst byte array.
 *data = 0;
 for (i=0;i<bits;i++)
  *data|=(misodata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?i:(bits-1-i)]?(1<<i):0);
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_drv_miso_32() reads: 0x%08X\n", *data
           );

 return res;
}

/**
 * This function sets interface buffers to MOSI direction.
 * MOSI (Master Output Slave Input) is a SWD Write operation.
 * Driver must support "RnW" signal to drive output buffers for TRN.
 *
 * \param *libswdctx is the swd context to work on.
 * \param bits specify how many clock cycles must be used for TRN.
 * \return number of bits transmitted or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_mosi_trn(libswd_ctx_t *libswdctx, int bits)
{
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_drv_mosi_trn(libswdctx=@%p, bits=%d)\n",
            (void *)libswdctx, bits
           );
 libswdapp_interface_t *interface=(libswdapp_interface_t*)libswdctx->driver->interface;

 if (bits<LIBSWD_TURNROUND_MIN_VAL || bits>LIBSWD_TURNROUND_MAX_VAL)
  return LIBSWD_ERROR_TURNAROUND;

 libswdapp_interface_signal_t *sig;
 if (!(sig=libswdapp_interface_signal_find(libswdctx->driver->ctx, "RnW")))
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "LIBSWD_E: libswd_drv_mosi_trn(libswdctx=@%p, bits=%d): Mandatory Interface Signal 'RnW' not defined!\n",
             (void *)libswdctx, bits
            );
  return LIBSWD_ERROR_DRIVER;
 }

 static int res;
 static unsigned int val = 0;
 static char buf[LIBSWD_TURNROUND_MAX_VAL];
 // Use driver method to set low (write) signal named RnW.
 res = interface->bitbang(libswdctx->driver->ctx, sig->mask, 0, &val);
 if (res < 0) return LIBSWD_ERROR_DRIVER;

 // Clock specified number of bits for proper TRN transaction.
 res = interface->transfer_bits(libswdctx->driver->ctx, bits, buf, buf, 0);
 if (res < 0) return LIBSWD_ERROR_DRIVER;

 return bits;
}

/**
 * This function sets interface buffers to MISO direction.
 * MISO (Master Input Slave Output) is a SWD Read operation.
 * Interface driver is kept in (libswd_driver_t) libswdctx->driver.
 * Driver must support "RnW" signal to drive output buffers for TRN.
 *
 * \param *libswdctx is the swd context to work on.
 * \param bits specify how many clock cycles must be used for TRN.
 * \return number of bits transmitted or negative LIBSWD_ERROR code on failure.
 */
int libswd_drv_miso_trn(libswd_ctx_t *libswdctx, int bits)
{
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_drv_miso_trn(libswdctx=@%p, bits=%d)\n",
            (void *)libswdctx, bits
           );
 libswdapp_interface_t *interface=(libswdapp_interface_t*)libswdctx->driver->interface;

 if (bits<LIBSWD_TURNROUND_MIN_VAL || bits>LIBSWD_TURNROUND_MAX_VAL)
  return LIBSWD_ERROR_TURNAROUND;

 libswdapp_interface_signal_t *sig;
 if (!(sig=libswdapp_interface_signal_find(libswdctx->driver->ctx, "RnW")))
 {
  libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
             "LIBSWD_E: libswd_drv_miso_trn(libswdctx=@%p, bits=%d): Mandatory Interface Signal 'RnW' not defined!\n",
             (void *)libswdctx, bits
            );
  return LIBSWD_ERROR_DRIVER;
 }

 static int res;
 static unsigned int val = 1;
 static char buf[LIBSWD_TURNROUND_MAX_VAL];

 // Use driver method to set high (read) signal named RnW.
 res = interface->bitbang(libswdctx->driver->ctx, sig->mask, 1, &val);
 if (res < 0) return LIBSWD_ERROR_DRIVER;

 // Clock specified number of bits for proper TRN transaction.
 res = interface->transfer_bits(libswdctx->driver->ctx, bits, buf, buf, 0);
 if (res < 0) return LIBSWD_ERROR_DRIVER;

 return bits;
}


/******************************************************************************
 * LOG OPERATIONS                                                             *
 ******************************************************************************/

/* We want to use internal logging. */
int libswd_log(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...){
 int retval;
 va_list ap;
 va_start(ap, msg);
 retval=libswd_log_internal_va(libswdctx, loglevel, msg, ap);
 va_end(ap);
 return retval;
};

int libswd_log_flush(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...){
 int retval;
 va_list ap;
 va_start(ap, msg);
 retval=libswd_log_internal_va(libswdctx, loglevel, msg, ap);
 va_end(ap);
 fflush(stdout);
 return retval;
};

/******************************************************************************
 * LIBUSB BASED ASYNCHRONOUS INTERFACE DRIVER FOR FTDI CHIPS                  *
 ******************************************************************************/

static int libswdapp_interface_aftdi_init(libswdapp_context_t *libswdappctx)
{
 return LIBSWD_ERROR_UNSUPPORTED;
}

static int libswdapp_interface_aftdi_deinit(libswdapp_context_t *libswdappctx)
{
 return LIBSWD_ERROR_UNSUPPORTED;
}

static int libswdapp_interface_aftdi_init_ktlink(libswdapp_context_t *libswdappctx)
{
 return LIBSWD_ERROR_UNSUPPORTED;
}

static int libswdapp_interface_aftdi_set_freq(libswdapp_context_t *libswdappctx, int freq)
{
 return LIBSWD_ERROR_UNSUPPORTED;
}

static int libswdapp_interface_aftdi_bitbang(libswdapp_context_t *libswdappctx, unsigned int bitmask, int GETnSET, unsigned int *value)
{
 return LIBSWD_ERROR_UNSUPPORTED;
}

static int libswdapp_interface_aftdi_transfer_bits(libswdapp_context_t *libswdappctx, int bits, char *mosidata, char *misodata, int nLSBfirst)
{
 return LIBSWD_ERROR_UNSUPPORTED;
}

static int libswdapp_interface_aftdi_transfer_bytes(libswdapp_context_t *libswdappctx, int bytes, char *mosidata, char *misodata, int nLSBfirst)
{
 return LIBSWD_ERROR_UNSUPPORTED;
}



/** @} */
