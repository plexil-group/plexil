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
// *** Ignore this file on systems that implement POSIX timers
//

#include <unistd.h>
#if !defined(_POSIX_TIMERS) || ((_POSIX_TIMERS - 200112L) < 0L) && !defined(PLEXIL_ANDROID)

#include "DarwinTimeAdapter.hh"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Error.hh"
#include "StateCache.hh"
#include "ThreadSpawn.hh"
#include "TimeAdapter.hh"
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
    : TimeAdapter(execInterface)
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
    : TimeAdapter(execInterface, xml)
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
  {
    timeval tv;
    int status = gettimeofday(&tv, NULL);
    assertTrueMsg(status == 0,
                  "TimeAdapter:getCurrentTime: gettimeofday() failed, errno = " << errno);
    double tym = timevalToDouble(tv);
    debugMsg("TimeAdapter:getCurrentTime", " returning " << Value::valueToString(tym));
    return tym;
  }

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
                  "TimeAdapter:setTimer: gettimeofday() failed, errno = " << errno);

    // Compute the interval
    itimerval myItimerval = {{0, 0}, {0, 0}};
    myItimerval.it_value = dateval - now;
    if (myItimerval.it_value.tv_usec < 0 || myItimerval.it_value.tv_sec < 0) {
      // Already past the scheduled time, submit wakeup
      debugMsg("TimeAdapter:setTimer",
               " new value " << Value::valueToString(date) << " is in past");
      return false;
    }

    // Set the timer 
    assertTrueMsg(0 == setitimer(ITIMER_REAL, &myItimerval, NULL),
                  "TimeAdapter:setTimer: setitimer failed, errno = " << errno);
    debugMsg("TimeAdapter:setTimer",
             " timer set for " << Value::valueToString(date));
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
                  "TimeAdapter:stopTimer: setitimer() failed, errno = " << errno);
  }

}

#endif // !defined(_POSIX_TIMERS) || (_POSIX_TIMERS - 200112L) < 0L
