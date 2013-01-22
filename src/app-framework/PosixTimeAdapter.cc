/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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
// *** Ignore this file on systems that do not implement POSIX timers
//

#include <unistd.h>
// sigh, Android only defines _POSIX_TIMERS as 1
#if defined(_POSIX_TIMERS) && ((_POSIX_TIMERS - 200112L) >= 0L || defined(PLEXIL_ANDROID))

#include "PosixTimeAdapter.hh"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Error.hh"
#include "TimeAdapter.hh"
#include "timespec-utils.hh"

#include <cerrno>

namespace PLEXIL
{
  /**
   * @brief Constructor.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   */
  PosixTimeAdapter::PosixTimeAdapter(AdapterExecInterface& execInterface)
    : TimeAdapter(execInterface)
  {
  }

  /**
   * @brief Constructor from configuration XML.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   * @param xml A const reference to the XML element describing this adapter
   * @note The instance maintains a shared pointer to the XML.
   */
  PosixTimeAdapter::PosixTimeAdapter(AdapterExecInterface& execInterface, 
                                     const pugi::xml_node& xml)
    : TimeAdapter(execInterface, xml)
  {
  }

  /**
   * @brief Destructor.
   */
  PosixTimeAdapter::~PosixTimeAdapter()
  {
  }

  /**
   * @brief Starts the adapter, possibly using its configuration data.  
   * @return true if successful, false otherwise.
   */
  bool PosixTimeAdapter::start()
  {
    // Initialize sigevent
    m_sigevent.sigev_notify = SIGEV_SIGNAL;
    m_sigevent.sigev_signo = SIGALRM;
    m_sigevent.sigev_value.sival_ptr = (void*) this; // parent PosixTimeAdapter instance
    m_sigevent.sigev_notify_function = NULL;
    m_sigevent.sigev_notify_attributes = NULL;

    // Create a timer
    int status = timer_create(CLOCK_REALTIME,
                              &m_sigevent,
                              &m_timer);
    if (status) {
      debugMsg("PosixTimeAdapter:start", " timer_create failed, errno = " << errno);
      return false;
    }
    // start the timer wait thread
    return TimeAdapter::start();
  }

  /**
   * @brief Stops the adapter.  
   * @return true if successful, false otherwise.
   */
  bool PosixTimeAdapter::stop()
  {
    int status = timer_delete(m_timer);
    if (status)
      debugMsg("TimeAdapter:stop",
               " timer_delete returned nonzero status " << status);
    return TimeAdapter::stop();
  }

  /**
   * @brief Get the current time from the operating system.
   * @return A double representing the current time.
   */
  double PosixTimeAdapter::getCurrentTime()
  {
    timespec ts;
    if (0 != clock_gettime(CLOCK_REALTIME, &ts)) {
      debugMsg("TimeAdapter:getCurrentTime",
               " clock_gettime() failed, errno = " << errno << "; returning UNKNOWN");
      return Value::UNKNOWN_VALUE();
    }
    double tym = timespecToDouble(ts);
    debugMsg("TimeAdapter:getCurrentTime", " returning " << Value::valueToString(tym));
    return tym;
  }

  /**
   * @brief Set the timer.
   * @param date The Unix-epoch wakeup time, as a double.
   * @return True if the timer was set, false if clock time had already passed the wakeup time.
   */
  bool PosixTimeAdapter::setTimer(double date)
  {
    // Get the current time
    timespec now;
    if (0 != clock_gettime(CLOCK_REALTIME, &now)) {
      debugMsg("TimeAdapter:setTimer", " clock_gettime() failed, errno = " << errno);
      return false;
    }

    // Set up a timer to go off at the high time
    itimerspec tymrSpec = {{0, 0}, {0, 0}};
    tymrSpec.it_value = doubleToTimespec(date) - now;
    if (tymrSpec.it_value.tv_nsec < 0 || tymrSpec.it_value.tv_sec < 0) {
      // Already past the scheduled time
      debugMsg("TimeAdapter:setTimer",
               " new value " << Value::valueToString(date) << " is in past, waking up Exec");
      return false;
    }

    tymrSpec.it_interval.tv_sec = tymrSpec.it_interval.tv_nsec = 0; // no repeats
    assertTrue(0 == timer_settime(m_timer,
                                  0, // flags: ~TIMER_ABSTIME
                                  &tymrSpec,
                                  NULL),
               "TimeAdapter::setTimer: timer_settime failed");
    debugMsg("TimeAdapter:setTimer",
             " timer set for " << Value::valueToString(date)
             << ", tv_nsec = " << tymrSpec.it_value.tv_nsec);
  }

  /**
   * @brief Stop the timer.
   */
  void PosixTimeAdapter::stopTimer()
  {
    static itimerspec sl_tymrDisable = {{0, 0}, {0, 0}};
    condDebugMsg(0 != timer_settime(m_timer,
                                    0, // flags: ~TIMER_ABSTIME
                                    &sl_tymrDisable,
                                    NULL),
                 "TimeAdapter:stopTimer",
                 " timer_settime failed, errno = " << errno);
  }

}

#endif // defined(_POSIX_TIMERS) && (_POSIX_TIMERS - 200112L) >= 0L

