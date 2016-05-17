/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

#include "TimeAdapterImpl.hh"

#include "AdapterConfiguration.hh"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExternalInterface.hh" // for g_interface
#include "State.hh"
#include "StateCacheEntry.hh"
#ifdef PLEXIL_WITH_THREADS
#include "ThreadSpawn.hh"
#endif

#include <cerrno>
#include <cmath> // for modf
#include <iomanip>

namespace PLEXIL
{

  TimeAdapterImpl::TimeAdapterImpl(AdapterExecInterface &mgr)
    : TimeAdapter(mgr),
      m_stopping(false)
  {
  }

  TimeAdapterImpl::TimeAdapterImpl(AdapterExecInterface &mgr,
				   pugi::xml_node const config)
    : TimeAdapter(mgr, config),
      m_stopping(false)
  {
  }

  TimeAdapterImpl::~TimeAdapterImpl()
  {
  }

  bool TimeAdapterImpl::initialize()
  {
    if (!configureSignalHandling()) {
      debugMsg("TimeAdapter:start", " signal handling initialization failed");
      return false;
    }

    // Automatically register self for time
    g_configuration->registerLookupInterface("time", this);
    return true;
  }

  bool TimeAdapterImpl::start()
  {
    if (!initializeTimer()) {
      debugMsg("TimeAdapter:start", " timer initialization failed");
      return false;
    }

#ifdef PLEXIL_WITH_THREADS
    threadSpawn(timerWaitThread, (void*) this, m_waitThread);
#endif

    return true;
  }

  bool TimeAdapterImpl::stop()
  {
    if (!stopTimer()) {
      debugMsg("TimeAdapter:stop", " stopTimer() failed");
    }

    if (!deleteTimer()) {
      debugMsg("TimeAdapter:stop", " deleteTimer() failed");
    }

    // N.B. on Linux SIGUSR1 does double duty as both terminate and timer wakeup,
    // so we need the stopping flag to figure out which is which.
#ifdef PLEXIL_WITH_THREADS
    m_stopping = true;
    pthread_kill(m_waitThread, SIGUSR1);
    pthread_join(m_waitThread, NULL);
#endif
    m_stopping = false;
    debugMsg("TimeAdapter:stop", " complete");
    return true;
  }

  bool TimeAdapterImpl::reset()
  {
    return true;
  }

  bool TimeAdapterImpl::shutdown()
  {
    return true;
  }

  void TimeAdapterImpl::lookupNow(State const &state, StateCacheEntry &cacheEntry)
  {
    assertTrueMsg(state == State::timeState(),
                  "TimeAdapter does not implement lookups for state " << state);
    cacheEntry.update(getCurrentTime());
  }

  void TimeAdapterImpl::subscribe(const State& state)
  {
    debugMsg("TimeAdapter:subscribe", " called");
  }

  void TimeAdapterImpl::unsubscribe(const State& state)
  {
    stopTimer();
    debugMsg("TimeAdapter:unsubscribe", " complete");
  }

  void TimeAdapterImpl::setThresholds(const State& state, double hi, double lo)
  {
    assertTrueMsg(state == State::timeState(),
                  "TimeAdapter does not implement lookups for state " << state);

    debugMsg("TimeAdapter:setThresholds", " setting wakeup at " << std::setprecision(15) << hi);
    if (setTimer(hi)) {
      debugMsg("TimeAdapter:setThresholds",
               " timer set for " << hi);
    }
    else {
      debugMsg("TimeAdapter:setThresholds",
               " sending wakeup for missed timer at " << hi);
      timerTimeout();
    }
  }

  void TimeAdapterImpl::setThresholds(const State& state, int32_t hi, int32_t lo)
  {
    assertTrue_2(ALWAYS_FAIL, "setThresholds of integer thresholds not implemented");
  }


#ifdef PLEXIL_WITH_THREADS
  /**
   * @brief Static member function which waits for timer wakeups.
   * @param this_as_void_ptr Pointer to the TimeAdapter instance, as a void *.
   */
  void* TimeAdapterImpl::timerWaitThread(void* this_as_void_ptr)
  {
    assertTrue_2(this_as_void_ptr != NULL,
                 "TimeAdapterImpl::timerWaitThread: argument is null!");
    TimeAdapterImpl* myInstance = reinterpret_cast<TimeAdapterImpl*>(this_as_void_ptr);
    return myInstance->timerWaitThreadImpl();
  }

  /**
   * @brief Internal function for the above.
   */
  void* TimeAdapterImpl::timerWaitThreadImpl()
  {
    // block most common signals for this thread
    sigset_t threadSigset;
    assertTrue_2(configureWaitThreadSigmask(&threadSigset),
                 "TimeAdapterImpl::timerWaitThreadImpl: signal mask initialization failed");
    int errnum = pthread_sigmask(SIG_BLOCK, &threadSigset, NULL);
    assertTrueMsg(0 == errnum,
                  "TimeAdapterImpl::timerWaitThreadImpl: pthread_sigmask failed, result = " << errnum);

    sigset_t waitSigset;
    assertTrue_2(initializeSigwaitMask(&waitSigset),
                 "TimeAdapterImpl::timerWaitThreadImpl: signal mask initialization failed");

    //
    // The wait loop
    //
    while (true) {
      int signalReceived = 0;
      int errnum = sigwait(&waitSigset, &signalReceived);
      assertTrueMsg(errnum == 0, 
                    "Fatal Error: sigwait failed, result = " << errnum);
      if (m_stopping) {
        debugMsg("TimeAdapter:timerWaitThread", " exiting on signal " << signalReceived);
        break;
      }
      // wake up the Exec
      timerTimeout();
    }
    return (void*) 0;
  }
#endif // PLEXIL_WITH_THREADS

  /**
   * @brief Wake up the exec
   */
  void TimeAdapterImpl::timerTimeout()
  {
    m_execInterface.notifyOfExternalEvent();
  }

} // namespace PLEXIL
