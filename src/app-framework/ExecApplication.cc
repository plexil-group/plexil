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
// *** TO DO ***
//  - Integrate all thread functionality here
//

#include "ExecApplication.hh"

#include "Debug.hh"
#include "Error.hh"
#include "ExecListener.hh"
#include "Expressions.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceSchema.hh"
#include "StateManagerInit.hh"
#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

namespace PLEXIL
{
  ExecApplication::ExecApplication()
    : m_id(this),
      m_exec(),
      m_interface(*this),
      m_parser(),
      m_execThread(),
      m_execMutex(),
      m_sem(),
      m_state(APP_UNINITED),
      m_runExecInBkgndOnly(true),
      m_suspended(false)
  {
    // connect exec and interface manager
    m_exec.setExternalInterface(m_interface.getId());
    m_interface.setExec(m_exec.getId());

    // Tell tinyxml to respect whitespace
    TiXmlBase::SetCondenseWhiteSpace(false);
  }

  ExecApplication::~ExecApplication()
  {
  }

  /**
   * @brief Initialize all internal data structures and interfaces.
   * @param configXml Configuration data to use.
   * @return true if successful, false otherwise.
   * @note The caller must ensure that all adapter and listener factories
   *       have been created and registered before this call.
   */
  bool ExecApplication::initialize(const TiXmlElement * configXml)
  {
    condDebugMsg(configXml == NULL, "ExecApplication:initialize", " configuration is NULL");
    condDebugMsg(configXml != NULL, "ExecApplication:initialize", " configuration = " << *configXml);

    if (m_state != APP_UNINITED)
      return false;

    // Perform one-time initializations

    // Load debug configuration from XML
    // *** NYI ***

    // Initialize Exec static data structures
    initializeExpressions();
    initializeStateManagers();

    // Construct interfaces
    if (!m_interface.constructInterfaces(configXml))
	  return false;

    // Initialize them
    if (!m_interface.initialize())
	  return false;

	// Set the application state and return
	return setApplicationState(APP_INITED);
  }

  /**
   * @brief Start all the interfaces prior to execution.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::startInterfaces()
  {
    if (m_state != APP_INITED)
      return false;

    // Start 'em up!

    if (!m_interface.start())
      {
        debugMsg("ExecApplication:startInterfaces",
                 " failed to start interfaces");
        return false;
      }
      
    return setApplicationState(APP_INTERFACES_STARTED);
  }

  /**
   * @brief Runs the initialized Exec.
   * @return true if successful, false otherwise.
   */

  bool ExecApplication::run()
  {
    if (m_state != APP_INTERFACES_STARTED)
      return false;

    // Clear suspended flag just in case
    m_suspended = false;

    // Start the event listener thread
    return spawnExecThread();
  }

  /**
   * @brief Suspends the running Exec.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::suspend()
  {
    if (m_state != APP_RUNNING)
      return false;

    // Suspend the Exec 
    m_suspended = true;
    
    // *** NYI: wait here til current step completes ***
    return setApplicationState(APP_SUSPENDED);
  }

  /**
   * @brief Resumes a suspended Exec.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::resume()
  {
    if (m_state != APP_SUSPENDED)
      return false;

    // Resume the Exec
    m_suspended = false;
    notifyExec();
    
    return setApplicationState(APP_RUNNING);
  }

  /**
   * @brief Stops the Exec.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::stop()
  {
    if (m_state != APP_RUNNING
        && m_state != APP_SUSPENDED)
      return false;

    // Stop interfaces
    m_interface.stop();

    // Stop the Exec
    debugMsg("ExecApplication:stop", " Halting top level thread");

    // Need to call PTHREAD_CANCEL_ASYNCHRONOUS since the wait loop 
    // does not quit on EINTR.
    int dummy;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &dummy);

    int status = pthread_cancel(m_execThread);
    if (status != 0)
      {
        debugMsg("ExecApplication:stop",
                 " pthread_cancel() failed, error = " << status);
        return false;
      }
    sleep(1);
    status = pthread_join(m_execThread, NULL);
    if (status != 0)
      {
        debugMsg("ExecApplication:stop", 
                 " pthread_join() failed, error = " << status);
        return false;
      }
    debugMsg("ExecApplication:stop", " Top level thread halted");
    
    return setApplicationState(APP_STOPPED);
  }

   
  /**
   * @brief Resets a stopped Exec so that it can be run again.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::reset()
  {
	debugMsg("ExecApplication:reset", " entered");
    if (m_state != APP_STOPPED)
      return false;

    // Reset interfaces
    m_interface.reset();

    // Clear suspended flag
    m_suspended = false;

    // Reset the Exec
    // *** NYI ***
    
	debugMsg("ExecApplication:reset", " completed");
    return setApplicationState(APP_INITED);
  }


  /**
   * @brief Shuts down a stopped Exec.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::shutdown()
  {
	debugMsg("ExecApplication:shutdown", " entered");
    if (m_state != APP_STOPPED)
      return false;

    // Shut down the Exec
    // *** NYI ***

    // Shut down interfaces
    m_interface.shutdown();
    
	debugMsg("ExecApplication:shutdown", " completed");
    return setApplicationState(APP_SHUTDOWN);
  }

  /**
   * @brief Add a library as an XML document.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::addLibrary(TiXmlDocument * libraryXml)
  {
    if (m_state != APP_RUNNING)
      return false;

    // grab the library itself from the document
    TiXmlElement* plexilXml = libraryXml->FirstChildElement("PlexilPlan");
    if (plexilXml == NULL) {
	  std::cerr << "Error parsing library from XML: No \"PlexilPlan\" tag found" << std::endl;
	  return false;
	}

    // parse XML into node structure
    PlexilNodeId root;
    try {
      root = m_parser.parse(plexilXml->FirstChildElement("Node"));
    }
    catch (const ParserException& e)
      {
        std::cerr << "Error parsing library from XML: \n" << e.what() << std::endl;
        return false;
      }

    m_interface.handleAddLibrary(root);
    debugMsg("ExecApplication:addLibrary", " Library added, stepping exec");
    m_interface.notifyOfExternalEvent();
    return true;
  }

  /**
   * @brief Add a plan as an XML document.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::addPlan(TiXmlDocument * planXml)
  {
    if (m_state != APP_RUNNING)
      return false;

    // grab the plan itself from the document
    TiXmlElement* plexilXml = planXml->FirstChildElement("PlexilPlan");
    if (plexilXml == NULL) {
	  std::cerr << "Error parsing plan from XML: No \"PlexilPlan\" tag found" << std::endl;
	  return false;
	}

    // parse XML into node structure
    PlexilNodeId root;
    try {
      root = m_parser.parse(plexilXml->FirstChildElement("Node"));
    }
    catch (const ParserException& e)
      {
        std::cerr << "Error parsing plan from XML: \n" << e.what() << std::endl;
        return false;
      }

    m_interface.handleAddPlan(root, EMPTY_LABEL());
    debugMsg("ExecApplication:addPlan", " Plan added, stepping exec\n");
    m_interface.notifyOfExternalEvent();
    return true;
  }

  /**
   * @brief Spawns a thread which runs the exec's top level loop.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::spawnExecThread()
  {
    debugMsg("ExecApplication:run", " Spawning top level thread");
    int success = pthread_create(&m_execThread,
                                 NULL,
                                 execTopLevel,
                                 this);
    if (success != 0) {
	  std::cerr << "Error: unable to spawn exec thread" << std::endl;
	  return false;
	}
    debugMsg("ExecApplication:run", " Top level thread running");
    return setApplicationState(APP_RUNNING);
  }

  void * ExecApplication::execTopLevel(void * this_as_void_ptr)
  {
    ( (ExecApplication*) this_as_void_ptr)->runInternal();
    return 0;
  }

  void ExecApplication::runInternal()
  {
    debugMsg("ExecApplication:runInternal", " (" << pthread_self() << ") Thread started");

    // must step exec once to initialize time
    runExec(true);
    debugMsg("ExecApplication:runInternal", " (" << pthread_self() << ") Initial step complete");

    while (waitForExternalEvent())
      {
        runExec(false);
      }
    debugMsg("ExecApplication:runInternal", " (" << pthread_self() << ") Ending the thread loop.");
  }

  /**
   * @brief Run the exec until the queue is empty.
   * @param stepFirst True if the exec should be stepped before checking the queue.
   * @note Acquires m_execMutex and holds until done.  
   */

  void
  ExecApplication::runExec(bool stepFirst)
  {
    RTMutexGuard guard(m_execMutex);
    if (stepFirst)
      {
        debugMsg("ExecApplication:runExec", " (" << pthread_self() << ") Stepping exec");
        m_exec.step();
        debugMsg("ExecApplication:runExec", " (" << pthread_self() << ") Step complete");
      }
    while (!m_suspended && m_interface.processQueue())
      {
        debugMsg("ExecApplication:runExec", " (" << pthread_self() << ") Stepping exec");
        m_exec.step();
        debugMsg("ExecApplication:runExec", " (" << pthread_self() << ") Step complete");
        // give an opportunity to cancel thread here
        pthread_testcancel();
      }
    condDebugMsg(!m_suspended, "ExecApplication:runExec", " (" << pthread_self() << ") No events are pending");
    condDebugMsg(m_suspended, "ExecApplication:runExec", " (" << pthread_self() << ") Suspended");
  }

  /**
   * @brief Suspends the calling thread until another thread has
   *         placed a call to notifyExec().  Can return
   *        immediately if the call to wait() returns an error.
   * @return true if resumed normally, false if wait resulted in an error.
   * @note Can wait here indefinitely while the application is suspended.
   */
  bool ExecApplication::waitForExternalEvent()
  {
    debugMsg("ExecApplication:wait", " (" << pthread_self() << ") waiting for external event");
	int status;
    do {
      status = m_sem.wait();
      if (status == 0)
        {
          condDebugMsg(!m_suspended, 
                       "ExecApplication:wait",
                       " (" << pthread_self() << ") acquired semaphore, processing external event");
          condDebugMsg(m_suspended, 
                       "ExecApplication:wait",
                       " Application is suspended, ignoring external event");
        }
	  // give an opportunity to cancel thread here
	  pthread_testcancel();
    } while (m_suspended);
    return (status == 0);
  }

  /**
   * @brief Suspend the current thread until the plan finishes executing.
   */
  void
  ExecApplication::waitForPlanFinished()
  {
    // Should never happen, but just in case...
    assertTrueMsg(!m_execMutex.isLockedByCurrentThread(),
                  "Internal error: waitForPlanFinished: called with Exec mutex locked!");
    bool finished = false;
    while (!finished)
      {
        // sleep for a bit so as not to hog the CPU
        sleep(1);
    
        // grab the exec and find out if it's finished yet
        RTMutexGuard guard(m_execMutex);
        finished = m_exec.allPlansFinished();
      }
  }

  /**
   * @brief Suspend the current thread until the application reaches APP_SHUTDOWN state.
   * @note May be called by multiple threads
   */
  void
  ExecApplication::waitForShutdown() {
    m_shutdownSem.wait();
    m_shutdownSem.post(); // pass it on to the next, if any
  }

  /**
   * @brief Whatever state the application may be in, bring it down in a controlled fashion.
   */
  void ExecApplication::terminate() {
	std::cout << "Terminating PLEXIL Exec application" << std::endl;

	ApplicationState initState = getApplicationState();
	debugMsg("ExecApplication:terminate", " from state " << getApplicationStateName(initState));

	switch (initState) {
	case APP_UNINITED:
	case APP_SHUTDOWN:
	  // nothing to do
	  break;

	case APP_INITED:
	case APP_INTERFACES_STARTED:
	  // Shut down interfaces
	  m_interface.shutdown();
	  break;

	case APP_RUNNING:
	  stop();
	  // fall through to shutdown

	case APP_SUSPENDED:
	case APP_STOPPED:
	  shutdown();
	  break;
	}
	std::cout << "PLEXIL Exec terminated" << std::endl;
  }

  /**
   * @brief Get the application's current state.
   */
  ExecApplication::ApplicationState 
  ExecApplication::getApplicationState() {
    ThreadMutexGuard guard(m_stateMutex);
    return m_state;
  }

  /**
   * @brief Transitions the application to the new state.
   * @return true if the new state is a legal transition from the current state, false if not.
   */ 
  bool 
  ExecApplication::setApplicationState(const ExecApplication::ApplicationState& newState) {
	debugMsg("ExecApplication:setApplicationState",
			 "(" << getApplicationStateName(newState)
			 << ") from " << getApplicationStateName(m_state));

    assertTrueMsg(newState != APP_UNINITED,
                  "APP_UNINITED is an invalid state for setApplicationState");

    // variable binding context for guard -- DO NOT DELETE THESE BRACES!
    {
      ThreadMutexGuard guard(m_stateMutex);
      switch (newState) {
      case APP_INITED:
        if (m_state != APP_UNINITED && m_state != APP_STOPPED) {
		  debugMsg("ExecApplication:setApplicationState",
				   " Illegal application state transition to APP_INITED");
		  return false;
		}
        m_state = newState;
        break;

      case APP_INTERFACES_STARTED:
        if (m_state != APP_INITED) {
		  debugMsg("ExecApplication:setApplicationState",
				   " Illegal application state transition to APP_INTERFACES_STARTED");
		  return false;
		}
        m_state = newState;
        break;

      case APP_RUNNING:
        if (m_state != APP_INTERFACES_STARTED && m_state != APP_SUSPENDED) {
		  debugMsg("ExecApplication::setApplicationState", 
				   " Illegal application state transition to APP_RUNNING");
		  return false;
		}
        m_state = newState;
        break;

      case APP_SUSPENDED:
        if (m_state != APP_RUNNING) {
		  debugMsg("ExecApplication::setApplicationState", 
				   " Illegal application state transition to APP_SUSPENDED");
		  return false;
		}
        m_state = newState;
        break;

      case APP_STOPPED:
        if (m_state != APP_RUNNING && m_state != APP_SUSPENDED) {
		  debugMsg("ExecApplication::setApplicationState", 
				   " Illegal application state transition to APP_STOPPED");
		  return false;
		}
        m_state = newState;
        break;

	  case APP_SHUTDOWN:
		if (m_state != APP_STOPPED) {
		  debugMsg("ExecApplication::setApplicationState", 
				   " Illegal application state transition to APP_SHUTDOWN");
		  return false;
		}
        m_state = newState;
        break;

      }
      // end variable binding context for guard -- DO NOT DELETE THESE BRACES!
    }

    if (newState == APP_SHUTDOWN) {
      // Notify any threads waiting for this state
      m_shutdownSem.post();
    }

	debugMsg("ExecApplication:setApplicationState",
			 " to " << getApplicationStateName(newState) << " successful");
	return true;
  }

  //
  // Static helper methods
  //

  /**
   * @brief Return a human-readable name for the ApplicationState.
   * @param state An ApplicationState.
   * @return The name of the state as a const char*.
   */

  const char* 
  ExecApplication::getApplicationStateName(ApplicationState state) {
	switch (state) {
	case APP_UNINITED:
	  return "APP_UNINITED";
	  break;

	case APP_INITED:
	  return "APP_INITED";
	  break;

	case APP_INTERFACES_STARTED:
	  return "APP_INTERFACES_STARTED";
	  break;

	case APP_RUNNING:
	  return "APP_RUNNING";
	  break;

	case APP_SUSPENDED:
	  return "APP_SUSPENDED";
	  break;

	case APP_STOPPED:
	  return "APP_STOPPED";
	  break;

	case APP_SHUTDOWN:
	  return "APP_SHUTDOWN";
	  break;

	default:
	  return "*** ILLEGAL APPLICATION STATE ***";
	  break;
	}
	// just in case
	return "*** ILLEGAL APPLICATION STATE ***";
  }

  /**
   * @brief Notify the executive that it should run one cycle.  
   This should be sent after each batch of lookup, command
   return, and function return data.
  */
  void
  ExecApplication::notifyExec()
  {
    if (m_runExecInBkgndOnly || m_execMutex.isLocked())
      {
        // Some thread currently owns the exec. Could be this thread.
        // runExec() could notice, or not.
        // Post to semaphore to ensure event is not lost.
        int status = m_sem.post();
        assertTrueMsg(status == 0,
					  "notifyExec: semaphore post failed, status = "
					  << status);
		debugMsg("ExecApplication:notify",
				 " (" << pthread_self() << ") released semaphore");
      }
    else
      {
		// Exec is idle, so run it
		// If another thread grabs it first, no worries.
		debugMsg("ExecApplication:notify",
				 " (" << pthread_self() << ") exec was idle, stepping it");
		this->runExec();
      }
  }

}
