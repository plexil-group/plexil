// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "plexil-config.h"

#include "Timebase.hh"

#include "Debug.hh"
#include "InterfaceError.hh"
#include "InterfaceSchema.hh"
#include "TimebaseFactory.hh" // REGISTER_TIMEBASE() macro

#include "pugixml.hpp"

#include <cerrno>
#include <cstring>  // strerror()
#include <iomanip> // std::fixed, std::setprecision()

// Issues:
// * Is a timer thread necessary?
//    POSIX per-process timer: no
//    POSIX itimer: no
//    Dispatch timer: no* (for global queues)
//    Kqueue timer: yes

// Local constants
namespace
{
  //! \brief The number of nanoseconds in one second.
  constexpr const uint64_t NSEC_PER_SEC = 1000000000;
}

namespace PLEXIL
{

  //
  // Timebase base class methods
  //

  Timebase *Timebase::s_instance = nullptr;

  Timebase::Timebase(WakeupFn const &f)
    : m_nextWakeup(0),
      m_wakeupFn(f),
      m_interval_usec(0),
      m_started(false)
  {
    if (!s_instance)
      s_instance = this;
  }

  Timebase::~Timebase()
  {
    if (s_instance == this)
      s_instance = nullptr;
  }

  double Timebase::getNextWakeup() const
  {
    return m_nextWakeup;
  }

  double Timebase::queryTime()
  {
    if (s_instance)
      return s_instance->getTime();
    return 0;
  }

  void Timebase::timebaseWakeup(Timebase *tb)
  {
    tb->m_wakeupFn();
  }

} // namespace PLEXIL

//
// Utilities for standard timebase types
//

// If both clock_gettime() and gettimeofday() are available,
// prefer clock_gettime() due to greater precision

#if defined(HAVE_CLOCK_GETTIME)

#include <ctime>

#include "timespec-utils.hh"

namespace PLEXIL
{

  double getPosixTime()
  {
    double tym = 0;
    struct timespec ts;
    checkInterfaceError(!clock_gettime(CLOCK_REALTIME, &ts),
                        "getPosixTime: clock_gettime failed, errno = " << errno
                        << ":\n " << strerror(errno));
    tym = timespecToDouble(ts);
    debugMsg("getTime",
	     " returning " << std::fixed << std::setprecision(6) << tym);
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
    checkInterfaceError(!gettimeofday(&tv, nullptr),
                        "getPosixTime: gettimeofday failed, errno = " << errno
                        << ":\n " << strerror(errno));
    tym = timevalToDouble(tv);
    debugMsg("getTime",
	     " returning " << std::fixed << std::setprecision(6) << tym);
    return tym;
  }

} // namespace PLEXIL

#endif

//
// Factory registration
//

#if defined(HAVE_TIMER_CREATE)
#include "PosixTimebase.cc"
#endif

#if defined(HAVE_DISPATCH_DISPATCH_H)
#include "DispatchTimebase.cc"
#endif

#if defined(HAVE_SETITIMER)
#include "ItimerTimebase.cc"
#endif

extern "C"
void initTimebaseFactories()
{
#if defined(HAVE_TIMER_CREATE)
  PLEXIL::registerPosixTimebase();
#endif

#if defined(HAVE_DISPATCH_DISPATCH_H)
  PLEXIL::registerDispatchTimebase();
#endif

#if defined(HAVE_SETITIMER)
  PLEXIL::registerItimerTimebase();
#endif
}
