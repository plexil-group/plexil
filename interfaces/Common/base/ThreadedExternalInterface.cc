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
//  - implement tracking of active commands (?)
//  - implement un/registerCommandReturnValue()
//  - utilities for adapters?
//

#include "ThreadedExternalInterface.hh"

#include "AdaptorFactory.hh"
#include "Debug.hh"
#include "Error.hh"
#include "InterfaceAdaptor.hh"
#include "InterfaceSchema.hh"
#include "ResourceArbiterInterface.hh"
#include "Node.hh"
#include "PlexilExec.hh"
#include "PlexilXmlParser.hh"
#include "StateCache.hh"
#include "CommandHandle.hh"

#include <sstream>

namespace PLEXIL
{

  #define ONE_SECOND 1

  // Initialize static singleton pointer
  ThreadedExternalInterfaceId ThreadedExternalInterface::s_threadedInterfaceInstance =
  ThreadedExternalInterfaceId::noId();

  /**
   * @brief Default constructor.
   */
  ThreadedExternalInterface::ThreadedExternalInterface()
    : ExternalInterface(),
      AdaptorExecInterface(),
      m_runExecInBkgndOnly(false),
      m_threadedInterfaceId(),
      m_execThread(),
      m_execMutex(),
      m_sem(),
      m_state(INTERFACE_UNINITED)
  {
    // cast from subclass
    m_threadedInterfaceId = (ThreadedExternalInterfaceId)m_adaptorInterfaceId;
  }

  /**
   * @brief Destructor.
   */
  ThreadedExternalInterface::~ThreadedExternalInterface()
  {
    // clear singleton pointer
    if (s_threadedInterfaceInstance == m_threadedInterfaceId)
      {
	s_threadedInterfaceInstance == ThreadedExternalInterfaceId::noId();
      }
  }

  /**
   * @brief Accessor to singleton.
   */
  ThreadedExternalInterfaceId ThreadedExternalInterface::instance()
  {
    if (s_threadedInterfaceInstance.isNoId())
      {
	s_threadedInterfaceInstance = (new ThreadedExternalInterface())->getId();
      }
    return s_threadedInterfaceInstance;
  }

  /**
   * @brief Select whether the exec runs opportunistically or only in background thread.
   * @param bkgndOnly True if background only, false if opportunistic.
   * @note Default is opportunistic.
   */
  void ThreadedExternalInterface::setRunExecInBkgndOnly(bool bkgndOnly)
  {
    m_runExecInBkgndOnly = bkgndOnly;
  }

  //
  // Top-level loop
  //

  /**
   * @brief Register this adaptor based on its XML configuration data.
   * @note The adaptor is presumed to be fully initialized and working at the time of this call.
   * @note TODO: strategy for handling redundant registrations
   */
  void ThreadedExternalInterface::defaultRegisterAdaptor(InterfaceAdaptorId adaptor)
  {
    // Walk the children of the configuration XML element
    // and register the adaptor according to the data found there
    TiXmlElement* element = adaptor->getXml()->FirstChildElement();
    while (element != 0)
      {
        const char* elementType = element->Value();
        // look for text as the only child of this element
        // to use below
        TiXmlNode* firstChild = element->FirstChild();
        TiXmlText* text = 0;
        if (firstChild == 0)
          text = firstChild->ToText();

        if (strcmp(elementType, InterfaceSchema::DEFAULT_ADAPTOR_TAG()) == 0)
          {
            setDefaultInterface(adaptor);
            // warn if children found
            if (text != 0)
              {
                warn("registerInterface: extraneous text in "
                     << InterfaceSchema::DEFAULT_ADAPTOR_TAG()
                     << " ignored");
              }
            else if (firstChild != 0)
              {
                warn("registerInterface: extraneous XML element(s) in "
                     << InterfaceSchema::DEFAULT_ADAPTOR_TAG()
                     << " ignored");
              }
          }
        else if (strcmp(elementType, InterfaceSchema::PLANNER_UPDATE_TAG()) == 0)
          {
            registerPlannerUpdateInterface(adaptor);
            // warn if children found
            if (text != 0)
              {
                warn("registerInterface: extraneous text in "
                     << InterfaceSchema::PLANNER_UPDATE_TAG()
                     << " ignored");
              }
            else if (firstChild != 0)
              {
                warn("registerInterface: extraneous XML element(s) in "
                     << InterfaceSchema::PLANNER_UPDATE_TAG()
                     << " ignored");
              }
          }
        else if (strcmp(elementType, InterfaceSchema::COMMAND_NAMES_TAG()) == 0)
          {
            checkError(text != 0,
                       "registerAdaptor: Invalid configuration XML: "
                       << InterfaceSchema::COMMAND_NAMES_TAG()
                       << " requires one or more comma-separated command names");
            std::vector<std::string> * cmdNames =
              InterfaceSchema::parseCommaSeparatedArgs(text->Value());
            for(std::vector<std::string>::const_iterator it = cmdNames->begin();
                it != cmdNames->end();
                it++)
              {
                registerCommandInterface(LabelStr(*it), adaptor);
              }
            delete cmdNames;
          }
        else if (strcmp(elementType, InterfaceSchema::FUNCTION_NAMES_TAG()) == 0)
          {
            checkError(text != 0,
                       "registerAdaptor: Invalid configuration XML: "
                       << InterfaceSchema::FUNCTION_NAMES_TAG()
                       << " requires one or more comma-separated function names");
            std::vector<std::string> * fnNames =
              InterfaceSchema::parseCommaSeparatedArgs(text->Value());
            for(std::vector<std::string>::const_iterator it = fnNames->begin();
                it != fnNames->end();
                it++)
              {
                registerFunctionInterface(LabelStr(*it), adaptor);
              }
            delete fnNames;
          }
        else if (strcmp(elementType, InterfaceSchema::LOOKUP_NAMES_TAG()) == 0)
          {
            checkError(text != 0,
                       "registerAdaptor: Invalid configuration XML: "
                       << InterfaceSchema::LOOKUP_NAMES_TAG()
                       << " requires one or more comma-separated lookup names");
            std::vector<std::string> * lookupNames =
              InterfaceSchema::parseCommaSeparatedArgs(text->Value());
            for(std::vector<std::string>::const_iterator it = lookupNames->begin();
                it != lookupNames->end();
                it++)
              {
                registerLookupInterface(LabelStr(*it), adaptor);
              }
            delete lookupNames;
          }
        // ignore other tags, they're for adaptor's use
        
        element = element->NextSiblingElement();
      }
  }

  /**
   * @brief Performs basic initialization of the interface and all adaptors.
   * @return true if successful, false otherwise.
   */
  bool ThreadedExternalInterface::initialize()
  {
    if (m_state != INTERFACE_UNINITED)
      {
        debugMsg("ExternalInterface:initialize", " called from invalid interface state");
        return false;
      }

    debugMsg("ExternalInterface:initialize", " initializing interface adaptors");
    bool success = true;
    for (std::set<InterfaceAdaptorId>::iterator it = m_adaptors.begin();
         success && it != m_adaptors.end();
         it++)
      success = (*it)->initialize();

    if (success)
      {
        debugMsg("ExternalInterface:initialize", " done.");
        m_state = INTERFACE_INITED;
      }
    else
      {
        debugMsg("ExternalInterface:initialize", " failed.");
      }
    return success;
  }

  /**
   * @brief Prepares the interface and adaptors for execution.
   * @return true if successful, false otherwise.
   */
  bool ThreadedExternalInterface::startInterface()
  {
    if (m_state != INTERFACE_INITED)
      {
        debugMsg("ExternalInterface:startInterface", " called from invalid interface state");
        return false;
      }

    debugMsg("ExternalInterface:startInterface", " starting interface adaptors");
    bool success = true;
    for (std::set<InterfaceAdaptorId>::iterator it = m_adaptors.begin();
         success && it != m_adaptors.end();
         it++)
      success = (*it)->start();
    if (success)
      {
        debugMsg("ExternalInterface:startInterface", " done.");
        m_state = INTERFACE_STARTED;
      }
    else
      {
        debugMsg("ExternalInterface:startInterface", " failed.");
      }
    return success;
  }

  /**
   * @brief Spawns a thread which runs the exec's top level loop, and waits for the thread to terminate.
   * @return true if successful, false otherwise.
   */
  bool ThreadedExternalInterface::run()
  {
    if (m_state != INTERFACE_STARTED)
      {
        debugMsg("ExternalInterface:run", " interfaces not started, cannot run exec");
        return false;
      }

    bool result = spawnExecThread();
    if (!result)
      return result;

    // Wait for exec thread here
    result = pthread_join(m_execThread, NULL);
    return result;
  }

  /**
   * @brief Spawns a thread which runs the exec's top level loop.
   * @return true if successful, false otherwise.
   */
  bool ThreadedExternalInterface::spawnExecThread()
  {
    checkError(m_exec.isValid(), "Attempted to run without an executive.");
    assertTrue(m_state != INTERFACE_STARTED,
               "ExternalInterface::run: interfaces not started");
    debugMsg("ExternalInterface:run", " Spawning top level thread");
    int success = pthread_create(&m_execThread,
				 NULL,
				 execTopLevel,
				 this);
    assertTrue(success == 0,
	       "ExternalInterface::run: unable to spawn exec thread!");
    debugMsg("ExternalInterface:run", " Top level thread running");
    m_state = INTERFACE_RUNNING;
    return success == 0;
  }


  /**
   * @brief Temporarily halts the exec's top level loop.
   * @return true if successful, false otherwise.
   */
  bool ThreadedExternalInterface::suspend()
  {
    if (m_state != INTERFACE_RUNNING)
      {
        debugMsg("ExternalInterface:suspend", " exec not running, can't suspend");
        return false;
      }

    // suspend exec thread
    // *** NYI ***


    debugMsg("ExternalInterface:suspend", " suspending interface adaptors");
    bool success = true;
    for (std::set<InterfaceAdaptorId>::iterator it = m_adaptors.begin();
         success && it != m_adaptors.end();
         it++)
      success == (*it)->suspend();
    if (success)
      {
        m_state = INTERFACE_SUSPENDED;
        debugMsg("ExternalInterface:suspend", " done.");
      }
    else
      {
        debugMsg("ExternalInterface:suspend", " failed.");
      }
    return success;
  }

  /**
   * @brief Resumes the exec's top level loop.
   * @return true if successful, false otherwise.
   */
  bool ThreadedExternalInterface::resume()
  {
    if (m_state != INTERFACE_SUSPENDED)
      {
        debugMsg("ExternalInterface:suspend", " exec not suspended, can't resume");
        return false;
      }

    debugMsg("ExternalInterface:resume", " resuming interface adaptors");
    bool success = true;
    for (std::set<InterfaceAdaptorId>::iterator it = m_adaptors.begin();
         success && it != m_adaptors.end();
         it++)
      success = (*it)->resume();

    if (!success)
      {
        debugMsg("ExternalInterface:resume", " failed resuming interface adaptors");
        return false;
      }

    // resume exec thread
    // *** NYI ***

    if (success)
      {
        debugMsg("ExternalInterface:resume", " done.");
        m_state = INTERFACE_RUNNING;
      }
    else
      {
        debugMsg("ExternalInterface:resume", " failed resuming exec thread");
      }
    return success;
  }

  /**
   * @brief Tells the exec's top level loop to exit and halts all interfaces.
   * @return true if successful, false otherwise.
   */
  bool ThreadedExternalInterface::stop()
  {
    if (m_state != INTERFACE_SUSPENDED)
      {
        debugMsg("ExternalInterface:stop", " exec not running or suspended, can't stop");
        return false;
      }

    debugMsg("ExternalInterface:stop", " Halting top level thread");
    int dummy;

    // Need to call PTHREAD_CANCEL_ASYNCHRONOUS since the wait loop 
    // does not quit on EINTR.
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &dummy);

    int status = pthread_cancel(m_execThread);
    if (status != 0)
      {
        debugMsg("ExternalInterface:stop",
                 " pthread_cancel() failed, error = " << status);
        return false;
      }
    sleep(1);
    status = pthread_join(m_execThread, NULL);
    if (status != 0)
      {
        debugMsg("ExternalInterface:stop", 
                 " pthread_join() failed, error = " << status);
        return false;
      }
    debugMsg("ExternalInterface:stop", " Top level thread halted");

    // halt adaptors
    bool success = true;
    for (std::set<InterfaceAdaptorId>::iterator it = m_adaptors.begin();
         success && it != m_adaptors.end();
         it++)
      success = (*it)->stop();

    if (success)
      {
        debugMsg("ExternalInterface:stop", " Interface halted");
        m_state = INTERFACE_STOPPED;
      }
    else
      {
        debugMsg("ExternalInterface:stop", " Unable to stop interface");
      }
    return success;
  }

  /**
   * @brief Resets the interface prior to restarting.
   * @return true if successful, false otherwise.
   */
  bool ThreadedExternalInterface::reset()
  {
    if (m_state != INTERFACE_STOPPED)
      {
        debugMsg("ExternalInterface:reset", " exec not stopped, can't reset");
        return false;
      }

    // reset queue etc. to freshly initialized state
    // *** NYI ***

    bool success = true;
    for (std::set<InterfaceAdaptorId>::iterator it = m_adaptors.begin();
         success && it != m_adaptors.end();
         it++)
      success = (*it)->reset();

    if (success)
      {
        debugMsg("ExternalInterface:reset", " succeeded");
        m_state = INTERFACE_INITED;
      }
    else
      {
        debugMsg("ExternalInterface:reset", " failed");
      }
    return success;
  }

  /**
   * @brief Shuts down the interface.
   * @return true if successful, false otherwise.
   */
  bool ThreadedExternalInterface::shutdown()
  {
    if (m_state != INTERFACE_STOPPED)
      {
        debugMsg("ExternalInterface:shutdown", " exec not stopped, can't shut down");
        return false;
      }

    bool success = true;
    for (std::set<InterfaceAdaptorId>::iterator it = m_adaptors.begin();
         success && it != m_adaptors.end();
         it++)
      success = (*it)->shutdown();

    // Clean up
    // *** NYI ***

    if (success)
      {
        debugMsg("ExternalInterface:shutdown", " done.");
        m_state = INTERFACE_SHUTDOWN;
      }
    else
      {
        debugMsg("ExternalInterface:shutdown", " done.");
      }

    return success;
  }

  void * ThreadedExternalInterface::execTopLevel(void * this_as_void_ptr)
  {
    ( (ThreadedExternalInterface*) this_as_void_ptr)->runInternal();
    return 0;
  }

  void ThreadedExternalInterface::runInternal()
  {
    debugMsg("ExternalInterface:runInternal", " (" << pthread_self() << ") Thread started");

    // must step exec once to initialize time
    this->runExec(true);
    debugMsg("ExternalInterface:runInternal", " (" << pthread_self() << ") Initial step complete");

    while (this->waitForExternalEvent())
      {
	this->runExec(false);
      }
    debugMsg("ExternalInterface:runInternal", " (" << pthread_self() << ") Ending the thread loop.");
  }

  //
  // API for exec
  //

  /**
   * @brief Run the exec until the queue is empty.
   * @param stepFirst True if the exec should be stepped before checking the queue.
   * @note Acquires m_execMutex and holds until done.  
   * @note This should be the only method that acquires m_execMutex.
   */

  void
  ThreadedExternalInterface::runExec(bool stepFirst)
  {
    RTMutexGuard guard(m_execMutex);
    if (stepFirst)
      {
	debugMsg("ExternalInterface:runExec", " (" << pthread_self() << ") Stepping exec");
	m_exec->step();
	debugMsg("ExternalInterface:runExec", " (" << pthread_self() << ") Step complete");
      }
    while (this->processQueue())
      {
	debugMsg("ExternalInterface:runExec", " (" << pthread_self() << ") Stepping exec");
	m_exec->step();
	debugMsg("ExternalInterface:runExec", " (" << pthread_self() << ") Step complete");
        // give an opportunity to cancel thread here
        pthread_testcancel();
      }
    debugMsg("ExternalInterface:runExec", " (" << pthread_self() << ") No events are pending");
  }

  /**
   * @brief Suspends the calling thread until another thread has
   *         placed a call to notifyOfExternalEvent().  Can return
   *	    immediately if the call to wait() returns an error.
   * @return true if resumed normally, false if wait resulted in an error.
   * @note ThreadSemaphore handles case of interrupted wait (errno == EINTR).
   */
  bool ThreadedExternalInterface::waitForExternalEvent()
  {
    debugMsg("ExternalInterface:wait", " waiting for external event");
    int status = m_sem.wait();
    if (status != 0)
      {
        assertTrueMsg(ALWAYS_FAIL,
		      "waitForExternalEvent: semaphore wait failed, status = "
		      << status);
        return false;
      }
    else
      {
        debugMsg("ExternalInterface:wait", " acquired semaphore, processing external event(s)");
        return true;
      }
  }

  /**
   * @brief Delete any entries in the queue.
   */
  void ThreadedExternalInterface::resetQueue()
  {
    debugMsg("ExternalInterface:resetQueue", " entered");
    while (!m_valueQueue.isEmpty())
      {
	m_valueQueue.pop();
      }
  }
    
    
  /**
   * @brief Updates the state cache from the items in the queue.
   * @return True if the Exec needs to be stepped, false otherwise.
   * @note Should only be called with m_execMutex held by the current thread
   */
  bool ThreadedExternalInterface::processQueue()
  {
    debugMsg("ExternalInterface:processQueue",
	     " (" << pthread_self() << ") entered");

    // Potential optimization (?): these could be member variables
    // Can't use static as that would cause collisions between multiple instances
    StateKey stateKey;
    std::vector<double> newStateValues;
    ExpressionId exp;
    double newExpValue;
    PlexilNodeId plan;
    LabelStr parent;
    QueueEntryType typ;

    bool firstTime = true;

    while (true)
      {
	// get next entry
	debugMsg("ExternalInterface:processQueue",
		 " (" << pthread_self() << ") getting next entry");
	typ = m_valueQueue.dequeue(stateKey, newStateValues,
				   exp, newExpValue,
				   plan, parent);
        // opportunity to cancel thread here
        pthread_testcancel();
	switch (typ)
	  {
	  case queueEntry_EMPTY:
	    if (firstTime)
	      {
		debugMsg("ExternalInterface:processQueue",
			 " (" << pthread_self() << ") queue empty at entry, returning false");
		return false;
	      }
	    debugMsg("ExternalInterface:processQueue",
		     " (" << pthread_self() << ") reached end of queue without finding mark, returning true");
	    return true;
	    break;

	  case queueEntry_MARK:
	    // Exit loop now, whether or not queue is empty
	    debugMsg("ExternalInterface:processQueue",
		     " (" << pthread_self() << ") Received mark, returning true");
	    return true;
	    break;

	  case queueEntry_LOOKUP_VALUES:
	    // State -- update all listeners
	    {
	      // state for debugging only
	      State state;
	      bool stateFound =
		m_exec->getStateCache()->stateForKey(stateKey, state);
	      if (stateFound)
		{
		  debugMsg("ExternalInterface:processQueue",
			   " (" << pthread_self()
			   << ") Handling state change for '"
			   << getText(state)
			   << "', " << newStateValues.size()
			   << " new value(s)");

		  if (newStateValues.size() == 0)
		    {
		      debugMsg("ExternalInterface:processQueue",
			       "(" << pthread_self()
			       << ") Ignoring empty state change vector for '"
			       << getText(state)
			       << "'");
		      break;
		    }

		  // If this is a time state update message, check if it's stale
		  if (stateKey == m_exec->getStateCache()->getTimeStateKey())
		    {
		      if (newStateValues[0] <= m_currentTime)
			{
			  debugMsg("ExternalInterface:processQueue",
				   " (" << pthread_self()
				   << ") Ignoring stale time update - new value "
				   << newStateValues[0] << " is not greater than cached value "
				   << m_currentTime);
			}
		      else
			{
			  debugMsg("ExternalInterface:processQueue",
				   " (" << pthread_self()
				   << ") setting current time to "
				   << valueToString(newStateValues[0]));
			  m_currentTime = newStateValues[0];
			  m_exec->getStateCache()->updateState(stateKey, newStateValues);
			}
		    }
		  else
		    {
		      // General case, update state cache
		      m_exec->getStateCache()->updateState(stateKey, newStateValues);
		    }
		}
	      else
		{
		  // State not found -- possibly stale update
		  debugMsg("ExternalInterface:processQueue", 
			   " (" << pthread_self()
			   << ") ignoring lookup for nonexistent state, key = "
			   << stateKey);
		}
	      break;
	    }

	  case queueEntry_RETURN_VALUE:
	    // Expression -- update the expression only
	    debugMsg("ExternalInterface:processQueue",
		     " (" << pthread_self()
		     << ") Updating expression " << exp
		     << ", new value is '" << valueToString(newExpValue) << "'");
	    this->releaseResourcesAtCommandTermination(exp);
	    exp->setValue(newExpValue);
	    break;

	  case queueEntry_PLAN:
	    // Plan -- add the plan
	    debugMsg("ExternalInterface:processQueue",
		     " (" << pthread_self() << ") Received plan");
	    getExec()->addPlan(plan, parent);
	    break;

	  case queueEntry_LIBRARY:
	    // Library -- add the library

	    debugMsg("ExternalInterface:processQueue",
		     " (" << pthread_self() << ") Received library");
	    // *** TODO: check here for duplicates ***
            getExec()->addLibraryNode(plan);
	    // no need to step here
	    break;

	  default:
	    // error
	    checkError(ALWAYS_FAIL,
		       "ExternalInterface::processQueue: Invalid entry type "
		       << typ);
	    break;
	  }

	firstTime = false;

        // Allow an opportunity to quit here
        pthread_testcancel();
      }
  }

  /**
   * @brief Register a change lookup on a new state, expecting values back.
   * @param source The unique key for this lookup.
   * @param state The state
   * @param key The key for the state to be used in future communications about the state.
   * @param tolerances The tolerances for the lookup.  May be used by the FL to reduce the number of updates sent to the exec.
   * @param dest The destination for the current values for the state.
   */

  // *** N.B. dest is stack allocated, therefore pointers to it should not be stored!
  void
  ThreadedExternalInterface::registerChangeLookup(const LookupKey& source,
						  const State& state,
						  const StateKey& key,
						  const std::vector<double>& tolerances, 
						  std::vector<double>& dest)
  {
    // Do an immediate lookup for effect
    lookupNow(state, key, dest);
    // Defer to method below
    registerChangeLookup(source, key, tolerances);
  }

  /**
   * @brief Register a change lookup on an existing state.
   * @param source The unique key for this lookup.
   * @param key The key for the state.
   * @param tolerances The tolerances for the lookup.  May be used by the FL to reduce the number of updates sent to the exec.
   */

  // *** To do:
  //  - optimize for multiple lookups on same state, e.g. time?
  void
  ThreadedExternalInterface::registerChangeLookup(const LookupKey& source,
						  const StateKey& key, 
						  const std::vector<double>& tolerances)
  {
    // Extract state name and arglist
    State state;
    m_exec->getStateCache()->stateForKey(key, state);
    const LabelStr& stateName(state.first);

    InterfaceAdaptorId adaptor = getLookupInterface(stateName);
    assertTrueMsg(!adaptor.isNoId(),
		  "registerChangeLookup: No interface adaptor found for lookup '"
		  << stateName.toString() << "'");

    m_lookupAdaptorMap.insert(std::pair<LookupKey, InterfaceAdaptorId>(source, adaptor));
    // for convenience of adaptor implementors
    adaptor->registerAsynchLookup(source, key);
    adaptor->registerChangeLookup(source, key, tolerances);
  }

  /**
   * @brief Register a frequency lookup on a new state, expecting values back.
   * @param source The unique key for this lookup.
   * @param state The state
   * @param key The key for the state to be used in future communications about the state.
   * @param lowFreq The most time allowable between updates, or the exec will assume UNKNOWN.
   * @param highFreq The least time allowable between updates.
   * @param dest The destination for the current values for the state.
   */

  // *** N.B. dest is stack allocated, therefore pointers to it should not be stored!
  void
  ThreadedExternalInterface::registerFrequencyLookup(const LookupKey& source,
						     const State& state,
						     const StateKey& key,
						     const double& lowFreq,
						     const double& highFreq,
						     std::vector<double>& dest)
  {
    // Do an immediate lookup for effect
    lookupNow(state, key, dest);
    // Defer to method below
    registerFrequencyLookup(source, key, lowFreq, highFreq);
  }

  /**
   * @brief Register a frequency lookup on an existing state.
   * @param source The unique key for this lookup.
   * @param key The key for the state.
   * @param lowFreq The most time allowable between updates, or the exec will assume UNKNOWN.
   * @param highFreq The least time allowable between updates.
   */
  void 
  ThreadedExternalInterface::registerFrequencyLookup(const LookupKey& source,
						     const StateKey& key,
						     const double& lowFreq,
						     const double& highFreq)
  {
    // Extract state name and arglist
    State state;
    m_exec->getStateCache()->stateForKey(key, state);
    const LabelStr& stateName(state.first);

    InterfaceAdaptorId adaptor = getLookupInterface(stateName);
    assertTrueMsg(!adaptor.isNoId(),
		  "registerFrequencyLookup: No interface adaptor found for lookup '"
		  << stateName.toString() << "'");

    m_lookupAdaptorMap.insert(std::pair<LookupKey, InterfaceAdaptorId>(source, adaptor));
    // for convenience of adaptor implementors
    adaptor->registerAsynchLookup(source, key);
    adaptor->registerFrequencyLookup(source, key, lowFreq, highFreq);
  }

  /**
   * @brief Perform an immediate lookup on a new state.
   * @param state The state
   * @param key The key for the state to be used in future communications about the state.
   * @param dest The destination for the current values for the state.
   */

  // *** N.B. dest is stack allocated, therefore pointers to it should not be stored!
  void 
  ThreadedExternalInterface::lookupNow(const State& state,
				       const StateKey& key,
				       std::vector<double>& dest)
  {
    const LabelStr& stateName(state.first);
    debugMsg("ExternalInterface:lookupNow", " of '" << stateName.toString() << "'");
    InterfaceAdaptorId adaptor = getLookupInterface(stateName);
    assertTrueMsg(!adaptor.isNoId(),
		  "lookupNow: No interface adaptor found for lookup '"
		  << stateName.toString() << "'");

    adaptor->lookupNow(key, dest);
    // update internal idea of time if required
    if (key == m_exec->getStateCache()->getTimeStateKey())
      {
	if (dest[0] <= m_currentTime)
	  {
	    debugMsg("ExternalInterface:lookupNow",
		     " Ignoring stale time update - new value "
		     << dest[0] << " is not greater than cached value "
		     << m_currentTime);
	  }
	else
	  {
	    debugMsg("ExternalInterface:lookupNow",
		     " setting current time to "
		     << valueToString(dest[0]));
	    m_currentTime = dest[0];
	  }
      }

    debugMsg("ExternalInterface:lookupNow", " of '" << stateName.toString() << "' complete");
  }

  /**
   * @brief Perform an immediate lookup on an existing state.
   * @param key The key for the state.
   * @param dest The destination for the current values for the state.
   */

  // *** N.B. dest is stack allocated, therefore pointers to it should not be stored!
  void 
  ThreadedExternalInterface::lookupNow(const StateKey& key, std::vector<double>& dest)
  {
    // Extract state name and arglist
    State state;
    m_exec->getStateCache()->stateForKey(key, state);
    // Defer to method above
    lookupNow(state, key, dest);
  }

  /**
   * @brief Inform the FL that a lookup should no longer receive updates.
   */
  void
  ThreadedExternalInterface::unregisterChangeLookup(const LookupKey& dest)
  {
    LookupAdaptorMap::iterator it = m_lookupAdaptorMap.find(dest);
    if (it == m_lookupAdaptorMap.end())
      {
	debugMsg("ExternalInterface:unregisterChangeLookup", 
		 " no lookup found for key " << dest);
	return;
      }

    InterfaceAdaptorId adaptor = (*it).second;
    assertTrueMsg(!adaptor.isNoId(),
		  "unregisterChangeLookup: Internal Error: No interface adaptor found for lookup key '"
		  << dest << "'");

    adaptor->unregisterChangeLookup(dest);
    adaptor->unregisterAsynchLookup(dest);
    m_lookupAdaptorMap.erase(dest);
  }

  /**
   * @brief Inform the FL that a lookup should no longer receive updates.
   */ 
  void 
  ThreadedExternalInterface::unregisterFrequencyLookup(const LookupKey& dest)
  {
    LookupAdaptorMap::iterator it = m_lookupAdaptorMap.find(dest);
    if (it == m_lookupAdaptorMap.end())
      {
	debugMsg("ExternalInterface:unregisterFrequencyLookup", 
		 " no lookup found for key " << dest);
	return;
      }

    InterfaceAdaptorId adaptor = (*it).second;
    assertTrueMsg(!adaptor.isNoId(),
		  "unregisterFrequencyLookup: Internal Error: No interface adaptor found for lookup key '"
		  << dest << "'");

    adaptor->unregisterFrequencyLookup(dest);
    adaptor->unregisterAsynchLookup(dest);
    m_lookupAdaptorMap.erase(dest);
  }

  // this batches the set of commands from quiescence completion.

  void 
  ThreadedExternalInterface::batchActions(std::list<CommandId>& commands)
  {
    if (commands.empty())
      return;

    debugMsg("ExternalInterface:batchActions", " entered");

    bool commandRejected = false;
    std::set<CommandId> acceptCmds;
    bool resourceArbiterExists = getResourceArbiterInterface().isId();

    if (resourceArbiterExists) 
      getResourceArbiterInterface()->arbitrateCommands(commands, acceptCmds);

    for (std::list<CommandId>::const_iterator it = commands.begin();
	 it != commands.end();
	 it++)
      {
	CommandId cmd = *it;

        if (!resourceArbiterExists || (acceptCmds.find(cmd) != acceptCmds.end()))
          {
            debugMsg("ExternalInterface:batchActions", 
                     " Permission to execute " << cmd->getName().toString()
                     << " has been granted by the resource arbiter (if one exists).");
            // Maintain a <acks, cmdId> map of commands
            m_ackToCmdMap[cmd->getAck()] = cmd;
            // Maintain a <dest, cmdId> map
            m_destToCmdMap[cmd->getDest()] = cmd;
            
            this->executeCommand(cmd->getName(),
                                 cmd->getArgValues(),
                                 cmd->getDest(),
                                 cmd->getAck());
          }
        else
          {
            commandRejected = true;
            debugMsg("ExternalInterface:batchActions ", 
                     "Permission to execute " << cmd->getName().toString()
                     << " has been denied by the resource arbiter.");
            
            this->rejectCommand(cmd->getName(),
                                 cmd->getArgValues(),
                                 cmd->getDest(),
                                 cmd->getAck());
          }
      }

    if (commandRejected)
      this->notifyOfExternalEvent();

    debugMsg("ExternalInterface:batchActions", " exited");
  }

  // this batches the set of function calls from quiescence completion.

  void 
  ThreadedExternalInterface::batchActions(std::list<FunctionCallId>& calls)
  {
    for (std::list<FunctionCallId>::const_iterator it = calls.begin();
	 it != calls.end();
	 it++)
      {
	FunctionCallId call = *it;
	this->executeFunctionCall(call->getName(),
				  call->getArgValues(),
				  call->getDest(),
				  call->getAck());
      }
  }

  // *** To do:
  //  - bookkeeping (i.e. tracking non-acked updates) ?

  void
  ThreadedExternalInterface::updatePlanner(std::list<UpdateId>& updates)
  {
    InterfaceAdaptorId intf = this->getPlannerUpdateInterface();
    if (intf.isNoId())
      {
	debugMsg("ExternalInterface:updatePlanner",
		 " no planner interface defined, not sending planner updates");
	return;
      }
    for (std::list<UpdateId>::const_iterator it = updates.begin();
	 it != updates.end();
	 it++)
      {
	UpdateId upd = *it;
	debugMsg("ExternalInterface:updatePlanner",
		 " sending planner update for node '"
		 << upd->getSource()->getNodeId().toString() << "'");
	intf->sendPlannerUpdate(upd->getSource(),
				upd->getPairs(),
				upd->getAck());
      }
  }

  // executes a command with the given arguments by looking up the command name 
  // and passing the information to the appropriate interface adaptor

  // *** To do:
  //  - bookkeeping (i.e. tracking active commands), mostly for invokeAbort() below
  void
  ThreadedExternalInterface::executeCommand(const LabelStr& name,
					    const std::list<double>& args,
					    ExpressionId dest,
					    ExpressionId ack)
  {
    InterfaceAdaptorId intf = getCommandInterface(name);
    assertTrueMsg(!intf.isNoId(),
		  "executeCommand: null interface adaptor for command " << name.toString());
    intf->executeCommand(name, args, dest, ack);
  }

    // rejects a command due to non-availability of resources
  void 
  ThreadedExternalInterface::rejectCommand(const LabelStr& name,
                                           const std::list<double>& args,
                                           ExpressionId dest,
                                           ExpressionId ack)
  {
    this->handleValueChange(ack, CommandHandleVariable::COMMAND_DENIED());
  }

  // executes a function call with the given arguments by looking up the name 
  // and passing the information to the appropriate interface adaptor

  // *** To do:
  //  - bookkeeping (i.e. tracking active calls), mostly for invokeAbort() below
  void
  ThreadedExternalInterface::executeFunctionCall(const LabelStr& name,
						 const std::list<double>& args,
						 ExpressionId dest,
						 ExpressionId ack)
  {
    InterfaceAdaptorId intf = getFunctionInterface(name);
    assertTrueMsg(!intf.isNoId(),
		  "executeFunctionCall: null interface adaptor for function " << name.toString());
    intf->executeFunctionCall(name, args, dest, ack);
  }

  //abort the given command with the given arguments.  store the abort-complete into dest
  void
  ThreadedExternalInterface::invokeAbort(const LabelStr& name,
					 const std::list<double>& args,
					 ExpressionId dest)
  {
    InterfaceAdaptorId intf = getCommandInterface(name);
    assertTrueMsg(!intf.isNoId(),
		  "invokeAbort: null interface adaptor for command " << name.toString());
    intf->invokeAbort(name, args, dest);
  }

  double 
  ThreadedExternalInterface::currentTime()
  {
    // *** punt for now
    return m_currentTime;
  }


  //
  // API to interface adaptors
  //

  /**
   * @brief Register the given interface adaptor for this command.  
   Returns true if successful.  Fails and returns false 
   iff the command name already has an adaptor registered.
   * @param commandName The command to map to this adaptor.
   * @param intf The interface adaptor to handle this command.
   */
  bool
  ThreadedExternalInterface::registerCommandInterface(const LabelStr & commandName,
						      InterfaceAdaptorId intf)
  {
    double commandNameKey = commandName.getKey();
    InterfaceMap::iterator it = m_commandMap.find(commandNameKey);
    if (it == m_commandMap.end())
      {
	// Not found, OK to add
	debugMsg("ExternalInterface:registerCommandInterface",
		 " registering interface for command '" << commandName.toString() << "'");
	m_commandMap.insert(std::pair<double, InterfaceAdaptorId>(commandNameKey, intf)) ;
        m_adaptors.insert(intf);
	return true;
      }
    else
      {
	debugMsg("ExternalInterface:registerCommandInterface",
		 " interface already registered for command '" << commandName.toString() << "'");
	return false;
      }
  }

  /**
   * @brief Register the given interface adaptor for this function.  
            Returns true if successful.  Fails and returns false 
            iff the function name already has an adaptor registered.
   * @param functionName The function to map to this adaptor.
   * @param intf The interface adaptor to handle this function.
   */
  bool
  ThreadedExternalInterface::registerFunctionInterface(const LabelStr & functionName,
						       InterfaceAdaptorId intf)
  {
    double functionNameKey = functionName.getKey();
    InterfaceMap::iterator it = m_functionMap.find(functionNameKey);
    if (it == m_functionMap.end())
      {
	// Not found, OK to add
	debugMsg("ExternalInterface:registerFunctionInterface",
		 " registering interface for function '" << functionName.toString() << "'");
	m_functionMap.insert(std::pair<double, InterfaceAdaptorId>(functionNameKey, intf)) ;
        m_adaptors.insert(intf);
	return true;
      }
    else
      {
	debugMsg("ExternalInterface:registerFunctionInterface",
		 " interface already registered for function '" << functionName.toString() << "'");
	return false;
      }
  }

  /**
   * @brief Register the given interface adaptor for lookups to this state.
   Returns true if successful.  Fails and returns false 
   if the state name already has an adaptor registered.
   * @param stateName The name of the state to map to this adaptor.
   * @param intf The interface adaptor to handle this lookup.
   */
  bool 
  ThreadedExternalInterface::registerLookupInterface(const LabelStr & stateName,
						     InterfaceAdaptorId intf)
  {
    double stateNameKey = stateName.getKey();
    InterfaceMap::iterator it = m_lookupMap.find(stateNameKey);
    if (it == m_lookupMap.end())
      {
	// Not found, OK to add
	debugMsg("ExternalInterface:registerLookupInterface",
		 " registering interface for lookup '" << stateName.toString() << "'");
	m_lookupMap.insert(std::pair<double, InterfaceAdaptorId>(stateNameKey, intf)) ;
        m_adaptors.insert(intf);
	return true;
      }
    else
      {
	debugMsg("ExternalInterface:registerLookupInterface",
		 " interface already registered for lookup '" << stateName.toString() << "'");
	return false;
      }
  }

  /**
   * @brief Register the given interface adaptor for planner updates.
            Returns true if successful.  Fails and returns false 
	    iff an adaptor is already registered.
   * @param intf The interface adaptor to handle planner updates.
   */
  bool
  ThreadedExternalInterface::registerPlannerUpdateInterface(InterfaceAdaptorId intf)
  {
    if (!m_plannerUpdateInterface.isNoId())
      {      
	debugMsg("ExternalInterface:registerPlannerUpdateInterface",
		 " planner update interface already registered");
	return false;
      }
    debugMsg("ExternalInterface:registerPlannerUpdateInterface",
	     " registering planner update interface"); 
    m_plannerUpdateInterface = intf;
    m_adaptors.insert(intf);
    return true;
  }

  /**
   * @brief Register the given interface adaptor as the default for all lookups and commands
   which do not have a specific adaptor.  Returns true if successful.
   Fails and returns false if there is already a default adaptor registered.
   * @param intf The interface adaptor to use as the default.
   */
  bool 
  ThreadedExternalInterface::setDefaultInterface(InterfaceAdaptorId intf)
  {
    if (!m_defaultInterface.isNoId())
      {
	debugMsg("ExternalInterface:setDefaultInterface",
		 " attempt to overwrite default interface adaptor " << m_defaultInterface);
	return false;
      }
    m_defaultInterface = intf;
    m_adaptors.insert(intf);
    debugMsg("ExternalInterface:setDefaultInterface",
	     " setting default interface " << intf);
    return true;
  }


  /**
   * @brief Removes the adaptor and deletes it iff nothing refers to it.
   */
  void ThreadedExternalInterface::deleteIfUnknown(InterfaceAdaptorId intf)
  {
    // Check the easy places first
    if (intf == m_defaultInterface
        || intf == m_plannerUpdateInterface)
      return;

    // See if the adaptor is in any of the tables
    InterfaceMap::iterator it = m_lookupMap.begin();
    for (; it != m_lookupMap.end(); it++)
      if (it->second == intf)
        return;
    it = m_commandMap.begin();
    for (; it != m_commandMap.end(); it++)
      if (it->second == intf)
        return;
    it = m_functionMap.begin();
    for (; it != m_functionMap.end(); it++)
      if (it->second == intf)
        return;

    // Not found, remove it and destroy it
    m_adaptors.erase(intf);
    intf.release();
  }

  /**
   * @brief Retract registration of the previous interface adaptor for this command.  
   * @param commandName The command.
   */
  void
  ThreadedExternalInterface::unregisterCommandInterface(const LabelStr & commandName)
  {
    double commandNameKey = commandName.getKey();
    InterfaceMap::iterator it = m_commandMap.find(commandNameKey);
    if (it != m_commandMap.end())
      {
	debugMsg("ExternalInterface:unregisterCommandInterface",
		 " removing interface for command '" << commandName.toString() << "'");
        InterfaceAdaptorId intf = it->second;
	m_commandMap.erase(it);
        deleteIfUnknown(intf);
      }
  }

  /**
   * @brief Retract registration of the previous interface adaptor for this function.  
   * @param functionName The function.
   */
  void
  ThreadedExternalInterface::unregisterFunctionInterface(const LabelStr & functionName)
  {
    double functionNameKey = functionName.getKey();
    InterfaceMap::iterator it = m_functionMap.find(functionNameKey);
    if (it != m_functionMap.end())
      {
	debugMsg("ExternalInterface:unregisterFunctionInterface",
		 " removing interface for function '" << functionName.toString() << "'");
        InterfaceAdaptorId intf = it->second;
	m_functionMap.erase(it);
        deleteIfUnknown(intf);
      }
  }

  /**
   * @brief Retract registration of the previous interface adaptor for this state.
   * @param stateName The state name.
   */
  void
  ThreadedExternalInterface::unregisterLookupInterface(const LabelStr & stateName)
  {
    double stateNameKey = stateName.getKey();
    InterfaceMap::iterator it = m_lookupMap.find(stateNameKey);
    if (it != m_lookupMap.end())
      {
	debugMsg("ExternalInterface:unregisterLookupInterface",
		 " removing interface for lookup '" << stateName.toString() << "'");
        InterfaceAdaptorId intf = it->second;
	m_lookupMap.erase(it);
        deleteIfUnknown(intf);
      }
  }

  /**
   * @brief Retract registration of the previous interface adaptor for planner updates.
   */
  void
  ThreadedExternalInterface::unregisterPlannerUpdateInterface()
  {
    debugMsg("ExternalInterface:unregisterPlannerUpdateInterface",
	     " removing planner update interface");
    InterfaceAdaptorId intf = m_plannerUpdateInterface;
    m_plannerUpdateInterface = InterfaceAdaptorId::noId();
    deleteIfUnknown(intf);
  }

  /**
   * @brief Retract registration of the previous default interface adaptor.
   */
  void
  ThreadedExternalInterface::unsetDefaultInterface()
  {
    debugMsg("ExternalInterface:unsetDefaultInterface",
	     " removing default interface");
    InterfaceAdaptorId intf = m_defaultInterface;
    m_defaultInterface = InterfaceAdaptorId::noId();
    deleteIfUnknown(intf);
  }

  /**
   * @brief Return the interface adaptor in effect for this command, whether 
   specifically registered or default. May return NoId().
   * @param commandName The command.
   */
  InterfaceAdaptorId
  ThreadedExternalInterface::getCommandInterface(const LabelStr & commandName)
  {
    double commandNameKey = commandName.getKey();
    InterfaceMap::iterator it = m_commandMap.find(commandNameKey);
    if (it != m_commandMap.end())
      {
	debugMsg("ExternalInterface:getCommandInterface",
		 " found specific interface " << (*it).second
		 << " for command '" << commandName.toString() << "'");
	return (*it).second;
      }
    debugMsg("ExternalInterface:getCommandInterface",
	     " returning default interface " << m_defaultInterface
	     << " for command '" << commandName.toString() << "'");
    return m_defaultInterface;
  }

  /**
   * @brief Return the interface adaptor in effect for this function, whether 
   specifically registered or default. May return NoId().
   * @param functionName The function.
   */
  InterfaceAdaptorId
  ThreadedExternalInterface::getFunctionInterface(const LabelStr & functionName)
  {
    double functionNameKey = functionName.getKey();
    InterfaceMap::iterator it = m_functionMap.find(functionNameKey);
    if (it != m_functionMap.end())
      {
	debugMsg("ExternalInterface:getFunctionInterface",
		 " found specific interface " << (*it).second
		 << " for function '" << functionName.toString() << "'");
	return (*it).second;
      }
    debugMsg("ExternalInterface:getFunctionInterface",
	     " returning default interface " << m_defaultInterface
	     << " for function '" << functionName.toString() << "'");
    return m_defaultInterface;
  }

  /**
   * @brief Return the interface adaptor in effect for lookups with this state name,
   whether specifically registered or default. May return NoId().
   * @param stateName The state.
   */
  InterfaceAdaptorId
  ThreadedExternalInterface::getLookupInterface(const LabelStr & stateName)
  {
    double stateNameKey = stateName.getKey();
    InterfaceMap::iterator it = m_lookupMap.find(stateNameKey);
    if (it != m_lookupMap.end())
      {
	debugMsg("ExternalInterface:getLookupInterface",
		 " found specific interface " << (*it).second
		 << " for lookup '" << stateName.toString() << "'");
	return (*it).second;
      }
    debugMsg("ExternalInterface:getLookupInterface",
	     " returning default interface " << m_defaultInterface
	     << " for lookup '" << stateName.toString() << "'");
    return m_defaultInterface;
  }

  /**
   * @brief Return the current default interface adaptor. May return NoId().
   */
  InterfaceAdaptorId 
  ThreadedExternalInterface::getDefaultInterface()
  {
    return m_defaultInterface;
  }

  /**
   * @brief Return the interface adaptor in effect for planner updates,
            whether specifically registered or default. May return NoId().
  */
  InterfaceAdaptorId
  ThreadedExternalInterface::getPlannerUpdateInterface()
  {
    if (m_plannerUpdateInterface.isNoId())
      {
	debugMsg("ExternalInterface:getPlannerUpdateInterface",
		 " returning default interface " << m_defaultInterface);
	return m_defaultInterface;
      }
    debugMsg("ExternalInterface:getPlannerUpdateInterface",
	     " found specific interface " << m_plannerUpdateInterface);
    return m_plannerUpdateInterface;
  }

  /**
   * @brief Register the given resource arbiter interface forr all commands
   Returns true if successful.
   Fails and returns false if there is already an interface registered.
   * @param raIntf The resource arbiter interface to use as the default.
   */
  bool 
  ThreadedExternalInterface::setResourceArbiterInterface(ResourceArbiterInterfaceId raIntf)
  {
    if (m_raInterface.isId())
      {
	debugMsg("ExternalInterface:setResourceArbiterInterface",
		 " attempt to overwrite resource arbiter interface " << m_raInterface);
	return false;
      }
    m_raInterface = raIntf;
    debugMsg("ExternalInterface::setResourceArbiterInterface",
	     " setting resource arbiter interface " << raIntf);
    return true;
  }

  /**
   * @brief Retract registration of the previous resource arbiter interface.
   */
  void 
  ThreadedExternalInterface::unsetResourceArbiterInterface()
  {
    debugMsg("ExternalInterface:unsetResourceArbiterInterface",
	     " removing resource arbiter interface");
    m_raInterface = ResourceArbiterInterfaceId::noId();
  }

  /**
   * @brief Notify of the availability of new values for a lookup.
   * @param key The state key for the new values.
   * @param values The new values.
   */
  void
  ThreadedExternalInterface::handleValueChange(const StateKey& key, 
					       const std::vector<double>& values)
  {
    debugMsg("ExternalInterface:handleValueChange", " for lookup values entered");
    m_valueQueue.enqueue(key, values);
  }

  /**
   * @brief Notify of the availability of (e.g.) a command return or acknowledgement.
   * @param exp The expression whose value is being returned.
   * @param value The new value of the expression.
   */
  void 
  ThreadedExternalInterface::handleValueChange(const ExpressionId & exp,
					       double value)
  {
    debugMsg("ExternalInterface:handleValueChange", " for return value entered");
    m_valueQueue.enqueue(exp, value);
  }

  /**
   * @brief Tells the external interface to expect a return value from this command.
            Use handleValueChange() to actually return the value.
   * @param dest The expression whose value will be returned.
   * @param name The command whose value will be returned.
   * @param params The parameters associated with this command.
   */
  void
  ThreadedExternalInterface::registerCommandReturnValue(ExpressionId dest,
							const LabelStr & name,
							const std::list<double> & params)
  {
    assertTrue(ALWAYS_FAIL, "registerCommandReturnValue not yet implemented!");
  }

  /**
   * @brief Tells the external interface to expect a return value from this function.
            Use handleValueChange() to actually return the value.
   * @param dest The expression whose value will be returned.
   * @param name The command whose value will be returned.
   * @param params The parameters associated with this command.
   */
  void
  ThreadedExternalInterface::registerFunctionReturnValue(ExpressionId dest,
							 const LabelStr & name,
							 const std::list<double> & params)
  {
    assertTrue(ALWAYS_FAIL, "registerFunctionReturnValue not yet implemented!");
  }

  /**
   * @brief Notify the external interface that this previously registered expression
            should not wait for a return value.
   * @param dest The expression whose value was to be returned.
   */
  void
  ThreadedExternalInterface::unregisterCommandReturnValue(ExpressionId dest)
  {
    assertTrue(ALWAYS_FAIL, "unregisterCommandReturnValue not yet implemented!");
  }

  /**
   * @brief Notify the external interface that this previously registered expression
            should not wait for a return value.
   * @param dest The expression whose value was to be returned.
   */
  void
  ThreadedExternalInterface::unregisterFunctionReturnValue(ExpressionId dest)
  {
    assertTrue(ALWAYS_FAIL, "unregisterFunctionReturnValue not yet implemented!");
  }

  /**
   * @brief Notify the executive of a new plan.
   * @param planXml The TinyXML representation of the new plan.
   * @param parent Label string naming the parent node.
   */
  void
  ThreadedExternalInterface::handleAddPlan(TiXmlElement * planXml,
					   const LabelStr& parent)
    throw(ParserException)
  {
    debugMsg("ExternalInterface:handleAddPlan(XML)", " entered");

    // check that the plan actually *has* a Node element!
    checkParserException(planXml->FirstChild() != NULL
			 && planXml->FirstChild()->Value() != NULL
			 && !(std::string(planXml->FirstChild()->Value()).empty())
			 && planXml->FirstChildElement() != NULL
			 && planXml->FirstChildElement("Node") != NULL,
			 "<" << planXml->Value() << "> is not a valid Plexil XML plan");

    // parse the plan
    static PlexilXmlParser parser;
    PlexilNodeId root =
      parser.parse(planXml->FirstChildElement("Node")); // can also throw ParserException

    this->handleAddPlan(root, parent);
  }

  /**
   * @brief Notify the executive of a new plan.
   * @param planStruct The PlexilNode representation of the new plan.
   * @param parent The node which is the parent of the new node.
   */
  void
  ThreadedExternalInterface::handleAddPlan(PlexilNodeId planStruct,
					   const LabelStr& parent)
  {
    debugMsg("ExternalInterface:handleAddPlan", " entered");
    m_valueQueue.enqueue(planStruct, parent);
  }

  /**
   * @brief Notify the executive of a new plan.
   * @param planStruct The PlexilNode representation of the new plan.
   */
  void
  ThreadedExternalInterface::handleAddLibrary(PlexilNodeId planStruct)
  {
    debugMsg("ExternalInterface:handleAddLibrary", " entered");
    m_valueQueue.enqueue(planStruct);
  }

  /**
   * @brief Notify the executive that it should run one cycle.  
            This should be sent after each batch of lookup, command
            return, and function return data.
  */
  void
  ThreadedExternalInterface::notifyOfExternalEvent()
  {
    debugMsg("ExternalInterface:notify",
	     " (" << pthread_self() << ") received external event");
    m_valueQueue.mark();
    if (m_execMutex.isLockedByCurrentThread())
      {
	// Called from inside runExec(), e.g. from within executeCommand()
	// runExec() will notice the event at the end of the current step.
	debugMsg("ExternalInterface:notify",
		 " (" << pthread_self() << ") inside runExec, ignoring event");
      }
    else if (m_runExecInBkgndOnly || m_execMutex.isLocked())
      {
	// Some other thread currently owns the exec.
	// runExec() could notice, or not.
	// Post to semaphore to ensure event is not lost.
	int status = m_sem.post();
	assertTrueMsg(status == 0,
		      "notifyOfExternalEvent: semaphore post failed, status = "
		      << status);
	debugMsg("ExternalInterface:notify",
		 " (" << pthread_self() << ") released semaphore");
      }
    else
      {
	// Exec is idle, so run it
	// If another thread grabs it first, no worries.
	debugMsg("ExternalInterface:notify",
		 " (" << pthread_self() << ") exec was idle, stepping it");
	this->runExec();
      }
  }

  //
  // Utility accessors
  //

  StateCacheId
  ThreadedExternalInterface::getStateCache() const
  { 
    return m_exec->getStateCache(); 
  }

  /**
   * @brief Get a unique key for a state, creating a new key for a new state.
   * @param state The state.
   * @param key The key.
   * @return True if a new key had to be generated.
   */
  bool
  ThreadedExternalInterface::keyForState(const State& state, StateKey& key)
  {
    return m_exec->getStateCache()->keyForState(state, key);
  }

  /**
   * @brief Get (a copy of) the State for this StateKey.
   * @param key The key to look up.
   * @param state The state associated with the key.
   * @return True if the key is found, false otherwise.
   */
  bool
  ThreadedExternalInterface::stateForKey(const StateKey& key, State& state) const
  {
    return m_exec->getStateCache()->stateForKey(key, state);
  }

  /**
   * @brief update the resoruce arbiter interface that an ack or return value
   * has been received so that resources can be released.
   * @param ackOrDest The expression id for which a value has been posted.
   */
  void ThreadedExternalInterface::releaseResourcesAtCommandTermination(ExpressionId ackOrDest)
  {
    // Check if the expression is an ack or a return value
    std::map<ExpressionId, CommandId>::iterator iter;

    if ((iter = m_ackToCmdMap.find(ackOrDest)) != m_ackToCmdMap.end())
      {
        CommandId cmdId = iter->second;
        debugMsg("ExternalInterface:releaseResourcesAtCommandTermination",
                 " The expression that was received is a valid acknowledgement"
                 << " for the command: " << cmdId->getName().toString());
        
        // Check if the command has a return value. If not, release resources
        // otherwise ignore
        if (cmdId->getDest().isNoId())
          {
            if (getResourceArbiterInterface().isId())
              getResourceArbiterInterface()->releaseResourcesForCommand(cmdId->getName().toString());
            // remove the ack expression from the map
            m_ackToCmdMap.erase(iter);
          }
      }
    else if ((iter = m_destToCmdMap.find(ackOrDest)) != m_destToCmdMap.end())
      {
        CommandId cmdId = iter->second;
        debugMsg("ExternalInterface:releaseResourcesForCommand",
                 " The expression that was received is a valid return value"
                 << " for the command: " << cmdId->getName().toString());

        //Release resources
        if (getResourceArbiterInterface().isId())
          getResourceArbiterInterface()->releaseResourcesForCommand(cmdId->getName().toString());
        //remove the ack from the map        
        m_ackToCmdMap.erase(m_ackToCmdMap.find(cmdId->getAck()));

        //remove the dest from the map
        m_destToCmdMap.erase(iter);
      }
    else
      debugMsg("ExternalInterface::releaseResourcesForCommand:",
               " The expression is neither an acknowledgement"
               << " nor a return value for a command. Ignoring.");

  }


  //
  // ValueQueue implementation
  //

  //
  // *** To do:
  //  - reimplement queue data structures to reduce copying
  //
  
  ThreadedExternalInterface::ValueQueue::ValueQueue()
    : m_queue(),
      m_mutex(new ThreadMutex())
  {
  }

  ThreadedExternalInterface::ValueQueue::~ValueQueue()
  {
    delete m_mutex;
  }

  void ThreadedExternalInterface::ValueQueue::enqueue(const ExpressionId & exp,
						      double newValue)
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.push(QueueEntry(exp, newValue));
  }

  void ThreadedExternalInterface::ValueQueue::enqueue(const StateKey& key, 
						      const std::vector<double> & newValues)
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.push(QueueEntry(key, newValues));
  }

  void ThreadedExternalInterface::ValueQueue::enqueue(PlexilNodeId newPlan,
						      const LabelStr & parent)
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.push(QueueEntry(newPlan, parent, queueEntry_PLAN));
  }

  void ThreadedExternalInterface::ValueQueue::enqueue(PlexilNodeId newLibraryNode)
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.push(QueueEntry(newLibraryNode, EMPTY_LABEL(), queueEntry_LIBRARY));
  }

  ThreadedExternalInterface::QueueEntryType
  ThreadedExternalInterface::ValueQueue::dequeue(StateKey& stateKey, std::vector<double>& newStateValues,
						 ExpressionId& exp, double& newExpValue,
						 PlexilNodeId& plan, LabelStr& planParent)
  {
    ThreadMutexGuard guard(*m_mutex);
    if (m_queue.empty())
      return queueEntry_EMPTY;
    QueueEntry e = m_queue.front();
    switch (e.type)
      {
      case queueEntry_MARK: // do nothing
	break;

      case queueEntry_LOOKUP_VALUES:
	stateKey = e.stateKey;
	newStateValues = e.values;
	break;

      case queueEntry_RETURN_VALUE:
	checkError(e.values.size() == 1,
		   "ExternalInterface::dequeue: Invalid number of values for return value entry");
	exp = e.expression;
	newExpValue = e.values[0];
	break;

      case queueEntry_PLAN:
	planParent = e.parent;
	// fall thru to library case

      case queueEntry_LIBRARY:
	plan = e.plan;
	break;

      default:
	assertTrue(ALWAYS_FAIL,
		   "ExternalInterface::dequeue: Invalid queue entry");
	break;
      }
    m_queue.pop();
    return e.type;
  }

  void ThreadedExternalInterface::ValueQueue::pop()
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.pop();
  }

  bool ThreadedExternalInterface::ValueQueue::isEmpty() const
  {
    ThreadMutexGuard guard(*m_mutex);
    return m_queue.empty();
  }
    
  void ThreadedExternalInterface::ValueQueue::mark()
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.push(QueueEntry(queueEntry_MARK));
  }


}
