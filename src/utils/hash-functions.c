/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Optimized hash functions for use in PLEXIL */

#include "hash-functions.h"

/************ Endian determination logic ************/
/* From http://www.burtleburtle.net/bob/c/lookup3.c */
/*
 * My best guess at if you are big-endian or little-endian.  This may
 * need adjustment.
 */
#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && \
     __BYTE_ORDER == __LITTLE_ENDIAN) || \
  (defined(i386) || defined(__i386__) || defined(__i486__) || \
   defined(__i586__) || defined(__i686__) || defined(vax) || defined(MIPSEL))
# define HASH_LITTLE_ENDIAN 1
# define HASH_BIG_ENDIAN 0
#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && \
       __BYTE_ORDER == __BIG_ENDIAN) || \
  (defined(sparc) || defined(POWERPC) || defined(mc68000) || defined(sel))
# define HASH_LITTLE_ENDIAN 0
# define HASH_BIG_ENDIAN 1
#else
# define HASH_LITTLE_ENDIAN 0
# define HASH_BIG_ENDIAN 0
#endif

/* Originally by Paul Hsieh. Found at http://www.azillionmonkeys.com/qed/hash.html. */
/* Released under LGPL 2.1 and BSD-style licenses. */

/* Helper macro get16bits() */
/* Added big-endian support. Chuck Fry Chuck.Fry@nasa.gov 2014-02-07 */
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#if !HASH_BIG_ENDIAN
#define get16bits(d) (*((const uint16_t *) (d)))
#else /* HASH_BIG_ENDIAN */
/* there ought to be a more optimal - and still portable - way to do this */
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[0])) << 8)\
		      | (uint32_t)(((const uint8_t *)(d))[1]) )
#endif
#else
#if !HASH_BIG_ENDIAN
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
		      +(uint32_t)(((const uint8_t *)(d))[0]) )
#else /* HASH_BIG_ENDIAN */
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[0])) << 8)\
		      | (uint32_t)(((const uint8_t *)(d))[1]) )
#endif
#endif

uint32_t PaulHsiehHash (const char * data, int len) {
  uint32_t hash = len, tmp;
  int rem;

  if (len <= 0 || data == 0) return 0;

  rem = len & 3;
  len >>= 2;

  /* Main loop */
  for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }

  /* Handle end cases */
  switch (rem) {
  case 3: hash += get16bits (data);
    hash ^= hash << 16;
    hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
    hash += hash >> 11;
    break;
  case 2: hash += get16bits (data);
    hash ^= hash << 11;
    hash += hash >> 17;
    break;
  case 1: hash += (signed char)*data;
    hash ^= hash << 10;
    hash += hash >> 1;
  }

  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
}

/* Variant of Paul Hsieh's hash function, optimized for double float */
/*  and intended to be endian-agnostic.                              */
/* Chuck Fry Chuck.Fry@nasa.gov 2014-02-06                           */

#if HASH_LITTLE_ENDIAN
#define _HD_IX_LSB 0
#define _HD_IX_NLSB 1
#define _HD_IX_NMSB 2
#define _HD_IX_MSB 3
#elif HASH_BIG_ENDIAN
#define _HD_IX_LSB 3
#define _HD_IX_NLSB 2
#define _HD_IX_NMSB 1
#define _HD_IX_MSB 0
#else /* some other order */
#define _HD_IX_LSB 1
#define _HD_IX_NLSB 0
#define _HD_IX_NMSB 3
#define _HD_IX_MSB 2
#endif

uint32_t HashDouble(const double* x)
{
  const uint16_t *d16 = (const uint16_t *) x;
  uint32_t hash = 8; /* init to length in bytes */
  uint32_t tmp;

  hash  += d16[_HD_IX_LSB];
  tmp    = (d16[_HD_IX_NLSB] << 11) ^ hash;
  hash   = (hash << 16) ^ tmp;
  hash  += hash >> 11;
  hash  += d16[_HD_IX_NMSB];
  tmp    = (d16[_HD_IX_MSB] << 11) ^ hash;
  hash   = (hash << 16) ^ tmp;
  hash  += hash >> 11;

  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
}
