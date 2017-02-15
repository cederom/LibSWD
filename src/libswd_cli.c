/*
 * Serial Wire Debug Open Library.
 * Command Line Interface Body File.
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

/** \file libswd_cli.c */

#include <libswd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

/*******************************************************************************
 * \defgroup libswd_cli Command Line Interface functions.
 * @{
 ******************************************************************************/

int libswd_cli_print_usage(libswd_ctx_t *libswdctx)
{
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLPOINTER;
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N: Available LibSWD CLI commands:\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [h]elp / [?]\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [i]nit [dap]|memap|debug\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [l]oglevel <newloglevel>\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [r]ead [d]ap/[a]p 0xAddress\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [w]rite [d]ap/[a]p 0xAddress 0x32BitData\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [r]ead [m]emap 0xAddress <0xCount>|4 <filename>\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [w]rite [m]emap 0xAddress 0xData[]|filename\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "\n");
 return LIBSWD_OK;
}

/** Command Line Interface parse and execution engine.
 * Multiple commands invocation is possible, split by '\n' or ';' characters.
 * \param *libswdctx libswd context
 * \param *cmd command string pointer
 * \return LIBSWD_ERROR_OK on success or negative value error code otherwise.
 */
int libswd_cli(libswd_ctx_t *libswdctx, char *command)
{
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_cli(libswdctx=@%p, command=@%p/\"%s\"): Entring function...\n",
            (void*)libswdctx, (void*)command, command);

 if (libswdctx==NULL) return LIBSWD_ERROR_NULLCONTEXT;
 if (command==NULL) return LIBSWD_ERROR_NULLPOINTER;

 int res, retval, addrstart, addrstop, count, data, *data32, i, j;
 char *cmd, *thiscommand, ap, *filename, *endptr;

 while ( (thiscommand=strsep(&command,"\n;")) )
 {
  while ( (cmd=strsep(&thiscommand," ")) )
  {
   // Strip heading and trailing spaces.
   if (cmd && thiscommand) if (!cmd[0] && thiscommand[0]) continue;
   if (cmd && thiscommand) if (!cmd[0] && !thiscommand[0]) break;

   // Check for HELP invocation.
   if ( strncmp(cmd,"?",1)==0 || strncmp(cmd,"help",4)==0 || strncmp(cmd,"h",1)==0 )
   {
    return libswd_cli_print_usage(libswdctx);
   }

   // Check for LOGLEVEL invocation.
   else if ( strncmp(cmd,"l",1)==0 || strncmp(cmd,"loglevel",8)==0 )
   {
    int loglevel_new=LIBSWD_LOGLEVEL_DEFAULT;
    cmd=strsep(&thiscommand," ");
    if (cmd==NULL)
    {
     libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
                "LIBSWD_I: libswd_cli(): Current loglevel is: %d (%s)\n",
                (int)libswdctx->config.loglevel,
                libswd_log_level_string(libswdctx->config.loglevel) );
    }
    else
    {
     errno=LIBSWD_OK;
     loglevel_new=atol(cmd);
     if (errno==LIBSWD_OK)
     {
      retval=libswd_log_level_set(libswdctx, (libswd_loglevel_t)loglevel_new );
      if (retval<0)
      {
       libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
                  "LIBSWD_E: libswd_cli(): Cannot set loglevel %d/%s): %s.\n",
                  loglevel_new,
                  libswd_log_level_string((libswd_loglevel_t)loglevel_new),
                  libswd_error_string(retval) );
      }
     }
     else if (errno!=LIBSWD_OK)
     {
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
                 "LIBSWD_I: libswd_cli(): Current loglevel is: %d (%s)\n",
                 (int)libswdctx->config.loglevel,
                 libswd_log_level_string(libswdctx->config.loglevel) );
     }
    }
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LOGLEVEL=%d/%s\n",
               (int)libswdctx->config.loglevel,
               libswd_log_level_string(libswdctx->config.loglevel));
    continue;
   }

   // Initialize Target subsystems.
   // This will bring components into known state and remove any pending errors.
   else if ( strncmp(cmd,"i",1)==0 || strncmp(cmd,"init",4)==0 )
   {
    cmd=strsep(&thiscommand," ");
    if (!cmd || strncmp(cmd,"da",2)==0 || strncmp(cmd,"dap",3)==0 )
    {
     int *idcode;
     retval=libswd_dap_init(libswdctx, LIBSWD_OPERATION_EXECUTE, &idcode);
     if (retval<0)
     {
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
                 "LIBSWD_E: libswd_cli(): Cannot Initialize DAP! (%s)\n",
                 libswd_error_string(retval) );
       libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
                 "LIBSWD_N: libswd_cli(): DAP INIT ERROR!\n" );
     } else libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
                       "LIBSWD_N: DAP INIT OK! IDCODE=0x%08X/%s\n",
                       *idcode, libswd_bin32_string(idcode) );
    }
    else if ( strncmp(cmd,"m",1)==0 || strncmp(cmd,"memap",5)==0 )
    {
     retval=libswd_memap_init(libswdctx, LIBSWD_OPERATION_EXECUTE);
     if (retval<0)
     {
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
                 "LIBSWD_E: libswd_cli(): MEM-AP INIT ERROR!\n");
     } else  libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
                        "LIBSWD_N: MEM-AP INIT OK!\n");
    }
    else if ( strncmp(cmd,"de",2)==0 || strncmp(cmd,"debug",5)==0 )
    {
     retval=libswd_debug_init(libswdctx, LIBSWD_OPERATION_EXECUTE);
     if (retval<0)
     {
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
                 "LIBSWD_E: libswd_cli(): DEBUG INIT ERROR!\n" );
     } else libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
                       "LIBSWD_N: DEBUG INIT OK!\n");
    }
    continue;
   }
   // Check for DEBUG invocation.
   else if ( strncmp(cmd,"d",1)==0 || strncmp(cmd,"debug",5)==0 )
   {
    cmd=strsep(&thiscommand," ");
    if (!cmd) goto libswd_cli_syntaxerror;
    if ( strncmp(cmd,"h",1)==0 || strncmp(cmd,"halt",4)==0 )
    {
     retval=libswd_debug_halt(libswdctx, LIBSWD_OPERATION_EXECUTE);
     if (retval<0)
     {
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
                 "LIBSWD_E: libswd_cli(): DEBUG HALT ERROR! (%s).\n",
                 libswd_error_string(retval) );
     }
    }
    else if ( strncmp(cmd,"r",1)==0 || strncmp(cmd,"run",3)==0 )
    {
     retval=libswd_debug_run(libswdctx, LIBSWD_OPERATION_EXECUTE);
     if (retval<0)
     {
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
                 "LIBSWD_E: libswd_cli(); DEBUG RUN ERROR! (%s)\n",
                 libswd_error_string(retval) );
     }
    }
   }
   // Check for READ invocation.
   else if ( strncmp(cmd,"r",1)==0 || strncmp(cmd,"read",4)==0 )
   {
    // Parse access port name parameter.
    cmd=strsep(&thiscommand," ");
    if (!cmd) goto libswd_cli_syntaxerror;
    if ( strncmp(cmd,"d",1)==0 || strncmp(cmd,"dap",3)==0 )
    {
     ap='d';
    }
    else if ( strncmp(cmd,"a",1)==0 || strncmp(cmd,"ap",2)==0 )
    {
     ap='a';
    }
    else if ( strncmp(cmd,"m",1)==0 || strncmp(cmd,"memap",5)==0 )
    {
     ap='m';
    } else goto libswd_cli_syntaxerror;
    // Parse address parameter.
    cmd=strsep(&thiscommand," ");
    if (!cmd) goto libswd_cli_syntaxerror;
    errno=LIBSWD_OK;
    addrstart=strtol(cmd, (char**)NULL, 0);
    if (errno!=LIBSWD_OK) goto libswd_cli_syntaxerror;
    // Perform operations on a given access port.
    switch (ap)
    {
     case 'd':
      retval=libswd_dp_read(libswdctx, LIBSWD_OPERATION_EXECUTE,
                            addrstart, &data32 );
      if (retval<0) goto libswd_cli_error;
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
                 "LIBSWD_N: DP[0x%08X]=0x%08X\n",
                 addrstart, *data32 );
      break;
     case 'a':
      retval=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE,
                            addrstart, &data32 );
      if (retval<0) goto libswd_cli_error;
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
                 "LIBSWD_N: AP[0x%08X]=0x%08X\n",
                 addrstart, *data32 );
      break;
     case 'm':
      // Parse count parameter for MEM-AP operation.
      if (thiscommand)
      {
       cmd=strsep(&thiscommand," ");
       errno=LIBSWD_OK;
       count=strtol(cmd, &endptr, 0);
       if (errno!=LIBSWD_OK || count<=0) goto libswd_cli_syntaxerror;
      } else count=4;
      // Make sure count is 32-bit boundary.
      if (count%4) count=count-(count%4);
      addrstop=addrstart+count;
      // Parse filename parameter for MEM-AP operation.
      if (thiscommand)
      {
       cmd=strsep(&thiscommand," ");
       if (cmd && cmd!='\0' && !isspace(*cmd))
       {
        filename=cmd;
       } else filename=NULL;
      } else filename=NULL;
      // Take care of proper memory (re)allocation.
      if (libswdctx->membuf.size<count)
      {
       if (libswdctx->membuf.data) free(libswdctx->membuf.data);
       libswdctx->membuf.size=count*sizeof(char);
       libswdctx->membuf.data=(unsigned char*)malloc(libswdctx->membuf.size);
       if (!libswdctx->membuf.data)
       {
        libswdctx->membuf.size=0;
        libswd_log(libswdctx, LIBSWD_ERROR_OUTOFMEM,
                   "LIBSWD_E: libswd_cli(): Cannot (re)allocate memory buffer!\n");
        retval=LIBSWD_ERROR_OUTOFMEM;
        goto libswd_cli_error;
       } else memset((void*)libswdctx->membuf.data, 0xFF, libswdctx->membuf.size);
      } else libswdctx->membuf.size=count*sizeof(char);
      // Perform MEM-AP read.
      retval=libswd_memap_read_char_32(libswdctx, LIBSWD_OPERATION_EXECUTE,
                                       addrstart, count,
                                       (char*)libswdctx->membuf.data );
      if (retval<0) goto libswd_cli_error;
      // Store result to a file if requested.
      if (filename)
      {
       FILE *fp;
       fp=fopen(filename,"w");
       if (!fp)
       {
        libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                   "LIBSWD_W: libswd_cli(): Cannot open '%s' data file!\n",
                   filename, strerror(errno) );
        break;
       }
       retval=fwrite(libswdctx->membuf.data, sizeof(char), count, fp);
       if (!retval)
        libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                   "LIBSWD_W: libswd_cli(): Cannot write to data file '%s' (%s)!\n",
                   filename, strerror(errno) );
       retval=fclose(fp);
       if (retval)
       {
        libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                   "LIBSWD_W: libswd_cli(): Cannot close data file '%s' (%s)!\n",
                   filename, strerror(errno) );
       }
      }
      // Print out the result.
      for (i=0; i<count; i=i+16)
      {
       printf("\n%08X: ", i+addrstart);
       for (j=0;j<16&&i+j<libswdctx->membuf.size;j++) printf("%02X ", (unsigned char)libswdctx->membuf.data[i+j]);
       if (j<16) for(;j<16;j++) printf("   ");
       printf(" ");
       for (j=0;j<16&&i+j<libswdctx->membuf.size;j++) printf("%c", isprint(libswdctx->membuf.data[i+j])?libswdctx->membuf.data[i+j]:'.');
       if (j<16) for (;j<16;j++) printf(".");
      }
      printf("\n");
      break;
     default:
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
                 "LIBSWD_E: libswd_cli(): Unknown Access Port given!\n");
      goto libswd_cli_syntaxerror;
    }
    continue;
   }

   // Check for WRITE invocation.
   else if ( strncmp(cmd,"w",1)==0 || strncmp(cmd,"write",5)==0 )
   {
    // Parse access port name parameter.
    cmd=strsep(&thiscommand," ");
    if (!cmd) goto libswd_cli_syntaxerror;
    if ( strncmp(cmd,"d",1)==0 || strncmp(cmd,"dap",3)==0 )
    {
     ap='d';
    }
    else if ( strncmp(cmd,"a",1)==0 || strncmp(cmd,"ap",2)==0 )
    {
     ap='a';
    }
    else if ( strncmp(cmd,"m",1)==0 || strncmp(cmd,"memap",5)==0 )
    {
     ap='m';
    } else goto libswd_cli_syntaxerror;
    // Parse address parameter.
    cmd=strsep(&thiscommand," ");
    if (!cmd) goto libswd_cli_syntaxerror;
    errno=LIBSWD_OK;
    addrstart=strtol(cmd, (char**)NULL, 0);
    if (errno!=LIBSWD_OK) goto libswd_cli_syntaxerror;
    // At this point parse for DP/AP and MEM-AP will vary, so skip it here.
    // Perform operations on a given access port.
    switch (ap)
    {
     case 'd':
      // Parse data parameter.
      if (!thiscommand) goto libswd_cli_syntaxerror;
      cmd=strsep(&thiscommand," ");
      if (!cmd) goto libswd_cli_syntaxerror;
      errno=LIBSWD_OK;
      data=strtol(cmd, &endptr, 0);
      if (errno!=LIBSWD_OK) goto libswd_cli_syntaxerror;
      retval=libswd_dp_write(libswdctx, LIBSWD_OPERATION_EXECUTE,
                            addrstart, &data );
      if (retval<0) goto libswd_cli_error;
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
                 "LIBSWD_N: DP[0x%08X]=0x%08X\n",
                 addrstart, data );
      break;
     case 'a':
      // Parse data parameter.
      if (!thiscommand) goto libswd_cli_syntaxerror;
      cmd=strsep(&thiscommand," ");
      if (!cmd) goto libswd_cli_syntaxerror;
      errno=LIBSWD_OK;
      data=strtol(cmd, &endptr, 0);
      if (errno!=LIBSWD_OK) goto libswd_cli_syntaxerror;
      retval=libswd_ap_write(libswdctx, LIBSWD_OPERATION_EXECUTE,
                            addrstart, &data );
      if (retval<0) goto libswd_cli_error;
      // Read-back the register value as it may differ from original request.
      retval=libswd_ap_read(libswdctx, LIBSWD_OPERATION_EXECUTE, addrstart, &data32);
      if (retval<0) goto libswd_cli_error;
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
                 "LIBSWD_N: AP[0x%08X]=0x%08X\n",
                 addrstart, *data32 );
      break;
     case 'm':
      // First data parameter can be a number or filename.
      // If number, it will also deretmine data type (char/int).
      // Parse data parameter.
      if (!thiscommand) goto libswd_cli_syntaxerror;
      errno=LIBSWD_OK;
      data=strtol(thiscommand, &endptr, 0);
      if (!*endptr || endptr==strstr(thiscommand," "))
      {
       // 'thiscommand' variable holds the data list.
       // First element length determines data type (char or int).
       int elmcnt, elmlen, elmbytes;
       for (elmlen=0;thiscommand[elmlen]&&thiscommand[elmlen]!=' ';elmlen++);
       // Count the number of elements on the list.
       elmcnt=1; for (i=0;thiscommand[i];i++) if (thiscommand[i]==' ') elmcnt++;
       if (elmlen<1 || elmlen>10) goto libswd_cli_syntaxerror;
       // Calculate and allocate memory buffer.
       // Buffer is char based, round up size to match 32-bit align if necessary.
       if (libswdctx->membuf.data) free(libswdctx->membuf.data);
       libswdctx->membuf.size=0;
       elmbytes=elmcnt*((elmlen>4)?(4*sizeof(char)):(sizeof(char)));
       if (elmbytes%4) elmbytes+=elmbytes%4;
       libswdctx->membuf.size=elmbytes;
       libswdctx->membuf.data=(unsigned char*)malloc(libswdctx->membuf.size);
       if (!libswdctx->membuf.data)
       {
        libswdctx->membuf.size=0;
        libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
                   "LIBSWD_E: libswd_cli(): Cannot allocate memory for data, aborting!\n" );
        retval=LIBSWD_ERROR_OUTOFMEM;
        goto libswd_cli_syntaxerror;
       } else memset((void*)libswdctx->membuf.data, 0xFF, libswdctx->membuf.size);
       // Fill the memory buffer with provided data.
       // Load char type data.
       if (elmlen<=4)
       {
        for (i=0;(i<libswdctx->membuf.size)&&thiscommand;i++)
        {
         cmd=strsep(&thiscommand," ");
         errno=LIBSWD_OK;
         data=strtol(cmd,(char**)NULL, 0);
         if (errno!=LIBSWD_OK) goto libswd_cli_syntaxerror;
         libswdctx->membuf.data[i]=(unsigned char)data;
        }
       }
       // Load int type data.
       else
       {
        for (i=0;(i<libswdctx->membuf.size/4)&&thiscommand;i++)
        {
         cmd=strsep(&thiscommand," ");
         errno=LIBSWD_OK;
         data=strtol(cmd,(char**)NULL, 0);
         if (errno!=LIBSWD_OK) goto libswd_cli_syntaxerror;
         libswdctx->membuf.data[i*4+0]=(unsigned char)data;
         libswdctx->membuf.data[i*4+1]=(unsigned char)(data>>8);
         libswdctx->membuf.data[i*4+2]=(unsigned char)(data>>16);
         libswdctx->membuf.data[i*4+3]=(unsigned char)(data>>24);
        }
       }
       libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
                  "LIBSWD_I: libswd_cli(): Parsed %d bytes of data into memory buffer!\n",
                  libswdctx->membuf.size );
      }
      // Given data parameter is a filename. Load file into memory.
      else
      {
       filename=thiscommand;
       res=LIBSWD_OK;
       FILE *fp;
       fp=fopen(filename,"r");
       if (!fp)
       {
        libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
                   "LIBSWD_E: libswd_cli(): Cannot open '%s' data file (%s), aborting!\n",
                   filename, strerror(errno));
        return LIBSWD_ERROR_FILE;
       }
       // Take care of (re)allocating memory for membuf based on a file size.
       if (libswdctx->membuf.size) free(libswdctx->membuf.data);
       fseek(fp, 0, SEEK_END);
       libswdctx->membuf.size=ftell(fp);
       libswdctx->membuf.data=(unsigned char*)malloc(libswdctx->membuf.size);
       fseek(fp, 0, SEEK_SET);
       if (!libswdctx->membuf.data)
       {
        libswdctx->membuf.size=0;
        libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
                   "LIBSWD_E: libswd_cli(): Cannot allocate memory to load '%s' file, aborting!\n",
                   filename );
        res=LIBSWD_ERROR_OUTOFMEM;
        goto libswd_cli_write_memap_file_load_error;
       } else memset((void*)libswdctx->membuf.data, 0xFF, libswdctx->membuf.size);
       // Load file content into memory.
       retval=fread(libswdctx->membuf.data, sizeof(char), libswdctx->membuf.size, fp);
       if (!retval)
       {
        libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                   "LIBSWD_E: libswd_cli(): Cannot load data from '%s' file (%s), aborting!\n",
                   filename, strerror(errno) );
        res=LIBSWD_ERROR_FILE;
        goto libswd_cli_write_memap_file_load_error;
       }
libswd_cli_write_memap_file_load_error:
       retval=fclose(fp);
       if (retval)
       {
        libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                   "LIBSWD_E: libswd_cli(): Cannot close data file '%s' (%s), aborting!\n",
                   filename, strerror(errno) );
        return LIBSWD_ERROR_FILE;
       }
       if (res!=LIBSWD_OK) return res;
libswd_cli_write_memap_file_load_ok:
       libswd_log(libswdctx, LIBSWD_LOGLEVEL_INFO,
                  "LIBSWD_I: libswd_cli(): %d bytes of data from '%s' file loaded!\n",
                  libswdctx->membuf.size, filename );
      }
      // At this point data are in membuf, sent them to MEM-AP.
      retval=libswd_memap_write_char_32(libswdctx, LIBSWD_OPERATION_EXECUTE,
                                        addrstart, libswdctx->membuf.size,
                                        (char*)libswdctx->membuf.data );
      if (retval<0) goto libswd_cli_error;
      // Print out the data.
      for (i=0; i<libswdctx->membuf.size; i=i+16)
      {
       printf("\n%08X: ", i+addrstart);
       for (j=0;j<16&&i+j<libswdctx->membuf.size;j++) printf("%02X ", (unsigned char)libswdctx->membuf.data[i+j]);
       if (j<16) for(;j<16;j++) printf("   ");
       printf(" ");
       for (j=0;j<16&&i+j<libswdctx->membuf.size;j++) printf("%c", isprint(libswdctx->membuf.data[i+j])?libswdctx->membuf.data[i+j]:'.');
       if (j<16) for (;j<16;j++) printf(".");
      }
      printf("\n");
      break;
     default:
      libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
                 "LIBSWD_E: libswd_cli(): Unknown Access Port given!\n");
      goto libswd_cli_syntaxerror;
    }
    break;
   }
   // No known thiscommand was invoked, show usage message and return message.
   else
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
               "LIBSWD_E: libswd_cli(): %s.\n",
               libswd_error_string(LIBSWD_ERROR_CLISYNTAX) );
               libswd_cli_print_usage(libswdctx);
    return LIBSWD_ERROR_CLISYNTAX;
   }
  }
 }

  return LIBSWD_OK;

libswd_cli_syntaxerror:
 retval=LIBSWD_ERROR_CLISYNTAX;
libswd_cli_error:
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
            "LIBSWD_E: libswd_cli(): %s.\n",
            libswd_error_string(retval) );
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG,
            "LIBSWD_D: libswd_cli(): CMD=%s COMMAND=%s.\n",
            cmd, command );
 return retval;
};


/** @} */
