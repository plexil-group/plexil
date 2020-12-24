// Copyright (c) 2006-2020, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//    // Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//    // Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    // Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_TIME_ADAPTER_IMPL_HH
#define PLEXIL_TIME_ADAPTER_IMPL_HH

#include "plexil-config.h"

#include "InterfaceAdapter.hh"

#if defined(HAVE_CSIGNAL)
#include <csignal> // sigset_t
#elif defined(HAVE_SIGNAL_H)
#include <signal.h> // sigset_t
#endif

#ifdef PLEXIL_WITH_THREADS
#if defined(HAVE_PTHREAD_H)
#include <pthread.h>
#endif
#endif

namespace PLEXIL
{

  class TimeAdapterImpl : public InterfaceAdapter
  {
  public:
    TimeAdapterImpl();
    TimeAdapterImpl(pugi::xml_node const);

    virtual ~TimeAdapterImpl();

    //
    // Public InterfaceAdapter API
    //

    //
    // API to ExecApplication
    //

    /**
     * @brief Initializes the adapter, possibly using its configuration data.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize(AdapterConfiguration *config) override;

  protected:

    //
    // Internal functions to be implemented by derived classes
    //

    /**
     * @brief Get the current time from the operating system.
     * @return A double representing the current time.
     * @note Default method uses clock_gettime() or gettimeofday() as available;
     *       specializations may override this method.
     */
    virtual Date getCurrentTime();

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
     * @param date The Unix-epoch wakeup time, as a Date.
     * @return True if the timer was set, false if clock time had already passed the wakeup time.
     */
    virtual bool setTimer(Date date) = 0;

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

    // Not implemented
    TimeAdapterImpl();
    TimeAdapterImpl(TimeAdapterImpl const &);
    TimeAdapterImpl &operator=(TimeAdapterImpl const &);
    
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

    // Next scheduled wakeup
    double m_nextWakeup;

    // Flag to wait thread
    bool m_stopping;

  }; // class TimeAdapterImpl

} // namespace PLEXIL


#endif // PLEXIL_TIME_ADAPTER_IMPL_HH
