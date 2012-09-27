/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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
// *** Ignore this file on systems that implement POSIX timers
//

#include <unistd.h>
#if !defined(_POSIX_TIMERS) || ((_POSIX_TIMERS - 200112L) < 0L)

#include "DarwinTimeAdapter.hh"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Error.hh"
#include "StateCache.hh"
#include "ThreadSpawn.hh"
#include "timeval-utils.hh"
#include <cerrno>
#include <cmath> // for modf
#include <mach/kern_return.h> // for KERN_ABORTED

namespace PLEXIL
{
  /**
   * @brief Constructor.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   */
  DarwinTimeAdapter::DarwinTimeAdapter(AdapterExecInterface& execInterface)
    : InterfaceAdapter(execInterface)
  {
  }

  /**
   * @brief Constructor from configuration XML.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   * @param xml A const reference to the XML element describing this adapter
   * @note The instance maintains a shared pointer to the XML.
   */
  DarwinTimeAdapter::DarwinTimeAdapter(AdapterExecInterface& execInterface, 
                                       const pugi::xml_node& xml)
    : InterfaceAdapter(execInterface, xml)
  {
  }

  /**
   * @brief Destructor.
   */
  DarwinTimeAdapter::~DarwinTimeAdapter()
  {
  }

  /**
   * @brief Initializes the adapter, possibly using its configuration data.
   * @return true if successful, false otherwise.
   */
  bool DarwinTimeAdapter::initialize()
  {
    // Automatically register self for time
    m_execInterface.registerLookupInterface(LabelStr("time"), getId());
    return true;
  }

  /**
   * @brief Starts the adapter, possibly using its configuration data.  
   * @return true if successful, false otherwise.
   */
  bool DarwinTimeAdapter::start()
  {
    // Spawn the timer thread
    threadSpawn(timerWaitThread, (void*) this, m_waitThread);

    return true;
  }

  /**
   * @brief Stops the adapter.  
   * @return true if successful, false otherwise.
   */
  bool DarwinTimeAdapter::stop()
  {
    // Disable the timer
    stopTimer();

    // FIXME (?): stop the timer thread

    debugMsg("DarwinTimeAdapter:stop", " complete");
    return true;
  }

  /**
   * @brief Resets the adapter.  
   * @return true if successful, false otherwise.
   */
  bool DarwinTimeAdapter::reset()
  {
    return true;
  }

  /**
   * @brief Shuts down the adapter, releasing any of its resources.
   * @return true if successful, false otherwise.
   */
  bool DarwinTimeAdapter::shutdown()
  {
    return true;
  }

  /**
   * @brief Perform an immediate lookup of the requested state.
   * @param state The state for this lookup.
   * @return The current value of the lookup.
   */
  double DarwinTimeAdapter::lookupNow(const State& state)
  {
    assertTrueMsg(state == m_execInterface.getStateCache()->getTimeState(),
                  "DarwinTimeAdapter does not implement lookups for state "
                  << LabelStr(state.first).toString());
    return getCurrentTime();
  }

  /**
   * @brief Inform the interface that it should report changes in value of this state.
   * @param state The state.
   */
  void DarwinTimeAdapter::subscribe(const State& state)
  {
    assertTrueMsg(state == m_execInterface.getStateCache()->getTimeState(),
                  "DarwinTimeAdapter does not implement lookups for state "
                  << LabelStr(state.first).toString());
    debugMsg("DarwinTimeAdapter:subscribe", " complete");
  }

  /**
   * @brief Inform the interface that a lookup should no longer receive updates.
   * @param state The state.
   */
  void DarwinTimeAdapter::unsubscribe(const State& state)
  {
    assertTrueMsg(state == m_execInterface.getStateCache()->getTimeState(),
                  "DarwinTimeAdapter does not implement lookups for state "
                  << LabelStr(state.first).toString());
    stopTimer();
    debugMsg("DarwinTimeAdapter:unsubscribe", " complete");
  }

  /**
   * @brief Advise the interface of the current thresholds to use when reporting this state.
   * @param state The state.
   * @param hi The upper threshold, at or above which to report changes.
   * @param lo The lower threshold, at or below which to report changes.
   */
  void DarwinTimeAdapter::setThresholds(const State& state, double hi, double /* lo */)
  {
    assertTrueMsg(state == m_execInterface.getStateCache()->getTimeState(),
                  "DarwinTimeAdapter does not implement lookups for state "
                  << LabelStr(state.first).toString());

    bool wasSet = setTimer(hi);
    debugMsg("DarwinTimeAdapter:setThresholds",
             (wasSet ? " timer set for " : " wakeup sent for missed timer at ")
             << Expression::valueToString(hi));
  }

  //
  // Static member functions
  //

  /**
   * @brief Get the current time from the operating system.
   * @return A double representing the current time.
   */
  double DarwinTimeAdapter::getCurrentTime()
  {
    timeval tv;
    int status = gettimeofday(&tv, NULL);
    assertTrueMsg(status == 0,
                  "DarwinTimeAdapter::getCurrentTime: gettimeofday() failed, errno = " << errno);
    double tym = timevalToDouble(tv);
    debugMsg("DarwinTimeAdapter:getCurrentTime", " returning " << Expression::valueToString(tym));
    return tym;
  }

  //
  // Internal member functions
  //


  /**
   * @brief Set the timer.
   * @param date The Unix-epoch wakeup time, as a double.
   * @return True if the timer was set, false if clock time had already passed the wakeup time.
   */
  bool DarwinTimeAdapter::setTimer(double date)
  {
    // Convert to timeval
    timeval dateval = doubleToTimeval(date);

    // Get the current time
    timeval now;
    int status = gettimeofday(&now, NULL);
    assertTrueMsg(status == 0,
                  "DarwinTimeAdapter::setTimer: gettimeofday() failed, errno = " << errno);

    // Compute the interval
    itimerval myItimerval = {{0, 0}, {0, 0}};
    myItimerval.it_value = dateval - now;
    if (myItimerval.it_value.tv_usec < 0 || myItimerval.it_value.tv_sec < 0) {
      // Already past the scheduled time, submit wakeup
      debugMsg("DarwinTimeAdapter:setTimer",
               " new value " << Expression::valueToString(date) << " is in past, waking up Exec");
      timerTimeout();
      return false;
    }

    // Set the timer 
    assertTrueMsg(0 == setitimer(ITIMER_REAL, &myItimerval, NULL),
                  "DarwinTimeAdapter::setTimer: setitimer failed, errno = " << errno);
    debugMsg("DarwinTimeAdapter:setTimer",
             " timer set for " << Expression::valueToString(date));
    return true;
  }

  /**
   * @brief Stop the timer.
   */
  void DarwinTimeAdapter::stopTimer()
  {
    static itimerval const sl_disableItimerval = {{0, 0}, {0, 0}};
    int status = setitimer(ITIMER_REAL, & sl_disableItimerval, NULL);
    assertTrueMsg(status == 0,
                  "DarwinTimeAdapter::stopTimer: call to setitimer() failed, errno = " << errno);
  }

  /**
   * @brief Static member function which waits for timer wakeups.
   * @param this_as_void_ptr Pointer to the DarwinTimeAdapter instance, as a void *.
   */
  void* DarwinTimeAdapter::timerWaitThread(void* this_as_void_ptr)
  {
    DarwinTimeAdapter* myInstance = reinterpret_cast<DarwinTimeAdapter*>(this_as_void_ptr);
    
    //
    // Set up signal handling environment.
    //

    // block SIGALRM for the process as a whole
    sigset_t processSigset, originalSigset;
    int errnum = sigemptyset(&processSigset);
    assertTrueMsg(errnum == 0,
                  "Fatal Error: signal mask initialization failed!");
    errnum = sigaddset(&processSigset, SIGALRM);
    assertTrueMsg(errnum == 0,
                  "Fatal Error: signal mask initialization failed!");
    errnum = sigprocmask(SIG_BLOCK, &processSigset, &originalSigset);
    assertTrueMsg(errnum == 0,
                  "Fatal Error: sigprocmask failed, result = " << errnum);

    // block most common signals for this thread
    sigset_t threadSigset;
    errnum = sigemptyset(&threadSigset);
    errnum = sigaddset(&threadSigset, SIGALRM);
    errnum = errnum | sigaddset(&threadSigset, SIGINT);
    errnum = errnum | sigaddset(&threadSigset, SIGHUP);
    errnum = errnum | sigaddset(&threadSigset, SIGQUIT);
    errnum = errnum | sigaddset(&threadSigset, SIGTERM);
    errnum = errnum | sigaddset(&threadSigset, SIGUSR1);
    errnum = errnum | sigaddset(&threadSigset, SIGUSR2);
    // FIXME: maybe more??
    assertTrueMsg(errnum == 0,
                  "Fatal Error: signal mask initialization failed!");
    errnum = pthread_sigmask(SIG_BLOCK, &threadSigset, NULL);
    assertTrueMsg(errnum == 0,
                  "Fatal Error: pthread_sigmask failed, result = " << errnum);

    //
    // Wait loop
    //

    // listen only for SIGALRM
    sigset_t waitSigset;
    errnum = sigemptyset(&waitSigset);
    errnum = errnum | sigaddset(&waitSigset, SIGALRM);
    assertTrueMsg(errnum == 0,
                  "Fatal Error: signal mask initialization failed!");

    while (true) {
      int signalReceived = 0;
      int errnum = sigwait(&waitSigset, &signalReceived);
      if (errnum == KERN_ABORTED) {
        // should only happen on pthread_cancel (?)
        debugMsg("DarwinTimeAdapter:timerWaitThread", " interrupted, exiting");
        break;
      }
      assertTrueMsg(errnum == 0, 
                    "Fatal Error: sigwait failed, result = " << errnum);
      // wake up the Exec
      myInstance->timerTimeout();
    }

    //
    // Clean up - 
    //  restore previous process signal mask
    //
    errnum = sigprocmask(SIG_SETMASK, &originalSigset, NULL);
    assertTrueMsg(errnum == 0,
                  "Fatal Error: sigprocmask failed, result = " << errnum);

    return (void*) 0;
  }

  /**
   * @brief Report the current time to the Exec as an asynchronous lookup value.
   */
  void DarwinTimeAdapter::timerTimeout()
  {
    // report the current time and kick-start the Exec
    double time = getCurrentTime();
    debugMsg("DarwinTimeAdapter:timerTimeout", " at " << Expression::valueToString(time));
    m_execInterface.handleValueChange(m_execInterface.getStateCache()->getTimeState(), time);
    m_execInterface.notifyOfExternalEvent();
  }

}

#endif // !defined(_POSIX_TIMERS) || (_POSIX_TIMERS - 200112L) < 0L
