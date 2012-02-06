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

#ifndef POSIX_TIME_ADAPTER_H
#define POSIX_TIME_ADAPTER_H

#include "InterfaceAdapter.hh"
#include <csignal>
#include <time.h> // *** better be POSIX time.h! ***
#include <map>

namespace PLEXIL
{

  /**
   * @brief An interface adapter using standard POSIX time facilities
   *        to implement LookupNow and LookupOnChange.
   */
  class PosixTimeAdapter : public InterfaceAdapter
  {
  public:
    /**
     * @brief Constructor.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     */
    PosixTimeAdapter(AdapterExecInterface& execInterface);

    /**
     * @brief Constructor from configuration XML.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     * @param xml A const reference to the XML element describing this adapter
     * @note The instance maintains a shared pointer to the XML.
     */
    PosixTimeAdapter(AdapterExecInterface& execInterface, 
                     const pugi::xml_node& xml);

    /**
     * @brief Destructor.
     */
    virtual ~PosixTimeAdapter();

    //
    // API to ExecApplication
    //

    /**
     * @brief Initializes the adapter, possibly using its configuration data.
     * @return true if successful, false otherwise.
     */
    bool initialize();

    /**
     * @brief Starts the adapter, possibly using its configuration data.  
     * @return true if successful, false otherwise.
     */
    bool start();

    /**
     * @brief Stops the adapter.  
     * @return true if successful, false otherwise.
     */
    bool stop();

    /**
     * @brief Resets the adapter.  
     * @return true if successful, false otherwise.
     */
    bool reset();

    /**
     * @brief Shuts down the adapter, releasing any of its resources.
     * @return true if successful, false otherwise.
     */
    bool shutdown();

    /**
     * @brief Perform an immediate lookup of the requested state.
     * @param state The state for this lookup.
     * @return The current value for this lookup.
     */

    double lookupNow(const State& state);

    /**
     * @brief Inform the interface that it should report changes in value of this state.
     * @param state The state.
     */
    void subscribe(const State& state);

    /**
     * @brief Inform the interface that a lookup should no longer receive updates.
     * @param state The state.
     */
    void unsubscribe(const State& state);

    /**
     * @brief Advise the interface of the current thresholds to use when reporting this state.
     * @param state The state.
     * @param hi The upper threshold, at or above which to report changes.
     * @param lo The lower threshold, at or below which to report changes.
     */
    void setThresholds(const State& state, double hi, double lo);

    //
    // Static member functions
    //

    /**
     * @brief Get the current time from the operating system.
     * @return A double representing the current time.
     */
    static double getCurrentTime();

  private:

    // Deliberately unimplemented
    PosixTimeAdapter();
    PosixTimeAdapter(const PosixTimeAdapter &);
    PosixTimeAdapter & operator=(const PosixTimeAdapter &);

    //
    // Internal member functions
    //

    /**
     * @brief Helper for constructor methods.
     */
    void initSigevent();

    /**
     * @brief Static member function invoked upon receiving a timer signal
     * @param this_as_sigval Pointer to the parent PosixTimeAdapter instance as a sigval.
     */
    static void timerNotifyFunction(sigval this_as_sigval);

    /**
     * @brief Report the current time to the Exec as an asynchronous lookup value.
     */
    void timerTimeout();

    /**
     * @brief Stop the timer.
     */
    void stopTimer();

    //
    // Member variables
    //

    sigevent m_sigevent;
    timer_t m_timer;
  };

}

#endif // POSIX_TIME_ADAPTER_H
