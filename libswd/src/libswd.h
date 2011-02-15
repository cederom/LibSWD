/*
 * $Id$
 *
 * Serial Wire Debug Open Library.
 * Library Header File.
 *
 * Copyright (C) 2010-2011, Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)
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
 * Written by Tomasz Boleslaw CEDRO <tomek.cedro@gmail.com>, 2010-2011;
 *
 */

/** \file libswd.h */

/** \mainpage Serial Wire Debug Open Library.
 * \section Introduction
 * Welcome to the source code documentation repository.
 * LibSWD is an Open-Source framework to deal with with Serial Wire Debug. 
 * It is released under 3-clause BSD license.
 * For more information please visit project website at http://libswd.sf.net
 * \section brief What is this about
 * Serial Wire Debug is an alternative to JTAG (IEEE1149.1) transport layer
 * to access Debug Access Port in ARM-Cortex's based devices.
 * LibSWD provides both bitstream generation and high/low level bus operations.
 * Every bus operation such as request, turnaround, acknowledge, data
 * and parity packet is represented by a swd_cmd_t element that can extend
 * command queue (a standard bidirectional queue) that later can be flushed into
 * real hardware using simple set of interface-specific driver functions.
 * This way LibSWD is almost standalone and can be easily integrated into
 * existing utilities for low-level access and only requires in return to define
 * drivers that controls the interface interconnecting host and target.
 * Such drivers are application specific therefore located in external file
 * crafted for that application and its hardware.
 *
 */



#ifndef __LIBSWD_H__
#define __LIBSWD_H__



/** SWD Packets Bit Fields and Values */
/// Packet Start bit, always set to 1.
#define SWD_REQUEST_START_BITNUM  7
/// Access Port (high) or Debug Port (low) access.
#define SWD_REQUEST_APnDP_BITNUM  6
/// Read (high) or Write (low) access.
#define SWD_REQUEST_RnW_BITNUM    5
/// LSB of the address field in request header.
#define SWD_REQUEST_ADDR_BITNUM   4
/// Target Register Address bit 2.
#define SWD_REQUEST_A2_BITNUM     4
/// Target Register Address bit 3.
#define SWD_REQUEST_A3_BITNUM     3
/// Odd Parity calculated from APnDP, RnW, A[2:3].
#define SWD_REQUEST_PARITY_BITNUM 2
/// Packet Stop bit, always 0.
#define SWD_REQUEST_STOP_BITNUM   1
/// Park wire and switch between receive/transmit.
#define SWD_REQUEST_PARK_BITNUM   0

/// Start Bit Value is always 1.
#define SWD_REQUEST_START_VAL     1
/// Stop Bit Value is always 0.
#define SWD_REQUEST_STOP_VAL      0
/// Park bus and put outputs into Hi-Z state.
#define SWD_REQUEST_PARK_VAL      1
/// Number of bits in request packet header.
#define SWD_REQUEST_BITLEN        8

/// Address field minimal value.
#define SWD_ADDR_MINVAL       0
/// Address field maximal value.
#define SWD_ADDR_MAXVAL       3

/// Number of bits in Acknowledge packet.
#define SWD_ACK_BITLEN        3
/// OK code value.
#define SWD_ACK_OK_VAL        4
/// WAIT code value.
#define SWD_ACK_WAIT_VAL      2
/// FAULT code value.
#define SWD_ACK_FAULT_VAL     1

/// IDCODE register address (RO).
#define SWD_DP_ADDR_IDCODE    0
/// ABORT register address (WO).
#define SWD_DP_ADDR_ABORT     0
/// CTRLSTAT register address (R/W, CTRLSEL=b0)
#define SWD_DP_ADDR_CTRLSTAT  1
/// WCR register address (R/W, CTRLSEL=b1)
#define SWD_DP_ADDR_WCR       1
/// RESEND register address (RO)
#define SWD_DP_ADDR_RESEND    2
/// SELECT register address (WO)
#define SWD_DP_ADDR_SELECT    2
/// RDBUF register address (RO)
#define SWD_DP_ADDR_RDBUF     3

/** SW-DP ABORT Register map */
/// DAPABORT bit number.
#define SWD_ABORT_BITNUM_DAPABORT    0
/// DSTKCMPCLR bit number.
#define SWD_ABORT_BITNUM_DSTKCMPCLR  1
/// DSTKERRCLR bit number.
#define SWD_ABORT_BITNUM_DSTKERRCLR  2
/// DWDERRCLR bit number.
#define SWD_ABORT_BITNUM_DWDERRCLR   3
/// DORUNERRCLR bit number.
#define SWD_ABORT_BITNUM_DORUNERRCLR 4

/** SW-DP CTRL/STAT Register map */
/// ORUNDETECT bit number.
#define SWD_CTRLSTAT_BITNUM_ORUNDETECT    0
/// OSTICKYORUN bit number.
#define SWD_CTRLSTAT_BITNUM_OSTICKYORUN   1
/// OTRNMODE bit number.
#define SWD_CTRLSTAT_BITNUM_OTRNMODE      2
/// OSTICKYCMP bit number.
#define SWD_CTRLSTAT_BITNUM_OSTICKYCMP    4
/// OSTICKYERR bit number.
#define SWD_CTRLSTAT_BITNUM_OSTICKYERR    5
/// OREADOK bit number.
#define SWD_CTRLSTAT_BITNUM_OREADOK       6
/// OWDATAERR bit number.
#define SWD_CTRLSTAT_BITNUM_OWDATAERR     7
/// OMASKLANE bit number.
#define SWD_CTRLSTAT_BITNUM_OMASKLANE     8
/// OTRNCNT bit number.
#define SWD_CTRLSTAT_BITNUM_OTRNCNT       12
/// OCDBGRSTREQ bit number.
#define SWD_CTRLSTAT_BITNUM_OCDBGRSTREQ   26
/// OCDBGRSTACK bit number.
#define SWD_CTRLSTAT_BITNUM_OCDBGRSTACK   27
/// OCDBGPWRUPREQ bit number.
#define SWD_CTRLSTAT_BITNUM_OCDBGPWRUPREQ 28
/// OCDBGPWRUPACK bit number.
#define SWD_CTRLSTAT_BITNUM_OCDBGPWRUPACK 29
/// OCSYSPWRUPREQ bit number.
#define SWD_CTRLSTAT_BITNUM_OCSYSPWRUPREQ 30
/// OCSYSPWRUPACK bit number.
#define SWD_CTRLSTAT_BITNUM_OCSYSPWRUPACK 31
/** SW-DP CTRLSTAT MASKLANE available values */
/// Compare byte lane 0 (0x------FF)
#define SWD_MASKLANE_0 0b0001
/// Compare byte lane 1 (0x----FF--)
#define SWD_MASKLANE_1 0b0010
/// Compare byte lane 2 (0x--FF----)
#define SWD_MASKLANE_2 0b0100
/// Compare byte lane 3 (0xFF------)
#define SWD_MASKLANE_3 0b1000

/** SW-DP SELECT Register map */
/// CTRLSEL bit number.
#define SWD_SELECT_BITNUM_CTRLSEL   0
/// APBANKSEL bit number.
#define SWD_SELECT_BITNUM_APBANKSEL 4
/// APSEL bit number.
#define SWD_SELECT_BITNUM_APSEL     24

/** SW-DP WCR Register map */
/// PRESCALER bit number.
#define SWD_WCR_BITNUM_PRESCALER  0          ///< PRESCALER bit number.
/// WIREMODE bit number.
#define SWD_WCR_BITNUM_WIREMODE   6          ///< WIREMODE bit number.
/// TURNROUND bit number.
#define SWD_WCR_BITNUM_TURNROUND  8          ///< TURNROUND bit number.
/** SW-DP WCR TURNROUND available values */
/// TRN takes one CLK cycle.
#define SWD_TURNROUND_1 0                    ///< TRN takes one CLK cycle. 
/// TRN takes two CLK cycles.
#define SWD_TURNROUND_2 1                    ///< TRN takes two CLK cycles.
/// TRN takes three CLK cycles.
#define SWD_TURNROUND_3 2                    ///< TRN takes three CLK cycles.
/// TRN takes four CLK cycles. ????
#define SWD_TURNROUND_4 3                    ///< TRN takes four CLK cycles. ????
/// shortest TRN time.
#define SWD_TURNROUND_MIN SWD_TURNROUND_1    ///< shortest TRN time.
/// longest TRN time.
#define SWD_TURNROUND_MAX SWD_TURNROUND_4    ///< longest TRN time.
/// Default TRN length is one CLK.
#define SWD_TURNROUND_DEFAULT SWD_TURNROUND_1 ///< Default TRN length is one CLK.

/** AHB-AP Registers Map. TODO!!!! */
/// R/W, 32bit, reset value: 0x43800042
#define AHB_AP_CONTROLSTATUS 0x00  ///< R/W, 32bit, reset value: 0x43800042 
/// R/W, 32bit, reset value: 0x00000000
#define AHB_AP_TAR           0x04  ///< R/W, 32bit, reset value: 0x00000000
/// R/W, 32bit
#define AHB_AP_DRW           0x0C  ///< R/W, 32bit
/// R/W, 32bit
#define AHB_AP_BD0           0x10  ///< R/W, 32bit
/// R/W, 32bit
#define AHB_AP_BD1           0x14  ///< R/W, 32bit
/// R/W, 32bit
#define AHB_AP_BD2           0x18  ///< R/W, 32bit
/// R/W, 32bit
#define AHB_AP_BD3           0x1C  ///< R/W, 32bit
/// RO, 32bit, reset value: 0xE00FF000
#define AHB_AP_DROMT         0xF8  ///< RO, 32bit, reset value: 0xE00FF000
/// RO, 32bit, reset value: 0x24770001
#define AHB_AP_IDR           0xFC  ///< RO, 32bit, reset value: 0x24770001

/** Payload for commands that will not change */
/// SW-DP Reset sequence.
static const char SWD_CMD_SWDPRESET[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 
/// Switches DAP from JTAG to SWD. 
static const char SWD_CMD_JTAG2SWD[]  = {0x79, 0xe7};
/// Switches DAP from SWD to JTAG.
static const char SWD_CMD_SWD2JTAG[]  = {0x3c, 0xe7};

/** Status and Error Codes definitions */
/// Error Codes definition, use this to have its name on debugger.
typedef enum SWD_ERROR_CODE {
 SWD_OK                = 0,  ///< No error.
 SWD_ERROR_GENERAL     =-1,  ///< General error.
 SWD_ERROR_NULLPOINTER =-2,  ///< Null pointer.
 SWD_ERROR_NULLQUEUE   =-3,  ///< Null queue pointer.
 SWD_ERROR_NULLTRN     =-4,  ///< Null TurnaRouNd pointer.
 SWD_ERROR_PARAM       =-5,  ///< Bad parameter.
 SWD_ERROR_OUTOFMEM    =-6,  ///< Out of memory.
 SWD_ERROR_RESULT      =-7,  ///< Bad result.
 SWD_ERROR_RANGE       =-8,  ///< Out of range.
 SWD_ERROR_DEFINITION  =-9,  ///< Definition (internal) error.
 SWD_ERROR_NULLCONTEXT =-10, ///< Null context pointer.
 SWD_ERROR_QUEUE       =-11, ///< Queue error.
 SWD_ERROR_ADDR        =-12, ///< Addressing error.
 SWD_ERROR_APnDP       =-13, ///< Bad APnDP value.
 SWD_ERROR_RnW         =-14, ///< Bad RnW value.
 SWD_ERROR_PARITY      =-15, ///< Parity error.
 SWD_ERROR_ACK         =-16, ///< Acknowledge error.
 SWD_ERROR_ACKUNKNOWN  =-19, ///< Unknown ACK value.
 SWD_ERROR_ACKNOTDONE  =-20, ///< ACK not yet executed on target.
 SWD_ERROR_ACKMISSING  =-21, ///< ACK command not found on the queue.
 SWD_ERROR_ACKMISMATCH =-22, ///< Bad ACK result address.
 SWD_ERROR_ACKORDER    =-23, ///< ACK not in order REQ->TRN->ACK.
 SWD_ERROR_BADOPCODE   =-24, ///< Unsupported operation requested.
 SWD_ERROR_NODATACMD   =-25, ///< Command not found on the queue.
 SWD_ERROR_DATAADDR    =-26, ///< Bad DATA result address.
 SWD_ERROR_NOPARITYCMD =-27, ///< Parity command missing or misplaced.
 SWD_ERROR_PARITYADDR  =-28, ///< Bad PARITY command result address.
 SWD_ERROR_NOTDONE     =-29, ///< Could not end selected task.
 SWD_ERROR_QUEUEROOT   =-30, ///< Queue root not found or null.
 SWD_ERROR_BADCMDTYPE  =-31, ///< Unknown command detected.
 SWD_ERROR_BADCMDDATA  =-32, ///< Bad command data.
 SWD_ERROR_TURNAROUND  =-33, ///< Error during turnaround switch.
 SWD_ERROR_DRIVER      =-34, ///< Driver error.
 SWD_ERROR_ACK_WAIT    =-35, ///< Received ACK WAIT.
 SWD_ERROR_ACK_FAULT   =-36, ///< Received ACK FAULT.
 SWD_ERROR_QUEUENOTFREE=-37, ///< Cannot free resources, queue not empty.
 SWD_ERROR_TRANSPORT   =-38  ///< Transport type unknown or undefined.
} swd_error_code_t;

/** Logging Level Codes definition */
///Logging Level codes definition, use this to have its name on debugger.
typedef enum SWD_LOGLEVEL{
 /// Remain silent.       
 SWD_LOGLEVEL_SILENT  = 0,
 /// Log only informational messages.
 SWD_LOGLEVEL_INFO    = 1,
 /// also log warnings.
 SWD_LOGLEVEL_WARNING = 2,
 /// also lod errors.
 SWD_LOGLEVEL_ERROR   = 3,
 /// Log everything including detailed details.
 SWD_LOGLEVEL_DEBUG   = 4
} swd_loglevel_t;

/** SWD queue and payload data definitions */
/// What is the maximal bit length of the data.
#define SWD_DATA_MAXBITCOUNT   32
/// How many bits are there in a byte.
#define SWD_DATA_BYTESIZE      8
/// How many bits are there in data payload.
#define SWD_DATA_BITLEN        32
/// How long is the command queue by default.
#define SWD_CMDQLEN_DEFAULT  1024;

/** SWD Command Codes definitions.
 * Available values: MISO>0, MOSI<0, undefined=0. To check command direction
 * (read/write) multiply tested value with one of the MOSI or MISO commands
 * - result is positive for equal direction and negative if direction differs.
 */
/// Command Type codes definition, use this to see names in debugger.
typedef enum SWD_CMDTYPE {
 SWD_CMDTYPE_MOSI_DATA    =-7, ///< Contains MOSI data (from host).
 SWD_CMDTYPE_MOSI_REQUEST =-6, ///< Contains MOSI request packet.
 SWD_CMDTYPE_MOSI_TRN     =-5, ///< Bus will switch into MOSI mode.
 SWD_CMDTYPE_MOSI_PARITY  =-4, ///< Contains MOSI data parity.
 SWD_CMDTYPE_MOSI_BITBANG =-3, ///< Allows MOSI operation bit-by-bit.
 SWD_CMDTYPE_MOSI_CONTROL =-2, ///< MOSI control sequence (ie. sw-dp reset).
 SWD_CMDTYPE_MOSI         =-1, ///< Master Output Slave Input direction.
 SWD_CMDTYPE_UNDEFINED    =0,  ///< undefined command, not transmitted.
 SWD_CMDTYPE_MISO         =1,  ///< Master Input Slave Output direction.
 SWD_CMDTYPE_MISO_ACK     =2,  ///< Contains ACK data from target.
 SWD_CMDTYPE_MISO_BITBANG =3,  ///< Allows MISO operation bit-by-bit.
 SWD_CMDTYPE_MISO_PARITY  =4,  ///< Contains MISO data parity.
 SWD_CMDTYPE_MISO_TRN     =5,  ///< Bus will switch into MISO mode.
 SWD_CMDTYPE_MISO_DATA    =6   ///< Contains MISO data (from target).
} swd_cmdtype_t;

/** What is the shift direction LSB-first or MSB-first. */
typedef enum SWD_SHIFTDIR {
 SWD_DIR_LSBFIRST =0, ///< Data is shifted in/out right (LSB-first).
 SWD_DIR_MSBFIRST =1  ///< Data is shifted in/out left (MSB-first).
} swd_shiftdir_t;

/** Command Queue operations codes. */
typedef enum SWD_OPERATION {
 SWD_OPERATION_FIRST         =1, ///< First operation to know its code.
 SWD_OPERATION_QUEUE_APPEND  =1, ///< Append command(s) to the queue.
 SWD_OPERATION_TRANSMIT_HEAD =2, ///< Transmit root..current (head).
 SWD_OPERATION_TRANSMIT_TAIL =3, ///< Transmit current..last (tail).
 SWD_OPERATION_TRANSMIT_ALL  =4, ///< Transmit all commands on the queue.
 SWD_OPERATION_TRANSMIT_ONE  =5, ///< Transmit only current command.
 SWD_OPERATION_TRANSMIT_LAST =6, ///< Transmit last command on the queue.
 SWD_OPERATION_EXECUTE       =7, ///< Queue commands then flush the queue.
 SWD_OPERATION_LAST          =7  ///< Last operation to know its code.
} swd_operation_t;

/** SWD Command Element Structure.
 * In libswd each operation is split into separate commands (request, trn, ack,
 * data, parity) that can be appended to the command queue and later executed. 
 * This organization allows better granularity for tracing bugs and makes
 * possible to compose complete bus/target operations made of simple commands.
 */
typedef struct swd_cmd_t {
 union {          ///< Payload data union.
  char TRNnMOSI;  ///< Holds/sets bus direction: MOSI when zero, MISO for other.
  char request;   ///< Request header data.
  char ack;       ///< Acknowledge response from target.
  int misodata;   ///< Data read from target (MISO).
  int mosidata;   ///< Data written to target (MOSI).
  char misobit;   ///< Single bit read from target (bit-per-char).
  char mosibit;   ///< Single bit written to target (bit-per-char).
  char parity;    ///< Parity bit for data payload.
  char control;   ///< Control transfer data (one byte).
 };
 char bits;       ///< Payload bit count == clk pulses on the bus.
 char cmdtype;    ///< Command type as defined by swd_cmdtype_t. 
 char done;       ///< Non-zero if operation already executed.
 struct swd_cmd_t *prev, *next; ///< Pointer to the previous/next command.
} swd_cmd_t;

/** Context configuration structure */
typedef struct {
 char initialized;        ///< Context must be initialized prior use.
 char trnlen;             ///< How many CLK cycles will TRN use.
 int  maxcmdqlen;         ///< How long command queue can be.
 swd_loglevel_t loglevel; ///< Holds Logging Level setting.
} swd_context_config_t;

/** Most actual Serial Wire Debug Port Registers */
typedef struct {
 char ack;     ///< Last known state of ACK response.
 int idcode;   ///< Target's IDCODE register value.
 int abort;    ///< Last known ABORT register value.
 int ctrlstat; ///< Last known CTRLSTAT register value.
 int wcr;      ///< Last known WCR register value.
 int select;   ///< Last known SELECT register value.
 int rdbuf;    ///< Last known RDBUF register (payload data) value.
} swd_swdp_t;

/** Most actual Advanced High Bandwidth Access Peripherial Bus Reisters */
typedef struct {
 int controlstatus; ///< Last known CONTROLSTATUS register value.
 int tar;           ///< Last known TAR register value.
 int drw;           ///< Last known DRW register value.
 int bd0;           ///< Last known BD0 register value.
 int bd1;           ///< Last known BD1 register value.
 int bd2;           ///< Last known BD2 register value.
 int bd3;           ///< Last known BD3 register value.
 int dromt;         ///< Last known DROMT register value.
 int idr;           ///< Last known IDR register value.
} swd_ahbap_t;

/** Interface Driver structure. It holds pointer to the driver structure that
 * keeps driver information necessary to work with the physical interface.
 */
typedef struct {
 void *device;
} swd_driver_t;

/** Boolean values definition */
typedef enum {
 SWD_FALSE=0, ///< False is 0.
 SWD_TRUE=1   ///< True is 1.
} swd_bool_t;

/** SWD Context Structure definition. It stores all the information about
 * the library, drivers and interface configuration, target status along
 * with DAP/AHBAP data/instruction internal registers, and the command
 * queue. Bus operations are stored on the command queue. There may be
 * more than one context in use by a host software, each one for single
 * interface-target pair. Most of the target operations made with libswd
 * are required to pass swd_ctx_t pointer structure that also remembers
 * last known state of the target's internal registers.
 */
typedef struct {
 swd_cmd_t *cmdq;             ///< Command queue, stores all bus operations.
 swd_context_config_t config; ///< Target specific configuration.
 swd_driver_t *driver;        ///< Pointer to the interface driver structure.
 swd_swdp_t misoswdp;         ///< Last known read from the SW-DP register.
 swd_swdp_t mosiswdp;         ///< Last known write to the SW-DP register.
 swd_ahbap_t misoahbap;       ///< Last known read from AHB-AP register.
 swd_ahbap_t mosiahbap;       ///< Last known write ti the AHB-AP register.
} swd_ctx_t;


/* DATA GENERATED BELOW ARE TO BE SHIFTED OUT MSB FIRST, UNLIKE IN JTAG!
 * These functions generate a bistream packets for SWD communications.
 * Bistream is returned in byte array pointed by *buffer parameter.
 * Actual positive byte count is returned by the function.
 * Negative return value means an error. Zero means no data was generated.
 * If you need to swap bit direction, use bitswap functions provided below.
 * All functions name start with "swd_bitgen_" prefix.
 */

/*******************************************************************************
 * \defgroup swd_bin Binary helper functions.
 * @{
 ******************************************************************************/
int swd_bin8_parity_even(char *data, char *parity);
int swd_bin32_parity_even(int *data, char *parity);
int swd_bin8_print(char *data);
int swd_bin32_print(int *data);
char *swd_bin8_string(char *data);
char *swd_bin32_string(int *data);

int swd_bin8_bitswap(unsigned char *buffer, int bitcount);
int swd_bin32_bitswap(unsigned int *buffer, int bitcount);
/** @} */

/*******************************************************************************
 * \defgroup swd_cmd_queue Command Queue helper functions
 * @{
 ******************************************************************************/

int swd_cmd_queue_init(swd_cmd_t *cmdq);
swd_cmd_t* swd_cmd_queue_find_root(swd_cmd_t *cmdq);
swd_cmd_t* swd_cmd_queue_find_tail(swd_cmd_t *cmdq);
int swd_cmd_queue_append(swd_cmd_t *cmdq, swd_cmd_t *cmd);
int swd_cmd_queue_free(swd_cmd_t *cmdq);
int swd_cmd_queue_free_head(swd_cmd_t *cmdq);
int swd_cmd_queue_free_tail(swd_cmd_t *cmdq);
/** @} */

/*******************************************************************************
 * \defgroup SWD Command queue elements generation routines.
 * These command quants are created in memory and can be easily appended
 * to the end of existing queue pointed by *cmdq element.
 * All functions here start with "swd_cmd_queue_append_" prefix.
 * @{
 ******************************************************************************/


/*******************************************************************************
 * SWD Commands Queue Elements Generator Routines.
 * These command quants are created in memory and can be easily appended
 * to the end of queue pointed by *cmdq element.
 * All functions here start with "swd_cmd_queue_append_" prefix.
 ******************************************************************************/

int swd_cmd_queue_append_mosi_request(swd_ctx_t *swdctx, char *request);
int swd_cmd_queue_append_mosi_trn(swd_ctx_t *swdctx);
int swd_cmd_queue_append_miso_trn(swd_ctx_t *swdctx);
int swd_cmd_queue_append_miso_nbit(swd_ctx_t *swdctx, char **data, int count);
int swd_cmd_queue_append_mosi_nbit(swd_ctx_t *swdctx, char *data, int count);
int swd_cmd_queue_append_mosi_parity(swd_ctx_t *swdctx, char *parity);
int swd_cmd_queue_append_miso_parity(swd_ctx_t *swdctx, char *parity);
int swd_cmd_queue_append_miso_data(swd_ctx_t *swdctx, int *data);
int swd_cmd_queue_append_miso_data_p(swd_ctx_t *swdctx, int *data, char *parity);
int swd_cmd_queue_append_miso_n_data_p(swd_ctx_t *swdctx, int **data, char **parity, int count);
int swd_cmd_queue_append_mosi_data(swd_ctx_t *swdctx, int *data);
int swd_cmd_queue_append_mosi_data_ap(swd_ctx_t *swdctx, int *data);
int swd_cmd_queue_append_mosi_data_p(swd_ctx_t *swdctx, int *data, char *parity);
int swd_cmd_append_mosi_n_data_ap(swd_ctx_t *swdctx, int **data, int count);
int swd_cmd_append_mosi_n_data_p(swd_ctx_t *swdctx, int **data, char **parity, int count);
int swd_cmd_queue_append_miso_ack(swd_ctx_t *swdctx, char *ack);
int swd_cmd_queue_append_mosi_control(swd_ctx_t *swdctx, char *ctlmsg, int len);
int swd_cmd_queue_append_swdpreset(swd_ctx_t *swdctx);
int swd_cmd_queue_append_jtag2swd(swd_ctx_t *swdctx);
int swd_cmd_queue_append_swd2jtag(swd_ctx_t *swdctx);

int swd_bus_setdir_mosi(swd_ctx_t *swdctx);
int swd_bus_setdir_miso(swd_ctx_t *swdctx);
/** @} */

/*******************************************************************************
 * \defgroup swd_bit_gen Packet generation helper routines.
 * @{
 ******************************************************************************/

int swd_bit8_gen_request(swd_ctx_t *swdctx, char *APnDP, char *RnW, char *addr, char *request);
/** @} */

/*******************************************************************************
 * \defgroup swd_drv SWD Bus and Interface Driver Transfer Functions that
 * executes command queue.
 * @{
 ******************************************************************************/

/*
extern int swd_drv_mosi_8(swd_ctx_t *swdctx, char *data, int bits, int nLSBfirst);
extern int swd_drv_mosi_32(int *data, int bits, int nLSBfirst);
extern int swd_drv_miso_8(char *data, int bits, int nLSBfirst);
extern int swd_drv_miso_32(int *data, int bits, int nLSBfirst);
*/
int swd_transmit(swd_ctx_t *swdctx, swd_cmd_t *cmd);
int swd_cmd_queue_flush(swd_ctx_t *swdctx, swd_operation_t operation);
/** @} */

/*******************************************************************************
 * \defgroup swd_packet SWD Operations Generators: Request, ACK, Data.
 * These functions generate payloads and queue up all elements/commands
 * necessary to perform requested operations on the SWD bus. Depending
 * on operation type, elements can be executed or queued up for future transfer.
 * @{
 ******************************************************************************/

int swd_mosi_request
(swd_ctx_t *swdctx, swd_operation_t operation, char *APnDP, char *RnW, char *addr);
int swd_miso_ack(swd_ctx_t *swdctx, swd_operation_t operation, char *ack);
int swd_mosi_data_p(swd_ctx_t *swdctx, swd_operation_t operation, int *data, char *parity);
int swd_mosi_data_ap(swd_ctx_t *swdctx, swd_operation_t operation, int *data);
int swd_miso_data_p(swd_ctx_t *swdctx, swd_operation_t operation, int *data, char *parity);
int swd_mosi_jtag2swd(swd_ctx_t *swdctx, swd_operation_t operation);
/** @} */

/*******************************************************************************
 * \defgroup swd_highlevel High-level SWD operation functions.
 * Operate on pointers, where target data is stored. Operation can be
 * SWD_OPERATION_QUEUE for queueing the command for later execution,
 * or SWD_OPERATION_EXECUTE to obtain results on function return.
 * Return values: negative number on error, data on success.
 ******************************************************************************/

int swd_jtag2swd(swd_ctx_t *swdctx, swd_operation_t operation);
int swd_idcode(swd_ctx_t *swdctx, swd_operation_t operation, int *idcode, char *ack, char *parity);
/** @} */

/*******************************************************************************
 * \defgroup swd_log Miscelanous logging functionalities.
 * @{
 ******************************************************************************/

int swd_log(swd_loglevel_t loglevel, char *msg);
/** @} */

/*******************************************************************************
 * \defgroup swd_error Error handling and information routines.
 * @{
 ******************************************************************************/

char *swd_error_string(swd_error_code_t error);
/** @} */

/*******************************************************************************
 * \defgroup swd_init Library and Context (de)initialization routines.
 * @{
 ******************************************************************************/

swd_ctx_t *swd_init(void);
int swd_deinit_ctx(swd_ctx_t *swdctx);
int swd_deinit_cmdq(swd_ctx_t *swdctx);
int swd_deinit(swd_ctx_t *swdctx);
/** @} */

/*******************************************************************************
 * \defgroup swd_drv LibSWD driver bridge functions to drive real hardware.
 * @{
 ******************************************************************************/

int swd_drv_mosi_8(swd_ctx_t *swdctx, char *data, int bits, int direction);
int swd_drv_mosi_32(swd_ctx_t *swdctx, int *data, int bits, int direction);
int swd_drv_miso_8(swd_ctx_t *swdctx, char *data, int bits, int direction);
int swd_drv_miso_32(swd_ctx_t *swdctx, int *data, int bits, int direction);
int swd_drv_mosi_trn(swd_ctx_t *swdctx, int clks);
int swd_drv_miso_trn(swd_ctx_t *swdctx, int clks);
/** @} */

#endif
