/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

//
// Utilities for timeval arithmetic
//

#include "plexil-config.h"

#include <limits>

#ifdef HAVE_SYS_TIME_H 
#include <sys/time.h>
#elif defined(__VXWORKS__)
#include <time.h>
#include <sys/times.h>
#endif

const long ONE_MILLION = 1000000;
const double ONE_MILLION_DOUBLE = 1000000.0;

//
// General utility for normalizing timevals after arithmetic
// Assumes |tv_usec| < 1,999,999
// Also assumes time_t is an integer type - which POSIX does not guarantee!
//

//
// what are typical boundary cases?
//  tv_sec = 0,  tv_usec = -1,999,998 (-0.999999 + -0.999999) - usec underflow
//  tv_sec = 1,  tv_usec = -1 (1 - .000001)
// is it possible to have over/underflow AND opposing signs from simple add/subtract
// of two valid timevals?
//

void timevalNormalize(struct timeval& tval)
{
  // check for usec over/underflow
  if (tval.tv_usec >= ONE_MILLION) {
    tval.tv_sec += 1;
    tval.tv_usec -= ONE_MILLION;
  }
  else if (tval.tv_usec + ONE_MILLION <= 0) {
    tval.tv_sec -= 1;
    tval.tv_usec += ONE_MILLION;
  }

  // now check that signs are consistent
  if (tval.tv_sec > 0 && tval.tv_usec < 0) {
    tval.tv_sec -= 1;
    tval.tv_usec += ONE_MILLION;
  }
  else if (tval.tv_sec < 0 && tval.tv_usec > 0) {
    tval.tv_sec += 1;
    tval.tv_usec -= ONE_MILLION;
  }
}

bool operator<(const struct timeval& tv1, const struct timeval& tv2)
{
  return tv1.tv_sec < tv2.tv_sec ||
    (tv1.tv_sec == tv2.tv_sec && tv1.tv_usec < tv2.tv_usec);
}

bool operator>(const struct timeval& tv1, const struct timeval& tv2)
{
  return tv1.tv_sec > tv2.tv_sec || 
    (tv1.tv_sec == tv2.tv_sec && tv1.tv_usec > tv2.tv_usec);
}

bool operator==(const struct timeval& tv1, const struct timeval& tv2)
{
  return (tv1.tv_sec == tv2.tv_sec) && (tv1.tv_usec == tv2.tv_usec);
}

struct timeval operator+(const struct timeval& tv1, const struct timeval& tv2)
{
  struct timeval time = {tv1.tv_sec + tv2.tv_sec,
                         tv1.tv_usec + tv2.tv_usec};
  timevalNormalize(time);
  return time;
}

struct timeval operator- (const struct timeval& tv1, const struct timeval& tv2)
{
  struct timeval time = {tv1.tv_sec - tv2.tv_sec,
                         tv1.tv_usec - tv2.tv_usec};
  timevalNormalize(time);
  return time;
}

void doubleToTimeval(double dbl, timeval& result)
{
  if (dbl > std::numeric_limits<time_t>::max()
      || dbl < std::numeric_limits<time_t>::min()) {
    // TODO: report out-of-range error
    return;
  }
  result.tv_sec = (time_t) dbl;
  result.tv_usec =
#ifdef HAVE_SUSECONDS_T
    (suseconds_t)
#else /* e.g. VxWorks */
    (long)
#endif
    (ONE_MILLION_DOUBLE * (dbl - (double) result.tv_sec));
  timevalNormalize(result);
}

struct timeval doubleToTimeval(double dbl)
{
  timeval result;
  doubleToTimeval(dbl, result);
  return result;
}

double timevalToDouble(const struct timeval& tval)
{
  double result = (double) tval.tv_sec;
  result += (double) tval.tv_usec / ONE_MILLION_DOUBLE;
  return result;
}
