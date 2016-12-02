/*
 * Serial Wire Debug Open Library.
 * Library Body File.
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

/** \file libswd_bin.c */

#include <libswd.h>

/*******************************************************************************
 * \defgroup libswd_bin Binary operations helper functions.
 * @{
 ******************************************************************************/

/**
 * Data parity calculator, calculates even parity on char type.
 * \param *data source data pointer.
 * \param *parity resulting data pointer.
 * \return negative value on error, 0 or 1 as parity result.
 */
int libswd_bin8_parity_even(char *data, char *parity){
 char i;
 unsigned char test=*data;
 *parity=0;
 for (i=0;i<=8;i++) *parity ^= ((test>>i)&1);
 if (*parity<0 || *parity>1) return LIBSWD_ERROR_PARITY;
 return (int)*parity;
}

/**
 * Data parity calculator, calculates even parity on integer type.
 * \param *data source data pointer.
 * \param *parity resulting data pointer.
 * \return negative value on error, 0 or 1 as parity result.
 */
int libswd_bin32_parity_even(int *data, char *parity){
 int i;
 unsigned int test=*data;
 *parity=0;
 for (i=0;i<32;i++) *parity ^= ((test>>i)&1);
 if (*parity<0 || *parity>1) return LIBSWD_ERROR_PARITY;
 return (int)*parity;
}

/**
 * Prints binary data of a char value on the screen.
 * \param *data source data pointer.
 * \return number of characters printed.
 */
int libswd_bin8_print(char *data){
 unsigned char i, bits=*data;
 for (i=0;i<8;i++) putchar(((bits<<i)&0x80)?'1':'0');
 return i;
}

/**
 * Prints binary data of an integer value on the screen.
 * \param *data source data pointer.
 * \return number of characters printed.
 */
int libswd_bin32_print(int *data){
 unsigned int i, bits=*data;
 for (i=0;i<32;i++) putchar(((bits<<i)&0x80000000)?'1':'0');
 return i;
}

/**
 * Generates string containing binary data of a char value.
 * \param *data source data pointer.
 * \return pointer to the resulting string.
 */
char *libswd_bin8_string(char *data){
 static char string[9]; string[8]=0;
 unsigned char i, bits=*data;
 for (i=0;i<8;i++) string[7-i]=(bits&(1<<i))?'1':'0';
 return string;
}

/**
 * Generates string containing binary data of an integer value.
 * \param *data source data pointer.
 * \return pointer to the resulting string.
 */
char *libswd_bin32_string(int *data){
 static char string[33]; string[32]=0;
 unsigned int i, bits=*data;
 for (i=0;i<32;i++) string[31-i]=(bits&(1<<i))?'1':'0';
 return string;
}

/**
 * Bit swap helper function that reverse bit order in char *buffer.
 * Most Significant Bit becomes Least Significant Bit.
 * It is possible to  swap only n-bits from char (8-bit) *buffer.
 * \param *buffer unsigned char (8-bit) data pointer.
 * \param bitcount how many bits to swap.
 * \return swapped bit count (positive) or error code (negative).
 */
int libswd_bin8_bitswap(unsigned char *buffer, unsigned int bitcount){
 if (buffer==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bitcount>8) return LIBSWD_ERROR_PARAM;
 unsigned char bit, result=0; //res must be unsigned for proper shifting result
 #ifdef __SWDDEBUG__
 printf("|LIBSWD_DEBUG: libswd_bin8_bitswap(%02X, %d);\n", *buffer, bitcount);
 #endif
 for (bit=0;bit<bitcount;bit++) {
  result=(result<<1)|(((*buffer>>bit)&1)?1:0);
  #ifdef __SWDDEBUG__
  printf("|LIBSWD_DEBUG: libswd_bin8_bitswap: in=%02X out=%02X bit=%d\n", *buffer, result, bit);
  #endif
 }
 *buffer=result;
 return bit;
}

/**
 * Bit swap helper function that reverse bit order in int *buffer.
 * Most Significant Bit becomes Least Significant Bit.
 * It is possible to  swap only n-bits from int (32-bit) *buffer.
 * \param *buffer unsigned char (32-bit) data pointer.
 * \param bitcount how many bits to swap.
 * \return swapped bit count (positive) or error code (negative).
 */
int libswd_bin32_bitswap(unsigned int *buffer, unsigned int bitcount){
 if (buffer==NULL) return LIBSWD_ERROR_NULLPOINTER;
 if (bitcount>32) return LIBSWD_ERROR_PARAM;
 unsigned int bit, result=0; //res must be unsigned for proper shifting result
 #ifdef __SWDDEBUG__
 printf("|LIBSWD_DEBUG: libswd_bin32_bitswap(%08X, %d);\n", *buffer, bitcount);
 #endif
 for (bit=0;bit<bitcount;bit++) {
  result=(result<<1)|(((*buffer>>bit)&1)?1:0);
  #ifdef __SWDDEBUG__
  printf("|LIBSWD_DEBUG: libswd_bin32_bitswap: in=%08X out=%08X bit=%d\n", *buffer, result, bit);
  #endif
 }
 *buffer=result;
 return bit;
}

/** @} */
