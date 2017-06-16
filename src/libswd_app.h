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
#if defined(__MINGW32__) || (defined(__APPLE__) && defined(__MACH__))
#include <libftdi1/ftdi.h>
#else
#include <ftdi.h>
#endif

#define LIBSWDAPP_INTERFACE_SIGNAL_NAME_MINLEN    1
#define LIBSWDAPP_INTERFACE_SIGNAL_NAME_MAXLEN    32
#define LIBSWDAPP_INTERFACE_NAME_MAXLEN           32
#define LIBSWDAPP_INTERFACE_CONFIG_NAME_MAXLEN    32
#define LIBSWDAPP_INTERFACE_VID_DEFAULT           0x0403
#define LIBSWDAPP_INTERFACE_PID_DEFAULT           0xbbe2
#define LIBSWDAPP_INTERFACE_NAME_DEFAULT          "ktlink"

#define LIBSWDAPP_CLI_HISTORY_FILENAME "/.libswd/libswdapp_cli_history"
#define LIBSWDAPP_CLI_HISTORY_MAXLEN  1024

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
 char description[LIBSWDAPP_INTERFACE_NAME_MAXLEN];
 void *ctx;
 void *handle;
 int vid, pid, vid_forced, pid_forced;
 libswdapp_interface_signal_t *signal;
 int (*init)(libswdapp_context_t *libswdappctx);
 int (*deinit)(libswdapp_context_t *libswdappctx);
 int (*set_freq)(libswdapp_context_t *libswdappctx, int freq);
 int (*bitbang)(libswdapp_context_t *libswdappctx, unsigned int bitmask, int GETnSET, unsigned int *value);
 int (*transfer_bits)(libswdapp_context_t *libswdappctx, int bits, char *mosidata, char *misodata, int nLSBfirst);
 int (*transfer_bytes)(libswdapp_context_t *libswdappctx, int bytes, char *mosidata, char *misodata, int nLSBfirst);
 char *sigsetupstr;
 // Below are CACHED values changed only by the interface functions.

 unsigned char latency;
 enum ftdi_interface ftdi_channel;
 int maxfrequency; /// Set by init. Used for frequency calculation.
 int frequency; /// This value shall only be changed by set_freq() routine.
 unsigned int chunksize;
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
 int (*bitbang)(libswdapp_context_t *libswdappctx, unsigned int bitmask, int GETnSET, unsigned int *value);
 int (*transfer_bits)(libswdapp_context_t *libswdappctx, int bits, char *mosidata, char *misodata, int nLSBfirst);
 int (*transfer_bytes)(libswdapp_context_t *libswdappctx, int bytes, char *mosidata, char *misodata, int nLSBfirst);
 int vid, pid;
 unsigned char latency;
 int frequency, maxfrequency;
 unsigned int gpioval, gpiodir; //Shouldnt we use array?
 unsigned int chunksize;
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
int libswdapp_handle_command_signal_usage(void);
int libswdapp_handle_command_signal(libswdapp_context_t *libswdappctx, char *cmd);
int libswdapp_handle_command_interface_init(libswdapp_context_t *libswdappctx, char *cmd);
int libswdapp_handle_command_flash_usage(void);
int libswdapp_handle_command_flash(libswdapp_context_t *libswdappctx, char *command);

int libswd_drv_mosi_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
int libswd_drv_mosi_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
int libswd_drv_miso_8(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
int libswd_drv_miso_32(libswd_ctx_t *libswdctx, libswd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
int libswd_drv_mosi_trn(libswd_ctx_t *libswdctx, int clks);
int libswd_drv_miso_trn(libswd_ctx_t *libswdctx, int clks);

static int libswdapp_interface_ftdi_init(libswdapp_context_t *libswdappctx);
static int libswdapp_interface_ftdi_deinit(libswdapp_context_t *libswdappctx);
static int libswdapp_interface_ftdi_init_ktlink(libswdapp_context_t *libswdappctx);
static int libswdapp_interface_ftdi_set_freq(libswdapp_context_t *libswdappctx, int freq);
static int libswdapp_interface_ftdi_bitbang(libswdapp_context_t *libswdappctx, unsigned int bitmask, int GETnSET, unsigned int *value);
static int libswdapp_interface_ftdi_transfer_bits(libswdapp_context_t *libswdappctx, int bits, char *mosidata, char *misodata, int nLSBfirst);
static int libswdapp_interface_ftdi_transfer_bytes(libswdapp_context_t *libswdappctx, int bytes, char *mosidata, char *misodata, int nLSBfirst);

static int libswdapp_interface_aftdi_init(libswdapp_context_t *libswdappctx);
static int libswdapp_interface_aftdi_deinit(libswdapp_context_t *libswdappctx);
static int libswdapp_interface_aftdi_init_ktlink(libswdapp_context_t *libswdappctx);
static int libswdapp_interface_aftdi_set_freq(libswdapp_context_t *libswdappctx, int freq);
static int libswdapp_interface_aftdi_bitbang(libswdapp_context_t *libswdappctx, unsigned int bitmask, int GETnSET, unsigned int *value);
static int libswdapp_interface_aftdi_transfer_bits(libswdapp_context_t *libswdappctx, int bits, char *mosidata, char *misodata, int nLSBfirst);
static int libswdapp_interface_aftdi_transfer_bytes(libswdapp_context_t *libswdappctx, int bytes, char *mosidata, char *misodata, int nLSBfirst);

int libswd_log(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...);
int libswd_log_flush(libswd_ctx_t *libswdctx, libswd_loglevel_t loglevel, char *msg, ...);

static const libswdapp_interface_config_t libswdapp_interface_configs[] = {
 {
  .name           = "ktlink",
  .description    = "KT-LINK FT2232H based device (using LibFTDI)",
  .init           = libswdapp_interface_ftdi_init_ktlink,
  .deinit         = libswdapp_interface_ftdi_deinit,
  .set_freq       = libswdapp_interface_ftdi_set_freq,
  .bitbang        = libswdapp_interface_ftdi_bitbang,
  .transfer_bits  = libswdapp_interface_ftdi_transfer_bits,
  .transfer_bytes = libswdapp_interface_ftdi_transfer_bytes,
  .vid            = 0x0403,
  .pid            = 0xbbe2,
  .latency        = 1,
  .maxfrequency   = 30000000,
  .frequency      = 1000000,
  .chunksize      = 32768,
  .sigsetupstr    = "signal add:CLK=0x0001 add:MOSI=0x0002 add:MISO=0x0004 add:TMS=0x0008 add:nSWDsel=0x0020 add:SRSTin=0x0040 add:RTCK=0x0080 add:TRST=0x0100 add:SRST=0x0200 add:nTRSTen=0x0400 add:nSRSTen=0x0800 add:RnW=0x1000 add:nMOSIen=0x2000 add:nCLKen=0x4000 add:LED=0x8000 CLK=lo MOSI=lo SRST=hi nCLKen=lo nSWDsel=lo RnW=lo nSRSTen=lo LED=lo MISO SRSTin RTCK",
 },
 {
  .name           = "ktlink-async",
  .description    = "KT-LINK FT2232H based device (using async LibUSB)",
  .init           = libswdapp_interface_aftdi_init_ktlink,
  .deinit         = libswdapp_interface_aftdi_deinit,
  .set_freq       = libswdapp_interface_aftdi_set_freq,
  .bitbang        = libswdapp_interface_aftdi_bitbang,
  .transfer_bits  = libswdapp_interface_aftdi_transfer_bits,
  .transfer_bytes = libswdapp_interface_aftdi_transfer_bytes,
  .vid            = 0x0403,
  .pid            = 0xbbe2,
  .latency        = 1,
  .maxfrequency   = 30000000,
  .frequency      = 1000000,
  .chunksize      = 32768,
  .sigsetupstr    = "signal add:CLK=0x0001 add:MOSI=0x0002 add:MISO=0x0004 add:TMS=0x0008 add:nSWDsel=0x0020 add:SRSTin=0x0040 add:RTCK=0x0080 add:TRST=0x0100 add:SRST=0x0200 add:nTRSTen=0x0400 add:nSRSTen=0x0800 add:RnW=0x1000 add:nMOSIen=0x2000 add:nCLKen=0x4000 add:LED=0x8000 CLK=lo MOSI=lo SRST=hi nCLKen=lo nSWDsel=lo RnW=lo nSRSTen=lo LED=lo MISO SRSTin RTCK",
 },

 {
   .name = '\0',
 }
};

typedef struct libswdapp_flash_stm32f1_memmap
{
 int page_start;
 int page_size;
 int page_end;
 int system_memory_start;
 int system_memory_size;
 int option_bytes_start;
 int option_bytes_size;
 int FLASH_ACR_ADDR;
 int FLASH_KEYR_ADDR;
 int FLASH_OPTKEYR_ADDR;
 int FLASH_SR_ADDR;
 int FLASH_CR_ADDR;
 int FLASH_AR_ADDR;
 int FLASH_OBR_ADDR;
 int FLASH_WRPR_ADDR;
 int idcode[];
} libswdapp_flash_stm32f1_memmap_t;

static const libswdapp_flash_stm32f1_memmap_t libswdapp_flash_stm321f_lowdensity = {
 .page_start          = 0x08000000,
 .page_size           = 0x000003FF,
 .page_end            = 0x08007FFF,
 .system_memory_start = 0x1FFFF000,
 .system_memory_size  = 0x000007FF,
 .option_bytes_start  = 0x1FFFF800,
 .option_bytes_size   = 0x0000000F,
 .FLASH_ACR_ADDR      = 0x40022000,
 .FLASH_KEYR_ADDR     = 0x40022004,
 .FLASH_OPTKEYR_ADDR  = 0x40022008,
 .FLASH_SR_ADDR       = 0x4002200C,
 .FLASH_CR_ADDR       = 0x40022010,
 .FLASH_AR_ADDR       = 0x40022014,
 .FLASH_OBR_ADDR      = 0x4002201C,
 .FLASH_WRPR_ADDR     = 0x40022020,
 .idcode              = {0}
};

static const libswdapp_flash_stm32f1_memmap_t libswdapp_flash_stm321f_mediumdensity = {
 .page_start          = 0x08000000,
 .page_size           = 0x000003FF,
 .page_end            = 0x0801FFFF,
 .system_memory_start = 0x1FFFF000,
 .system_memory_size  = 0x000007FF,
 .option_bytes_start  = 0x1FFFF800,
 .option_bytes_size   = 0x0000000F,
 .FLASH_ACR_ADDR      = 0x40022000,
 .FLASH_KEYR_ADDR     = 0x40022004,
 .FLASH_OPTKEYR_ADDR  = 0x40022008,
 .FLASH_SR_ADDR       = 0x4002200C,
 .FLASH_CR_ADDR       = 0x40022010,
 .FLASH_AR_ADDR       = 0x40022014,
 .FLASH_OBR_ADDR      = 0x4002201C,
 .FLASH_WRPR_ADDR     = 0x40022020,
 .idcode              = {0}
};

static const libswdapp_flash_stm32f1_memmap_t libswdapp_flash_stm321f_highdensity = {
 .page_start          = 0x08000000,
 .page_size           = 0x000007FF,
 .page_end            = 0x0807FFFF,
 .system_memory_start = 0x1FFFF000,
 .system_memory_size  = 0x000007FF,
 .option_bytes_start  = 0x1FFFF800,
 .option_bytes_size   = 0x0000000F,
 .FLASH_ACR_ADDR      = 0x40022000,
 .FLASH_KEYR_ADDR     = 0x40022004,
 .FLASH_OPTKEYR_ADDR  = 0x40022008,
 .FLASH_SR_ADDR       = 0x4002200C,
 .FLASH_CR_ADDR       = 0x40022010,
 .FLASH_AR_ADDR       = 0x40022014,
 .FLASH_OBR_ADDR      = 0x4002201C,
 .FLASH_WRPR_ADDR     = 0x40022020,
 .idcode              = {0x1BA01477,0}
};

static const libswdapp_flash_stm32f1_memmap_t libswdapp_flash_stm321f_connectivityline = {
 .page_start          = 0x08000000,
 .page_size           = 0x000007FF,
 .page_end            = 0x0803FFFF,
 .system_memory_start = 0x1FFFB000,
 .system_memory_size  = 0x000047FF,
 .option_bytes_start  = 0x1FFFF800,
 .option_bytes_size   = 0x0000000F,
 .FLASH_ACR_ADDR      = 0x40022000,
 .FLASH_KEYR_ADDR     = 0x40022004,
 .FLASH_OPTKEYR_ADDR  = 0x40022008,
 .FLASH_SR_ADDR       = 0x4002200C,
 .FLASH_CR_ADDR       = 0x40022010,
 .FLASH_AR_ADDR       = 0x40022014,
 .FLASH_OBR_ADDR      = 0x4002201C,
 .FLASH_WRPR_ADDR     = 0x40022020,
 .idcode              = {0}
};

static const libswdapp_flash_stm32f1_memmap_t *libswdapp_flash_stm32f1_devices[] = {
 &libswdapp_flash_stm321f_lowdensity,
 &libswdapp_flash_stm321f_mediumdensity,
 &libswdapp_flash_stm321f_highdensity,
 &libswdapp_flash_stm321f_connectivityline,
 NULL
};

#define LIBSWDAPP_FLASH_STM32F1_FLASH_OBR_RDPRT_VAL 0x000000A5
#define LIBSWDAPP_FLASH_STM32F1_FLASH_KEYR_KEY1_VAL 0x45670123
#define LIBSWDAPP_FLASH_STM32F1_FLASH_KEYR_KEY2_VAL 0xCDEF89AB

#define LIBSWDAPP_FLASH_STM32F1_FLASH_ACR_PRFTBS   (1<<5)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_ACR_PRFTBE   (1<<4)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_ACR_HLFCYA   (1<<3)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_ACR_LATENCY  (7<<2)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_SR_EOP       (1<<5)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_SR_WRPRTERR  (1<<4)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_SR_PGERR     (1<<2)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_SR_BSY       (1<<0)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_CR_EOPIE     (1<<12)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_CR_ERRIE     (1<<10)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_CR_OPTWRE    (1<<9)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_CR_LOCK      (1<<7)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_CR_STRT      (1<<6)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_CR_OPTER     (1<<5)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_CR_OPTPG     (1<<4)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_CR_MER       (1<<2)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_CR_PER       (1<<1)
#define LIBSWDAPP_FLASH_STM32F1_FLASH_CR_PG        (1<<0)

#endif
