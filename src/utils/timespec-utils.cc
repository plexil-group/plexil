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
// Utilities for timespec arithmetic
//

#include "plexil-config.h"

#include <limits>

#if defined(HAVE_CTIME)
#include <ctime>
#elif defined(HAVE_TIME_H)
#include <time.h>
#endif

static const long ONE_BILLION = 1000000000;
static const double ONE_BILLION_DOUBLE = 1000000000.0;

//
// General utility for normalizing timespecs after arithmetic
// Assumes |tv_nsec| < 1,999,999,999
// Also assumes time_t is an integer type - which POSIX does not guarantee!
//

//
// what are typical boundary cases?
//  tv_sec = 0,  tv_nsec = -1,999,999,998 (-0.999999999 + -0.999999999) - nsec underflow
//  tv_sec = 1,  tv_nsec = -1 (1 - .000000001)
// is it possible to have over/underflow AND opposing signs from simple add/subtract
// of two valid timespecs?
//

void timespecNormalize(struct timespec& tspec)
{
  // check for nsec over/underflow
  if (tspec.tv_nsec >= ONE_BILLION) {
    tspec.tv_sec += 1;
    tspec.tv_nsec -= ONE_BILLION;
  }
  else if (tspec.tv_nsec + ONE_BILLION <= 0) {
    tspec.tv_sec -= 1;
    tspec.tv_nsec += ONE_BILLION;
  }

  // now check that signs are consistent
  if (tspec.tv_sec > 0 && tspec.tv_nsec < 0) {
    tspec.tv_sec -= 1;
    tspec.tv_nsec += ONE_BILLION;
  }
  else if (tspec.tv_sec < 0 && tspec.tv_nsec > 0) {
    tspec.tv_sec += 1;
    tspec.tv_nsec -= ONE_BILLION;
  }
}

bool operator<(const struct timespec& ts1, const struct timespec& ts2)
{
  return ts1.tv_sec < ts2.tv_sec || 
          (ts1.tv_sec == ts2.tv_sec && ts1.tv_nsec < ts2.tv_nsec);
}

bool operator>(const struct timespec& ts1, const struct timespec& ts2)
{
  return ts1.tv_sec > ts2.tv_sec || 
    (ts1.tv_sec == ts2.tv_sec && ts1.tv_nsec > ts2.tv_nsec);
}

bool operator==(const struct timespec& ts1, const struct timespec& ts2)
{
  return ts1.tv_sec == ts2.tv_sec && ts1.tv_nsec == ts2.tv_nsec;
}

struct timespec operator+(const struct timespec& ts1, const struct timespec& ts2)
{
  struct timespec time = {ts1.tv_sec + ts2.tv_sec,
                          ts1.tv_nsec + ts2.tv_nsec};
  timespecNormalize(time);
  return time;
}

struct timespec operator-(const struct timespec& ts1, const struct timespec& ts2)
{
  struct timespec time = {ts1.tv_sec - ts2.tv_sec,
                          ts1.tv_nsec - ts2.tv_nsec};
  timespecNormalize(time);
  return time;
}

void doubleToTimespec(double dbl, timespec& result)
{
  if (dbl > std::numeric_limits<time_t>::max()
      || dbl < std::numeric_limits<time_t>::min()) {
    // TODO: report out-of-range error
    return;
  }
  result.tv_sec = (time_t) dbl;
  result.tv_nsec =
    (long) (ONE_BILLION_DOUBLE * (dbl - (double) result.tv_sec));
  timespecNormalize(result);
}

struct timespec doubleToTimespec(double dbl)
{
  timespec result;
  doubleToTimespec(dbl, result);
  return result;
}

double timespecToDouble(const struct timespec& tspec)
{
  double result = (double) tspec.tv_nsec / ONE_BILLION_DOUBLE;
  result += (double) tspec.tv_sec;
  return result;
}
