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

#ifndef DARWIN_TIME_ADAPTER_H
#define DARWIN_TIME_ADAPTER_H

#include "InterfaceAdapter.hh"
#include <csignal>
// This platform DOES NOT support the POSIX Advanced Realtime Option
#include <sys/time.h>
#include <map>

// forward reference w/o namespace

namespace PLEXIL
{

  /**
   * @brief An interface adapter using Darwin native time facilities
   *        to implement LookupNow and LookupOnChange.
   */
  class DarwinTimeAdapter : public InterfaceAdapter
  {
  public:
    /**
     * @brief Constructor.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     */
    DarwinTimeAdapter(AdapterExecInterface& execInterface);

    /**
     * @brief Constructor from configuration XML.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     * @param xml A const pointer to the TiXmlElement describing this adapter
     * @note The instance maintains a shared pointer to the TiXmlElement.
     */
    DarwinTimeAdapter(AdapterExecInterface& execInterface, 
                     const TiXmlElement * xml);

    /**
     * @brief Destructor.
     */
    virtual ~DarwinTimeAdapter();

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
     * @brief Register one LookupOnChange.
     * @param uniqueId The unique ID of this lookup.
     * @param stateKey The state key for this lookup.
     * @param tolerances A vector of tolerances for the LookupOnChange.
     */

    void registerChangeLookup(const LookupKey& uniqueId,
                              const StateKey& stateKey,
                              const std::vector<double>& tolerances);

    /**
     * @brief Terminate one LookupOnChange.
     * @param uniqueId The unique ID of the lookup to be terminated.
     */

    void unregisterChangeLookup(const LookupKey& uniqueId);

    /**
     * @brief Perform an immediate lookup of the requested state.
     * @param stateKey The state key for this lookup.
     * @param dest A (reference to a) vector of doubles where the result is to be stored.
     */

    void lookupNow(const StateKey& stateKey,
                   std::vector<double>& dest);

    //
    // Static member functions
    //

    /**
     * @brief Get the current time from the operating system.
     * @return A double representing the current time.
     */
    static double getCurrentTime();

    /**
     * @brief Convert a timeval value into a double.
     * @param ts Reference to a constant timeval instance.
     * @return The timeval value converted to a double float.
     */
    static double timevalToDouble(const timeval& ts);

    /**
     * @brief Convert a double value into a timeval.
     * @param tym The double to be converted.
     * @param result Reference to a writable timeval instance.
     */
    static void doubleToTimeval(double tym, timeval& result);

    /**
     * @brief Compare two timevals, return true iff they are equal.
     */
    inline static bool timevalEqual(const timeval& tv1, const timeval& tv2)
    {
      return tv1.tv_sec == tv2.tv_sec && tv1.tv_usec == tv2.tv_usec;
    }

    /**
     * @brief Compare two timevals, return true iff the first is greater.
     */
    inline static bool timevalGreater(const timeval& tv1, const timeval& tv2)
    {
      return tv1.tv_sec > tv2.tv_sec
	|| (tv1.tv_sec == tv2.tv_sec && tv1.tv_usec > tv2.tv_usec);
    }

    /**
     * @brief Compare two timevals, return true iff the first is greater.
     */
    inline static bool timevalLess(const timeval& tv1, const timeval& tv2)
    {
      return tv1.tv_sec < tv2.tv_sec
	|| (tv1.tv_sec == tv2.tv_sec && tv1.tv_usec < tv2.tv_usec);
    }

  private:

    // Deliberately unimplemented
    DarwinTimeAdapter();
    DarwinTimeAdapter(const DarwinTimeAdapter &);
    DarwinTimeAdapter & operator=(const DarwinTimeAdapter &);

    //
    // Internal member functions
    //

    /**
     * @brief Helper for constructor methods.
     */
    void commonInit();

    /**
     * @brief Stop the timer.
     */
    void stopTimer();

    /**
     * @brief Static member function which waits for timer wakeups.
	 * @param this_as_void_ptr Pointer to the DarwinTimeAdapter instance, as a void *.
     */
    static void* timerWaitThread(void* this_as_void_ptr);

    /**
     * @brief Report the current time to the Exec as an asynchronous lookup value.
     */
    void timerTimeout();

    //
    // Member variables
    //

    typedef std::map<LookupKey, timeval> LookupToleranceMap;
    LookupToleranceMap m_lookupToleranceMap;

	// Wait thread
	pthread_t m_waitThread;

    // Storage for system call parameters
    itimerval m_disableItimerval;
    itimerval m_lastItimerval;

	// Storage for current time in timerTimeout()
	std::vector<double> m_timeVector;
  };

}

#endif // DARWIN_TIME_ADAPTER_H
