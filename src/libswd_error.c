/*
 * $Id$
 *
 * Serial Wire Debug Open Library.
 * Library Body File.
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
 * Written by Tomasz Boleslaw CEDRO <cederom@tlen.pl>, 2010-2011;
 *
 */

/** \file libswd_error.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup swd_error Error handling and information routines.
 * @{
 ******************************************************************************/

char *swd_error_string(swd_error_code_t error){
 switch (error){
  case SWD_OK:                 return "[SWD_OK] hmm, there was no error";
  case SWD_ERROR_GENERAL:      return "[SWD_ERROR_GENERAL] general error";
  case SWD_ERROR_NULLPOINTER:  return "[SWD_ERROR_NULLPOINTER] null pointer";
  case SWD_ERROR_NULLQUEUE:    return "[SWD_ERROR_NULLQUEUE] null queue";
  case SWD_ERROR_NULLTRN:      return "[SWD_ERROR_NULLTRN] null turnaround";
  case SWD_ERROR_PARAM:        return "[SWD_ERROR_PARAM] bad parameter";
  case SWD_ERROR_OUTOFMEM:     return "[SWD_ERROR_OUTOFMEM] out of memory";
  case SWD_ERROR_RESULT:       return "[SWD_ERROR_RESULT] bad result";
  case SWD_ERROR_RANGE:        return "[SWD_ERROR_RANGE] out of range";
  case SWD_ERROR_DEFINITION:   return "[SWD_ERROR_DEFINITION] definition error";
  case SWD_ERROR_NULLCONTEXT:  return "[SWD_ERROR_NULLCONTEXT] null context";
  case SWD_ERROR_QUEUE:        return "[SWD_ERROR_QUEUE] queue error";
  case SWD_ERROR_ADDR:         return "[SWD_ERROR_ADDR] addressing error";
  case SWD_ERROR_APnDP:        return "[SWD_ERROR_APnDP] bad APnDP value";
  case SWD_ERROR_RnW:          return "[SWD_ERROR_RnW] bad RnW value";
  case SWD_ERROR_PARITY:       return "[SWD_ERROR_PARITY] parity error";
  case SWD_ERROR_ACK:          return "[SWD_ERROR_ACK] acknowledge error";
  case SWD_ERROR_ACKUNKNOWN:   return "[SWD_ERROR_ACKUNKNOWN] got unknown acknowledge";
  case SWD_ERROR_ACKNOTDONE:   return "[SWD_ERROR_ACKNOTDONE] not yet executed on target";
  case SWD_ERROR_ACKMISSING:   return "[SWD_ERROR_ACKMISSING] command not found on the queue";
  case SWD_ERROR_ACKMISMATCH:  return "[SWD_ERROR_ACKMISMATCH] different result address expected";
  case SWD_ERROR_ACKORDER:     return "[SWD_ERROR_ACKORDER] cmdq not in sequence REQ->TRN->ACK";
  case SWD_ERROR_BADOPCODE:    return "[SWD_ERROR_BADOPCODE] unsupported operation requested";
  case SWD_ERROR_NODATACMD:    return "[SWD_ERROR_NODATACMD] command not found on the queue";
  case SWD_ERROR_DATAPTR:      return "[SWD_ERROR_DATAPTR] bad data pointer address";
  case SWD_ERROR_NOPARITYCMD:  return "[SWD_ERROR_NOPARITYCMD] parity command missing or misplaced";
  case SWD_ERROR_PARITYPTR:    return "[SWD_ERROR_PARITYPTR] bad parity pointer address";
  case SWD_ERROR_NOTDONE:      return "[SWD_ERROR_NOTDONE] could not end selected task";
  case SWD_ERROR_QUEUEROOT:    return "[SWD_ERROR_QUEUEROOT] queue root not found or null";
  case SWD_ERROR_QUEUETAIL:    return "[SWD_ERROR_QUEUETAIL] queue tail not found or null";
  case SWD_ERROR_BADCMDTYPE:   return "[SWD_ERROR_BADCMDTYPE] unknown command detected";
  case SWD_ERROR_BADCMDDATA:   return "[SWD_ERROR_BADCMDDATA] command contains bad data (out of range, etc)";
  case SWD_ERROR_ACK_WAIT:     return "[SWD_ERROR_ACK_WAIT] got ACK_WAIT response";
  case SWD_ERROR_ACK_FAULT:    return "[SWD_ERROR_ACK_FAULT] got ACK_FAULT response";
  case SWD_ERROR_QUEUENOTFREE: return "[SWD_ERROR_QUEUENOTFREE] cannot free resources, queue not empty";
  case SWD_ERROR_TRANSPORT:    return "[SWD_ERROR_TRANSPORT] transport error or undefined";
  case SWD_ERROR_DIRECTION:    return "[SWD_ERROR_DIRECTION] MSb/LSb direction error";
  default:                     return "undefined error";
 }
 return "undefined error";
}

/** @} */
