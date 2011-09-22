/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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
#include "StateCache.hh"
#include "timespec-utils.hh"

#include <cerrno>

namespace PLEXIL
{
  /**
   * @brief Constructor.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   */
  PosixTimeAdapter::PosixTimeAdapter(AdapterExecInterface& execInterface)
    : InterfaceAdapter(execInterface)
  {
    initSigevent();
  }

  /**
   * @brief Constructor from configuration XML.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   * @param xml A const pointer to the TiXmlElement describing this adapter
   * @note The instance maintains a shared pointer to the TiXmlElement.
   */
  PosixTimeAdapter::PosixTimeAdapter(AdapterExecInterface& execInterface, 
                                     const TiXmlElement * xml)
    : InterfaceAdapter(execInterface, xml)
  {
    initSigevent();
  }

  /**
   * @brief Destructor.
   */
  PosixTimeAdapter::~PosixTimeAdapter()
  {
  }

  /**
   * @brief Initializes the adapter, possibly using its configuration data.
   * @return true if successful, false otherwise.
   */
  bool PosixTimeAdapter::initialize()
  {
    // Automatically register self for time
    m_execInterface.registerLookupInterface(LabelStr("time"), getId());
    return true;
  }

  /**
   * @brief Starts the adapter, possibly using its configuration data.  
   * @return true if successful, false otherwise.
   */
  bool PosixTimeAdapter::start()
  {
    // Create a timer
    int status = timer_create(CLOCK_REALTIME,
                              &m_sigevent,
                              &m_timer);
    condDebugMsg(0 != status,
		 "PosixTimeAdapter:start",
		 " timer_create failed, errno = " << errno);
    return (status == 0);
  }

  /**
   * @brief Stops the adapter.  
   * @return true if successful, false otherwise.
   */
  bool PosixTimeAdapter::stop()
  {
    // Disable the timer
    stopTimer();
    // Now delete it
    int status = timer_delete(m_timer);
    condDebugMsg(status != 0,
		 "PosixTimeAdapter:stop",
		 " timer_delete failed, errno = " << errno);
    return (status == 0);
  }

  /**
   * @brief Resets the adapter.  
   * @return true if successful, false otherwise.
   */
  bool PosixTimeAdapter::reset()
  {
    return true;
  }

  /**
   * @brief Shuts down the adapter, releasing any of its resources.
   * @return true if successful, false otherwise.
   */
  bool PosixTimeAdapter::shutdown()
  {
    return true;
  }

  /**
   * @brief Perform an immediate lookup of the requested state.
   * @param state The state for this lookup.
   * @return The current value for this lookup.
   */
  double PosixTimeAdapter::lookupNow(const State& state)
  {
    assertTrueMsg(state == m_execInterface.getStateCache()->getTimeState(),
                  "PosixTimeAdapter only implements lookups for \"time\"");
    return getCurrentTime();
  }

  /**
   * @brief Inform the interface that it should report changes in value of this state.
   * @param state The state.
   */

  void PosixTimeAdapter::subscribe(const State& state)
  {
    assertTrueMsg(state == m_execInterface.getStateCache()->getTimeState(),
                  "PosixTimeAdapter only implements lookups for \"time\"");
  }

  /**
   * @brief Inform the interface that a lookup should no longer receive updates.
   * @param state The state.
   */
  void PosixTimeAdapter::unsubscribe(const State& state)
  {
    assertTrueMsg(state == m_execInterface.getStateCache()->getTimeState(),
                  "PosixTimeAdapter only implements lookups for \"time\"");

    // Disable the timer
    stopTimer();
  }

  /**
   * @brief Advise the interface of the current thresholds to use when reporting this state.
   * @param state The state.
   * @param hi The upper threshold, at or above which to report changes.
   * @param lo The lower threshold, at or below which to report changes.
   */
  void PosixTimeAdapter::setThresholds(const State& state, double hi, double lo)
  {
    assertTrueMsg(state == m_execInterface.getStateCache()->getTimeState(),
                  "PosixTimeAdapter only implements lookups for \"time\"");

    // Get the current time
    timespec now;
    assertTrueMsg(0 == clock_gettime(CLOCK_REALTIME, &now),
		  "PosixTimeAdapter::setThresholds: clock_gettime() failed, errno = " << errno);

    // Set up a timer to go off at the high time
    itimerspec tymrSpec = {{0, 0}, {0, 0}};
    tymrSpec.it_value = doubleToTimespec(hi) - now;
    if (tymrSpec.it_value.tv_nsec < 0 || tymrSpec.it_value.tv_sec < 0) {
      // Already past the scheduled time, submit wakeup
      debugMsg("PosixTimeAdapter:setThresholds",
	       " new value " << Expression::valueToString(hi) << " is in past, waking up Exec");
      timerTimeout();
      return;
    }

    tymrSpec.it_interval.tv_sec = tymrSpec.it_interval.tv_nsec = 0; // no repeats
    assertTrueMsg(0 == timer_settime(m_timer,
				     0, // flags: ~TIMER_ABSTIME
				     &tymrSpec,
				     NULL),
                  "PosixTimeAdapter::setThresholds: timer_settime failed, errno = " << errno);
    debugMsg("PosixTimeAdapter:setThresholds",
	     " timer set for " << Expression::valueToString(hi)
	     << ", tv_nsec = " << tymrSpec.it_value.tv_nsec);
  }

  //
  // Static member functions
  //

  /**
   * @brief Get the current time from the operating system.
   * @return A double representing the current time.
   */
  double PosixTimeAdapter::getCurrentTime()
  {
    timespec ts;
    if (0 != clock_gettime(CLOCK_REALTIME, &ts)) {
      debugMsg("PosixTimeAdapter:getCurrentTime",
	       " clock_gettime() failed, errno = " << errno << "; returning UNKNOWN");
      return Expression::UNKNOWN();
    }
    double tym = timespecToDouble(ts);
    debugMsg("PosixTimeAdapter:getCurrentTime", " returning " << Expression::valueToString(tym));
    return tym;
  }

  //
  // Internal member functions
  //

  /**
   * @brief Helper for constructor methods.
   */
  void PosixTimeAdapter::initSigevent()
  {
    // Pre-fill sigevent fields
    m_sigevent.sigev_notify = SIGEV_THREAD;          // invoke notify function
    m_sigevent.sigev_signo = SIGALRM;                // use alarm clock signal
    m_sigevent.sigev_value.sival_ptr = (void*) this; // parent PosixTimeAdapter instance
    m_sigevent.sigev_notify_function = PosixTimeAdapter::timerNotifyFunction;
    m_sigevent.sigev_notify_attributes = NULL;
  }

  /**
   * @brief Static member function invoked upon receiving a timer signal
   * @param this_as_sigval Pointer to the parent PosixTimeAdapter instance as a sigval.
   */
  void PosixTimeAdapter::timerNotifyFunction(sigval this_as_sigval)
  {
    // Simply invoke the timeout method
    PosixTimeAdapter* adapter = (PosixTimeAdapter*) this_as_sigval.sival_ptr;
    adapter->timerTimeout();
  }

  /**
   * @brief Report the current time to the Exec as an asynchronous lookup value.
   */
  void PosixTimeAdapter::timerTimeout()
  {
    double time = getCurrentTime();
    debugMsg("PosixTimeAdapter:lookupOnChange",
	     " timer timeout at " << Expression::valueToString(time));
    m_execInterface.handleValueChange(m_execInterface.getStateCache()->getTimeState(),
                                      time);
    m_execInterface.notifyOfExternalEvent();
  }

  /**
   * @brief Stop the timer.
   */
  void PosixTimeAdapter::stopTimer()
  {
    static itimerspec sl_tymrDisable = {{0, 0}, {0, 0}};
    // tymrSpec.it_interval.tv_sec = tymrSpec.it_interval.tv_nsec = 
    //   tymrSpec.it_value.tv_sec = tymrSpec.it_value.tv_nsec = 0;
    condDebugMsg(0 != timer_settime(m_timer,
				    0, // flags: ~TIMER_ABSTIME
				    &sl_tymrDisable,
				    NULL),
		 "PosixTimeAdapter:stopTimer",
		 " timer_settime failed, errno = " << errno);
  }

}

#endif // defined(_POSIX_TIMERS) && (_POSIX_TIMERS - 200112L) >= 0L

