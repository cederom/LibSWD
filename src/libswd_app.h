/*
 * Serial Wire Debug Open Library.
 * Application Header File.
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

/** \file libswd_app.h */

#ifndef __LIBSWDAPP_H__
#define __LIBSWDAPP_H__

#include <libswd.h>
#include <ftdi.h>

#define LIBSWDAPP_INTERFACE_SIGNAL_NAME_MINLEN	1
#define LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN	32
#define LIBSWDAPP_INTERFACE_NAME_MAXLEN           32
#define LIBSWDAPP_INTERFACE_CONFIG_NAME_MAXLEN    32
#define LIBSWDAPP_INTERFACE_VID_DEFAULT           0x0403
#define LIBSWDAPP_INTERFACE_PID_DEFAULT           0xbbe2
#define LIBSWDAPP_INTERFACE_NAME_DEFAULT          "ktlink"

typedef struct libswdapp_interface_signal {
	char *name;                         /// Signal name string.
	unsigned int mask;                  /// Mask value for selected signal.
	int value;                          /// Cached signal value.
	struct libswdapp_interface_signal *next; /// Next signal on the list.
} libswdapp_interface_signal_t;

typedef struct libswdapp_context {
 libswd_ctx_t *libswdctx;
 struct libswdapp_interface *interface; 
 int loglevel;
 int retval;
} libswdapp_context_t;

typedef struct libswdapp_interface {
 char name[LIBSWDAPP_INTERFACE_NAME_MAXLEN];
 struct ftdi_context *ftdictx;
 int vid, pid, vid_forced, pid_forced;
 libswdapp_interface_signal_t *signal;
 int (*init)(libswdapp_context_t *libswdappctx);
 int (*deinit)(libswdapp_context_t *libswdappctx);
 int (*set_freq)(libswdapp_context_t *libswdappctx, int freq);
 char *sigsetupstr;
 // Below are CACHED values changed only by the interface functions.

 unsigned char latency;
 enum ftdi_interface ftdi_channel;
 int frequency; /// This value shall only be changed by set_freq() routine.
 char initialized;
 unsigned int gpioval, gpiodir;
} libswdapp_interface_t;

typedef struct libswdapp_interface_config {
 char name[LIBSWDAPP_INTERFACE_NAME_MAXLEN];
 char *description;
 char *sigsetupstr;
 int (*init)(libswdapp_context_t *libswdappctx);
 int (*deinit)(libswdapp_context_t *libswdappctx);
 int (*set_freq)(libswdapp_context_t *libswdappctx, int freq);
 int vid, pid;
 unsigned char latency;
 int frequency;
 unsigned int gpioval, gpiodir; //Shouldnt we use array?
} libswdapp_interface_config_t;

typedef enum libswdapp_interface_operation {
	OOCD_INTERFACE_SIGNAL_OPERATION_UNDEFINED = 0,
	OOCD_INTERFACE_SIGNAL_OPERATION_READ,
	OOCD_INTERFACE_SIGNAL_OPERATION_WRITE,
	OOCD_INTERFACE_SIGNAL_OPERATION_SET,
	OOCD_INTERFACE_SIGNAL_OPERATION_CLEAR
} libswdapp_interface_operation_t;



void libswdapp_shutdown(int sig);
int libswdapp_interface_signal_add(libswdapp_context_t *libswdappctx, char *name, unsigned int mask);
int libswdapp_interface_signal_del(libswdapp_context_t *libswdappctx, char *name);
libswdapp_interface_signal_t *libswdapp_interface_signal_find(libswdapp_context_t *libswdappctx, char *name);
int libswdapp_print_banner(void);
int libswdapp_print_usage(void);
int libswdapp_handle_command_signal(libswdapp_context_t *libswdappctx, char *cmd);
int libswdapp_handle_command_interface(libswdapp_context_t *libswdappctx, char *cmd);
int libswdapp_interface_ftdi_set_freq(libswdapp_context_t *libswdappctx, int freq);

int ftdi_bitbang(void *device, char *signal_name, int GETnSET, int *value);
int ftdi_transfer(void *device, int bits, char *mosidata, char *misodata, int nLSBfirst);
int libswd_drv_mosi_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
int libswd_drv_mosi_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
int libswd_drv_miso_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
int libswd_drv_miso_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
int libswd_drv_mosi_trn(libswd_ctx_t *libswdctx, int clks);
int libswd_drv_miso_trn(libswd_ctx_t *libswdctx, int clks);
int libswd_log(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...);

static int libswdapp_interface_ftdi_init(libswdapp_context_t *libswdappctx);
static int libswdapp_interface_ftdi_deinit(libswdapp_context_t *libswdappctx);
static int libswdapp_interface_ftdi_init_ktlink(libswdapp_context_t *libswdappctx);

static const libswdapp_interface_config_t libswdapp_interface_configs[] = {
 {
  .name        = "ktlink",
  .description = "KT-LINK FT2232H based device",
  .init        = libswdapp_interface_ftdi_init,
  .deinit      = libswdapp_interface_ftdi_deinit,
  .set_freq    = libswdapp_interface_ftdi_set_freq,
  .vid         = 0x0403,
  .pid         = 0xbbe2, 
  .latency     = 1,
  .frequency   = 10000,
  .sigsetupstr = "signal add:CLK=0x0001 add:MOSI=0x0002 add:MISO=0x0004 add:TMS=0x0008 add:nSWDsel=0x0020 add:SRSTin=0x0040 add:RTCK=0x0080 add:TRST=0x0100 add:SRST=0x0200 add:nTRSTen=0x0400 add:nSRSTen=0x0800 add:RnW=0x1000 add:nMOSIen=0x2000 add:nCLKen=0x4000 add:LED=0x8000 CLK=lo MOSI=lo SRST=hi nCLKen=lo nSWDsel=lo RnW=lo nSRSTen=lo LED=lo MISO SRSTin RTCK",
 },
 {
   .name = NULL,
 }
};

#endif
