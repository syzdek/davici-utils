/*
 *  Davici Utilities for Strongswan
 *  Copyright (C) 2026 David M. Syzdek <david@syzdek.net>.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     1. Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *     2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 *     3. Neither the name of the copyright holder nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define __SRC_DAVICICTL_MISC_C 1


///////////////
//           //
//  Headers  //
//           //
///////////////
// MARK: - Headers

#include "davici-utils.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <ctype.h>
#include <inttypes.h>


///////////////////
//               //
//  Definitions  //
//               //
///////////////////
// MARK: - Definitions


//////////////
//          //
//  Macros  //
//          //
//////////////
// MARK: - Macros


/////////////////
//             //
//  Datatypes  //
//             //
/////////////////
#pragma mark - Datatypes


//////////////////
//              //
//  Prototypes  //
//              //
//////////////////
// MARK: - Prototypes

/////////////////
//             //
//  Variables  //
//             //
/////////////////
// MARK: - Variables

// MARK: base64_chars[]
static const char * my_base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";


/////////////////
//             //
//  Functions  //
//             //
/////////////////
// MARK: - Functions

int
my_base64_encode(
         char *                        dst,
         size_t                        s,
         const uint8_t *               src,
         size_t                        n )
{
   size_t         req_len;
   ssize_t        len;
   size_t         dpos;
   size_t         spos;
   size_t         byte;
   uint8_t *      dat;

   assert(dst != NULL);
   assert(src != NULL);
   assert(s   >  0);

   // determine if enough space is available to encode using base64
   req_len = ((n / 3) + (((n % 3)) ? 1 : 0)) * 4;
   if (s <= (req_len+1))
      return(-ENOBUFS);

   dat = (uint8_t *)dst;

   // calculates each digit's value
   byte = 0;
   dpos = 0;
   for(spos = 0; (spos < n); spos++)
   {
      // MSB is Most Significant Bits  (0x80 == 10000000 ~= MSB)
      // MB is middle bits             (0x7E == 01111110 ~= MB)
      // LSB is Least Significant Bits (0x01 == 00000001 ~= LSB)
      switch(byte)
      {
         case 0:
         dat[dpos++]  = (src[spos] & 0xfc) >> 2;  // 6 MSB
         dat[dpos++]  = (src[spos] & 0x03) << 4;  // 2 LSB
         byte++;
         break;

         case 1:
         dat[dpos-1] |= (src[spos] & 0xf0) >> 4;  // 4 MSB
         dat[dpos++]  = (src[spos] & 0x0f) << 2;  // 4 LSB
         byte++;
         break;

         case 2:
         default:
         dat[dpos-1] |= (src[spos] & 0xc0) >> 6;  // 2 MSB
         dat[dpos++]  =  src[spos] & 0x3f;        // 6 LSB
         byte = 0;
         break;
      };
   };

   // encodes each value
   for(len = 0; ((size_t)len) < dpos; len++)
      dst[len] = my_base64_chars[dat[len]];

   // add padding
   for(; ((len % 4)); len++)
      dst[len] = '=';

   dst[len] = '\0';

   return((int)len);
}


size_t
my_strlcat(
         char * restrict               dst,
         const char * restrict         src,
         size_t                        dstsize )
{
   size_t      len;

   assert(src     != NULL);
   assert(dstsize  > 0);

   for(len = 0; ((*dst)); len++, dst++);
   if (!(src))
      return(len);

   dstsize--;
   for(; ( (len < dstsize) && ((*dst = *src)) ); len++, dst++, src++);
   *dst = '\0';

   for(; ((*src)); len++, src++);

   return(len);
}


size_t
my_strlcpy(
         char * restrict               dst,
         const char * restrict         src,
         size_t                        dstsize )
{
   size_t      len;

   assert(dst     != NULL);
   assert(src     != NULL);
   assert(dstsize  > 0);

   dstsize--;
   for(len = 0; ( (len < dstsize) && ((*dst = *src)) ); len++, dst++, src++);
   *dst = '\0';

   for(; ((*src)); len++, src++);

   return(len);
}


/* end of source */
