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
// PLEXIL timebase implmentation based on setitimer()
// Fallback for older macOS and possibly others
//

//
// FIXME: Signal handler has no user-specifiable context in ancient POSIX API.
// Means we must do something ugly like global variables.
//

#include "Timebase.hh"

#include "Debug.hh"
#include "InterfaceError.hh"
#include "TimebaseFactory.hh"
#include "timeval-utils.hh"

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

#if defined(HAVE_CSTRING)
#include <cstring>   // strerror()
#elif defined(HAVE_STRING_H)
#include <string.h> // strerror()
#endif

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif

namespace PLEXIL
{

  class ItimerTimebase : public Timebase
  {
  public:

    ItimerTimebase(pugi::xml_node const xml, WakeupFn fn, void *arg)
      : Timebase(fn, arg),
        m_interval_usec(0)
    {
      debugMsg("ItimerTimebase", " enter constructor");
      m_interval_usec = parseInterval(xml);

      debugMsg("ItimerTimebase", " exit constructor");
    }

    virtual ~ItimerTimebase() = default;

    virtual double getTime() const
    {
      return getPosixTime();
    }

    virtual void start()
    {
      debugMsg("ItimerTimebase:start", " entered");

      // Set up timer signal handler

      // UGLY HACK
      // Necessary because I cannot find a way to pass user data
      // into the handler.
      s_timebase = this;

      struct sigaction saction;
      saction.sa_handler = sigalrmHandler;
      sigemptyset(&saction.sa_mask);
      sigaddset(&saction.sa_mask, SIGHUP);
      sigaddset(&saction.sa_mask, SIGINT);
      sigaddset(&saction.sa_mask, SIGQUIT);
      sigaddset(&saction.sa_mask, SIGTERM);
      sigaddset(&saction.sa_mask, SIGUSR1);
      sigaddset(&saction.sa_mask, SIGUSR2);
      saction.sa_flags = 0;  // !SA_SIGINFO
      saction.sa_sigaction = nullptr;

      checkInterfaceError(!sigaction(SIGALRM, &saction, nullptr),
                          "ItimerTimebase::start: sigaction failed, errno = "
                          << errno << ":\n " << strerror(errno));

      if (m_interval_usec) {
        // Start a repeating timer
        struct itimerval mytimerval = {{0, 0}, {0, 0}};
        mytimerval.it_value.tv_sec =  m_interval_usec / 1000000;
        mytimerval.it_value.tv_usec = m_interval_usec % 1000000;
        mytimerval.it_interval = mytimerval.it_value;

        checkInterfaceError(!setitimer(ITIMER_REAL,
                                       &mytimerval,
                                       NULL),
                            "ItimerTimebase::setTimer: setitimer failed, errno = "
                            << errno << ":\n " << strerror(errno));

        debugMsg("ItimerTimebase:start", " tick timer started");
      }
    }

    virtual void stop()
    {
      debugMsg("ItimerTimebase:stop", " entered");

      // Uninstall signal handler
      struct sigaction saction;
      saction.sa_handler = SIG_DFL;
      sigemptyset(&saction.sa_mask);
      saction.sa_flags = 0;
      saction.sa_sigaction = nullptr;
      if (!sigaction(SIGALRM, &saction, nullptr)) {
        warn("ItimerTimebase::stop: sigaction failed, errno = "
             << errno << ":\n " << strerror(errno));
      }

      // Whether tick or deadline, disable the timer
      struct itimerval tymrVal = {{0, 0}, {0, 0}};
      if (!setitimer(ITIMER_REAL,
                     &tymrVal,
                     NULL)) {
        warn("ItimerTimebase::stop: setitimer failed, errno = "
             << errno << ":\n " << strerror(errno));
      }

      debugMsg("ItimerTimebase:stop", " complete");
    }

    virtual void setTimer(double d)
    {
      if (m_interval_usec) {
        debugMsg("ItimerTimebase:setTimer", " tick based, ignoring");
        return;
      }

      debugMsg("ItimerTimebase:setTimer", 
               " for " << std::setprecision(15) << d);

      // Convert the deadline to timeval and get the current time
      struct timeval deadline, now;
      deadline = doubleToTimeval(d);
      checkInterfaceError(!gettimeofday(&now, nullptr),
                          "ItimerTimebase:setTimer: gettimeofday failed, errno = " << errno
                          << ":\n " << strerror(errno));
                          
      // Have we missed the deadline already?
      if (deadline < now) {
        // Already past the scheduled time
        debugMsg("ItimerTimebase:setTimer",
                 " new value " << std::setprecision(15) << d
                 << " is in past, calling wakeup function now");
        m_nextWakeup = 0;
        (m_wakeupFn)(m_wakeupArg);
        return;
      }

      // Calculate the interval and set the timer for the deadline
      struct itimerval mytimerval = {{0, 0}, {0, 0}};
      mytimerval.it_value = deadline - now;
      checkInterfaceError(!setitimer(ITIMER_REAL,
                                     &mytimerval,
                                     NULL),
                          "ItimerTimebase::setTimer: setitimer failed, errno = "
                          << errno << ":\n " << strerror(errno));

      // Report what we've done
      m_nextWakeup = timevalToDouble(deadline);
      debugMsg("ItimerTimebase:setTimer",
               " timer set for " << std::setprecision(15) << m_nextWakeup);
    }

  private:

    // UGLY HACK
    // This was the easiest way I could figure out to
    // pass our wakeup function and args through the
    // archaic sigaction() API.

    static ItimerTimebase* s_timebase;
    static void sigalrmHandler(int /* signo */)
    {
      (s_timebase->m_wakeupFn)(s_timebase->m_wakeupArg);
    }

    uint32_t m_interval_usec;
  };

  void registerItimerTimebase()
  {
    REGISTER_TIMEBASE(ItimerTimebase, "Itimer");
  }

} // namespace PLEXIL
