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

#include "TimeAdapter.hh"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Error.hh"
#include "StateCache.hh"
#include "ThreadSpawn.hh"
#include <cerrno>
#include <cmath> // for modf
#include <csignal>

#define STOP_WAIT_THREAD_SIGNAL SIGUSR1

namespace PLEXIL
{

  /**
   * @brief Constructor.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   */
  TimeAdapter::TimeAdapter(AdapterExecInterface& execInterface)
    : InterfaceAdapter(execInterface)
  {
  }

  /**
   * @brief Constructor from configuration XML.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   * @param xml A const reference to the XML element describing this adapter
   * @note The instance maintains a shared pointer to the XML.
   */
  TimeAdapter::TimeAdapter(AdapterExecInterface& execInterface, 
                                       const pugi::xml_node& xml)
    : InterfaceAdapter(execInterface, xml)
  {
  }

  /**
   * @brief Destructor.
   */
  TimeAdapter::~TimeAdapter()
  {
  }

  /**
   * @brief Initializes the adapter, possibly using its configuration data.
   * @return true if successful, false otherwise.
   */
  bool TimeAdapter::initialize()
  {
    // Automatically register self for time
    m_execInterface.registerLookupInterface(LabelStr("time"), getId());
    return true;
  }

  /**
   * @brief Starts the adapter, possibly using its configuration data.  
   * @return true if successful, false otherwise.
   */
  bool TimeAdapter::start()
  {
    // Spawn the timer thread
    threadSpawn(timerWaitThread, (void*) this, m_waitThread);
    return true;
  }

  bool TimeAdapter::stop()
  {
    pthread_kill(m_waitThread, STOP_WAIT_THREAD_SIGNAL);
    pthread_join(m_waitThread, NULL);

    debugMsg("TimeAdapter:stop", " complete");
    return true;
  }

  /**
   * @brief Resets the adapter.  
   * @return true if successful, false otherwise.
   */
  bool TimeAdapter::reset()
  {
    return true;
  }

  /**
   * @brief Shuts down the adapter, releasing any of its resources.
   * @return true if successful, false otherwise.
   */
  bool TimeAdapter::shutdown()
  {
    return true;
  }

  /**
   * @brief Perform an immediate lookup of the requested state.
   * @param state The state for this lookup.
   * @return The current value of the lookup.
   */
  Value TimeAdapter::lookupNow(const State& state)
  {
    assertTrueMsg(state == m_execInterface.getStateCache()->getTimeState(),
                  "TimeAdapter does not implement lookups for state "
                  << state.first.toString());
    return Value(getCurrentTime());
  }

  /**
   * @brief Inform the interface that it should report changes in value of this state.
   * @param state The state.
   */
  void TimeAdapter::subscribe(const State& /* state */)
  {
    debugMsg("TimeAdapter:subscribe", " complete");
  }

  /**
   * @brief Inform the interface that a lookup should no longer receive updates.
   * @param state The state.
   */
  void TimeAdapter::unsubscribe(const State& /* state */)
  {
    stopTimer();
    debugMsg("TimeAdapter:unsubscribe", " complete");
  }

  /**
   * @brief Advise the interface of the current thresholds to use when reporting this state.
   * @param state The state.
   * @param hi The upper threshold, at or above which to report changes.
   * @param lo The lower threshold, at or below which to report changes.
   */
  void TimeAdapter::setThresholds(const State& state, double hi, double /* lo */)
  {
    assertTrueMsg(state == m_execInterface.getStateCache()->getTimeState(),
                  "TimeAdapter does not implement lookups for state "
                  << state.first.toString());

    if (setTimer(hi)) {
      debugMsg("TimeAdapter:setThresholds",
               " timer set for " << Value::valueToString(hi));
    }
    else {
      debugMsg("TimeAdapter:setThresholds",
               " sending wakeup for missed timer at " << Value::valueToString(hi));
      timerTimeout();
    }
  }

  /**
   * @brief Static member function which waits for timer wakeups.
   * @param this_as_void_ptr Pointer to the TimeAdapter instance, as a void *.
   */
  void* TimeAdapter::timerWaitThread(void* this_as_void_ptr)
  {
    TimeAdapter* myInstance = reinterpret_cast<TimeAdapter*>(this_as_void_ptr);
    
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

    // listen only for SIGALRM and STOP_WAIT_THREAD_SIGNAL
    sigset_t waitSigset;
    errnum = sigemptyset(&waitSigset);
    errnum = errnum | sigaddset(&waitSigset, SIGALRM);
    errnum = errnum | sigaddset(&waitSigset, STOP_WAIT_THREAD_SIGNAL);
    assertTrueMsg(errnum == 0,
                  "Fatal Error: signal mask initialization failed!");

    while (true) {
      int signalReceived = 0;
      int errnum = sigwait(&waitSigset, &signalReceived);
      assertTrueMsg(errnum == 0, 
                    "Fatal Error: sigwait failed, result = " << errnum);
      if (signalReceived == STOP_WAIT_THREAD_SIGNAL) {
        debugMsg("TimeAdapter:timerWaitThread", " exiting on signal " << signalReceived);
        break;
      }
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
  void TimeAdapter::timerTimeout()
  {
    // report the current time and kick-start the Exec
    double time = getCurrentTime();
    debugMsg("TimeAdapter:timerTimeout", " at " << Value::valueToString(time));
    m_execInterface.handleValueChange(m_execInterface.getStateCache()->getTimeState(), time);
    m_execInterface.notifyOfExternalEvent();
  }

}
