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
// PLEXIL timebase implementation based on Grand Central Dispatch
// Provided on Darwin (macOS and others) since macOS 10.9
// Available for BSDs and Linux
//

//
// *** FIXME: First tick timer wakeup comes right after timer set ***
//

// N.B. This file is included into Timebase.cc.

#include "timespec-utils.hh"
#include <dispatch/dispatch.h>
#include <ctime>

namespace PLEXIL
{

  //! \class DispatchTimebase
  //! \brief An implementation of the Timebase API for use with the
  //! Grand Central Dispatch framework. Primarily used on macOS, but
  //! available on any platform which implements Grand Central Dispatch.
  //! \see Timebase
  class DispatchTimebase : public Timebase
  {
  public:

    //! \brief Public constructor.
    //! \param fn Pointer to the wakeup function.
    DispatchTimebase(WakeupFn const &fn)
      : Timebase(fn),
        m_timer(),
        m_queue(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0))
    {
      debugMsg("DispatchTimebase", " constructor");
    }

    //! \brief Virtual destructor.

    virtual double getTime() const
    {
      return getPosixTime();
    }

    virtual void setTickInterval(uint32_t intvl)
    {
      checkInterfaceError(!m_started,
                          "DispatchTimebase: setTickInterval() called while running");
      m_interval_usec = intvl;
    }

    virtual uint32_t getTickInterval() const
    {
      return m_interval_usec;
    }

    virtual void start()
    {
      if (m_started) {
        debugMsg("DispatchTimebase:start", " already running, ignored");
        return;
      }

      m_started = true;
      debugMsg("DispatchTimebase:start", " entered");

      // Construct the timer whether we are in deadline or tick mode
      m_timer =
        dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
                               0,                     // handle
                               DISPATCH_TIMER_STRICT, // flags
                               m_queue);
      // Set handler
      dispatch_source_set_event_handler_f(m_timer,
                                          reinterpret_cast<void (*)(void *)>(&timebaseWakeup));
      // Set timer context, i.e. argument for wakeup fn
      dispatch_set_context(m_timer, static_cast<void *>(this));

      if (!m_interval_usec) {
        debugMsg("DispatchTimebase:start", " deadline mode");
      }
      else {
        // Set the timer start time and repeat interval
        int64_t interval_nsec = NSEC_PER_USEC * m_interval_usec;
        struct timespec now;
        checkInterfaceError(!clock_gettime(CLOCK_REALTIME, &now),
                            "DispatchTimebase:start: clock_gettime failed, errno = "
                            << errno << ":\n " << strerror(errno));
        debugMsg("DispatchTimebase::start",
                 " setting timer interval of " << interval_nsec << " ns");
        dispatch_source_set_timer(m_timer,
                                  dispatch_time(DISPATCH_TIME_NOW, interval_nsec),
                                  interval_nsec,
                                  NSEC_PER_MSEC); // 1 ms leeway

        // Start the timer
        dispatch_activate(m_timer);

        debugMsg("DispatchTimebase:start", " tick mode");
      }
    }

    virtual void stop()
    {
      if (!m_started) {
        debugMsg("DispatchTimebase:stop", " not running, ignored");
        return;
      }

      // Unset the event handler pointer in case timer gets recycled
      dispatch_source_set_event_handler_f(m_timer, nullptr);
      dispatch_source_cancel(m_timer);
      // If we have never started the timer,
      // we must resume (activate?) it before we can release it.
      if (!m_interval_usec && !m_nextWakeup)
        dispatch_activate(m_timer);
      dispatch_release(m_timer);
      m_started = false;
      debugMsg("DispatchTimebase:stop", " complete");
    }

    virtual void setTimer(double d)
    {
      checkInterfaceError(m_started,
                          "DispatchTimer: setTimer() called when inactive");

      if (m_interval_usec) {
        debugMsg("DispatchTimebase:setTimer", " tick mode, ignoring");
        return;
      }
      
      // Deadline based
      debugMsg("DispatchTimebase:setTimer",
               " deadline " << std::fixed << std::setprecision(6) << d);
      struct timespec deadline_ts, now;
      doubleToTimespec(d, deadline_ts);
      checkInterfaceError(!clock_gettime(CLOCK_REALTIME, &now),
                          "DispatchTimebase:setTimer: clock_gettime failed, errno = "
                          << errno << ":\n " << strerror(errno));

      // Have we missed the deadline already?
      if (deadline_ts < now) {
        // Already past the scheduled time
        debugMsg("DispatchTimebase:setTimer",
                 " new value " << std::fixed << std::setprecision(6) << d
                 << " is in past, calling wakeup function now");
        m_nextWakeup = 0;
        m_wakeupFn();
        return;
      }

      // Set the timer start time and tell it to (effectively) never repeat
      struct timespec delta_ts = deadline_ts - now;
      int64_t delta_nsec = delta_ts.tv_sec * NSEC_PER_SEC + delta_ts.tv_nsec;
      dispatch_source_set_timer(m_timer,
                                dispatch_walltime(&now, delta_nsec),
                                86400 * NSEC_PER_SEC, // i.e. 24 hours
                                NSEC_PER_MSEC); // 1 ms leeway

      // Activate the timer if not already started
      if (!m_nextWakeup) 
        dispatch_activate(m_timer);

      m_nextWakeup = timespecToDouble(deadline_ts);
      debugMsg("DispatchTimebase:setTimer",
               " deadline set to "
               << std::fixed << std::setprecision(6) << m_nextWakeup);
    }

  private:

    //
    // Member variables
    //

    dispatch_source_t m_timer;  //! The timer object.
    dispatch_queue_t  m_queue;  //! The queue on which our handler gets called.
  };

  void registerDispatchTimebase()
  {
    REGISTER_TIMEBASE(DispatchTimebase, "Dispatch", 1000);
  }

} // namespace PLEXIL
