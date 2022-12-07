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

//
// PLEXIL timebase implmentation based on setitimer()
// Fallback for older macOS and possibly others
//

// N.B. This file is included into Timebase.cc.

#include "timeval-utils.hh"

#include <csignal>

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif

namespace PLEXIL
{

  //! \class ItimerTimebase
  //! \brief An implementation of the Timebase API for platforms which
  //! support the older itimer POSIX API.
  //! \see Timebase
  class ItimerTimebase : public Timebase
  {
  public:

    //! \brief Public constructor.
    //! \param fn Pointer to the wakeup function.
    ItimerTimebase(WakeupFn const &fn)
      : Timebase(fn)
    {
      debugMsg("ItimerTimebase", " constructor");
    }

    virtual ~ItimerTimebase() = default;

    virtual double getTime() const
    {
      return getPosixTime();
    }

    virtual void setTickInterval(uint32_t intvl)
    {
      checkInterfaceError(!m_started,
                          "ItimerTimebase: setTickInterval() called while running");
      m_interval_usec = intvl;
    }

    virtual uint32_t getTickInterval() const
    {
      return m_interval_usec;
    }

    virtual void start()
    {
      if (m_started) {
        debugMsg("ItimerTimebase:start", " already running, ignored");
        return;
      }

      m_started = true;
      debugMsg("ItimerTimebase:start", " entered");

      // Set up timer signal handling
      struct sigaction saction;
      sigemptyset(&saction.sa_mask);
      sigaddset(&saction.sa_mask, SIGHUP);
      sigaddset(&saction.sa_mask, SIGINT);
      sigaddset(&saction.sa_mask, SIGQUIT);
      sigaddset(&saction.sa_mask, SIGTERM);
      sigaddset(&saction.sa_mask, SIGUSR1);
      sigaddset(&saction.sa_mask, SIGUSR2);
      saction.sa_flags = 0;  // !SA_SIGINFO
      saction.sa_handler = &sigalrmHandler;

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
                                       nullptr),
                            "ItimerTimebase::setTimer: setitimer failed, errno = "
                            << errno << ":\n " << strerror(errno));

        debugMsg("ItimerTimebase:start", " tick mode");
      }
      else {
        debugMsg("ItimerTimebase:start", " deadline mode");
      }        
    }

    virtual void stop()
    {
      if (!m_started) {
        debugMsg("ItimerTimebase:stop", " not running, ignored");
        return;
      }

      debugMsg("ItimerTimebase:stop", " entered");

      // Disable the timer
      struct itimerval tymrVal = {{0, 0}, {0, 0}};
      if (setitimer(ITIMER_REAL, &tymrVal, nullptr)) {
        warn("ItimerTimebase::stop: setitimer failed, errno = "
             << errno << ":\n " << strerror(errno));
      }

      // Restore default signal handler
      struct sigaction saction;
      sigemptyset(&saction.sa_mask);
      saction.sa_flags = 0;
      saction.sa_handler = SIG_DFL;
      if (sigaction(SIGALRM, &saction, nullptr)) {
        warn("ItimerTimebase::stop: sigaction failed, errno = "
             << errno << ":\n " << strerror(errno));
      }

      debugMsg("ItimerTimebase:stop", " complete");
    }

    virtual void setTimer(double d)
    {
      checkInterfaceError(m_started,
                          "ItimerTimebase: setTimer() called when inactive");

      if (m_interval_usec) {
        debugMsg("ItimerTimebase:setTimer", " tick mode, ignoring");
        return;
      }

      debugMsg("ItimerTimebase:setTimer", 
               " deadline "
	       << std::fixed << std::setprecision(6) << d);

      // Convert the deadline to timeval and get the current time
      struct timeval deadline, now;
      deadline = doubleToTimeval(d);
      checkInterfaceError(!gettimeofday(&now, nullptr),
                          "ItimerTimebase:setTimer: gettimeofday failed, errno = "
                          << errno << ":\n " << strerror(errno));
                          
      // Have we missed the deadline already?
      if (deadline < now) {
        // Already past the scheduled time
        debugMsg("ItimerTimebase:setTimer",
                 " new value " << std::fixed << std::setprecision(6) << d
                 << " is in past, calling wakeup function now");
        m_nextWakeup = 0;
        m_wakeupFn();
        return;
      }

      // Calculate the interval and set the timer for the deadline
      struct itimerval mytimerval = {{0, 0}, {0, 0}};
      mytimerval.it_value = deadline - now;
      checkInterfaceError(!setitimer(ITIMER_REAL,
                                     &mytimerval,
                                     nullptr),
                          "ItimerTimebase::setTimer: setitimer failed, errno = "
                          << errno << ":\n " << strerror(errno));

      // Report what we've done
      m_nextWakeup = timevalToDouble(deadline);
      debugMsg("ItimerTimebase:setTimer",
               " deadline set for "
	       << std::fixed << std::setprecision(6) << m_nextWakeup);
    }

  private:

    // UGLY HACK
    // This was the easiest way I could figure out to
    // pass our wakeup function and args through the
    // archaic sigaction() API.

    //! \brief Handler function passed to setitimer().
    //! \param signo Signal number for which the handler is called; ignored.
    static void sigalrmHandler(int signo)
    {
      timebaseWakeup(Timebase::s_instance);
    }

  };

  void registerItimerTimebase()
  {
    REGISTER_TIMEBASE(ItimerTimebase, "Itimer", 1);
  }

} // namespace PLEXIL
