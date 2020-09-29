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
#include "lookupHandlerDefs.hh"
#include "StateCacheEntry.hh"
#ifdef PLEXIL_WITH_THREADS
#include "ThreadSpawn.hh"
#endif

#include <iomanip>

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <cerrno>
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

  //
  // Time lookup handler implementation
  //

  class TimeLookupHandler : public LookupHandler
  {
  private:

    TimeAdapterImpl &m_adapter;

  public:

    TimeLookupHandler(TimeAdapterImpl &adapter)
    : LookupHandler(),
      m_adapter(adapter)
    {
    }

    virtual ~TimeLookupHandler()
    {
    }

    virtual void lookupNow(const State & /* state */, StateCacheEntry &cacheEntry)
    {
      double now = m_adapter.getCurrentTime();
      debugMsg("TimeAdapter:lookupNow",
               " The time is now " << std::setprecision(15) << now);
      cacheEntry.update(now);
    }

    virtual void subscribe(const State & /* state */, AdapterExecInterface * /* intf */)
    {
      debugMsg("TimeAdapter:subscribe", " called");
    }
      
    virtual void unsubscribe(const State &state)
    {
      m_adapter.stopTimer();
      debugMsg("TimeAdapter:unsubscribe", " complete");
    }

    virtual void setThresholds(const State & /* state */, double hi, double /* lo */)
    {
      debugMsg("TimeAdapter:setThresholds", " high threshold is " << std::setprecision(15) << hi);
      m_adapter.setNextWakeup(hi);
    }

    virtual void setThresholds(const State &state, int32_t hi, int32_t lo)
    {
      // This is an internal error, shouldn't be reachable from a plan
      errorMsg("TimeAdapter: setThresholds not implemented for Integer thresholds");
    }
  };

  //
  // TimeAdapterImpl
  //

  TimeAdapterImpl::TimeAdapterImpl(AdapterExecInterface &mgr)
    : InterfaceAdapter(mgr),
#ifdef PLEXIL_WITH_THREADS
      m_timerMutex(),
#endif      
      m_nextWakeup(0),
      m_stopping(false)
  {
  }

  TimeAdapterImpl::TimeAdapterImpl(AdapterExecInterface &mgr,
                                   pugi::xml_node const config)
    : InterfaceAdapter(mgr, config),
#ifdef PLEXIL_WITH_THREADS
      m_timerMutex(),
#endif      
      m_nextWakeup(0),
      m_stopping(false)
  {
  }

  TimeAdapterImpl::~TimeAdapterImpl()
  {
  }

  bool TimeAdapterImpl::initialize(AdapterConfiguration *config)
  {
    if (!this->configureSignalHandling()) {
      debugMsg("TimeAdapter:start", " signal handling initialization failed");
      return false;
    }

    // Automatically register self for time
    config->registerLookupHandler("time", new TimeLookupHandler(*this));
    return true;
  }

  bool TimeAdapterImpl::start()
  {
    if (!this->initializeTimer()) {
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
    if (!this->stopTimer()) {
      debugMsg("TimeAdapter:stop", " stopTimer() failed");
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
    if (!this->deleteTimer()) {
      debugMsg("TimeAdapter:shutdown", " deleteTimer() failed");
      return false;
    }
    debugMsg("TimeAdapter:shutdown", " complete");
    return true;
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
    checkInterfaceError(!clock_gettime(PLEXIL_CLOCK_GETTIME, &ts),
                        "getCurrentTime: clock_gettime() failed, errno = " << errno);
    tym = timespecToDouble(ts);
#elif defined(HAVE_GETTIMEOFDAY)
    timeval tv;
    checkInterfaceError(0 == gettimeofday(&tv, NULL),
                        "getCurrentTime: gettimeofday() failed, errno = " << errno);
    tym = timevalToDouble(tv);
#else
    // Developer should have implemented this in a derived class
    reportInterfaceError("TimeAdapterImpl::getCurrentTime not implemented for this platform");
#endif

    debugMsg("TimeAdapter:getCurrentTime", " returning " << std::setprecision(15) << tym);
    return tym;
  }

  void TimeAdapterImpl::setNextWakeup(double date)
  {
    debugMsg("TimeAdapter:setThresholds",
             " setting wakeup at " << std::setprecision(15) << date);
    bool timerSet = false;

    // begin critical section
    {
#ifdef PLEXIL_WITH_THREADS    
      ThreadMutexGuard g(m_timerMutex);
#endif
      timerSet = this->setTimer(date);
      if (timerSet)
        m_nextWakeup = date;
    }
    // end critical section

    if (!timerSet) {
      debugMsg("TimeAdapter:setThresholds",
               " notifying Exec for missed wakeup at " << std::setprecision(15) << date);
      timerTimeout();
    }
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
    assertTrue_2(myInstance,
                 "TimeAdapterImpl::timerWaitThread: argument is not a pointer to a TimeAdapterImpl instance");
    return myInstance->timerWaitThreadImpl();
  }

  /**
   * @brief Internal function for the above.
   */
  void* TimeAdapterImpl::timerWaitThreadImpl()
  {
    // block most common signals for this thread
    sigset_t threadSigset;
    if (!this->configureWaitThreadSigmask(&threadSigset)) {
      warn("TimeAdapter: signal mask initialization failed, unable to start timer thread");
      return (void *) 0;
    }
    int errnum;
    if ((errnum = pthread_sigmask(SIG_BLOCK, &threadSigset, NULL))) {
      warn ("TimeAdapter: pthread_sigmask failed, result = " << errnum
            << "; unable to start timer thread");
      return (void *) 0;
    }

    sigset_t waitSigset;
    if (!this->initializeSigwaitMask(&waitSigset)) {
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
    double now = 0;
    double was = 0;
  
    // begin critical section
    {
#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard g(m_timerMutex);
#endif
      was = m_nextWakeup;
      now = getCurrentTime();
      if (m_nextWakeup) {
        if (now < m_nextWakeup) {
          // Alarm went off too early. Hit the snooze button.
          this->setTimer(m_nextWakeup);
        }
        else {
          m_nextWakeup = 0;
        }
      }
    }
    // end critical section
    
    // Report what happened for debugging purposes
    condDebugMsg(was && now < was,
                 "TimeAdapter:timerTimeout",
                 " wakeup at " << std::setprecision(15) << now
                 << " is early, reset to " << std::setprecision(15) << was);
    condDebugMsg(was && now >= was,
                 "TimeAdapter:timerTimeout",
                 " wakeup at " << std::setprecision(15) << now
                 << ", scheduled for " << std::setprecision(15) << was);
    condDebugMsg(!was,
                 "TimeAdapter:timerTimeout",
                 " unscheduled wakeup at " << std::setprecision(15) << now);

    // Notify in any case, something might be ready to execute.
    m_execInterface.notifyOfExternalEvent();
  }

} // namespace PLEXIL
