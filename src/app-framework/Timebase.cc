/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "Timebase.hh"

#include "Debug.hh"
#include "InterfaceError.hh"
#include "InterfaceSchema.hh"

#include "pugixml.hpp"

#include <iomanip> // std::setprecision()

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

#if defined(HAVE_CSTRING)
#include <cstring>  // strerror()
#elif defined(HAVE_STRING_H)
#include <string.h> // strerror()
#endif

// Issues:
// * Is a timer thread necessary?
//    POSIX per-process timer: no
//    POSIX itimer: no
//    Dispatch timer: no* (for global queues)
//    Kqueue timer: yes

  //
  // Utilities for standard timebase types
  //

  // If both clock_gettime() and gettimeofday() are available,
  // prefer clock_gettime() due to greater precision

#if defined(HAVE_CLOCK_GETTIME)

#if defined(HAVE_CTIME)
#include <ctime>
#elif defined(HAVE_TIME_H)
#include <time.h>
#endif

#include "timespec-utils.hh"

namespace PLEXIL
{

  double getPosixTime()
  {
    double tym = 0;
    struct timespec ts;
    checkInterfaceError(!clock_gettime(CLOCK_REALTIME, &ts),
                        "getTime: clock_gettime() failed, errno = " << errno
                        << ":\n " << strerror(errno));
    tym = timespecToDouble(ts);
    debugMsg("getTime", " returning " << std::setprecision(15) << tym);
    return tym;
  }

} // namespace PLEXIL

#elif defined(HAVE_GETTIMEOFDAY)

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif

#include "timeval-utils.hh"

namespace PLEXIL
{

  double getPosixTime()
  {
    double tym = 0;
    timeval tv;
    checkInterfaceError(0 == gettimeofday(&tv, nullptr),
                        "getTime: gettimeofday() failed, errno = " << errno
                        << ":\n " << strerror(errno));
    tym = timevalToDouble(tv);
    debugMsg("getTime", " returning " << std::setprecision(15) << tym);
    return tym;
  }

} // namespace PLEXIL

#endif

namespace PLEXIL
{

  // Parse the string as an interval in microseconds.
  // Valid formats TBD
  static uint32_t parseInterval(pugi::xml_node const xml)
  {
    const char *spec =
      xml.attribute(InterfaceSchema::TICK_INTERVAL_ATTR).value();
    if (!spec || !*spec) {
      debugMsg("Timebase:parseInterval",
               " no " << InterfaceSchema::TICK_INTERVAL_ATTR
               << " specified, will use tickless");
      return 0;
    }

    // TODO
    std::cerr << "Error: tick-based timing not yet implemented. Defaulting to tickless."
              << std::endl;
    return 0;
  }

} // namespace PLEXIL

#if defined(HAVE_TIMER_CREATE)
#include "PosixTimebase.cc"
#endif

#if defined(HAVE_DISPATCH_DISPATCH_H)
#include "DispatchTimebase.cc"
#endif

#if defined(HAVE_SETITIMER)
#include "ItimerTimebase.cc"
#endif
