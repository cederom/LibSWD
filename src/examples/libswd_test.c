/*
 * $Id$
 *
 * Serial Wire Debug Open API.
 * Simple Test Program.
 *
 * Copyright (C) 2010, Tomasz Boleslaw CEDRO (http://www.tomek.cedro.info)
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
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Written by Tomasz Boleslaw CEDRO <cederom@tlen.pl>, 2010;
 *
 */

#include <libswd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char argv[]){
 printf("Hello to SWD world, a simple SWD API and general test program.\n");

 int i, x, res;
 unsigned char c, uc, uc2;
 char sc, sc2, par8, par32;

 printf("\n=> Bitswap test:\n");
 i=0x01a0cede;
 c=0x6f;
 printf("C size %d bits, I size %d bits.\n", sizeof(c)*LIBSWD_DATA_BYTESIZE, sizeof(i)*LIBSWD_DATA_BYTESIZE);
 printf("Before bitswap: C=%s, I=%s\n", libswd_bin8_string(&c), libswd_bin32_string(&i));
 libswd_bin8_bitswap(&c, LIBSWD_DATA_BYTESIZE);
 libswd_bin32_bitswap(&i, sizeof(i)*LIBSWD_DATA_BYTESIZE);
 printf("After bitswap : C=%s, I=%s\n", libswd_bin8_string(&c), libswd_bin32_string(&i));

 printf("\n=> Parity test:\n");
 for (x=0;x<=0xff;x++){
  i=x*x*x*x;
  c=x;
  res=libswd_bin8_parity_even(&c, &par8);
  if (res<0) return res;
  res=libswd_bin32_parity_even(&c, &par32);
  if (res<0) return res;
  printf("C=%s P=%d  | I=%s P=%d\n", libswd_bin8_string(&c), par8, libswd_bin32_string(&i), par32);
 }

 printf("\n=> Casting test:\n");
 uc=10;
 sc=-10;
 printf("Signed char: %d, Unsigned char: %d\n", sc, uc);
 printf(" (char)unsigned     : %d\n", (char)uc);
 printf(" (unsigned char)char: %d\n", (unsigned char)sc);
 printf(" unsigned = signed  : %d\n", uc2=sc);
 printf(" signed = unsigned  : %d\n", sc2=uc);
 printf(" (signed)(uns=sign) : %d\n", (signed char)(uc2=sc));
 printf(" (unsigned)(sig=uns): %d\n", (unsigned char)(sc2=uc));

 int *ip=NULL;
 char *cp=NULL;
 printf("\n=> Memory Allocation test:\n");
 printf(" Before: IP=%8X, CP=%8X\n", ip, cp);
 ip=(int *)calloc(1,sizeof(ip));
 cp=(char *)calloc(1,sizeof(cp));
 printf(" After ip=(int *)calloc(1,sizeof(ip)) : %8X\n", ip);
 printf(" After cp=(char *)callow(1,sizeof(cp)): %8X\n", cp);
 free(ip); free(cp);
 ip=calloc(1,sizeof(ip));
 cp=calloc(1,sizeof(cp));
 printf(" After ip=calloc(1,sizeof(ip)): %8X\n", ip);
 printf(" After cp=calloc(1,sizeof(cp)): %8X\n", cp);

 return 0;
}
