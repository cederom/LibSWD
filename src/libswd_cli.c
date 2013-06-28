/*
 * Serial Wire Debug Open Library.
 * Command Line Interface Body File.
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

/** \file libswd_cli.c */

#include <libswd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

/*******************************************************************************
 * \defgroup libswd_cli Command Line Interface functions.
 * @{
 ******************************************************************************/

int libswd_cli_print_usage(libswd_ctx_t *libswdctx)
{
 if (libswdctx==NULL) return LIBSWD_ERROR_NULLPOINTER;
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N: Available LibSWD CLI commands:\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [h]elp / [?]\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [d]etect\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [l]oglevel <newloglevel>\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [r]ead [d]ap/[a]p 0xAddress\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [w]rite [d]ap/[a]p 0xAddress\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [r]ead [m]emap 0xAddress <0xCount>|4 <filename>\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [w]rite [m]emap 0xAddress 0x32BitData[]|filename\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "\n");
 return LIBSWD_OK;
}

/** Command Line Interface parse and execution engine.
 * Only one command should be given per invocation, but multiple commands
 * per line are possible, one after another, separated with space.
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

 int retval, addrstart, addrstop, count, *data32, i, j;
 char *cmd, ap, *filename;

 while ( cmd=strsep(&command," ") )
 {
  // Strip heading and trailing spaces.
  if (cmd && command) if (!cmd[0] && command[0]) continue;
  if (cmd && command) if (!cmd[0] && !command[0]) break;

  // Check for HELP invocation.
  if ( strncmp(cmd,"?",1)==0 || strncmp(cmd,"help",4)==0 || strncmp(cmd,"h",1)==0 )
  {
   return libswd_cli_print_usage(libswdctx); 
  }

  // Check for LOGLEVEL invocation.
  else if ( strncmp(cmd,"l",1)==0 || strncmp(cmd,"loglevel",8)==0 )
  {
   int loglevel_new=LIBSWD_LOGLEVEL_DEFAULT;
   cmd=strsep(&command," ");
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

  // Check for DETECT invocation.
  else if ( strncmp(cmd,"d",1)==0 || strncmp(cmd,"detect",6)==0 )
  {
   int *idcode;
   retval=libswd_dap_detect(libswdctx, LIBSWD_OPERATION_EXECUTE, &idcode);
   if (retval<0)
   {
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
               "LIBSWD_E: libswd_cli(): Cannot read IDCODE! %s.\n",
               libswd_error_string(retval) ); 
    libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
               "LIBSWD_N: libswd_cli(): IDCODE READ ERROR!\n" );
   } else libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
                     "LIBSWD_N: IDCODE=0x%08X/%s\n",
                     *idcode, libswd_bin32_string(idcode) );
   continue;
  }

  // Check for READ invocation.
  else if ( strncmp(cmd,"r",1)==0 || strncmp(cmd,"read",4)==0 )
  {
   // Parse access port name parameter.
   cmd=strsep(&command," ");
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
   cmd=strsep(&command," ");
   if (!cmd) goto libswd_cli_syntaxerror;
   errno=LIBSWD_OK;
   addrstart=strtol(cmd, (char**)NULL, 16);
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
     if (command)
     {
      cmd=strsep(&command," ");
      errno=LIBSWD_OK;
      count=strtol(cmd, (char**)NULL, 16); 
      if (errno!=LIBSWD_OK || count<=0)
      {
       libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                  "LIBSWD_W: libswd_cli(): Bad 'count' value (0x%X -> 0x04).\n",
                  count);
       count=4;
      }
     } else count=4;
     addrstop=addrstart+count;
     // Parse count parameter for MEM-AP operation.
     if (command)
     {
      cmd=strsep(&command," ");
      errno=LIBSWD_OK;
      count=strtol(cmd, (char**)NULL, 16); 
      if (errno!=LIBSWD_OK || count<=0)
      {
       libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                  "LIBSWD_W: libswd_cli(): Bad 'count' value (0x%X -> 0x04).\n",
                  count);
       count=4;
      }
     } else count=4;
     addrstop=addrstart+count;
     // Parse filename parameter for MEM-AP operation.
     if (command && ap=='m')
     {
      cmd=strsep(&command," ");
      filename=cmd;
     } else filename=NULL;
     // Take care of proper memory (re)allocation.
     if (ap=='m' && (libswdctx->membuf.size<count) )
     {
      char *membuf;
      membuf=(char*)libswdctx->membuf.data;
      if (membuf) free(membuf);
      membuf=(char*)calloc(count,sizeof(char));
      if (!membuf)
      {
       libswdctx->membuf.size=0;
       libswd_log(libswdctx, LIBSWD_ERROR_OUTOFMEM, 
                  "LIBSWD_E: libswd_cli(): Cannot (re)allocate memory buffer!\n");
       return LIBSWD_ERROR_OUTOFMEM;
      }
      libswdctx->membuf.data=membuf;
      libswdctx->membuf.size=count*sizeof(char);
     } 
     retval=libswd_memap_read(libswdctx, LIBSWD_OPERATION_EXECUTE,
                              addrstart, count,
                              (int**)&libswdctx->membuf.data);
     if (retval<0) goto libswd_cli_error;
     // Store result to a file if requested.
     if (filename)
     {
      FILE *fp;
      fp=fopen(filename,"w");
      if (!fp)
      {
       libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING,
                  "LIBSWD_W: libswd_cli(): Cannot open '%s' resul file!\n",
                  filename);
       break;
      }
      retval=fwrite(libswdctx->membuf.data, count, sizeof(char), filename);
      if (!retval)
       libswd_log(libswdctx, LIBSWD_LOGLEVEL_WARNING, 
                  "LIBSWD_W: libswd_cli(): Cannot write result to file '%s'!\n",
                  filename);
     }
     // Print out the result.
     for (i=0; i<libswdctx->membuf.size; i++)
     {
      if (!(i%16)) printf("\n%08X: ", i);
      printf("%02X ", (unsigned char)libswdctx->membuf.data[i]);
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
   printf("LIBSWD WRITE not yet implemented, stay tuned :-)\n");
   continue;
  }

  // No known command was invoked, show usage message and return message.
  else
  {
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_ERROR,
              "LIBSWD_E: libswd_cli(): %s.\n",
              libswd_error_string(LIBSWD_ERROR_CLISYNTAX) );
              libswd_cli_print_usage(libswdctx);
   return LIBSWD_ERROR_CLISYNTAX; 
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
