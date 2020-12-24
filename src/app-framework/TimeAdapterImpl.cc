/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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
#include "InterfaceError.hh"
#include "LookupReceiver.hh"
#include "State.hh"
#ifdef PLEXIL_WITH_THREADS
#include "ThreadSpawn.hh"
#endif

#include <iomanip>

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

#if defined(HAVE_CLOCK_GETTIME)

#if defined(HAVE_CTIME)
#include <ctime>
#elif defined(HAVE_TIME_H)
#include <time.h>
#endif
#include "timespec-utils.hh"

#elif defined(HAVE_GETTIMEOFDAY)

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif
#include "timeval-utils.hh"

#endif

namespace PLEXIL
{

  TimeAdapterImpl::TimeAdapterImpl(AdapterExecInterface &mgr)
    : InterfaceAdapter(mgr),
      m_nextWakeup(0),
      m_stopping(false)
  {
  }

  TimeAdapterImpl::TimeAdapterImpl(AdapterExecInterface &mgr,
                                   pugi::xml_node const config)
    : InterfaceAdapter(mgr, config),
      m_nextWakeup(0),
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

    // N.B. on Linux SIGUSR1 does double duty as both terminate and timer wakeup,
    // so we need the stopping flag to figure out which is which.
#ifdef PLEXIL_WITH_THREADS
    m_stopping = true;
    pthread_kill(m_waitThread, SIGUSR1);
    pthread_join(m_waitThread, nullptr);
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
    stopTimer();
    if (!deleteTimer()) {
      debugMsg("TimeAdapter:shutdown", " deleteTimer() failed");
      return false;
    }
    debugMsg("TimeAdapter:shutdown", " complete");
    return true;
  }

  void TimeAdapterImpl::lookupNow(State const &state, LookupReceiver *rcvr)
  {
    if (state != State::timeState()) {
      warn("TimeAdapter does not implement lookups for state " << state);
      rcvr->setUnknown();
      return;
    }

    debugMsg("TimeAdapter:lookupNow", " called");
    rcvr->update(getCurrentTime());
  }

  /**
   * @brief Get the current time from the operating system.
   * @return A double representing the current time.
   * @note Default method. May be overridden.
   */
  double TimeAdapterImpl::getCurrentTime()
  {
    double tym;

    // Prefer clock_gettime() due to greater precision
#if defined(HAVE_CLOCK_GETTIME)
    timespec ts;
    checkInterfaceError(!clock_gettime(CLOCK_REALTIME, &ts),
                        "getCurrentTime: clock_gettime() failed, errno = " << errno);
    tym = timespecToDouble(ts);
#elif defined(HAVE_GETTIMEOFDAY)
    timeval tv;
    checkInterfaceError(0 == gettimeofday(&tv, nullptr),
                        "getCurrentTime: gettimeofday() failed, errno = " << errno);
    tym = timevalToDouble(tv);
#endif

    debugMsg("TimeAdapter:getCurrentTime", " returning " << std::setprecision(15) << tym);
    return tym;
  }

  void TimeAdapterImpl::setThresholds(const State& state, Real hi, Real lo)
  {
    if (state != State::timeState()) {
      warn("TimeAdapter does not implement lookups for state " << state);
      return;
    }

    debugMsg("TimeAdapter:setThresholds", " setting wakeup at " << std::setprecision(15) << hi);
    if (setTimer(hi)) {
      m_nextWakeup = hi;
      debugMsg("TimeAdapter:setThresholds",
               " timer set for " << std::setprecision(15) << hi);
    }
    else {
      debugMsg("TimeAdapter:setThresholds",
               " sending wakeup for missed timer at " << std::setprecision(15) << hi);
      timerTimeout();
    }
  }

  void TimeAdapterImpl::setThresholds(const State& state, Integer hi, Integer lo)
  {
    // This is an internal error, shouldn't be reachable from a plan
    errorMsg("setThresholds of integer thresholds not implemented");
  }


#ifdef PLEXIL_WITH_THREADS
  /**
   * @brief Static member function which waits for timer wakeups.
   * @param this_as_void_ptr Pointer to the TimeAdapter instance, as a void *.
   */
  void* TimeAdapterImpl::timerWaitThread(void* this_as_void_ptr)
  {
    assertTrue_2(this_as_void_ptr,
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
    if (!configureWaitThreadSigmask(&threadSigset)) {
      warn("TimeAdapter: signal mask initialization failed, unable to start timer thread");
      return (void *) 0;
    }
    int errnum;
    if ((errnum = pthread_sigmask(SIG_BLOCK, &threadSigset, nullptr))) {
      warn ("TimeAdapter: pthread_sigmask failed, result = " << errnum
            << "; unable to start timer thread");
      return (void *) 0;
    }

    sigset_t waitSigset;
    if (!initializeSigwaitMask(&waitSigset)) {
      warn("TimeAdapter: signal mask initialization failed, unable to start timer thread");
      return (void *) 0;
    }

    //
    // The wait loop
    //
    while (true) {
      int signalReceived = 0;

      int errnum = sigwait(&waitSigset, &signalReceived);
      if (errnum) {
        warn("TimeAdapter: sigwait failed, result = " << errnum
             << "; exiting timer thread");
        return (void *) 0;
      }
      
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
    double now = getCurrentTime();
    debugMsg("TimeAdapter:timerTimeout", " at " << std::setprecision(15) << getCurrentTime());
    if (m_nextWakeup) {
      if (now < m_nextWakeup) {
        // Alarm went off too early. Hit the snooze button.
        debugMsg("TimeAdapter:timerTimeout", " early wakeup, resetting");
        setTimer(m_nextWakeup);
      }
      else
        m_nextWakeup = 0;
    }
    // Notify in any case
    m_execInterface.notifyOfExternalEvent();
  }

} // namespace PLEXIL
