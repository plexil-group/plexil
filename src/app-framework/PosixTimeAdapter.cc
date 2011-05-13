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
#include <cerrno>
#include <cmath> // for modf
#include <iomanip> // for setprecision()

namespace PLEXIL
{
  /**
   * @brief Constructor.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   */
  PosixTimeAdapter::PosixTimeAdapter(AdapterExecInterface& execInterface)
    : InterfaceAdapter(execInterface),
      m_timeVector(1, 0.0)
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
    : InterfaceAdapter(execInterface, xml),
      m_timeVector(1, 0.0)
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
    return true;
  }

  /**
   * @brief Stops the adapter.  
   * @return true if successful, false otherwise.
   */
  bool PosixTimeAdapter::stop()
  {
    // Delete all active timers
    LookupTimerMap::const_iterator it = m_lookupTimerMap.begin();
    while (it != m_lookupTimerMap.end())
      {
        // Delete the timer
        int status = timer_delete(it->second);
        assertTrueMsg(status == 0,
                      "PosixTimeAdapter::stop: timer_delete failed, errno = " << errno);
        it++;
      }

    // Clear the map
    m_lookupTimerMap.clear();

    return true;
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
   * @brief Register one LookupOnChange.
   * @param uniqueId The unique ID of this lookup.
   * @param stateKey The state key for this lookup.
   * @param tolerances A vector of tolerances for the LookupOnChange.
   */
  void PosixTimeAdapter::registerChangeLookup(const LookupKey& uniqueId,
                                              const StateKey& stateKey,
                                              const std::vector<double>& tolerances)
  {
    assertTrueMsg(stateKey == m_execInterface.getStateCache()->getTimeStateKey(),
                  "PosixTimeAdaptor only implements lookups for \"time\"");
    assertTrueMsg(tolerances.size() == 1,
                  "Wrong number of tolerances for LookupOnChange(\"time\")");
    assertTrueMsg(tolerances[0] > 0,
                  "LookupOnChange(\"time\") requires a positive tolerance");
    checkError(m_lookupTimerMap.find(uniqueId) == m_lookupTimerMap.end(),
               "Internal error: lookup key already in use!");

    // Create a timer
    timer_t tymr;
    int status = timer_create(CLOCK_REALTIME,
                              &m_sigevent,
                              &tymr);
    assertTrueMsg(status == 0,
                  "lookupOnChange: timer_create failed, errno = " << errno);

    // Set up a timer to repeat at the specified tolerance
    // *** N.B. Tolerance is assumed to be in seconds ***
    itimerspec tymrSpec;
    doubleToTimespec(tolerances[0], tymrSpec.it_interval);
    tymrSpec.it_value = tymrSpec.it_interval;
    status = timer_settime(tymr,
                           0, // flags: ~TIMER_ABSTIME
                           &tymrSpec,
                           NULL);
    assertTrueMsg(status == 0,
                  "lookupOnChange: timer_settime failed, errno = " << errno);

    // Add it to the map
    m_lookupTimerMap[uniqueId] = tymr;
  }

  /**
   * @brief Terminate one LookupOnChange.
   * @param uniqueId The unique ID of the lookup to be terminated.
   */
  void PosixTimeAdapter::unregisterChangeLookup(const LookupKey& uniqueId)
  {
    LookupTimerMap::iterator where = m_lookupTimerMap.find(uniqueId);
    if (where == m_lookupTimerMap.end())
      {
        return; // no such lookup, or already unregistered
      }

    // Delete the timer
    int status = timer_delete(where->second);
    assertTrueMsg(status == 0,
                  "lookupOnChange: timer_delete failed, errno = " << errno);

    // Delete the map entry
    m_lookupTimerMap.erase(where);
  }

  /**
   * @brief Perform an immediate lookup of the requested state.
   * @param stateKey The state key for this lookup.
   * @param dest A (reference to a) vector of doubles where the result is to be stored.
   */
  void PosixTimeAdapter::lookupNow(const StateKey& stateKey,
                                   std::vector<double>& dest)
  {
    assertTrueMsg(stateKey == m_execInterface.getStateCache()->getTimeStateKey(),
                  "PosixTimeAdaptor only implements lookups for \"time\"");
    dest.resize(1);
    dest[0] = getCurrentTime();
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
    int status = clock_gettime(CLOCK_REALTIME, &ts);
    assertTrueMsg(status == 0,
                  "lookupNow: clock_gettime() failed, errno = " << errno);
    double tym = timespecToDouble(ts);
    debugMsg("PosixTimeAdapter:getCurrentTime", " returning " << std::setprecision(15) << tym);
    return tym;
  }


  /**
   * @brief Convert a timespec value into a double.
   * @param ts Reference to a constant timespec instance.
   * @return The timespec value converted to a double float.
   */
  double PosixTimeAdapter::timespecToDouble(const timespec& ts)
  {
    return ((double) ts.tv_sec) +
      ((double) ts.tv_nsec) / 1.0e9;
  }

  /**
   * @brief Convert a double value into a timespec.
   * @param tym The double to be converted.
   * @param result Reference to a writable timespec instance.
   */

  void PosixTimeAdapter::doubleToTimespec(double tym, timespec& result)
  {
    double seconds = 0;
    double fraction = modf(tym, &seconds);

    result.tv_sec = (time_t) seconds;
    result.tv_nsec = (long) (fraction * 1.0e9);
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
    m_timeVector[0] = getCurrentTime();
    m_execInterface.handleValueChange(m_execInterface.getStateCache()->getTimeStateKey(),
                                      m_timeVector);
    m_execInterface.notifyOfExternalEvent();
  }

}

#endif // defined(_POSIX_TIMERS) && (_POSIX_TIMERS - 200112L) >= 0L

