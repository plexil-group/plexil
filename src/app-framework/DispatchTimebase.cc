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
// PLEXIL timebase implementation based on Grand Central Dispatch
// Provided on Darwin (macOS and others) since macOS 10.9
// Available for BSDs and Linux
//

#include "Timebase.hh"

#include "Debug.hh"
#include "InterfaceError.hh"
#include "TimebaseFactory.hh"
#include "timespec-utils.hh"

#include "pugixml.hpp"

#if defined(HAVE_DISPATCH_DISPATCH_H)
#include <dispatch/dispatch.h>
#endif

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

#if defined(HAVE_CTIME)
#include <ctime>
#elif defined(HAVE_TIME_H)
#include <time.h>
#endif

namespace PLEXIL
{

  class DispatchTimebase : public Timebase
  {
  public:
    DispatchTimebase(pugi::xml_node const xml, WakeupFn fn, intptr_t arg)
      : Timebase(fn, arg),
        m_queue(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0)),
        m_interval_usec(0)
    {
      debugMsg("DispatchTimebase", " enter constructor");
      m_interval_usec = parseInterval(xml);
      if (m_interval_usec > 0) {
        // Interval is valid, create a timer
        m_timer =
          dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, m_queue);
        // And set its event handler
        dispatch_source_set_event_handler_f(m_timer,
                                            fn); // *** this is not right ***
      }
      debugMsg("DispatchTimebase", " exit constructor");
    }

    virtual double getTime() const
    {
      return getPosixTime();
    }

    virtual void start()
    {
      if (m_interval_usec) {
        // Tick based
        debugMsg("DispatchTimebase:start", " tick based");
        uint64_t interval_nsec = 1000ull * m_interval_usec;
        dispatch_source_set_timer(m_timer,
                                  dispatch_time(DISPATCH_TIME_NOW, interval_nsec),
                                  interval_nsec,
                                  NSEC_PER_SEC/1000);
        dispatch_resume(m_timer);
      }
      else {
        debugMsg("DispatchTimebase:start", " deadline based");
      }

      // TODO: call dispatch_main on a background thread?
    }

    virtual void stop()
    {
      if (m_interval_usec) {
        debugMsg("DispatchTimebase:stop", " tick based");
        dispatch_source_cancel(m_timer);
        dispatch_release(m_timer);
      }
      else {
        debugMsg("DispatchTimebase:stop", " deadline based");
      }
    }

    virtual void setTimer(double d)
    {
      if (m_interval_usec) {
        debugMsg("DispatchTimebase:setTimer", " tick based, ignoring");
        return;
      }
      
      // Deadline based
      debugMsg("DispatchTimebase:setTimer",
               " for " << std::setprecision(15) << d);
      struct timespec deadline_ts, now;
      doubleToTimespec(d, deadline_ts);
      checkInterfaceError(!clock_gettime(CLOCK_REALTIME, &now),
                          "DispatchTimebase:setTimer: clock_gettime failed, errno = "
                          << errno << ":\n " << strerror(errno));

      // Have we missed the deadline already?
      if (deadline_ts < now) {
        // Already past the scheduled time
        debugMsg("DispatchTimebase:setTimer",
                 " new value " << std::setprecision(15) << d
                 << " is in past, calling wakeup function now");
        m_nextWakeup = 0;
        (m_wakeupFn)((void *) m_wakeupArg);
        return;
      }

      // Set the timer
      dispatch_time_t deadline = dispatch_walltime(&deadline_ts, 0);
      dispatch_after_f(deadline,
                       m_queue,
                       (void *) m_wakeupArg,
                       m_wakeupFn);

      m_nextWakeup = timespecToDouble(deadline_ts);
      debugMsg("DispatchTimebase:setTimer",
               " timer set to " << std::setprecision(15) << m_nextWakeup);
    }

  private:

    dispatch_source_t m_timer;
    dispatch_queue_t m_queue;
    uint32_t m_interval_usec;
  };

  void registerDispatchTimebase()
  {
    REGISTER_TIMEBASE(DispatchTimebase, "Dispatch");
  }

} // namespace PLEXIL
