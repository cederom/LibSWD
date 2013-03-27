/*
 * Serial Wire Debug Open Library.
 * Application Body File.
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

/** \file libswd_app.c */

#include <libswd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ftdi.h>
#include <readline/readline.h>
#include <readline/history.h>

/*******************************************************************************
 * \defgroup libswd_app LibSWD Application functions.
 * @{
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

/* Define the interface drivers. */
int libswd_drv_mosi_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst){
 printf("libswd_drv_mosi_8()\n");
 return LIBSWD_OK;
};

int libswd_drv_mosi_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst){
 printf("libswd_drv_mosi_32()\n");
 return LIBSWD_OK;
};

int libswd_drv_miso_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst){
 printf("libswd_drv_miso_8()\n");
 return LIBSWD_OK;
};

int libswd_drv_miso_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst){
 printf("libswd_drv_miso_32()\n");
 return LIBSWD_OK;
};

int libswd_drv_mosi_trn(libswd_ctx_t *libswdctx, int clks){
 printf("libswd_drv_mosi_trn()\n");
 return LIBSWD_OK;
};

int libswd_drv_miso_trn(libswd_ctx_t *libswdctx, int clks){
 printf("libswd_drv_miso_trn()\n");
 return LIBSWD_OK;
};

int libswd_app_print_banner(void){
 printf("*******************************************************************\n");
 printf("* Welcome to LibSWD CLI application! Type '?' or 'help' for help. *\n");
 printf("* See http://libswd.sf.net for more information. (C) CeDeROM 2013 *\n");
 printf("*******************************************************************\n\n");
 return LIBSWD_OK;
}

int libswd_app_print_usage(void){
 printf(" Available options: \n");
 printf("  -l : Use this log level (min=0..6=max)\n");
 printf("  -q : Quiet mode, no verbose output (equals '-l 0')\n");
 printf("  -v : Interface VID (default 0x0403 if not specified)\n");
 printf("  -p : Interface PID (default 0xbbe2 if not specified)\n");
 printf("  -h : Display this help\n");
 printf("\n");
 return LIBSWD_OK;
}

/** LibSWD Application
 * \param *argc program parameter count.
 * \param **argv program invocation parameter values.
 * \return ERROR_OK on success, negative error code otherwise.
 */
int main(int argc, char **argv){
 char *cmd;
 int retval=0, vid=0, pid=0, vid_default=0x0403, pid_default=0xbbe2;
 int i, loglevel=LIBSWD_LOGLEVEL_DEFAULT, ftdi_channel=INTERFACE_ANY;
 libswd_ctx_t *libswdctx;
 struct ftdi_context *ftdi;


 /* Handle program execution arguments. */
 while ( (i=getopt(argc,argv,"hql:p:v:"))!=-1 ) {
  switch (i) {
   case 'q':
    loglevel=LIBSWD_LOGLEVEL_SILENT;
    break;
   case 'l':
    loglevel=atol(optarg);
    break; 
   case 'v':
    vid=strtol(optarg, (char**)NULL, 16);
    break;
   case 'p':
    pid=strtol(optarg, (char**)NULL, 16);
    break;
   case 'h':
   case '?':
   default:
    libswd_app_print_banner();
    libswd_app_print_usage();
    if (optopt=='h'){
     return LIBSWD_OK;
    } else return LIBSWD_ERROR_PARAM;
  }
 }

 /* Set default values and internals. */
 if (loglevel) libswd_app_print_banner();
 if (vid==0) vid=vid_default;
 if (pid==0) pid=pid_default;

 /* Initialize FTDI Interface first. */
 if (loglevel) printf("Initializing LibFTDI...");
 ftdi=ftdi_new();
 if (ftdi==NULL) {
  printf("ERROR: Cannot initialize LibFTDI!\n");
  retval -1;
  goto quit;
 } else if (loglevel) printf("OK\n");


 /* Open FTDI interface with given VID:PID pair. */
 if (loglevel) printf("Opening FTDI interface USB[%04X:%04X]...", vid, pid);
 retval=ftdi_usb_open(ftdi, vid, pid);
 if (retval<0){
  if (loglevel) printf("FAILED (%s)\n", ftdi_get_error_string(ftdi));
  goto quit;
 } else if (loglevel) printf("OK\n");

 /* Initialize LibSWD. */
 libswdctx=libswd_init();
 if (libswdctx==NULL){
  if (loglevel) printf("ERROR: Cannot initialize libswd!\n");
  retval -1;
  goto quit;
 }
 if (LIBSWD_OK!=libswd_log_level_set(libswdctx, loglevel)){
  if (loglevel) printf("Cannot set %s loglevel for LibSWD!\n",\
                       libswd_log_level_string(loglevel));
  goto quit;
 }

 /* Run the Command Line Interpreter loop. */
 while ((cmd=readline("libswd>")) != NULL){
  if ( strncmp(cmd, "q", 1)==0 || strncmp(cmd, "quit", 4)==0 ) break;
  retval=libswd_cli(libswdctx, cmd);
  if (retval!=LIBSWD_OK) if (retval!=LIBSWD_ERROR_CLISYNTAX) goto quit; 
 }

 /* Cleanup and quit. */
quit:
 if (ftdi!=NULL) ftdi_free(ftdi); 
 if (libswdctx!=NULL) libswd_deinit(libswdctx);
 return retval;
}


/** @} */
