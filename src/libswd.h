/*
 * $Id$
 *
 * Serial Wire Debug Open Library.
 * Library Header File.
 *
 * Copyright (C) 2010-2012, Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)
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
 * Written by Tomasz Boleslaw CEDRO <cederom@tlen.pl>, 2010-2012;
 *
 */

/** \file libswd.h Serial Wire Debug Open Library Header File. */

/** \mainpage Serial Wire Debug Open Library.
 *
 * \section doc_introduction Introduction
 * LibSWD is an Open-Source framework to deal with with Serial Wire Debug Port in accordance to ADI (Arm Debug Interface, version 5.0 at the moment) specification. It is released under 3-clause BSD license. For more information please visit project website at http://libswd.sf.net
 * \section doc_brief What is this about
 * Serial Wire Debug is an alternative to JTAG (IEEE1149.1) transport layer for accessing the Debug Access Port in ARM-Cortex based devices. LibSWD provides methods for bitstream generation on the wire using simple but flexible API that can reuse capabilities of existing applications for easier integration.
 * Every bus operation such as control, request, turnaround, acknowledge, data and parity packet is named a "command" represented by a swd_cmd_t data type that builds up the queue that later can be flushed into real hardware using standard set of (application-specific) driver functions.
 * This way LibSWD is almost standalone and can be easily integrated into existing utilities for low-level access and only requires in return to define driver bridge that controls the physical interface interconnecting host and target.
 * Drivers and other application-specific functions are "extern" and located in external file crafted for that application and its hardware. LibSWD is therefore best way to make your application SWD aware.
 *
 * \section doc_details How it works
 * \subsection doc_context SWD Context
 * The most important data type in LibSWD is swd_ctx_t structure, a context that represents logical entity of the swd bus (transport layer between host and target) with all its parameters, configuration and command queue. Context is being created with swd_init() function that returns pointer to allocated virgin structure, and it can be destroyed with swd_deinit() function taking the pointer as argument. Context can be set only for one interface-target pair, but there might be many different contexts in use if necessary, so amount of devices in use is not limited. 
 *
 * \subsection doc_functions Functions
 * All functions in general operates on pointer type and returns number of processed elements on success or negative value with swd_error_code_t on failure. Functions are grouped by functionality that is denoted by function name prefix (ie. swd_bin* are for binary operations, swd_cmdq* deals with command queue, swd_cmd_enqueue* deals with creating commands and attaching them to queue, swd_bus* performs operation on the swd transport system, swd_drv* are the interface drivers, etc). Because programs using libswd for transport can queue multiple operations and don't handle errors of each transaction apropriately, swd_drv_transmit() function verifies the ACK and PARITY operation results directly after execution (read from target) and return error code if necessary. When error is detected and there were some pending perations enqueued for execution, they are discarded and removed from the queue (they would not be accepted by the target anyway), the queue is then again ready to accept new transactions (i.e. error handling operations).
 *
 * Standard end-users are encouraged to only use high level functions (swd_bus*, swd_dap*, swd_dp*) to perform operations on the swd transport layer and the target's DAP (Debug Access Port) and its components such as DP (Debug Port) and the AP (Access Port). More advanced users however may use low level functions (swd_cmd*, swd_cmdq*) to group them into new high-level functions that automates some tasks (such as high-level functions does). Functions of type "extern" are the ones to implement in external file by developers that want to incorporate LibSWD into their application. Context structure also has void pointer in the swd_driver_t structure that can hold address of the external driver structure to be passed into internal swd drivers (extern swd_drv* functions) that wouldn't be accessible otherwise.
 *
 * \subsection doc_commands Commands
 * Bus operations are split into "commands" represented by swd_cmd_t data type. They form a bidirectional command queue that is part of swd_ctx_t structure. Command type, and so its payload, can be one of: control (user defined 8-bit payload), request (according to the standard), ack, data, parity (data and parity are separate commands!), trn, bitbang and idle (equals to control with zero data). Command type is defined by swd_cmdtype_t and its code can be negative (for MOSI operations) or positive (for MISO operations) - this way bus direction can be easily calculated by multiplying two operation codes (when the result is negative bus will have to change direction), so the libswd "knows" when to put additional TRN command of proper type between enqueued commands.
 *
 * Payload is stored within union type and its data can be accessed according to payload name, or simply with data8 (char) and data32 (int) fields. Payload for write (MOSI) operations is stored on command creation, but payload for read (MISO) operations becomes available only after command is executed by the interface driver. There are 3 methods of accessing read data - flushing the queue into driver then reading queue directly, single stepping queue execution (flush one-by-one) then reading context log of last executed command results (there are separate fields of type swd_transaction_t in swd_ctx_t's log structure for read and write operations that are updated by swd_drv_transmit() function before write and after read), or  providing a double pointer on command creation to have constant access to its data after execution.
 *
 * After all commands are enqueued with swd_cmd_enqueue* function set, it is time to send them into physical device with swd_cmdq_flush() funtion. According to the swd_operation_t parameter commands can be flushed one-by-one, all of them, only to the selected command or only after selected command. For low level functions all of these options are available, but for high-level functions only two of them can be used - SWD_OPERATION_ENQUEUE (but not send to the driver) and SWD_OPERATION_EXECUTE (all unexecuted commands on the queue are executed by the driver sequentially) - that makes it possible to perform bus operations one after another having their result just at function return, or compose more advanced sequences leading to preferred result at execution time. Because high-level functions provide simple and elegant manner to get the operation result, it is advised to use them instead dealing with low-level functions (implementing memory management, data allocation and queue operation) that exist only to make high-level functions possible. 
 *
 * \section doc_drivers Drivers
 * Calling the swd_cmdq_flush() function leads to execution of not yet executed commands from the queue (in a manner specified by the operation parameter) on the SWD bus (transport layer between interface and target, not the bus of the target itself) by swd_drv_transmit() function that use application specific "extern" functions defined in external file (ie. libswd_drv_urjtag.c) to operate on a real hardware using drivers from existing application. LibSWD use only swd_drv_{mosi,miso}_{8,32} (separate for 8-bit char and 32-bit int data cast type) and swd_drv_{mosi,miso}_trn functions to interact with drivers, so it is possible to easily reuse low-level and high-level devices for communications, as they have all information necessary to perform exact actions - number of bits, payload, command type, shift direction and bus direction. It is even possible to send raw bytes on the bus (control command) or bitbang the bus (bitbang command) if necessary. MOSI (Master Output Slave Input) and MISO (Master Input Slave Output) was used to clearly distinguish transfer direction (from master-interface to target-slave), as opposed to ambiguous read/write statements, so after swd_drv_mosi_trn() master should have its buffers set to output and target inputs active. Drivers, as most of the LibSWD functions, works on data pointers instead data copy and returns number of elements processed (bits in this case) or negative error code on failure.
 *
 * \section Error and Retry handling
 * LibSWD is equipped with optional automatic error handling in order to make error and retry handling easier for external applications that were meant for JTAG applications (such as OpenOCD) which first enqueue lots of operations and then flushes them into hardware loosing information on where the target reported problem with ACK!=OK. The default behavior of LibSWD for ACK!=OK response from Target is to truncate the queue right after the bad ACK (eventually executing the necessary data phase before doing that) to preserve synchronization between command queue (swd_ctx_t->cmdq) and the Target state. This can be changed by clearing out the swd_ctx_t.config.trunccmdqonerror field that disables queue truncate on error, then applying the swd_dap_retry() in the application flush mechanism for both DP and AP operations. swd_dap_retry() will try to find the ACK!=OK on the queue that caused an error then perform operation retry to fix the situation, or fail permanently (Protocol Error Sequence, Retry Count, etc). Note that retry will be handled in a different way than it was performed on the original command queue and it will use separate command queue attached to a bad ACK command element on the queue. This approach gives ability to handle different situations accordingly, does not interfere with the original queue and does not loose information what additional operations had been performed, in perfect situation it should end up in having the original queue executed as there was no error/retry. 
 *
 * \section doc_example Example
 * \code
 *  #include <libswd.h>
 *  int main(){
 *   swd_ctx_t *swdctx;
 *   int res, *idcode;
 *   swdctx=swd_init();
 *   if (swdctx==NULL) return -1;
 *   //we might need to pass external driver structure to swd_drv* functions 
 *   //swdctx->driver->device=...
 *   res=swd_dap_detect(swdctx, SWD_OPERATION_EXECUTE, &idcode);
 *   if (res<0){
 *    printf("ERROR: %s\n", swd_error_string(res));
 *    return res;
 *   } else printf("IDCODE: 0x%X (%s)\n", *idcode, swd_bin32_string(idcode));
 *   swd_deinit(swdctx);
 *   return 0;
 *  }
 * \endcode
 */

#include <stdlib.h>
#include <stdarg.h>

#ifndef __LIBSWD_H__
#define __LIBSWD_H__

/** SWD Packets Bit Fields and Values */
/// Request packet Start field bitnumber, always set to 1.
#define SWD_REQUEST_START_BITNUM  0
/// Requect packet APnDP field bitnumber. Access Port ot Debug Port access.
#define SWD_REQUEST_APnDP_BITNUM  1
/// Request packet RnW field bitnumber. Read or Write operation.
#define SWD_REQUEST_RnW_BITNUM    2
/// Request packet ADDR field bitnumber. AP/DP register address (two bit field).
#define SWD_REQUEST_ADDR_BITNUM   3
/// Request packet A2 field bitnumber.
#define SWD_REQUEST_A2_BITNUM     3
/// Request packet A3 field bitnumber.
#define SWD_REQUEST_A3_BITNUM     4
/// Request packet PARITY field bitnumber. Odd Parity calculated from APnDP, RnW, A[2:3].
#define SWD_REQUEST_PARITY_BITNUM 5
/// Request packet STOP field bitnumber. Packet Stop bit, always 0.
#define SWD_REQUEST_STOP_BITNUM   6
/// Request packet PARK field bitnumber. Park wire and switch between receive/transmit.
#define SWD_REQUEST_PARK_BITNUM   7

/// Request Start field bitmask
#define SWD_REQUEST_START         (1 << SWD_REQUEST_START_BITNUM)
/// Requect packet APnDP field bitmask.
#define SWD_REQUEST_APnDP         (1 << SWD_REQUEST_APnDP_BITNUM)
/// Request packet RnW field bitmask.
#define SWD_REQUEST_RnW           (1 << SWD_REQUEST_RnW_BITNUM)
/// Request packet ADDR field bitmask.
#define SWD_REQUEST_ADDR          (1 << SWD_REQUEST_A2_BITNUM) | (1 << SWD_REQUEST_A3_BITNUM)
/// Request packet A2 field bitmask
#define SWD_REQUEST_A2            (1 << SWD_REQUEST_A2_BITNUM)
/// Request packet A3 field bitmask.
#define SWD_REQUEST_A3            (1 << SWD_REQUEST_A3_BITNUM)
/// Request packet PARITY field bitmask.
#define SWD_REQUEST_PARITY        (1 << SWD_REQUEST_PARITY_BITNUM)
/// Request packet STOP field bitmask.
#define SWD_REQUEST_STOP          (1 << SWD_REQUEST_STOP_BITNUM)
/// Request packet PARK field bitmask.
#define SWD_REQUEST_PARK          (1 << SWD_REQUEST_PARK_BITNUM)

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
#define SWD_ADDR_MAXVAL       0xC 

/// Number of bits in Acknowledge packet.
#define SWD_ACK_BITLEN        3
/// OK code value.
#define SWD_ACK_OK_VAL        1
/// WAIT code value.
#define SWD_ACK_WAIT_VAL      2
/// FAULT code value.
#define SWD_ACK_FAULT_VAL     4

/// IDCODE register address (RO).
#define SWD_DP_IDCODE_ADDR    0
/// ABORT register address (WO).
#define SWD_DP_ABORT_ADDR     0
/// CTRLSTAT register address (R/W, CTRLSEL=b0)
#define SWD_DP_CTRLSTAT_ADDR  0x4
/// WCR register address (R/W, CTRLSEL=b1)
#define SWD_DP_WCR_ADDR       0x4
/// RESEND register address (RO)
#define SWD_DP_RESEND_ADDR    0x8
/// SELECT register address (WO)
#define SWD_DP_SELECT_ADDR    0x8
/// RDBUFF register address (RO)
#define SWD_DP_RDBUFF_ADDR    0xC
/// ROUTESEL register address (WO)
#define SWD_DP_ROUTESEL_ADDR  0xC

/** SW-DP ABORT Register map */
/// DAPABORT bit number.
#define SWD_DP_ABORT_DAPABORT_BITNUM   0
/// DSTKCMPCLR bit number.
#define SWD_DP_ABORT_STKCMPCLR_BITNUM  1
/// DSTKERRCLR bit number.
#define SWD_DP_ABORT_STKERRCLR_BITNUM  2
/// DWDERRCLR bit number.
#define SWD_DP_ABORT_WDERRCLR_BITNUM   3
/// DORUNERRCLR bit number.
#define SWD_DP_ABORT_ORUNERRCLR_BITNUM 4

/// DAPABORT bitmask
#define SWD_DP_ABORT_DAPABORT           (1 << SWD_DP_ABORT_DAPABORT_BITNUM)
/// DSTKCMPCLR bitmask
#define SWD_DP_ABORT_STKCMPCLR         (1 << SWD_DP_ABORT_STKCMPCLR_BITNUM)
/// DSTKERRCLR bitmask
#define SWD_DP_ABORT_STKERRCLR         (1 << SWD_DP_ABORT_STKERRCLR_BITNUM)
/// DWDERRCLR bitmask
#define SWD_DP_ABORT_WDERRCLR          (1 << SWD_DP_ABORT_WDERRCLR_BITNUM)
/// DORUNERRCLR bitmask
#define SWD_DP_ABORT_ORUNERRCLR        (1 << SWD_DP_ABORT_ORUNERRCLR_BITNUM)


/** SW-DP CTRL/STAT Register map */
/// ORUNDETECT bit number.
#define SWD_DP_CTRLSTAT_ORUNDETECT_BITNUM    0
/// STICKYORUN bit number.
#define SWD_DP_CTRLSTAT_STICKYORUN_BITNUM    1
/// TRNMODE bit number.
#define SWD_DP_CTRLSTAT_TRNMODE_BITNUM       2
/// STICKYCMP bit number.
#define SWD_DP_CTRLSTAT_STICKYCMP_BITNUM     4
/// STICKYERR bit number.
#define SWD_DP_CTRLSTAT_STICKYERR_BITNUM     5
/// READOK bit number.
#define SWD_DP_CTRLSTAT_READOK_BITNUM        6
/// WDATAERR bit number.
#define SWD_DP_CTRLSTAT_WDATAERR_BITNUM      7
/// MASKLANE bit number.
#define SWD_DP_CTRLSTAT_MASKLANE_BITNUM      8
/// TRNCNT bit number.
#define SWD_DP_CTRLSTAT_TRNCNT_BITNUM        12
/// CDBGRSTREQ bit number.
#define SWD_DP_CTRLSTAT_CDBGRSTREQ_BITNUM    26
/// CDBGRSTACK bit number.
#define SWD_DP_CTRLSTAT_CDBGRSTACK_BITNUM    27
/// CDBGPWRUPREQ bit number.
#define SWD_DP_CTRLSTAT_CDBGPWRUPREQ_BITNUM  28
/// CDBGPWRUPACK bit number.
#define SWD_DP_CTRLSTAT_CDBGPWRUPACK_BITNUM  29
/// CSYSPWRUPREQ bit number.
#define SWD_DP_CTRLSTAT_CSYSPWRUPREQ_BITNUM  30
/// CSYSPWRUPACK bit number.
#define SWD_DP_CTRLSTAT_CSYSPWRUPACK_BITNUM  31

/// ORUNDETECT bitmask
#define SWD_DP_CTRLSTAT_ORUNDETECT           (1 << SWD_DP_CTRLSTAT_ORUNDETECT_BITNUM)
/// STICKYORUN bitmask
#define SWD_DP_CTRLSTAT_STICKYORUN           (1 << SWD_DP_CTRLSTAT_OSTICKYORUN_BITNUM)
/// STICKYCMP bitmask
#define SWD_DP_CTRLSTAT_STICKYCMP            (1 << SWD_DP_CTRLSTAT_OSTICKYCMP_BITNUM)
/// STICKYERR bitmask
#define SWD_DP_CTRLSTAT_STICKYERR            (1 << SWD_DP_CTRLSTAT_OSTICKYERR_BITNUM)
/// READOK bitmask
#define SWD_DP_CTRLSTAT_READOK               (1 << SWD_DP_CTRLSTAT_OREADOK_BITNUM)
/// WDATAERR bitmask
#define SWD_DP_CTRLSTAT_WDATAERR             (1 << SWD_DP_CTRLSTAT_OWDATAERR_BITNUM)
/// CDBGRSTREQ bitmask
#define SWD_DP_CTRLSTAT_CDBGRSTREQ           (1 << SWD_DP_CTRLSTAT_OCDBGRSTREQ_BITNUM)
/// CDBGRSTACK bitmask
#define SWD_DP_CTRLSTAT_CDBGRSTACK           (1 << SWD_DP_CTRLSTAT_OCDBGRSTACK_BITNUM)
/// CDBGPWRUPREQ bitmask
#define SWD_DP_CTRLSTAT_CDBGPWRUPREQ         (1 << SWD_DP_CTRLSTAT_OCDBGPWRUPREQ_BITNUM)
/// CDBGPWRUPACK bitmask
#define SWD_DP_CTRLSTAT_CDBGPWRUPACK         (1 << SWD_DP_CTRLSTAT_OCDBGPWRUPACK_BITNUM)
/// CSYSPWRUPREQ bitmask
#define SWD_DP_CTRLSTAT_CSYSPWRUPREQ         (1 << SWD_DP_CTRLSTAT_OCSYSPWRUPREQ_BITNUM)
/// CSYSPWRUPACK bitmask
#define SWD_DP_CTRLSTAT_CSYSPWRUPACK         (1 << SWD_DP_CTRLSTAT_OCSYSPWRUPACK_BITNUM)

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
#define SWD_DP_SELECT_CTRLSEL_BITNUM   0
/// APBANKSEL bit number.
#define SWD_DP_SELECT_APBANKSEL_BITNUM 4
/// APSEL bit number.
#define SWD_DP_SELECT_APSEL_BITNUM     24

///CTRLSEL bitmask
#define SWD_DP_SELECT_CTRLSEL          (0xFF << SWD_DP_SELECT_CTRLSEL_BITNUM)
/// APBANKSEL bitmask
#define SWD_DP_SELECT_APBANKSEL        (0xF << SWD_DP_SELECT_APBANKSEL_BITNUM)
/// APSEL bitmask
#define SWD_DP_SELECT_APSEL            (1 << SWD_DP_SELECT_APSEL_BITNUM)

/** SW-DP WCR Register map */
/// PRESCALER bit number.
#define SWD_DP_WCR_PRESCALER_BITNUM  0          ///< PRESCALER bit number.
/// WIREMODE bit number.
#define SWD_DP_WCR_WIREMODE_BITNUM   6          ///< WIREMODE bit number.
/// TURNROUND bit number.
#define SWD_DP_WCR_TURNROUND_BITNUM  8          ///< TURNROUND bit number.

/** SW-DP WCR TURNROUND available values */
/// TRN takes one CLK cycle.
#define SWD_TURNROUND_1_CODE  0              ///< TRN takes one CLK cycle. 
#define SWD_TURNROUND_1_VAL   1
/// TRN takes two CLK cycles.
#define SWD_TURNROUND_2_CODE  1                    ///< TRN takes two CLK cycles.
#define SWD_TURNROUNT_2_VAL   2
/// TRN takes three CLK cycles.
#define SWD_TURNROUND_3_CODE  2                    ///< TRN takes three CLK cycles.
#define SWD_TURNROUND_3_VAL   3
/// TRN takes four CLK cycles. ????
#define SWD_TURNROUND_4_CODE  3                    ///< TRN takes four CLK cycles. ????
#define SWD_TURNROUND_4_VAL   4
/// shortest TRN time.
#define SWD_TURNROUND_MIN_VAL SWD_TURNROUND_1_VAL    ///< shortest TRN time.
#define SWD_TURNROUND_MIN_CODE SWD_TURNOUND_1_CODE
/// longest TRN time.
#define SWD_TURNROUND_MAX_VAL SWD_TURNROUND_4_VAL    ///< longest TRN time.
#define SWD_TURNROUND_MAX_CODE SWD_TURNROUND_4_CODE
/// Default TRN length is one CLK.
#define SWD_TURNROUND_DEFAULT_VAL SWD_TURNROUND_1_VAL ///< Default TRN length is one CLK.

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

/** Payload for commands that will not change, transmitted MSBFirst */
/// SW-DP Reset sequence.
static const char SWD_CMD_SWDPRESET[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00}; 
/// Switches DAP from JTAG to SWD. 
static const char SWD_CMD_JTAG2SWD[]  = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9e, 0xe7};
/// Switches DAP from SWD to JTAG.
static const char SWD_CMD_SWD2JTAG[]  = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3c, 0xe7};
/// Inserts idle clocks for proper data processing.
static const char SWD_CMD_IDLE[] = {0x00};

/** Status and Error Codes definitions */
/// Error Codes definition, use this to have its name on debugger.
typedef enum {
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
 SWD_ERROR_DATAPTR     =-26, ///< Bad DATA pointer address.
 SWD_ERROR_NOPARITYCMD =-27, ///< Parity after Data missing or misplaced.
 SWD_ERROR_PARITYPTR   =-28, ///< Bad PARITY pointer address.
 SWD_ERROR_NOTDONE     =-29, ///< Could not end selected task.
 SWD_ERROR_QUEUEROOT   =-30, ///< Queue root not found or null.
 SWD_ERROR_QUEUETAIL   =-31, ///< Queue tail not found or null.
 SWD_ERROR_BADCMDTYPE  =-32, ///< Unknown command detected.
 SWD_ERROR_BADCMDDATA  =-33, ///< Bad command data.
 SWD_ERROR_TURNAROUND  =-34, ///< Error during turnaround switch.
 SWD_ERROR_DRIVER      =-35, ///< Driver error.
 SWD_ERROR_ACK_WAIT    =-36, ///< Received ACK WAIT.
 SWD_ERROR_ACK_FAULT   =-37, ///< Received ACK FAULT.
 SWD_ERROR_QUEUENOTFREE=-38, ///< Cannot free resources, queue not empty.
 SWD_ERROR_TRANSPORT   =-39, ///< Transport type unknown or undefined.
 SWD_ERROR_DIRECTION   =-40, ///< Direction error (LSb/MSb first).
 SWD_ERROR_LOGLEVEL    =-41  ///< Invalid loglevel number.
} swd_error_code_t;

/** Logging Level Codes definition */
///Logging Level codes definition, use this to have its name on debugger.
typedef enum {
 SWD_LOGLEVEL_MIN     = 0,
 SWD_LOGLEVEL_SILENT  = 0, ///< Remain silent.
 SWD_LOGLEVEL_ERROR   = 1, ///< Show errors.
 SWD_LOGLEVEL_WARNING = 2, ///< Show warnings.
 SWD_LOGLEVEL_NORMAL  = 3, ///< Normal verbosity.
 SWD_LOGLEVEL_INFO    = 4, ///< Show messages.
 SWD_LOGLEVEL_DEBUG   = 5, ///< Show debug information.
 SWD_LOGLEVEL_PAYLOAD = 6, ///< Show packet payload.
 SWD_LOGLEVEL_MAX     = 6
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
typedef enum {
 SWD_CMDTYPE_MOSI_DATA    =-7, ///< Contains MOSI data (from host).
 SWD_CMDTYPE_MOSI_REQUEST =-6, ///< Contains MOSI request packet.
 SWD_CMDTYPE_MOSI_TRN     =-5, ///< Bus will switch into MOSI mode.
 SWD_CMDTYPE_MOSI_PARITY  =-4, ///< Contains MOSI data parity.
 SWD_CMDTYPE_MOSI_BITBANG =-3, ///< Allows MOSI operation bit-by-bit.
 SWD_CMDTYPE_MOSI_CONTROL =-2, ///< MOSI control sequence (ie. sw-dp reset, idle).
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
typedef enum {
 SWD_DIR_LSBFIRST =0, ///< Data is shifted in/out right (LSB-first).
 SWD_DIR_MSBFIRST =1  ///< Data is shifted in/out left (MSB-first).
} swd_shiftdir_t;

/** Command Queue operations codes. */
typedef enum {
 SWD_OPERATION_FIRST         =1, ///< First operation to know its code.
 SWD_OPERATION_ENQUEUE       =1, ///< Append command(s) to the queue.
 SWD_OPERATION_EXECUTE       =2, ///< Queue commands then flush the queue.
 SWD_OPERATION_TRANSMIT_HEAD =3, ///< Transmit root..current (head).
 SWD_OPERATION_TRANSMIT_TAIL =4, ///< Transmit current..last (tail).
 SWD_OPERATION_TRANSMIT_ALL  =5, ///< Transmit all commands on the queue.
 SWD_OPERATION_TRANSMIT_ONE  =6, ///< Transmit only current command.
 SWD_OPERATION_TRANSMIT_LAST =7, ///< Transmit last command on the queue.
 SWD_OPERATION_LAST          =7  ///< Last operation to know its code.
} swd_operation_t;

/** SWD Command Element Structure.
 * In libswd each operation is split into separate commands (request, trn, ack,
 * data, parity) that can be appended to the command queue and later executed. 
 * This organization allows better granularity for tracing bugs and makes
 * possible to compose complete bus/target operations made of simple commands.
 */
typedef struct swd_cmd_t {
 union {
  char TRNnMOSI;  ///< Holds/sets bus direction: MOSI when zero, MISO for others.
  char request;   ///< Request header data.
  char ack;       ///< Acknowledge response from target.
  int misodata;   ///< Data read from target (MISO).
  int mosidata;   ///< Data written to target (MOSI).
  int data32;     ///< Holds "int" data type for inspection.
  char misobit;   ///< Single bit read from target (bit-per-char).
  char mosibit;   ///< Single bit written to target (bit-per-char).
  char parity;    ///< Parity bit for data payload.
  char control;   ///< Control transfer data (one byte).
  char data8;     ///< Holds "char" data type for inspection.
 };
 char bits;       ///< Payload bit count == clk pulses on the bus.
 swd_cmdtype_t cmdtype; ///< Command type as defined by swd_cmdtype_t. 
 char done;       ///< Non-zero if operation already executed.
 struct swd_cmd_t *errors;///<Pointer to the error/retry handling command/queue.
 struct swd_cmd_t *prev; ///< Pointer to the previous command.
 struct swd_cmd_t *next; ///< Pointer to the next command.
} swd_cmd_t;

/** Context configuration structure */
typedef struct {
 char initialized;        ///< Context must be initialized prior use.
 char trnlen;             ///< How many CLK cycles will TRN use.
 int  maxcmdqlen;         ///< How long command queue can be.
 swd_loglevel_t loglevel; ///< Holds Logging Level setting.
 char trunccmdqonerror;   ///< Truncate command queue after bad ACK. 
} swd_context_config_t;

/** Most actual Serial Wire Debug Port Registers */
typedef struct {
 char ack;     ///< Last known state of ACK response.
 char parity;  ///< Parity bit of the data transfer.
 int idcode;   ///< Target's IDCODE register value.
 int abort;    ///< Last known ABORT register value.
 int ctrlstat; ///< Last known CTRLSTAT register value.
 int wcr;      ///< Last known WCR register value.
 int select;   ///< Last known SELECT register value.
 int resend;   ///< Last known RESEND register value.
 int rdbuff;   ///< Last known RDBUFF register (payload data) value.
 int routesel; ///< Last known ROUTESEL register value.
} swd_swdp_t;

/** Most actual Advanced High Bandwidth Access Peripherial Bus Reisters */
typedef struct {
 char ack;     ///< Last known state of ACK response.
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

/** Most actual SWD bus transaction/packet data.
 * This structure is updated by swd_drv_transmit() function.
 * For clarity, it should not be updated by any other function.
 */
typedef struct {
 char request;      ///< Last known request on the bus.
 char ack;          ///< Last known ack on the bus.
 int data;          ///< Last known data on the bus.
 int control;       ///< Last known control data on the bus.
 char bitbang;      ///< Last known bitbang data on the bus.
 char parity;       ///< Last known parity on the bus.
} swd_transaction_t;

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
 struct {
  swd_swdp_t dp;              ///< Last known value of the SW-DP registers.
  swd_ahbap_t ap;             ///< Last known value of the AHB-AP registers.
  swd_transaction_t read;     ///< Last read operation fields.
  swd_transaction_t write;    ///< Last write operation fields.
 } log;
 struct {
  swd_transaction_t read;     ///< Data queued for read.
  swd_transaction_t write;    ///< Data queued for write.
 } qlog;
} swd_ctx_t;

/** Some comments on the function behavior **/

int swd_bin8_parity_even(char *data, char *parity);
int swd_bin32_parity_even(int *data, char *parity);
int swd_bin8_print(char *data);
int swd_bin32_print(int *data);
char *swd_bin8_string(char *data);
char *swd_bin32_string(int *data);
int swd_bin8_bitswap(unsigned char *buffer, int bitcount);
int swd_bin32_bitswap(unsigned int *buffer, int bitcount);

int swd_cmdq_init(swd_cmd_t *cmdq);
swd_cmd_t* swd_cmdq_find_root(swd_cmd_t *cmdq);
swd_cmd_t* swd_cmdq_find_tail(swd_cmd_t *cmdq);
int swd_cmdq_append(swd_cmd_t *cmdq, swd_cmd_t *cmd);
int swd_cmdq_free(swd_cmd_t *cmdq);
int swd_cmdq_free_head(swd_cmd_t *cmdq);
int swd_cmdq_free_tail(swd_cmd_t *cmdq);
int swd_cmdq_flush(swd_ctx_t *swdctx, swd_operation_t operation);

int swd_cmd_enqueue(swd_ctx_t *swdctx, swd_cmd_t *cmd);
int swd_cmd_enqueue_mosi_request(swd_ctx_t *swdctx, char *request);
int swd_cmd_enqueue_mosi_trn(swd_ctx_t *swdctx);
int swd_cmd_enqueue_miso_trn(swd_ctx_t *swdctx);
int swd_cmd_enqueue_miso_nbit(swd_ctx_t *swdctx, char **data, int count);
int swd_cmd_enqueue_mosi_nbit(swd_ctx_t *swdctx, char *data, int count);
int swd_cmd_enqueue_mosi_parity(swd_ctx_t *swdctx, char *parity);
int swd_cmd_enqueue_miso_parity(swd_ctx_t *swdctx, char **parity);
int swd_cmd_enqueue_miso_data(swd_ctx_t *swdctx, int **data);
int swd_cmd_enqueue_miso_data_p(swd_ctx_t *swdctx, int **data, char **parity);
int swd_cmd_enqueue_miso_n_data_p(swd_ctx_t *swdctx, int **data, char **parity, int count);
int swd_cmd_enqueue_mosi_data(swd_ctx_t *swdctx, int *data);
int swd_cmd_enqueue_mosi_data_ap(swd_ctx_t *swdctx, int *data);
int swd_cmd_enqueue_mosi_data_p(swd_ctx_t *swdctx, int *data, char *parity);
int swd_cmd_enqueue_mosi_n_data_ap(swd_ctx_t *swdctx, int **data, int count);
int swd_cmd_enqueue_mosi_n_data_p(swd_ctx_t *swdctx, int **data, char **parity, int count);
int swd_cmd_enqueue_miso_ack(swd_ctx_t *swdctx, char **ack);
int swd_cmd_enqueue_mosi_control(swd_ctx_t *swdctx, char *ctlmsg, int len);
int swd_cmd_enqueue_mosi_dap_reset(swd_ctx_t *swdctx);
int swd_cmd_enqueue_mosi_idle(swd_ctx_t *swdctx);
int swd_cmd_enqueue_mosi_jtag2swd(swd_ctx_t *swdctx);
int swd_cmd_enqueue_mosi_swd2jtag(swd_ctx_t *swdctx);

char *swd_cmd_string_cmdtype(swd_cmd_t *cmd);

int swd_bus_setdir_mosi(swd_ctx_t *swdctx);
int swd_bus_setdir_miso(swd_ctx_t *swdctx);
int swd_bus_write_request
(swd_ctx_t *swdctx, swd_operation_t operation, char *APnDP, char *RnW, char *addr);
int swd_bus_read_ack(swd_ctx_t *swdctx, swd_operation_t operation, char **ack);
int swd_bus_write_data_p(swd_ctx_t *swdctx, swd_operation_t operation, int *data, char *parity);
int swd_bus_write_data_ap(swd_ctx_t *swdctx, swd_operation_t operation, int *data);
int swd_bus_read_data_p(swd_ctx_t *swdctx, swd_operation_t operation, int **data, char **parity);
int swd_bus_write_control(swd_ctx_t *swdctx, swd_operation_t operation, char *ctlmsg, int len);

int swd_bitgen8_request(swd_ctx_t *swdctx, char *APnDP, char *RnW, char *addr, char *request);

int swd_drv_transmit(swd_ctx_t *swdctx, swd_cmd_t *cmd);
extern int swd_drv_mosi_8(swd_ctx_t *swdctx, swd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
extern int swd_drv_mosi_32(swd_ctx_t *swdctx, swd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
extern int swd_drv_miso_8(swd_ctx_t *swdctx, swd_cmd_t *cmd, char *data, int bits, int nLSBfirst);
extern int swd_drv_miso_32(swd_ctx_t *swdctx, swd_cmd_t *cmd, int *data, int bits, int nLSBfirst);
extern int swd_drv_mosi_trn(swd_ctx_t *swdctx, int clks);
extern int swd_drv_miso_trn(swd_ctx_t *swdctx, int clks);

int swd_dap_reset(swd_ctx_t *swdctx, swd_operation_t operation);
int swd_dap_select(swd_ctx_t *swdctx, swd_operation_t operation);
int swd_dap_detect(swd_ctx_t *swdctx, swd_operation_t operation, int **idcode);

int swd_dp_read_idcode(swd_ctx_t *swdctx, swd_operation_t operation, int **idcode);
int swd_dp_read(swd_ctx_t *swdctx, swd_operation_t operation, char addr, int **data);
int swd_dp_write(swd_ctx_t *swdctx, swd_operation_t operation, char addr, int *data);
int swd_ap_read(swd_ctx_t *swdctx, swd_operation_t operation, char addr, int **data);
int swd_ap_write(swd_ctx_t *swdctx, swd_operation_t operation, char addr, int *data);

extern int swd_log(swd_ctx_t *swdctx, swd_loglevel_t loglevel, char *msg, ...);
int swd_log_internal(swd_ctx_t *swdctx, swd_loglevel_t loglevel, char *msg, ...);
int swd_log_level_set(swd_ctx_t *swdctx, swd_loglevel_t loglevel);
extern int swd_log_level_inherit(swd_ctx_t *swdctx, int loglevel);
const char *swd_log_level_string(swd_loglevel_t loglevel);
char *swd_error_string(swd_error_code_t error);
const char *swd_operation_string(swd_operation_t operation);
const char *swd_request_string(swd_ctx_t *swdctx, char request);

swd_ctx_t *swd_init(void);
int swd_deinit_ctx(swd_ctx_t *swdctx);
int swd_deinit_cmdq(swd_ctx_t *swdctx);
int swd_deinit(swd_ctx_t *swdctx);

#endif
