/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#ifndef TIME_ADAPTER_H
#define TIME_ADAPTER_H

#include "InterfaceAdapter.hh"

#include <plexil-config.h>

#include <csignal>

#ifdef PLEXIL_WITH_THREADS
#include <pthread.h>
#endif

#include <unistd.h>
#if defined(_POSIX_TIMERS) && ((_POSIX_TIMERS - 200112L) >= 0L || defined(PLEXIL_ANDROID))
// POSIX timers are supported
#define TIME_ADAPTER_CLASS PosixTimeAdapter
#elif defined(HAVE_SETITIMER)
// BSD timers are supported
// Currently only Mac OS X
#define TIME_ADAPTER_CLASS DarwinTimeAdapter
#else
// Custom implementation needed
#error No time adapter class implemented for this platform.
#endif

namespace PLEXIL
{

  /**
   * @brief An interface adapter for Unix-like systems, using native time facilities
   *        to implement LookupNow and LookupOnChange.
   */
  class TimeAdapter : public InterfaceAdapter
  {
  public:
    /**
     * @brief Constructor.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     */
    TimeAdapter(AdapterExecInterface& execInterface);

    /**
     * @brief Constructor from configuration XML.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     * @param xml An XML element describing this adapter
     * @note The instance maintains a shared pointer to the XML element.
     */
    TimeAdapter(AdapterExecInterface& execInterface, 
                pugi::xml_node const xml);

    /**
     * @brief Destructor.
     */
    virtual ~TimeAdapter();

    //
    // API to ExecApplication
    //

    /**
     * @brief Initializes the adapter, possibly using its configuration data.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize();

    /**
     * @brief Starts the adapter, possibly using its configuration data.  
     * @return true if successful, false otherwise.
     */
    virtual bool start();

    /**
     * @brief Stops the adapter.  
     * @return true if successful, false otherwise.
     */
    virtual bool stop();

    /**
     * @brief Resets the adapter.  
     * @return true if successful, false otherwise.
     */
    virtual bool reset();

    /**
     * @brief Shuts down the adapter, releasing any of its resources.
     * @return true if successful, false otherwise.
     */
    virtual bool shutdown();

    /**
     * @brief Perform an immediate lookup of the requested state.
     * @param state The state for this lookup.
     * @return The current value of the lookup.
     */
    void lookupNow(State const &state, StateCacheEntry &cacheEntry);

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
    void setThresholds(const State& state, int32_t hi, int32_t lo);

    /**
     * @brief Get the current time from the operating system.
     * @return A double representing the current time.
     */
    virtual double getCurrentTime() = 0;

  protected:

    //
    // Internal functions to be implemented by derived classes
    //

    /**
     * @brief Initialize signal handling for the process.
     * @return True if successful, false otherwise.
     */
    virtual bool configureSignalHandling() = 0;

    /**
     * @brief Construct and initialize the timer as required.
     * @return True if successful, false otherwise.
     */
    virtual bool initializeTimer() = 0;

    /**
     * @brief Set the timer.
     * @param date The Unix-epoch wakeup time, as a double.
     * @return True if the timer was set, false if clock time had already passed the wakeup time.
     */
    virtual bool setTimer(double date) = 0;

    /**
     * @brief Stop the timer.
     * @return True if successful, false otherwise.
     */
    virtual bool stopTimer() = 0;

    /**
     * @brief Shut down and delete the timer as required.
     * @return True if successful, false otherwise.
     */
    virtual bool deleteTimer() = 0;

    /**
     * @brief Initialize the wait thread signal mask.
     * @return True if successful, false otherwise.
     */
    virtual bool configureWaitThreadSigmask(sigset_t* mask) = 0;

    /**
     * @brief Initialize the sigwait mask.
     * @param Pointer to the mask.
     * @return True if successful, false otherwise.
     */
    virtual bool initializeSigwaitMask(sigset_t* mask) = 0;

  private:

    // Deliberately unimplemented
    TimeAdapter();
    TimeAdapter(const TimeAdapter &);
    TimeAdapter & operator=(const TimeAdapter &);
    
    /**
     * @brief Report the current time to the Exec as an asynchronous lookup value.
     */
    void timerTimeout();

#ifdef PLEXIL_WITH_THREADS
    /**
     * @brief Static member function which waits for timer wakeups.
     * @param this_as_void_ptr Pointer to the TimeAdapter instance, as a void *.
     */
    static void* timerWaitThread(void* this_as_void_ptr);

    /**
     * @brief Internal function for the above.
     */
    virtual void* timerWaitThreadImpl();

    //
    // Member variables
    //

    // Wait thread
    pthread_t m_waitThread;
#endif
    // Flag to wait thread
    bool m_stopping;

  };

} // namespace PLEXIL

#endif // TIME_ADAPTER_H
