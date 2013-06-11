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
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [r]ead [d]ap/[a]p/[m]emap address count\n");
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LIBSWD_N:  [w]rite [d]ap/[a]p/[m]emap address data[0] .. data[n]\n");
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

 int retval;
 char *cmd, *commandstart;
 commandstart=command;

 while ( cmd=strsep(&command," ") )
 {
  // Strip trailing spaces.
  if (cmd && command) if (!cmd[0] && command[0]) continue;
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
   libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL, "LOGLEVEL=%d\n",
              (int)libswdctx->config.loglevel);
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
   }
   else libswd_log(libswdctx, LIBSWD_LOGLEVEL_NORMAL,
                   "LIBSWD_N: FOUND IDCODE: 0x%08X / %s\n",
                   *idcode, libswd_bin32_string(idcode) );
   continue;
  }
  // Check for READ invocation.
  else if ( strncmp(cmd,"r",1)==0 || strncmp(cmd,"read",4)==0 )
  {
   printf("LIBSWD READ not yet implemented, stay tuned :-)\n");
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
};


/** @} */
