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

//
// Fallback for systems with setitimer() but no timer_create()
//

#include "plexil-config.h"

#if defined(HAVE_SETITIMER) && !defined(HAVE_TIMER_CREATE)

#include "AdapterFactory.hh"
#include "Debug.hh"
#include "InterfaceError.hh"
#include "TimeAdapterImpl.hh"
#include "timespec-utils.hh"
#include "timeval-utils.hh"

#include <iomanip>

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h> // for gettimeofday, itimerval, TIMESPEC_TO_TIMEVAL
#endif

namespace PLEXIL
{
  /**
   * @brief An interface adapter using Darwin native time facilities
   *        to implement LookupNow and LookupOnChange.
   */
  class DarwinTimeAdapter : public TimeAdapterImpl
  {
  public:
    /**
     * @brief Constructor.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     */
    DarwinTimeAdapter(AdapterExecInterface& execInterface)
      : TimeAdapterImpl(execInterface)
    {
    }

    /**
     * @brief Constructor from configuration XML.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     * @param xml A const reference to the XML element describing this adapter
     * @note The instance maintains a shared pointer to the XML.
     */
    DarwinTimeAdapter(AdapterExecInterface& execInterface, 
                      pugi::xml_node const xml)
      : TimeAdapterImpl(execInterface, xml)
    {
    }

    /**
     * @brief Destructor.
     */
    virtual ~DarwinTimeAdapter()
    {
    }

  protected:

    /**
     * @brief Initialize signal handling for the process.
     * @return True if successful, false otherwise.
     */
    virtual bool configureSignalHandling()
    {
      // block SIGALRM and SIGUSR1 for the process as a whole
      sigset_t processSigset, originalSigset;
      if (0 != sigemptyset(&processSigset)) {
        warn("DarwinTimeAdapter: sigemptyset failed!");
        return false;
      }

      int errnum = sigaddset(&processSigset, SIGALRM);
      errnum = errnum | sigaddset(&processSigset, SIGUSR1);
      if (errnum != 0) {
        warn("DarwinTimeAdapter: sigaddset failed!");
        return false;
      }

      if (0 != sigprocmask(SIG_BLOCK, &processSigset, &originalSigset)) {
        warn("DarwinTimeAdapter: sigprocmask failed!, errno = " << errno);
        return false;
      }

      return true;
    }

    /**
     * @brief Construct and initialize the timer as required.
     * @return True if successful, false otherwise.
     */
    virtual bool initializeTimer()
    {
      return true; // nothing to do
    }

    /**
     * @brief Set the timer.
     * @param date The Unix-epoch wakeup time, as a double.
     * @return True if the timer was set, false if clock time had already passed the wakeup time.
     * @note This function is not reentrant! Acquire m_timerMutex before calling.
     */

    // N.B. gettimeofday() on macOS rarely performs an actual syscall:
    // https://stackoverflow.com/questions/40967594/does-gettimeofday-on-macos-use-a-system-call

    // N.B. setitimer() wants timevals, and won't let us specify a monotonic clock.
    // So wing it.

    virtual bool setTimer(double date)
    {
      static struct timespec const sl_timezero_ts = {0, 0};
      struct timespec now_ts;
      struct itimerval myItimerval;

      checkInterfaceError(!clock_gettime(PLEXIL_CLOCK_GETTIME, &now_ts),
                          "TimeAdapter:setTimer: clock_gettime() failed, errno = " << errno);

      // Check if we're already past the desired time
      struct timespec interval_ts = doubleToTimespec(date) - now_ts;
      if (interval_ts < sl_timezero_ts) {
        // Already past the scheduled time, tell caller to submit wakeup
        debugMsg("TimeAdapter:setTimer",
                 " desired time " << std::setprecision(15) << date << " is in the past");
        return false;
      }

      struct timeval interval_tv;
      TIMESPEC_TO_TIMEVAL(&interval_tv, &interval_ts);

      // Is timer already set for an earlier time?
      checkInterfaceError(!getitimer(ITIMER_REAL, &myItimerval),
                          "TimeAdapter:setTimer: getitimer failed, errno = " << errno);
      if (timerisset(&myItimerval.it_value)
          && !(interval_tv < myItimerval.it_value)) {
        debugStmt("TimeAdapter:setTimer",
                  {
                    struct timeval now_tv;
                    TIMESPEC_TO_TIMEVAL(&now_tv, &now_ts);
                    debugMsg("TimeAdapter:setTimer",
                             " already set for " << std::setprecision(15)
                             << timevalToDouble(now_tv + myItimerval.it_value));
                  });
        return true;
      }

      // Actually set the timer (but clear recurrence interval first)
      myItimerval.it_interval.tv_sec = 0; 
      myItimerval.it_interval.tv_usec = 0; 
      myItimerval.it_value = interval_tv;
      checkInterfaceError(0 == setitimer(ITIMER_REAL, &myItimerval, NULL),
                          "TimeAdapter:setTimer: setitimer failed, errno = " << errno);

      // Report time actually given to setitimer
      debugStmt("TimeAdapter:setTimer",
                {
                  struct timeval now_tv;
                  TIMESPEC_TO_TIMEVAL(&now_tv, &now_ts);
                  debugMsg("TimeAdapter:setTimer",
                           " timer set for "
                           << std::setprecision(15)
                           << timevalToDouble(now_tv + myItimerval.it_value));
                });

      return true;
    }

    /**
     * @brief Stop the timer.
     * @return True if successful, false otherwise.
     */
    virtual bool stopTimer()
    {
      static itimerval const sl_disableItimerval = {{0, 0}, {0, 0}};
      int status = setitimer(ITIMER_REAL, & sl_disableItimerval, NULL);
      condDebugMsg(status != 0,
                   "DarwinTimeAdapter:stopTimer",
                   " setitimer() failed, errno = " << errno);
      condDebugMsg(status == 0,
                   "TimeAdapter:stopTimer", " succeeded");
      return status == 0;
    }

    /**
     * @brief Shut down and delete the timer as required.
     * @return True if successful, false otherwise.
     */
    virtual bool deleteTimer()
    {
      return true; // nothing to do
    }

    /**
     * @brief Initialize the wait thread signal mask.
     * @return True if successful, false otherwise.
     */
    virtual bool configureWaitThreadSigmask(sigset_t* mask)
    {
      if (0 != sigemptyset(mask)) {
        warn("DarwinTimeAdapter: sigemptyset failed!");
        return false;
      }

      int errnum = sigaddset(mask, SIGINT);
      errnum = errnum | sigaddset(mask, SIGHUP);
      errnum = errnum | sigaddset(mask, SIGQUIT);
      errnum = errnum | sigaddset(mask, SIGTERM);
      errnum = errnum | sigaddset(mask, SIGUSR2);
      if (errnum != 0) {
        warn("DarwinTimeAdapter: sigaddset failed!");
      }

      return errnum == 0;
    }

    /**
     * @brief Initialize the sigwait mask.
     * @param Pointer to the mask.
     * @return True if successful, false otherwise.
     */
    virtual bool initializeSigwaitMask(sigset_t* mask)
    {
      // listen for SIGALRM and SIGUSR1
      if (0 != sigemptyset(mask)) {
        warn("DarwinTimeAdapter: sigemptyset failed!");
        return false;
      }

      int status = sigaddset(mask, SIGUSR1);
      status = status | sigaddset(mask, SIGALRM);
      if (0 != status) {
        warn("DarwinTimeAdapter: sigaddset failed!");
      }
      return 0 == status;
    }

  private:

    // Deliberately unimplemented
    DarwinTimeAdapter();
    DarwinTimeAdapter(const DarwinTimeAdapter &);
    DarwinTimeAdapter & operator=(const DarwinTimeAdapter &);
  };

  void registerTimeAdapter()
  {
    REGISTER_ADAPTER(DarwinTimeAdapter, "OSNativeTime");
  }

}

#endif // defined(HAVE_SETITIMER) && !defined(HAVE_TIMER_CREATE)
