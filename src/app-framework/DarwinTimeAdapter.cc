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

//
// *** Ignore this file on systems that implement POSIX timers
//

#include "plexil-config.h"
#include <unistd.h>

#if defined(HAVE_SETITIMER) && (!defined(_POSIX_TIMERS) || (((_POSIX_TIMERS - 200112L) < 0L) && !defined(PLEXIL_ANDROID)))

#include "DarwinTimeAdapter.hh"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "InterfaceError.hh"
#include "TimeAdapter.hh"
#include "timeval-utils.hh"
#include <cerrno>
#include <cmath> // for modf
#include <iomanip>
#include <mach/kern_return.h> // for KERN_ABORTED
#include <sys/time.h> // for gettimeofday, itimerval

namespace PLEXIL
{
  /**
   * @brief Constructor.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   */
  DarwinTimeAdapter::DarwinTimeAdapter(AdapterExecInterface& execInterface)
    : TimeAdapterImpl(execInterface)
  {
  }

  /**
   * @brief Constructor from configuration XML.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   * @param xml A const reference to the XML element describing this adapter
   * @note The instance maintains a shared pointer to the XML.
   */
  DarwinTimeAdapter::DarwinTimeAdapter(AdapterExecInterface& execInterface, 
                                       pugi::xml_node const xml)
    : TimeAdapterImpl(execInterface, xml)
  {
  }

  /**
   * @brief Destructor.
   */
  DarwinTimeAdapter::~DarwinTimeAdapter()
  {
  }

  /**
   * @brief Get the current time from the operating system.
   * @return A double representing the current time.
   */
  double DarwinTimeAdapter::getCurrentTime()
    throw (InterfaceError)
  {
    timeval tv;
    checkInterfaceError(0 == gettimeofday(&tv, NULL),
                        "getCurrentTime: gettimeofday() failed, errno = " << errno);
    double tym = timevalToDouble(tv);
    debugMsg("TimeAdapter:getCurrentTime", " returning " << std::setprecision(15) << tym);
    return tym;
  }

  /**
   * @brief Initialize signal handling for the process.
   * @return True if successful, false otherwise.
   */
  bool DarwinTimeAdapter::configureSignalHandling()
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
  bool DarwinTimeAdapter::initializeTimer()
  {
    return true; // nothing to do
  }

  /**
   * @brief Set the timer.
   * @param date The Unix-epoch wakeup time, as a double.
   * @return True if the timer was set, false if clock time had already passed the wakeup time.
   */

  // N.B. gettimeofday() on macOS rarely performs an actual syscall:
  // https://stackoverflow.com/questions/40967594/does-gettimeofday-on-macos-use-a-system-call

  bool DarwinTimeAdapter::setTimer(double date)
    throw (InterfaceError)
  {
    static timeval const sl_timezero = {0, 0};
    struct timeval dateval = doubleToTimeval(date);

    struct timeval now;
    checkInterfaceError(0 == gettimeofday(&now, NULL),
                        "TimeAdapter:setTimer: gettimeofday() failed, errno = " << errno);

    // Check if we're already past the desired time
    dateval = dateval - now;
    if (dateval < sl_timezero) {
      // Already past the scheduled time, tell caller to submit wakeup
      debugMsg("TimeAdapter:setTimer",
               " desired time " << std::setprecision(15) << date << " is in the past");
      return false;
    }

    // Is timer already set for an earlier time?
    struct itimerval myItimerval = {{0, 0}, {0, 0}};
    checkInterfaceError(0 == getitimer(ITIMER_REAL, &myItimerval),
                        "TimeAdapter:setTimer: getitimer failed, errno = " << errno);
    if (timerisset(&myItimerval.it_value)
        && (dateval > myItimerval.it_value)) {
      debugMsg("TimeAdapter:setTimer",
               " already set for " << std::setprecision(15)
               << timevalToDouble(now + myItimerval.it_value));
      return true;
    }

    // Compute the interval and set the timer
    myItimerval.it_interval = sl_timezero;
    myItimerval.it_value = dateval;
    checkInterfaceError(0 == setitimer(ITIMER_REAL, &myItimerval, NULL),
                        "TimeAdapter:setTimer: setitimer failed, errno = " << errno);
    debugMsg("TimeAdapter:setTimer",
             " set timer for " << std::setprecision(15) << date);
    return true;
  }

  /**
   * @brief Stop the timer.
   * @return True if successful, false otherwise.
   */
  bool DarwinTimeAdapter::stopTimer()
  {
    static itimerval const sl_disableItimerval = {{0, 0}, {0, 0}};
    int status = setitimer(ITIMER_REAL, & sl_disableItimerval, NULL);
    condDebugMsg(status != 0,
                 "TimeAdapter:stopTimer",
                 " setitimer() failed, errno = " << errno);
    condDebugMsg(status == 0,
                 "TimeAdapter:stopTimer", " succeeded");
    return status == 0;
  }

  /**
   * @brief Shut down and delete the timer as required.
   * @return True if successful, false otherwise.
   */
  bool DarwinTimeAdapter::deleteTimer()
  {
    return true; // nothing to do
  }

  /**
   * @brief Initialize the wait thread signal mask.
   * @return True if successful, false otherwise.
   */
  bool DarwinTimeAdapter::configureWaitThreadSigmask(sigset_t* mask)
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
  bool DarwinTimeAdapter::initializeSigwaitMask(sigset_t* mask)
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


}

#endif // defined(HAVE_SETITIMER) && (!defined(_POSIX_TIMERS) || (((_POSIX_TIMERS - 200112L) < 0L) && !defined(PLEXIL_ANDROID)))
