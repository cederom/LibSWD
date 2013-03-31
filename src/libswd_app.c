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
 printf(" Available options (also available via cli): \n");
 printf("  -l : Use this log level (min=0..6=max)\n");
 printf("  -q : Quiet mode, no verbose output (equals '-l 0')\n");
 printf("  -i : Interface selection (by name)\n");
 printf("  -s : Interface Signal manipulation (multiple choice)\n");
 printf("  -v : Interface VID (default 0x0403 if not specified)\n");
 printf("  -p : Interface PID (default 0xbbe2 if not specified)\n");
 printf("  -h : Display this help\n");
 printf("\n Press Ctrl+C on prompt to exit application.\n");
 printf("\n");
 libswdapp_handle_command_signal_usage();
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
  if ( strncmp(cmd,"q",1)==0 || strncmp(cmd,"quit",4)==0 ) break;
  if (strncmp(cmd,"s",1)==0){
   libswdapp_handle_command_signal(libswdappctx, cmd);
   continue;
  }
  if (strncmp(cmd,"i",1)==0){
   libswdapp_handle_command_interface(libswdappctx, cmd);
   continue;
  }
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
 printf("LibSWD Application Interface Signal ('s') usage:\n");
 printf(" list              lists available signals and cached values\n");
 printf(" add:<name>=<mask> adds signal with given <name>\n");
 printf(" del:<name>        removes signal with given <name>\n");
 printf(" find:<name>       find given signal\n");
 printf(" <name>=<value>    write <value> to given <name> signal mask\n");
 printf(" <name>            reads the <name> signal value\n");
 printf("\n");
 return LIBSWD_OK;
}

/** Handle signal command (cli and commandline parameter).
 * \param *libswdappctx context to work on.
 * \param *cmd is the signal command argument.
 * \return LIBSWD_OK on success, negative value LIBSWD_ERROR code otehrwise.
 */
int libswdapp_handle_command_signal(libswdapp_context_t *libswdappctx, char *cmd){
 printf("DEBUG: Entering Interface Signal command handler function...");

 int sigmask;
 char eqloc;
 char *signame;    //[LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN];
 char *sigmaskstr; //[LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN];

 if (!strncasecmp(cmd,"?",1))
  return libswdapp_handle_command_signal_usage();

 if (!strncasecmp(cmd,"list",4))
 {
  libswdapp_interface_signal_t *sig;
  sig = libswdappctx->interface->signal;
  printf("      Interface Signal Name      |    Mask    |   Value   ");
  printf("----------------------------------------------------------");
  while (sig)
  {
   printf("%32s | 0x%08X | 0x%08X", sig->name, sig->mask, sig->value);
   sig = sig->next;
  }
  return LIBSWD_OK;
 }

 // Extract signal name from add/del/find commands.
 if (
     !strncasecmp(cmd,"add",3) ||
     !strncasecmp(cmd,"del",3) ||
     !strncasecmp(cmd,"find",4)
    )
 {
  if (strchr(cmd,':')==0)
  {
   printf("ERROR: Syntax Error, see '?' for more information...\n");
   libswdapp_handle_command_signal_usage();
   return LIBSWD_ERROR_CLISYNTAX; 
  }
  if (strlen(strncpy(signame,&cmd[4],LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN))==0)
  {
   printf("ERROR: Cannot extract signal '%s' name!\n", cmd);  
   return LIBSWD_ERROR_PARAM;
  }
 }
 // At this point we have signal name in signame char array.

 // Handle signal delete command
 if (!strncasecmp(cmd,"del",3))
  return libswdapp_interface_signal_add(libswdappctx, signame, sigmask);

 // Handle find signal command
 if (!strncasecmp(cmd,"find",4))
 {
  libswdapp_interface_signal_t *sig;
  sig=libswdapp_interface_signal_find(libswdappctx, signame);
  if (sig!=NULL)
  {
   printf("%s: mask=0x%08X value=0x%08X\n", sig->name, sig->mask, sig->value);
   return LIBSWD_OK;
  } else
  {
   printf("Signal '%s' not found!\n", signame);
   return LIBSWD_ERROR_PARAM;
  }
 }

 if (!strncasecmp(cmd,"add",3))
 {
  signame=strsep(&cmd,"=");
  if (!strlen(signame) || !strlen(cmd))
  {
   printf("ERROR: Syntax Error, see '?' for more information...\n");
   libswdapp_handle_command_signal_usage();
   return LIBSWD_ERROR_CLISYNTAX; 
  }
/*  if ((=strchr(cmd,'='))==NULL)
  {
   printf("ERROR: Syntax Error, see '?' for more information...\n");
   libswdapp_handle_command_signal_usage();
   return LIBSWD_ERROR_CLISYNTAX; 
  }
  if (strlen(strncpy(signame,cmd[4],eqloc-4))==0)
  {
   printf("ERROR: Cannot extract signal '%s' name!\n", cmd);
   return LIBSWD_ERROR_PARAM;     
  }
  if (strlen(strncpy(sigmaskstr,eqloc,LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN))==0)
  {
   printf("ERROR: Cannot extract signal '%s' mask!\n", cmd);
   return LIBSWD_ERROR_PARAM; 
  }
*/
  errno=LIBSWD_OK;
  sigmask=strtol(sigmaskstr,(char**)NULL,16); 
  if (errno!=LIBSWD_OK)
  {
   printf("ERROR: Cannot convert '%s' mask to integer!\n", sigmaskstr);
   return LIBSWD_ERROR_PARAM;
  }
  return libswdapp_interface_signal_add(libswdappctx, signame, sigmask);
 }

 // Not above available commands, check if asked for signal value
  
}


/** It will prepare interface for use or fail.
 * If an interface is already configured, it will check if requested interface
 * is available and reinitialize driver (remove old driver and load new one).
 * cmd==NULL means that we need to load default interface (defined in header file)
 * or we need to (re)initialize an interface that is already set in interface->name.
 */
int libswdapp_handle_command_interface(libswdapp_context_t *libswdappctx, char *cmd){
 int retval, i, interface_number;
 char *param;

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
 libswdappctx->interface->initialized=0;

 // Load selected interface configuration.
 strncpy(libswdappctx->interface->name,libswdapp_interface_configs[interface_number].name,LIBSWDAPP_INTERFACE_NAME_MAXLEN);
 libswdappctx->interface->init   = libswdapp_interface_configs[interface_number].init;
 libswdappctx->interface->deinit = libswdapp_interface_configs[interface_number].deinit;
 libswdappctx->interface->vid    = libswdapp_interface_configs[interface_number].vid;
 libswdappctx->interface->pid    = libswdapp_interface_configs[interface_number].pid;
 libswdappctx->interface->ftdi_latency=libswdapp_interface_configs[interface_number].ftdi_latency;
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

 libswdapp_interface_ftdi_freq(libswdappctx, 1000000);
 // Now call the interface specific initialization routine.
 retval=libswdappctx->interface->init(libswdappctx);
 if (retval==LIBSWD_OK) libswdappctx->interface->initialized=1; 
 return retval;
}


/******************************************************************************
 * INTERFACE SIGNAL INFRASTRUCTURE AND OPERATIONS                             *
 ******************************************************************************/

/** Check if specified signal is already defined (case insensitive) and return
 * its pointer if defined.
 * \param *name signal name to check
 * \return pointer to signal structure in memory if found, NULL otherwise.
 */
libswdapp_interface_signal_t *libswdapp_interface_signal_find(libswdapp_context_t *libswdappctx, char *name)
{
 printf("Interface signal - searching for '%s' signal...", name);
 // Check if interface signal to already exists
 if (!libswdappctx->interface->signal) {
  printf("WARNING: Interface signal list is empty..\n");
  return NULL;
 }
 //Check if signal name is correct
 if (!name || *name==' ') {
  printf("ERROR: Interface signal name cannot be empty!\n");
  return NULL;
 }
 // Check if signal name already exist
 libswdapp_interface_signal_t *sig;
 sig = libswdappctx->interface->signal;
 while (sig)
 {
  if (!strncasecmp(sig->name, name, LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN)) {
  printf("Interface signal '%s' found.\n", sig->name);
  return sig;
  }
  sig = sig->next;
 }
 // If signal is not found return null pointer.
 printf("WARNING: Interface signal '%s' not found.\n", name);
 return NULL;
}

/** Add new signal to the interface.
 * Signal will be allocated in memory with provided name and mask.
 * There is no sense for giving value field at this time because signal create
 * can take place during initialization where interface is not yet ready, also
 * they can be used for read and write, so this is higher level script task
 * to initialize their default value with appropriate 'bitbang' call.
 * The default value for new signal equals provided mask to maintain Hi-Z.
 *
 * \param *name is the signal name (max 32 char).
 * \param mask is the signal mask (unsigned int).
 * \param value is the initial value for signal to set.
 * \return ERROR_OK on success or ERROR_FAIL on failure.
 */
int libswdapp_interface_signal_add(libswdapp_context_t *libswdappctx, char *name, unsigned int mask)
{
 printf("Interface signal adding '%s'...\n", name);

 // Check if name is correct string. 
 if (!name || *name==' ')
 {
  printf("Interface signal - name cannot be empty!");
  return LIBSWD_ERROR_PARAM;
 }

 libswdapp_interface_signal_t *newsignal, *lastsignal;
 int snlen;

 snlen = strnlen(name, 2*LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN);
 if (snlen < LIBSWDAPP_INTERFACE_SIGNAL_NAME_MINLEN || snlen > LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN)
 {
  printf("Interface signal name too short or too long!\n");
  return LIBSWD_ERROR_PARAM;
 }

 // Check if signal name already exist and return error if so.
 if (libswdapp_interface_signal_find(libswdappctx, name))
 {
  printf("Interface signal '%s' already exist!", name);
  return LIBSWD_ERROR_PARAM;
 }

 // Allocate memory for new signal structure.
 newsignal = (libswdapp_interface_signal_t*)calloc(1,sizeof(libswdapp_interface_signal_t));
 if (!newsignal)
 {
  printf("ERROR: Interface signal - cannot allocate memory for new signal '%s'!\n", name);
  return LIBSWD_ERROR_OUTOFMEM;
 }
 newsignal->name = (char *)calloc(1, snlen+1);
 if (!newsignal->name)
 {
  printf("ERROR: Interface signal - cannot allocate memory '%s' name!", name);
  return LIBSWD_ERROR_OUTOFMEM;
 }

 // Initialize structure data and return or break on error. 
 for (;;)
 {
  if (!strncpy(newsignal->name, name, snlen))
  {
   printf("WARNING: Interface signal cannot copy '%s' name!", name);
   break;
  }

  newsignal->mask = mask;
  newsignal->value = mask;

  if (&libswdappctx->interface->signal!=NULL)
  {
   libswdappctx->interface->signal = newsignal;
  }
  else
  {
   lastsignal = libswdappctx->interface->signal;
   while (lastsignal->next) lastsignal=lastsignal->next;
   lastsignal->next=newsignal;
  }
  printf("INFO: Interface signal - '%s' added.", name);
  return LIBSWD_OK;
 }

 // If there was an error free up resources and return error.
 free(newsignal->name);
 free(newsignal);
 return LIBSWD_ERROR_DRIVER;
}

/** Delete interface signal.
 * Removes signal from singly linked list of interface signals and free memory.
 * \param name is the name of the signal to remove.
 * \return ERROR_OK on success, ERROR_FAIL on failure.
 */
int libswdapp_interface_signal_del(libswdapp_context_t *libswdappctx, char *name)
{
 //printf("DEBUG: Interface signal: deleting signal '%s'...\n", name);
 // Check if interface any signal exist
 if (!libswdappctx->interface->signal)
 {
  //printf("WARNING: Interface signal list is empty!\n");
  return LIBSWD_ERROR_NULLPOINTER;
 }
 // Check if signal name is correct.
 if (!name || *name==' ')
 {
  printf("ERROR: Interface signal name cannot be empty!\n");
  return LIBSWD_ERROR_DRIVER;
 }
 libswdapp_interface_signal_t *delsig = NULL, *prevsig = NULL;
 // See if we want to remove all signals ('*' name).
 if (strchr(name,'*'))
 {
  for (delsig=libswdappctx->interface->signal;delsig;delsig=delsig->next)
  {
   free(delsig->name); 
   free(delsig);
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
  libswdappctx->interface->signal = libswdappctx->interface->signal->next;
 }
 else
 {
  for (; prevsig->next; prevsig = prevsig->next)
  {
   if (prevsig->next == delsig)
   {
    prevsig->next = prevsig->next->next;
    break;
   }
  }
 }
 // now free memory of detached element.
 free(delsig->name);
 free(delsig);
 printf("INFO: Interface signal '%s' deleted.\n", name);
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
 */
int libswdapp_interface_bitbang(libswdapp_context_t *libswdappctx, unsigned int bitmask, int GETnSET, unsigned int *value)
{
 unsigned char  buf[3];
 int retval, vall = 0, valh = 0;
 unsigned int bytes_written, bytes_read;
 unsigned int low_output, high_output, low_direction, high_direction; 

 if (!GETnSET) {
  /* We will SET port pins selected by bitmask. */
  /* Modify our pins value, but remember about other pins and their previous value */
  low_output  = (low_output & ~bitmask) | ((*value & bitmask) & 0x0ff);
  high_output = (high_output & ~(bitmask >> 8)) | (((*value & bitmask) >> 8) & 0x0ff);
  /* Modify our pins direction, but remember about other pins and their previous direction */
  low_direction  |= bitmask & 0x0ff;
  high_direction |= (bitmask >> 8) & 0x0ff;
  /* Now send those settings to the interface chip */
  buf[0] = 0x80;  /* Set Data Bits LowByte */
  buf[1] = low_output;
  buf[2] = low_direction;
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3) return bytes_written;
  buf[0] = 0x82;   /* Set Data Bits HighByte */
  buf[1] = high_output;
  buf[2] = high_direction;
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3) return bytes_written;
  *value = ((high_output << 8) | low_output) & bitmask;
 } else {
  /* Modify our pins value, but remember about other pins and their previous value */
  /* DO WE REALLY NEED TO PULL-UP PINS TO READ THEIR STATE OR SIMPLY LEAVE AS IS? */
  /* low_output  = (low_output & ~sigmask) | (sigmask & 0x0ff); */
  /* high_output = (high_output & ~sigmask) | (sigmask>>8) & 0x0ff); */
  /* Modify our pins direction to input, but remember about other pins and their previous direction */
  low_direction  &= ~(bitmask);
  high_direction &= ~(bitmask >> 8);
  /* Now send those settings to the interface chip */
  /* First change desired pins to input */
  buf[0] = 0x80;  /* Set Data Bits LowByte */
  buf[1] = low_output;
  buf[2] = low_direction;
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3) return bytes_written;
  buf[0] = 0x82;   /* Set Data Bits HighByte */
  buf[1] = high_output;
  buf[2] = high_direction;
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 3);
  if (bytes_written<0 || bytes_written!=3) return bytes_written;
  /* Then read pins designated by a signal mask */
  buf[0] = 0x81;    /* Read Data Bits LowByte. */
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 1);
  if (bytes_written<0 || bytes_written!=1) return bytes_written;
  bytes_read = ftdi_read_data(libswdappctx->interface->ftdictx, (unsigned char*)&vall, 1);
  if (bytes_read<0 || bytes_read!=1) return bytes_read;
  buf[0] = 0x83;    /* Read Data Bits HighByte. */
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 1);
  if (bytes_written<0 || bytes_written!=1) return bytes_written;
  bytes_read = ftdi_read_data(libswdappctx->interface->ftdictx, (unsigned char*)&valh, 1);
  if (bytes_read<0 || bytes_read!=1) return bytes_read;
  *value = ((valh << 8) | vall) & bitmask; /* Join result bytes and apply signal bitmask */
 }
 return LIBSWD_OK;
}

/** Transfer bits in/out stored in char array starting from LSB first or MSB first,
 * alternatively if you want to make MSB-first shift on LSB-first mode put data
 * in reverse order into input/output array.
 * \param *device void pointer to pass driver details to the function.
 * \param bits is the number of bits (char array elements) to transfer.
 * \param *mosidata pointer to char array with data to be send.
 * \param *misodata pointer to char array with data to be received.
 * \param nLSBfirst if zero shift data LSB-first, otherwise MSB-first.
 * \return number of bits sent on success, or ERROR_FAIL on failure.
 */
int libswdapp_interface_transfer(libswdapp_context_t *libswdappctx, int bits, char *mosidata, char *misodata, int nLSBfirst)
{
 static uint8_t buf[65539], databuf;
 int i, retval, bit = 0, byte_ = 0, bytes = 0;	/* underscore in byte_ to prevent shadowing of global variable */
 unsigned int bytes_written, bytes_read;

 if (bits > 65535) {
  printf("ERROR: Cannot transfer more than 65536 bits at once!\n");
  return LIBSWD_ERROR_DRIVER;
 }

 if (bits >= 8) {
 /* Try to pack as many bits into bytes for better performance. */
 bytes = bits / 8;
 bytes--;		      /* MPSSE starts counting bytes from 0. */
 buf[0] = (nLSBfirst) ? 0x31 : 0x39; /* Clock Bytes In and Out LSb or MSb first. */
 buf[1] = (char)bytes & 0x0ff;
 buf[2] = (char)((bytes >> 8) & 0x0ff);
 bytes++;
 for (byte_ = 0; byte_ * 8 < bits; byte_++) {
  databuf = 0;
  for (i = 0; i < 8; i++)
   databuf |= mosidata[byte_ * 8 + i] ? (1 << i) : 0;
   buf[byte_ + 3] = databuf;
  }
  bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, bytes + 3);
  if (bytes_written<0 || bytes_written!=(bytes+3)) {
   printf("ERROR: ft2232_write() returns %d\n", bytes_written);
   return ;
  }
  bytes_read = ftdi_read_data(libswdappctx->interface->ftdictx, (uint8_t *)buf, bytes);
  if (bytes_read<0 || bytes_read!=bytes) {
   printf("ERROR: ft2232_read() returns %d\n", bytes_read);
  return LIBSWD_ERROR_DRIVER;
 }
 /* Explode read bytes into bit array. */
 for (byte_ = 0; byte_ * 8 < bits; byte_++)
 for (bit = 0; bit < 8; bit++)
  misodata[byte_ * 8 + bit] = buf[byte_] & (1 << bit) ? 1 : 0;
 }

 /* Now send remaining bits that cannot be packed as bytes. */
 /* Because "Clock Data Bits In and Out LSB/MSB" of FTDI is a mess, pack single */
 /* bit read/writes into buffer and then flush it using single USB transfer. */
 for (bit = bytes * 8; bit < bits; bit++) {
  buf[3 * bit + 0] = (nLSBfirst) ? 0x33 : 0x3b;     /* Clock Bits In and Out LSb or MSb first. */
  buf[3 * bit + 1] = 0;				 /* One bit per element. */
  buf[3 * bit + 2] = mosidata[bit] ? 0xff : 0;      /* Take data from supplied array. */
 }
 bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 3 * (bits - (bytes * 8)));
 if (bytes_written < 0) {
  printf("ERROR: ft2232_write() returns %d\n", bytes_written);
  return LIBSWD_ERROR_DRIVER;
 }
 bytes_read = ftdi_read_data(libswdappctx->interface->ftdictx, (uint8_t *)misodata, bits - (bytes * 8));
 if (bytes_read<0 || bytes_read!=(bits-(bytes*8))) {
  printf("ERROR: ft2232_read() returns %d\n", bytes_read);
  return LIBSWD_ERROR_DRIVER;
 }
 /* FTDI MPSSE returns shift register value, our bit is MSb */
 for (bit = bytes * 8; bit < bits; bit++)
  misodata[bit] = (misodata[bit] & (nLSBfirst ? 0x01 : 0x80)) ? 1 : 0;
 /* USE THIS FOR WIRE-LEVEL DEBUG */
 /* LOG_DEBUG("read 0x%02X written 0x%02X", misodata[bit], mosidata[bit]); */

 return bit;
}


/** Set interface frequency in Hz.
 */
int libswdapp_interface_ftdi_freq(libswdapp_context_t *libswdappctx, int freq)
{
 unsigned int reg;
 char buf[3], bytes_written;

 if (!libswdappctx || !libswdappctx->interface || !libswdappctx->interface->ftdictx)
  return LIBSWD_ERROR_NULLPOINTER;

 reg=((12000000/freq)-1)/2;
 buf[0] = 0x86;
 buf[1] = reg&0xff;
 buf[2] = (reg>>8)&0xff;
 bytes_written = ftdi_write_data(libswdappctx->interface->ftdictx, buf, 3);
 if (bytes_written<0 || bytes_written!=3) return bytes_written;
 return LIBSWD_OK; 
}


//KT-LINK Interface Init
int libswdapp_interface_ftdi_init_ktlink(libswdapp_context_t *libswdappctx)
{
 int ftdi_channel=INTERFACE_ANY;
 int ftdi_latency=0;
 unsigned char latency_timer;
 unsigned int port_direction, port_value;

 if (ftdi_channel == INTERFACE_ANY)
  ftdi_channel = INTERFACE_A;
 if (ftdi_set_interface(libswdappctx->interface->ftdictx, ftdi_channel)<0){
  printf("ERROR: Unable to select FT2232 channel A: %s\n",
         libswdappctx->interface->ftdictx->error_str);
  return LIBSWD_ERROR_DRIVER;
 }

 if (ftdi_usb_reset(libswdappctx->interface->ftdictx) < 0) {
  printf("ERROR: Unable to reset ftdi device!\n");
  return LIBSWD_ERROR_DRIVER;
 }

 if (ftdi_set_latency_timer(libswdappctx->interface->ftdictx,libswdappctx->interface->ftdi_latency)<0){
  printf("ERROR: Unable to set latency timer!\n");
  return LIBSWD_ERROR_DRIVER;
 }

 if (ftdi_get_latency_timer(libswdappctx->interface->ftdictx,&latency_timer)<0){
  printf("ERROR: Unable to get latency timer!\n");
  return LIBSWD_ERROR_DRIVER;
 } else
  printf("FTDI latency timer is: %i\n", latency_timer);

 ftdi_set_bitmode(libswdappctx->interface->ftdictx, 0x0b, 2); /* ctx, JTAG I/O mask */

 /* High Byte (ACBUS) members. */
 const unsigned int nSWCLKen=0x40, nTDIen=0x20, TRST=0x01, nTRSTen=0x04,
                     SRST=0x02, nSRSTen=0x08, LED=0x80, RnW=0x10;
 /* Low Byte (ADBUS) members. */
 const unsigned int SWCLK=0x01, TDI=0x02, TDO=0x04, nSWDIOsel=0x20;

 //nTRST    = TRST;
 //nSRST    = SRST;
 //nTRSTnOE = nTRSTen;
 //nSRSTnOE = nSRSTen;

 /* Set ADBUS Port Data: SWCLK=0, TDI=0,TDO=1, nSWDIOsel=0 */
 const unsigned int low_output = 0 | TDO;
 /* Set ADBUS Port Direction (1=Output) */
 const unsigned int low_direction = 0 | SWCLK | TDI | nSWDIOsel;

 /* initialize low byte port (ADBUS) */
 //if (ft2232_set_data_bits_low_byte(low_output, low_direction) != LIBSWD_OK) {
 //printf("ERROR: couldn't initialize FT2232 ADBUS with ktlink_swd layout!");
 // return LIBSWD_ERROR_DRIVER;
 //}

 /* Set Data Bits High Byte (ACBUS)                                */
 /* Enable SWD pins  : nTCKen=0, RnW=1, nSRSTen=0, nLED=0, SRST=1  */
 /* Disable JTAG pins: nTDIen=1, nSWDIOen=1, nTRSTen=1             */
 const unsigned int high_output = 0 | RnW | SRST | nTDIen | nTRSTen;
 /* Set ACBUS Port Direction (1=Output) */
 const unsigned int high_direction = 0 | RnW | nSWCLKen | nTDIen | nTRSTen | nSRSTen | SRST | LED;

 /* initialize high byte port (ACBUS) */
 //if (ft2232_set_data_bits_high_byte(high_output, high_direction) != LIBSWD_OK) {
 // printf("ERROR: couldn't initialize FT2232 ACBUS with 'ktlink_swd' layout\n");
 // return LIBSWD_ERROR_DRIVER;
 //}
 port_direction=(high_direction<<8)|low_direction;
 port_value=(high_output<<8)|low_output;
 libswdapp_interface_bitbang(libswdappctx, port_direction, 0, &port_value);

 /* Additional bit-bang signals should be placed in a configuration file. */

 printf("KT-LINK SWD-Mode initialization complete!\n");
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
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_drv_mosi_8(libswdctx=@%p, cmd=@%p, data=0x%02X, bits=%d, nLSBfirst=0x%02X)",
            (void *)libswdctx, (void *)cmd, *data, bits, nLSBfirst);

	if (data == NULL)
		return LIBSWD_ERROR_NULLPOINTER;
	if (bits < 0 && bits > 8)
		return LIBSWD_ERROR_PARAM;
	if (nLSBfirst != 0 && nLSBfirst != 1)
		return LIBSWD_ERROR_PARAM;

	static unsigned int i;
	static signed int res;
	static char misodata[8], mosidata[8];

	/* Split output data into char array. */
	for (i = 0; i < 8; i++)
		mosidata[(nLSBfirst == LIBSWD_DIR_LSBFIRST) ? i : (bits - 1 - i)] = ((1 << i) & (*data)) ? 1 : 0;
	/* Then send that array into interface hardware. */
	res = libswdapp_interface_transfer(libswdctx->driver->ctx, bits, mosidata, misodata, 0);
	if (res < 0)
		return LIBSWD_ERROR_DRIVER;

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
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_drv_mosi_32(libswdctx=@%p, cmd=@%p, data=0x%08X, bits=%d, nLSBfirst=0x%02X)",
            (void *)libswdctx, (void *)cmd, *data, bits, nLSBfirst);

	if (data == NULL)
		return LIBSWD_ERROR_NULLPOINTER;
	if (bits < 0 && bits > 8)
		return LIBSWD_ERROR_PARAM;
	if (nLSBfirst != 0 && nLSBfirst != 1)
		return LIBSWD_ERROR_PARAM;

	static unsigned int i;
	static signed int res;
	static char misodata[32], mosidata[32];

	/* UrJTAG drivers shift data LSB-First. */
	for (i = 0; i < 32; i++)
		mosidata[(nLSBfirst == LIBSWD_DIR_LSBFIRST) ? i : (bits - 1 - i)] = ((1 << i) & (*data)) ? 1 : 0;
	res = libswdapp_interface_transfer(libswdctx->driver->ctx, bits, mosidata, misodata, 0);
	if (res < 0)
		return LIBSWD_ERROR_DRIVER;
	return res;
}

/**
 * Use UrJTAG's driver to read 8-bit data (char type).
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
	if (data == NULL)
		return LIBSWD_ERROR_NULLPOINTER;
	if (bits < 0 && bits > 8)
		return LIBSWD_ERROR_PARAM;
	if (nLSBfirst != 0 && nLSBfirst != 1)
		return LIBSWD_ERROR_PARAM;

	static int i;
	static signed int res;
	static char misodata[8], mosidata[8];

	res = libswdapp_interface_transfer(libswdctx->driver->ctx, bits, mosidata, misodata, LIBSWD_DIR_LSBFIRST);
	if (res < 0)
		return LIBSWD_ERROR_DRIVER;
	/* Now we need to reconstruct the data byte from shifted in LSBfirst byte array. */
	*data = 0;
	for (i = 0; i < bits; i++)
		*data |= misodata[(nLSBfirst == LIBSWD_DIR_LSBFIRST) ? i : (bits - 1 - i)] ? (1 << i) : 0;
libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_drv_miso_8(libswdctx=@%p, cmd=@%p, data=@%p, bits=%d, nLSBfirst=0x%02X) reads: 0x%02X",
           (void *)libswdctx, (void *)cmd, (void *)data, bits, nLSBfirst, *data);

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
	if (data == NULL)
		return LIBSWD_ERROR_NULLPOINTER;
	if (bits < 0 && bits > 8)
		return LIBSWD_ERROR_PARAM;
	if (nLSBfirst != 0 && nLSBfirst != 1)
		return LIBSWD_ERROR_PARAM;

	static int i;
	static signed int res;
	static char misodata[32], mosidata[32];

	res = libswdapp_interface_transfer(libswdctx->driver->ctx, bits, mosidata, misodata, LIBSWD_DIR_LSBFIRST);
	if (res < 0)
		return LIBSWD_ERROR_DRIVER;
	/* Now we need to reconstruct the data byte from shifted in LSBfirst byte array. */
	*data = 0;
	for (i = 0; i < bits; i++)
		*data |= (misodata[(nLSBfirst == LIBSWD_DIR_LSBFIRST) ? i : (bits - 1 - i)] ? (1 << i) : 0);
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_drv_miso_32() reads: 0x%08X\n", *data);
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
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_drv_mosi_trn(libswdctx=@%p, bits=%d)\n", (void *)libswdctx, bits);

	if (bits < LIBSWD_TURNROUND_MIN_VAL && bits > LIBSWD_TURNROUND_MAX_VAL)
		return LIBSWD_ERROR_TURNAROUND;

	int res, val = 0;
	static char buf[LIBSWD_TURNROUND_MAX_VAL];
	/* Use driver method to set low (write) signal named RnW. */
	res = libswdapp_interface_bitbang(libswdctx->driver->ctx, "RnW", 0, &val);
	if (res < 0)
		return LIBSWD_ERROR_DRIVER;

	/* Clock specified number of bits for proper TRN transaction. */
	res = libswdapp_interface_transfer(libswdctx->driver->ctx, bits, buf, buf, 0);
	if (res < 0)
		return LIBSWD_ERROR_DRIVER;

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
 libswd_log(libswdctx, LIBSWD_LOGLEVEL_DEBUG, "LIBSWD_D: libswd_drv_miso_trn(libswdctx=@%p, bits=%d)\n", (void *)libswdctx, bits);

 if (bits < LIBSWD_TURNROUND_MIN_VAL || bits > LIBSWD_TURNROUND_MAX_VAL)
  return LIBSWD_ERROR_TURNAROUND;

 static int res, val = 1;
 static char buf[LIBSWD_TURNROUND_MAX_VAL];

 /* Use driver method to set high (read) signal named RnW. */
 res = libswdapp_interface_bitbang(libswdctx->driver->ctx, "RnW", 0xFFFFFFFF, &val);
 if (res < 0)
 return LIBSWD_ERROR_DRIVER;

 /* Clock specified number of bits for proper TRN transaction. */
 res = libswdapp_interface_transfer(libswdctx->driver->ctx, bits, buf, buf, 0);
 if (res < 0)
 return LIBSWD_ERROR_DRIVER;

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
