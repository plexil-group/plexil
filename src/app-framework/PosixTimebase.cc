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

//
// PLEXIL timebase implmentation based on timer_create
// Provided on Linux and several BSDs
//

#include "Timebase.hh"

#include "Debug.hh"
#include "InterfaceError.hh"
#include "timespec-utils.hh"

#include "pugixml.hpp"

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

#if defined(HAVE_CSIGNAL)
#include <csignal>
#elif defined(HAVE_SIGNAL_H)
#include <signal.h>
#endif

#if defined(HAVE_CTIME)
#include <ctime>
#elif defined(HAVE_TIME_H)
#include <time.h>
#endif

namespace PLEXIL
{

  class PosixTimebase : public Timebase
  {
  public:

    PosixTimebase(pugi::xml_node const xml, WakeupFn fn, intptr_t arg)
      : Timebase(fn, arg),
        m_interval_usec(0)
    {
      debugMsg("PosixTimebase", " enter constructor");
      if (xml && xml.attribute("TickInterval")) {
        // Tick-based - parse interval
        m_interval_usec = parseInterval(xml.attribute("TickInterval").value());
        if (m_interval_usec > 0) {
          // Interval is valid, create a repeating timer
        }
      }

      // Construct the timer
      strict sigevent event;
      event.sigev_notify = SIGEV_THREAD;
      // event.sigev_signo = SIGUSR1; // ???
      event.sigev_value.sival_int = arg;
      event.sigev_notify_function = fn;
      event.sigev_notify_attributes = nullptr;

      checkInterfaceError(timer_create(CLOCK_REALTIME,
                                       &event,
                                       &m_timer),
                          "PosixTimebase: timer_create failed, errno = "
                          << errno << ":\n " << strerror(errno));

      debugMsg("PosixTimebase", " exit constructor");
    }

    virtual ~PosixTimebase() = default;

    virtual Date getTime() const
    {
      return getPosixTime();
    }

    virtual void start()
    {
      if (!m_interval_usec) {
        debugMsg("PosixTimebase:start", " deadline based");
        return;
      }

      debugMsg("PosixTimebase:start", " entered");

      // Start a repeating timer
      struct itimerspec tymrSpec = {{0, 0}, {0, 0}};

      // Calculate interval in nanosec.
      uint64_t nanos = 1000 * m_interval_usec;
      tymrSpec.it_interval.tv_sec  = nanos / NSEC_PER_SEC; // better be *integer* divide!
      tymrSpec.it_interval.tv_nsec = nanos % NSEC_PER_SEC;

      // Calculate first expiration at now + interval
      struct timespec now;
      checkInterfaceError(!clock_gettime(CLOCK_REALTIME, &now),
                          "PosixTimebase:start: clock_gettime failed, errno = " << errno
                          << ":\n " << strerror(errno));
      tymrSpec.it_value = now + tymrSpec.it_interval;

      // Set the timer
      checkInterfaceError(!timer_settime(m_timer,
                                         0, // flags
                                         &tymrSpec,
                                         NULL),
                          "PosixTimebase::start: timer_settime failed, errno = "
                          << errno << ":\n " << strerror(errno));

      debugMsg("PosixTimebase:start", " tick timer started");
    }

    virtual void stop()
    {
      debugMsg("PosixTimebase:stop", " entered");

      // Whether tick or deadline, disable the timer
      struct itimerspec tymrSpec = {{0, 0}, {0, 0}};
      if (!timer_settime(m_timer,
                         0, // flags
                         &tymrSpec,
                         NULL)) {
        warn("PosixTimebase::stop: timer_settime failed, errno = "
             << errno << ":\n " << strerror(errno));
      }

      if (timer_delete(m_timer)) {
        warn("PosixTimebase:stop: error in timer_delete, errno " << errno
             << ":\n " << strerror(errno));
      }
      debugMsg("PosixTimebase:stop", " exited");
    }

    virtual void setTimer(Date d)
    {
      if (m_interval_usec) {
        debugMsg("PosixTimebase:setTimer", " tick based, ignoring");
        return;
      }

      debugMsg("PosixTimebase:setTimer", 
               " for " << std::setprecision(15) << d);

      // Get the wakeup time into the format timer_settime wants.
      struct itimerspec tymrSpec = {{0, 0}, {0, 0}};
      tymrSpec.it_value = doubleToTimespec(d);
      
      // Get the current time
      struct timespec now;
      checkInterfaceError(!clock_gettime(CLOCK_REALTIME, &now),
                          "PosixTimebase:setTimer: clock_gettime failed, errno = " << errno
                          << ":\n " << strerror(errno));

      // Have we missed the deadline already?
      if (tymrSpec.it_value < now) {
        // Already past the scheduled time
        debugMsg("PosixTimebase:setTimer",
                 " new value " << std::setprecision(15) << d
                 << " is in past, calling wakeup function now");
        m_nextWakeup = 0;
        (m_wakeupFn)(m_wakeupArg);
        return;
      }

      // Set the timer for the deadline
      checkInterfaceError(!timer_settime(m_timer,
                                         TIMER_ABSTIME, // flags
                                         &tymrSpec,
                                         NULL),
                          "PosixTimebase::setTimer: timer_settime failed, errno = "
                          << errno << ":\n " << strerror(errno));

      // Truth in advertising
      m_nextWakeup = timespecToDouble(tymrSpec.it_value);
      debugMsg("PosixTimebase:setTimer",
               " timer set for " << std::setprecision(15) << m_nextWakeup);
    }

  private:

    struct timer_t m_timer;
    uint32_t m_interval_usec;
  };

    void registerPosixTimebase()
    {
      REGISTER_TIMEBASE("Posix", PosixTimebase);
    }

} // namespace PLEXIL
