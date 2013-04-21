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

#include <libswd_app.h>
#include <libswd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ftdi.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include <signal.h>

/*******************************************************************************
 * \defgroup libswd_app LibSWD Application functions.
 * @{
 ******************************************************************************/

libswdapp_context_t *appctx;


void libswdapp_shutdown(int sig)
{
 int retval=appctx->retval;
 printf("\nShutting down...\n");
 if (appctx->interface->deinit) appctx->interface->deinit;
 libswdapp_interface_signal_del(appctx,"*");
 if (appctx->interface->ftdictx!=NULL) ftdi_free(appctx->interface->ftdictx);
 if ((appctx->interface)!=NULL) free(appctx->interface);
 if (appctx->libswdctx!=NULL) libswd_deinit(appctx->libswdctx);
 if (appctx!=NULL) free(appctx);
 printf("Exit OK\n");
 exit(retval);
}

int libswdapp_print_banner(void){
 printf("*******************************************************************\n");
 printf("* Welcome to LibSWD CLI application! Type '?' or 'help' for help. *\n");
 printf("* See http://libswd.sf.net for more information. (C) CeDeROM 2013 *\n");
 printf("*******************************************************************\n\n");
 return LIBSWD_OK;
}

int libswdapp_print_usage(void){
 printf(" LibSWD Application available options (also available via cli): \n");
 printf("  -l : Use this log level (min=0..6=max)\n");
 printf("  -q : Quiet mode, no verbose output (equals '-l 0')\n");
 printf("  -i : Interface selection (by name)\n");
 printf("  -s : Interface Signal manipulation (multiple choice)\n");
 printf("  -v : Interface VID (default 0x0403 if not specified)\n");
 printf("  -p : Interface PID (default 0xbbe2 if not specified)\n");
 printf("  -h : Display this help\n\n");
 libswdapp_handle_command_signal_usage();
 printf(" Press Ctrl+C or type [q]uit on prompt to exit LibSWD Application.\n");
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
 int i, retval=0;
 libswdapp_context_t *libswdappctx;

 // Initialize application context in memory.
 libswdappctx=(libswdapp_context_t*)calloc(1,sizeof(libswdapp_context_t));
 if (libswdappctx==NULL)
 {
  printf("ERROR: Cannot initialize LibSWD Application Context, aborting!\n");
  retval=LIBSWD_ERROR_OUTOFMEM;
  goto quit;
 }
 libswdappctx->interface=(libswdapp_interface_t*)calloc(1,sizeof(libswdapp_interface_t));
 if (libswdappctx->interface==NULL)
 {
  printf("ERROR: Cannot initialize Interface structure, aborting!\n");
  retval=LIBSWD_ERROR_OUTOFMEM;
  goto quit;
 }
 libswdappctx->loglevel=LIBSWD_LOGLEVEL_DEFAULT;

 // Catch SIGINT signal and call shutdown.
 appctx=libswdappctx;
 signal(SIGINT, libswdapp_shutdown);

 // Handle program commandline execution arguments.
 while ( (i=getopt(argc,argv,"hqi:s:l:p:v:"))!=-1 )
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
printf("Interface cmdparam: %s\n", optarg);
    break;
   case 's':
    printf("S ARG: %s\n", optarg);
    break;
   case 'h':
   case '?':
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
 if (libswdappctx->loglevel) libswdapp_print_banner();

 // Initialize the Interface
 retval=libswdapp_handle_command_interface(libswdappctx, NULL);
 if (retval!=LIBSWD_OK)
  printf("WARNING: Cannot initialize '%s' interface!\n",
         libswdappctx->interface->name);

 // Initialize LibSWD.
 libswdappctx->libswdctx=libswd_init();
 if (libswdappctx->libswdctx==NULL)
 {
  if (libswdappctx->loglevel) printf("ERROR: Cannot initialize libswd!\n");
  retval=LIBSWD_ERROR_NULLCONTEXT;
  goto quit;
 }
 // Store program Context for use with interface drivers.
 libswdappctx->libswdctx->driver->ctx=libswdappctx;
 // Set default LogLevel.
 if (LIBSWD_OK!=libswd_log_level_set(libswdappctx->libswdctx,libswdappctx->loglevel))
 {
  if (libswdappctx->loglevel)
   printf("Cannot set %s loglevel for LibSWD!\n",\
          libswd_log_level_string(libswdappctx->loglevel));
  goto quit;
 }

 /* Run the Command Line Interpreter loop. */
 while ((cmd=readline("libswd>")) != NULL){
  if (!strncmp(cmd,"q",1) || !strncmp(cmd,"quit",4)) break;
  if (!strncmp(cmd,"s",1) || !strncmp(cmd,"signal",5))
  {
   libswdapp_handle_command_signal(libswdappctx, cmd);
   continue;
  }
  if (!strncmp(cmd,"i",1) || !strncmp(cmd,"interface",9))
  {
   libswdapp_handle_command_interface(libswdappctx, cmd);
   continue;
  }
  if (!strncmp(cmd,"h",1) || !strncmp(cmd,"help",4) || !strncmp(cmd,"?",1))
   libswdapp_print_usage();
  retval=libswd_cli(libswdappctx->libswdctx, cmd);
  if (retval!=LIBSWD_OK) if (retval!=LIBSWD_ERROR_CLISYNTAX) goto quit; 
 }

 /* Cleanup and quit. */
quit:
 libswdapp_shutdown(2);
 return LIBSWD_OK;
}


/** Print out the Interface Signal command usage.
 * \return Always LIBSWD_OK.
 */
int libswdapp_handle_command_signal_usage(void){
 printf(" LibSWD Application Interface Signal ('[s]ignal') usage:\n");
 printf("  list              lists available signals and cached values\n");
 printf("  add:<name>=<mask> adds signal with given <name>\n");
 printf("  del:<name>        removes signal with given <name>\n");
 printf("  <name>=<value>    write hex <value> to given <name> signal mask\n");
 printf("  <name>            reads the <name> signal value\n");
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

 command=(char*)calloc(strlen(cmd),sizeof(char));
 if (!command)
 {
  printf("ERROR: Cannot allocate memory for command anaysis!\n");
  return LIBSWD_ERROR_OUTOFMEM;
 }
 strncpy(command, cmd, strlen(cmd));

 cmdp=strsep(&command," ");
 if (!cmd)
  return libswdapp_handle_command_signal_usage();

 while (cmdp=strsep(&command," "))
 {
  // Check if list command, handle if necessary.
  if (!strncasecmp(cmdp,"list",4))
  {
   sig = libswdappctx->interface->signal;
   while (sig)
   {
    printf("%s[0x%0X]=0x%0X\n", sig->name, sig->mask, sig->value);
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
     printf("WARNING: Syntax Error, see '?' or '[s]ignal' for more information...\n");
     continue;
    }
    retval=libswdapp_interface_signal_del(libswdappctx, signamep);
    if (retval!=LIBSWD_OK)
     printf("WARNING: Cannot remove signal '%s'!\n", signamep);
    continue;
   }
   else if (!strncmp(cmdip,"add",3))
   {
    signamep=strsep(&cmdp,"=");
    sigmaskp=cmdp;
    if (!signamep || !sigmaskp)
    {
     printf("WARNING: Syntax Error, see '?' or '[s]ignal' for more information...\n");
     continue;
    }
    errno=LIBSWD_OK;
    sigmask=strtol(sigmaskp,NULL,16);
    if (errno!=LIBSWD_OK)
    {
     printf("WARNING: Syntax Error, see '?' or '[s]ignal' for more information...\n");
     continue;
    }
    retval=libswdapp_interface_signal_add(libswdappctx, signamep, sigmask);
    if (retval!=LIBSWD_OK)
     printf("WARNING: Cannot add '%s' signal with '%x' mask!\n", signamep, sigmaskp, sigmask);
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
     printf("WARNING: Syntax Error, see '?' or '[s]ignal' for more information...\n");
     continue;
    }
   }
  }
  sig=libswdapp_interface_signal_find(libswdappctx, signamep);
  if (!sig)
  {
   printf("WARNING: Signal '%s' not found!\n", signamep);
   continue;
  } 
  if (signamep && sigvalp)
  {
   retval=libswdapp_interface_bitbang(libswdappctx, sig->mask, 0, &sigval);
   if (retval==LIBSWD_OK)
   {
    sig->value=sigval;
    printf("%s[0x%X]=0x%X\n", signamep, sig->mask, sig->value);
   } else printf("WARNING: Cannot set signal '%s' value!\n", signamep);
   continue;
  }
  else if (signamep)
  {
   retval=libswdapp_interface_bitbang(libswdappctx, sig->mask, 1, &sigval);
   if (retval==LIBSWD_OK)
   {
    sig->value=sigval;
    printf("%s[0x%X]=0x%X\n", signamep, sig->mask, sig->value);
   } else printf("WARNING: Cannot read signal '%s' value!\n", signamep);
   continue;
  }
 }
 free(command);
 return LIBSWD_OK;
  
libswdapp_handle_command_signal_error:
  free(command);
  printf("ERROR: Syntax Error, see '?' or '[s]ignal' for more information...\n");
  return LIBSWD_ERROR_CLISYNTAX;
}
 

/** It will prepare interface for use or fail.
 * If an interface is already configured, it will check if requested interface
 * is available and reinitialize driver (remove old driver and load new one).
 * cmd==NULL means that we need to load default interface (defined in header file)
 * or we need to (re)initialize an interface that is already set in interface->name.
 */
int libswdapp_handle_command_interface(libswdapp_context_t *libswdappctx, char *cmd){
 int retval, i, interface_number;
 char *param, buf[8];

 // Verify the working context.
 if (!libswdappctx)
 {
  printf("ERROR: libswdapp_hanle_command_interface() Null libswdappctx context!\n");
  return LIBSWD_ERROR_NULLCONTEXT;
 }

 // If cmd==NULL check if interface name already set, if no use default.
 if (!cmd)
 {
  if (!libswdappctx->interface->name[0]) 
  {
   printf("INFO: Using default interface '%s'...\n", LIBSWDAPP_INTERFACE_NAME_DEFAULT);
   strncpy(
           libswdappctx->interface->name,
           LIBSWDAPP_INTERFACE_NAME_DEFAULT,
           LIBSWDAPP_INTERFACE_NAME_MAXLEN
          );
  }
 }
 else
 {
  // If cmd!=NULL parse parameters: ? or copy interface name.
  // But first make sure we have parsed the command already.
  param=strchr(cmd,' ');
  if (param) cmd=strncpy(cmd,param+1,LIBSWDAPP_INTERFACE_NAME_MAXLEN);
  // Print out the list of available interfaces if asked.
  if (strchr(cmd,'?') || strchr(cmd,'i'))
  {
   if (libswdappctx->interface->initialized)
    printf("Active interface: %s\n", libswdappctx->interface->name);
   printf("Available Interfaces:");
   for (i=0;libswdapp_interface_configs[i].name[0];i++)
    printf(" %s", libswdapp_interface_configs[i].name);
   printf("\n");
   return LIBSWD_OK;
  } else strncpy(libswdappctx->interface->name,cmd,LIBSWDAPP_INTERFACE_NAME_MAXLEN);
 } 

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
  printf("ERROR: Interface '%s' is not supported!\n", libswdappctx->interface->name);
  return LIBSWD_ERROR_PARAM;
 }
 printf("Loading '%s' interface...", libswdapp_interface_configs[interface_number].name);

 // Unload existing interface settings if necessary.
 if (!libswdappctx->interface->name) return LIBSWD_ERROR_DRIVER;
 if (libswdappctx->interface->deinit)
  libswdappctx->interface->deinit(libswdappctx);
 if (libswdappctx->interface->ftdictx)
  ftdi_deinit(libswdappctx->interface->ftdictx);
 if (libswdappctx->interface->signal)
  libswdapp_interface_signal_del(libswdappctx, "*"); 
 libswdappctx->interface->init=NULL;
 libswdappctx->interface->deinit=NULL;
 libswdappctx->interface->set_freq=NULL;
 libswdappctx->interface->maxfrequency=0;
 libswdappctx->interface->frequency=-1;
 libswdappctx->interface->chunksize=0;
 libswdappctx->interface->initialized=0;

 // Load selected interface configuration.
 strncpy(libswdappctx->interface->name,libswdapp_interface_configs[interface_number].name,LIBSWDAPP_INTERFACE_NAME_MAXLEN);
 libswdappctx->interface->init   = libswdapp_interface_configs[interface_number].init;
 libswdappctx->interface->deinit = libswdapp_interface_configs[interface_number].deinit;
 libswdappctx->interface->set_freq = libswdapp_interface_configs[interface_number].set_freq;
 libswdappctx->interface->vid    = libswdapp_interface_configs[interface_number].vid;
 libswdappctx->interface->pid    = libswdapp_interface_configs[interface_number].pid;
 libswdappctx->interface->latency= libswdapp_interface_configs[interface_number].latency;
 libswdappctx->interface->maxfrequency=libswdapp_interface_configs[interface_number].maxfrequency;
 libswdappctx->interface->chunksize=libswdapp_interface_configs[interface_number].chunksize;
 if (libswdappctx->interface->vid_forced)
  libswdappctx->interface->vid    = libswdappctx->interface->vid_forced;
 if (libswdappctx->interface->pid_forced)
  libswdappctx->interface->pid    = libswdappctx->interface->pid_forced;
 printf("OK\n");

 // Initialize LibFTDI Context.
 if (libswdappctx->loglevel) printf("Initializing LibFTDI...");
 libswdappctx->interface->ftdictx=ftdi_new();
 if (libswdappctx->interface->ftdictx==NULL)
 {
  printf("ERROR: Cannot initialize LibFTDI!\n");
  return LIBSWD_ERROR_DRIVER;
 } else if (libswdappctx->loglevel) printf("OK\n");

 // Open FTDI interface with given VID:PID pair.
 if (libswdappctx->loglevel)
  printf("Opening FTDI interface USB[%04X:%04X]...",
         libswdappctx->interface->vid,
         libswdappctx->interface->pid
        );
 retval=ftdi_usb_open(libswdappctx->interface->ftdictx,
                      libswdappctx->interface->vid,
                      libswdappctx->interface->pid
                     );
 if (retval<0)
 {
  if (libswdappctx->loglevel)
    printf("FAILED (%s)\n",
           ftdi_get_error_string(libswdappctx->interface->ftdictx));
  return LIBSWD_ERROR_DRIVER;
 }
 else
 {
  if (libswdappctx->interface->ftdictx->type==TYPE_R)
  {
   unsigned int chipid;
   retval=ftdi_read_chipid(libswdappctx->interface->ftdictx, &chipid);
   if (retval<0)
   {
    if (libswdappctx->loglevel) printf("OK\n");
   }
   else if (libswdappctx->loglevel) printf("OK (ChipId=%X)\n", chipid); 
  } else if (libswdappctx->loglevel) printf("OK\n");
 }
 // Reeset the FTDI device.
 if (ftdi_usb_reset(libswdappctx->interface->ftdictx)<0)
 {
  printf("ERROR: Unable to reset ftdi device!\n");
  return LIBSWD_ERROR_DRIVER;
 }
// Reset Command Controller
 if(ftdi_set_bitmode(libswdappctx->interface->ftdictx, 0, BITMODE_RESET)<0)
 {
  printf("ERROR: Cannot (re)set bitmode for FTDI interface!\n");
  return LIBSWD_ERROR_DRIVER;
 }
 // Call the interface specific binary initialization routine.
 retval=libswdappctx->interface->init(libswdappctx);
 if (retval!=LIBSWD_OK)
 {
  printf("ERROR: Interface specific intialization routine failed!\n");
  return retval;
 }
 // Set default interface speed/frequency
 libswdapp_interface_ftdi_set_freq(libswdappctx, libswdapp_interface_configs[interface_number].frequency);
 // Call the interface specific signal configuration command string.
 retval=libswdapp_handle_command_signal(libswdappctx, libswdapp_interface_configs[interface_number].sigsetupstr);
 if (retval!=LIBSWD_OK)
 {
  printf("ERROR: Cannot setup Interface specific signals!\n");
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

 // Check if name is correct string. 
 if (!name || *name==' ')
 {
  printf("ERROR: Interface signal name cannot be empty!\n");
  return LIBSWD_ERROR_PARAM;
 }
 // Verify signal name length.
 snlen = strnlen(name, 2*LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN);
 if (snlen < LIBSWDAPP_INTERFACE_SIGNAL_NAME_MINLEN || snlen > LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN)
 {
  printf("ERROR: Interface signal name '%s' too short or too long!\n", name);
  return LIBSWD_ERROR_PARAM;
 }

 // Check if signal name already exist and return error if so.
 if (libswdapp_interface_signal_find(libswdappctx, name))
 {
  printf("ERROR: Interface signal '%s' already exist!\n", name);
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
  printf("WARNING: Interface signal cannot copy '%s' name!", name);
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
 printf("INFO: Interface signal '%s' added.\n", name);
 return LIBSWD_OK;

 // If there was an error free up resources and return error.
libswdapp_interface_signal_add_end:
 printf("ERROR: Cannot add signal '%s'!\n", name);
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
 // Check if interface any signal exist
 if (!libswdappctx->interface->signal)
  return LIBSWD_ERROR_NULLPOINTER;
 // Check if signal name is correct.
 if (!name || *name==' ')
 {
  printf("ERROR: Interface signal name cannot be empty!\n");
  return LIBSWD_ERROR_DRIVER;
 }
 // See if we want to remove all signals ('*' name).
 if (strchr(name,'*'))
 {
  for (delsig=libswdappctx->interface->signal;delsig;delsig=delsig->next)
  {
   printf("INFO: Removing Interface Signal '%s'...", delsig->name);
   free(delsig->name); 
   free(delsig);
   printf("OK\n");
  }
  libswdappctx->interface->signal=NULL; 
  return LIBSWD_OK;
 }
 // look for the signal name on the list.
 delsig = libswdapp_interface_signal_find(libswdappctx, name);
 // return error if signal is not on the list.
 if (!delsig)
 {
  printf("ERROR: Interface signal '%s' not found!", name);
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
 printf("INFO: Removing Interface Signal '%s'...", name);
 free(delsig->name);
 free(delsig);
 printf("OK\n");
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
 * TODO: Bitbang Read must also update other bits status, otherwise cached values are invalid and aboguous!!!
 */
int libswdapp_interface_bitbang(libswdapp_context_t *libswdappctx, unsigned int bitmask, int GETnSET, unsigned int *value)
{
 unsigned char  buf[3];
 int retval, retry;
 unsigned int bytes_written, bytes_read;
 unsigned int vall=0, valh=0, gpioval=0, gpiodir=0; 

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
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3) return bytes_written;
  buf[0] = 0x82;   // Set Data Bits HighByte.
  buf[1] = (gpioval>>8)&0x00ff;
  buf[2] = (gpiodir>>8)&0x00ff;
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3)
  {
   printf("ERROR: Interface bitbang error!\n");
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
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3)
  {
   printf("ERROR: libswdapp_interface_bitbang(): ftdi_write_data() returns invalid bytes count: %d\n", bytes_written);
   return LIBSWD_ERROR_DRIVER;
  }
  buf[0] = 0x82;   // Set Data Bits HighByte.
  buf[1] = (gpioval>>8)&0x00ff;
  buf[2] = (gpiodir>>8)&0x00ff;
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3)
  {
   printf("ERROR: libswdapp_interface_bitbang(): ftdi_write_data() returns invalid bytes count: %d\n", bytes_written);
   return LIBSWD_ERROR_DRIVER;
  }
  // Then read pins designated by a signal mask.
  buf[0] = 0x81;    // Read Data Bits LowByte.
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 1);
  if (bytes_written<0 || bytes_written!=1)
  {
   printf("ERROR: libswdapp_interface_bitbang(): ftdi_write_data() returns invalid bytes count: %d\n", bytes_written);
   return LIBSWD_ERROR_DRIVER;
  }
  for (retry=0;retry<LIBSWD_RETRY_COUNT_DEFAULT;retry++)
  {
   bytes_read = ftdi_read_data(libswdappctx->interface->ftdictx, (unsigned char*)&vall, 1);
   if (bytes_read>0) break;
  }
  if (bytes_read<0 || bytes_read!=1)
  {
   printf("ERROR: libswdapp_interface_bitbang(): ftdi_read_data() returns invalid bytes count: %d\n", bytes_read);
   return LIBSWD_ERROR_DRIVER;
  }
  buf[0] = 0x83;    // Read Data Bits HighByte.
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 1);
  if (bytes_written<0 || bytes_written!=1)
  {
   printf("ERROR: libswdapp_interface_bitbang(): ftdi_write_data() returns invalid bytes count: %d\n", bytes_written);
   return LIBSWD_ERROR_DRIVER;
  }
  for (retry=0;retry<LIBSWD_RETRY_COUNT_DEFAULT;retry++)
  {
   bytes_read = ftdi_read_data(libswdappctx->interface->ftdictx, (unsigned char*)&valh, 1);
   if (bytes_read>0) break;
  }
  if (bytes_read<0 || bytes_read!=1)
  {
   printf("ERROR: libswdapp_interface_bitbang(): ftdi_read_data() returns invalid bytes count: %d\n", bytes_read);
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
int libswdapp_interface_transfer_bits(libswdapp_context_t *libswdappctx, int bits, char *mosidata, char *misodata, int nLSBfirst)
{
 static unsigned char buf[65539], databuf;
 int i, retval, bit=0, byte=0, bytes=0, retry;
 unsigned int bytes_written, bytes_read;

 if (bits>65535)
 {
  printf("ERROR: Cannot transfer more than 65536 bits at once!\n");
  return LIBSWD_ERROR_DRIVER;
 }

 if (bits>=8)
 {
  // Try to pack as many bits into bytes for better performance.
  bytes=bits/8;
  bytes--;		              // MPSSE starts counting bytes from 0.
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
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, bytes+3);
  if (bytes_written<0 || bytes_written!=(bytes+3))
  {
   printf("ERROR: libswdapp_interface_transfer_bits(): ft2232_write() returns %d\n", bytes_written);
   return ;
  }
  // This retry is necessary because sometimes FTDI Chip returns 0 bytes.
  for (retry=0;retry<LIBSWD_RETRY_COUNT_DEFAULT;retry++)
  {
   bytes_read=ftdi_read_data(libswdappctx->interface->ftdictx, (unsigned char*)buf, bytes);
   if (bytes_read>0) break;
  }
  if (bytes_read<0 || bytes_read!=bytes)
  {
   printf("ERROR: libswdapp_interface_transfer_bits(): ft2232_read() returns %d\n", bytes_read);
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
  buf[3*bit+1] = 0;				     // One bit per element.
  buf[3*bit+2] = mosidata[bit]?0xff:0;  // Take data from supplied array.
 }
 bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx,buf,3*(bits-(bytes*8)));
 if (bytes_written<0)
 {
  printf("ERROR: libswdapp_interface_transfer_bits(): ft2232_write() returns invalid bytes count: %d\n", bytes_written);
  return LIBSWD_ERROR_DRIVER;
 }
 // This retry is necessary because sometimes FTDI Chip returns 0 bytes.
 for (retry=0;retry<LIBSWD_RETRY_COUNT_DEFAULT;retry++)
 {
  bytes_read=ftdi_read_data(libswdappctx->interface->ftdictx, (unsigned char*)buf, bits-(bytes*8));
  if (bytes_read>0) break;
 }
 if (bytes_read<0 || bytes_read!=(bits-(bytes*8)))
 {
  printf("ERROR: libswdapp_interface_transfer_bits(): ft2232_read() returns invalid bytes count: %d\n", bytes_read);
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
int libswdapp_interface_transfer_bytes(libswdapp_context_t *libswdappctx, int bytes, char *mosidata, char *misodata, int nLSBfirst)
{
 static unsigned char buf[65539], databuf;
 int i, retval, byte=0, retry;
 unsigned int bytes_written, bytes_read;

 if (bytes>65535)
 {
  printf("ERROR: Cannot transfer more than 65536 bits at once!\n");
  return LIBSWD_ERROR_DRIVER;
 }

 bytes--;		                  // MPSSE starts counting bytes from 0.
 buf[0] = (nLSBfirst)?0x31:0x39; // Clock Bytes In and Out MSb or LSb first.
 buf[1] = (char)bytes&0x0ff;
 buf[2] = (char)((bytes>>8)&0x0ff);
 bytes++;
 // Fill in the data buffer.
 for (byte=0;byte<bytes;byte++) buf[byte+3]=*mosidata;
 bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, bytes+3);
 if (bytes_written<0 || bytes_written!=(bytes+3))
 {
  printf("ERROR: libswdapp_interface_transfer_bytes(): ft2232_write() returns %d\n", bytes_written);
  return ;
 }
 // This retry is necessary because sometimes FTDI Chip returns 0 bytes.
 for (retry=0;retry<LIBSWD_RETRY_COUNT_DEFAULT;retry++)
 {
  bytes_read=ftdi_read_data(libswdappctx->interface->ftdictx, buf, bytes);
  if (bytes_read>0) break;
 }
 if (bytes_read<0 || bytes_read!=bytes)
 {
  printf("ERROR: libswdapp_interface_transfer_bytes(): ft2232_read() returns %d\n", bytes_read);
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

 // Set interface channel to use.
 if (ftdi_channel == INTERFACE_ANY) ftdi_channel = INTERFACE_A;
 if (ftdi_set_interface(libswdappctx->interface->ftdictx, ftdi_channel)<0)
 {
  printf("ERROR: Unable to select FT2232 channel A: %s\n",
         libswdappctx->interface->ftdictx->error_str);
  return LIBSWD_ERROR_DRIVER;
 }
 // Set the Latency Timer.
 if (ftdi_set_latency_timer(libswdappctx->interface->ftdictx,libswdappctx->interface->latency)<0)
 {
  printf("ERROR: Unable to set latency timer!\n");
  return LIBSWD_ERROR_DRIVER;
 }
 if (ftdi_get_latency_timer(libswdappctx->interface->ftdictx,&latency_timer)<0)
 {
  printf("ERROR: Unable to get latency timer!\n");
  return LIBSWD_ERROR_DRIVER;
 } else printf("FTDI latency timer is: %i\n", latency_timer);
 // Set MPSSE mode for FT2232 chip.
 retval=ftdi_set_bitmode(libswdappctx->interface->ftdictx, 0, BITMODE_MPSSE);
 if (retval<0)
 {
  printf("ERROR: Cannot set bitmode BITMODE_MPSSE for '%s' interface!\n", libswdappctx->interface->name);
  return LIBSWD_ERROR_DRIVER;
 } 
 // Set large chunksize for faster transfers of large data.
 retval=ftdi_write_data_set_chunksize(libswdappctx->interface->ftdictx, libswdappctx->interface->chunksize);
  if (retval<0)
 {
  printf("ERROR: Cannot set %d write chunksize for '%s' interface!\n", libswdappctx->interface->chunksize, libswdappctx->interface->name);
  return LIBSWD_ERROR_DRIVER;
 } 
 retval=ftdi_read_data_set_chunksize(libswdappctx->interface->ftdictx, libswdappctx->interface->chunksize);
  if (retval<0)
 {
  printf("ERROR: Cannot set %d read chunksize for '%s' interface!\n", libswdappctx->interface->chunksize, libswdappctx->interface->name);
  return LIBSWD_ERROR_DRIVER;
 } 
 // Set default max clock frequency (6MHz backward compatible).
 libswdappctx->interface->maxfrequency=6000000;

 printf("FTDI MPSSE initialization complete!\n");
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
 char buf[3], bytes_written;
 if (!libswdappctx || !libswdappctx->interface || !libswdappctx->interface->ftdictx)
  return LIBSWD_ERROR_NULLPOINTER;
 if (freq<0)
 {
  printf("ERROR: Invalid interface frequency value '%d'!\n", freq);
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
 bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 3);
 if (bytes_written<0 || bytes_written!=3) return bytes_written;
 libswdappctx->interface->frequency=freq;
 return LIBSWD_OK; 
}


//KT-LINK Interface Init
int libswdapp_interface_ftdi_init_ktlink(libswdapp_context_t *libswdappctx)
{
 int retval;
 unsigned char buf[4];

 retval=libswdapp_interface_ftdi_init(libswdappctx);
 if (retval<0) return retval;

 printf("Disabling CLK/5 (set max CLK=30MHz)...");
 buf[0]=0x8A; 
 retval=ftdi_write_data(libswdappctx->interface->ftdictx, buf, 1);
 if (retval<0 || retval!=1)
 {
  printf("FAILED!\nERROR: Cannot switch off clock divisor!\n");
  return retval;
 } 
 libswdappctx->interface->maxfrequency=30000000;
 printf("OK\n");

 return LIBSWD_OK;
}

// Generic FTDI interface deinit routine (all GPIO=Input=HI-Z)
int libswdapp_interface_ftdi_deinit(libswdapp_context_t *libswdappctx)
{
 unsigned int dir,val;
 return libswdapp_interface_bitbang(libswdappctx, dir, 0, &val); 
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

 // Split output data into char array.
 for (i=0;i<8;i++)
  mosidata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?i:(bits-1-i)]=((1<<i)&(*data))?1:0;
 // Then send that array into interface hardware.
 res=libswdapp_interface_transfer_bits(libswdctx->driver->ctx,bits,mosidata,misodata,0);
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

 // UrJTAG drivers shift data LSB-First.
 for (i=0;i<32;i++)
  mosidata[(nLSBfirst==LIBSWD_DIR_LSBFIRST)?i:(bits-1-i)]=((1<<i)&(*data))?1:0;
 res=libswdapp_interface_transfer_bits(libswdctx->driver->ctx,bits,mosidata,misodata,0);
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

 res=libswdapp_interface_transfer_bits(libswdctx->driver->ctx,bits,mosidata,misodata,LIBSWD_DIR_LSBFIRST);
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

 res = libswdapp_interface_transfer_bits(libswdctx->driver->ctx, bits, mosidata, misodata, LIBSWD_DIR_LSBFIRST);
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

 int res, val = 0;
 static char buf[LIBSWD_TURNROUND_MAX_VAL];
 // Use driver method to set low (write) signal named RnW.
 res = libswdapp_interface_bitbang(libswdctx->driver->ctx, sig->mask, 0, &val);
 if (res < 0) return LIBSWD_ERROR_DRIVER;

 // Clock specified number of bits for proper TRN transaction.
 res = libswdapp_interface_transfer_bits(libswdctx->driver->ctx, bits, buf, buf, 0);
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

 static int res, val = 1;
 static char buf[LIBSWD_TURNROUND_MAX_VAL];

 // Use driver method to set high (read) signal named RnW.
 res = libswdapp_interface_bitbang(libswdctx->driver->ctx, sig->mask, 1, &val);
 if (res < 0) return LIBSWD_ERROR_DRIVER;

 // Clock specified number of bits for proper TRN transaction.
 res = libswdapp_interface_transfer_bits(libswdctx->driver->ctx, bits, buf, buf, 0);
 if (res < 0) return LIBSWD_ERROR_DRIVER;

 return bits;
}

/* We want to use internal logging. */
int libswd_log(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...){
 int retval;
 va_list ap;
 va_start(ap, msg);
 retval=libswd_log_internal_va(libswdctx, loglevel, msg, ap);
 va_end(ap);
 return retval;
};



/** @} */
