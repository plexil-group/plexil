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

#include "InterfaceManager.hh"

#include "AdapterConfigurationFactory.hh"
#include "AdapterFactory.hh"
#include "AdapterConfiguration.hh"
#include "Debug.hh"
#include "DummyAdapter.hh"
#include "Error.hh"
#include "ExecApplication.hh"
#include "ExecListener.hh"
#include "ExecListenerFactory.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceSchema.hh"
#include "NewLuvListener.hh"
#include "TimeAdapter.hh"
#include "ResourceArbiterInterface.hh"
#include "Node.hh"
#include "PlexilExec.hh"
#include "PlexilXmlParser.hh"
#include "StateCache.hh"
#include "CommandHandle.hh"
#include "DynamicLoader.hh"
#include "adapterconfigs/DefaultAdapterConfiguration.hh"

#include <limits>
#include <sstream>

#include <pthread.h>

namespace PLEXIL
{

  /**
   * @brief Default constructor.
   */
  InterfaceManager::InterfaceManager(ExecApplication & app)
    : ExternalInterface(),
      AdapterExecInterface(),
      m_interfaceManagerId((InterfaceManagerId)m_adapterInterfaceId),
      m_application(app),
      m_adapterConfig(),
      m_valueQueue(),
      m_adapters(),
      m_listeners(),
      m_lookupAdapterMap(),
      m_raInterface(),
      m_ackToCmdMap(),
      m_destToCmdMap(),
      m_currentTime(std::numeric_limits<double>::min())
  {

    // Every application has access to the dummy adapter
    REGISTER_ADAPTER(DummyAdapter, "Dummy");

    // Every application has access to the OS-native time adapter
    REGISTER_ADAPTER(TIME_ADAPTER_CLASS, "OSNativeTime");

    // Every application should have access to the LUV Listener
    REGISTER_EXEC_LISTENER(NewLuvListener, "LuvListener");

    // Every application has access to the default adapter configuration
    REGISTER_ADAPTER_CONFIGURATION(DefaultAdapterConfiguration, "default");
  }

  /**
   * @brief Destructor.
   */
  InterfaceManager::~InterfaceManager()
  {
    m_adapterConfig.release();

    // unregister and delete listeners
    std::vector<ExecListenerId>::iterator lit = m_listeners.begin();
    while (lit != m_listeners.end()) {
      ExecListenerId l = *lit;
      lit = m_listeners.erase(lit);
      m_exec->removeListener(l);
      delete (ExecListener*) l;
    }

    // unregister and delete adapters
    // *** kludge for buggy std::set template ***
    std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
    while (it != m_adapters.end()) {
      InterfaceAdapterId ia = *it;
      m_adapters.erase(it); // these two lines should be:
      it = m_adapters.begin(); // it = m_adapters.erase(it)
      delete (InterfaceAdapter*) ia;
    }
  }

  //
  // Top-level loop
  //

  /**
   * @brief Register this adapter using the set AdapterConfiguration
   * @note The adapter is presumed to be fully initialized and working at the time of this call.
   * @note TODO: strategy for handling redundant registrations
   */
  void InterfaceManager::defaultRegisterAdapter(InterfaceAdapterId adapter)
  {
    m_adapterConfig->defaultRegisterAdapter(adapter);
  }

  /**
   * @brief Constructs interface adapters from the provided XML.
   * @param configXml The XML element used for interface configuration.
   */
  void InterfaceManager::constructInterfaces(const TiXmlElement * configXml)
  {
    debugMsg("InterfaceManager:constructInterfaces", " constructing interface adapters");
    if (configXml != 0)
      {
        const char* elementType = configXml->Value();
        checkError(strcmp(elementType, InterfaceSchema::INTERFACES_TAG()) == 0,
                   "constructInterfaces: invalid configuration XML: \n"
                   << *configXml);
        const char* configType =
          configXml->Attribute(InterfaceSchema::CONFIGURATION_TYPE_ATTR());
        if (configType == 0) {
          m_adapterConfig = AdapterConfigurationFactory::createInstance(LabelStr("default"), this);
        } else {
          m_adapterConfig = AdapterConfigurationFactory::createInstance(LabelStr(configType), this);
        }
        m_adapterConfig->getId();
        // Walk the children of the configuration XML element
        // and register the adapter according to the data found there
        const TiXmlElement* element = configXml->FirstChildElement();
        while (element != 0)
          {
            const char* elementType = element->Value();
            debugMsg("InterfaceManager:constructInterfaces",
                     " found tag " << elementType);
            // *** TO DO ***
            // generalize adapter factories to add support for (e.g.) ActorAdapter
            // w/o requiring knowledge of (e.g.) PlexilGenericActor
            if (strcmp(elementType, InterfaceSchema::ADAPTER_TAG()) == 0)
              {
                // Get the kind of adapter to make
                const char* adapterType = 
                  element->Attribute(InterfaceSchema::ADAPTER_TYPE_ATTR());
                checkError(adapterType != 0,
                           "constructInterfaces: no "
                           << InterfaceSchema::ADAPTER_TYPE_ATTR()
                           << " attribute for adapter XML:\n"
                           << *element);
                
                // load external library (linux only for now) if the adapter is not registered
                // TODO: support mac os
                if (!AdapterFactory::isRegistered(adapterType)) {
                  const char* libCPath =
                    element->Attribute(InterfaceSchema::LIB_PATH_ATTR());
                  std::string *libPath;
                  if (libCPath == 0) {
                    debugMsg("true", "constructInterfaces: no "
                               << InterfaceSchema::LIB_PATH_ATTR()
                               << " attribute for adapter XML:\n"
                               << *element);
                    std::stringstream libStream;
                    libStream << "lib" << adapterType << LIB_EXT;
                    libPath = new std::string(libStream.str());
                  } else {
                    libPath = new std::string(libCPath);
                  }
                  std::string funcName = (std::string("init") + adapterType);
                  void (*func)();
                  *(void **)(&func) = DynamicLoader::getDynamicSymbol(libPath->c_str(), funcName.c_str());
                  checkError(func != 0, "Failed to load library " << libPath->c_str() << DynamicLoader::getError());
                  (*func)();
                }

                // Construct the adapter
                InterfaceAdapterId adapter = 
                  AdapterFactory::createInstance(LabelStr(adapterType),
                                                 element,
                                                 *((AdapterExecInterface*)this));
                checkError(adapter.isId(),
                           "constructInterfaces: failed to construct adapter of type "
                           << adapterType);
                m_adapters.insert(adapter);
              }
            // *** TO DO ***
            // generalize listener factories to add support for (e.g.) ActorListener
            // w/o requiring knowledge of (e.g.) PlexilGenericActor
            else if (strcmp(elementType, InterfaceSchema::LISTENER_TAG()) == 0)
              {
                // Get the kind of listener to make
                const char* listenerType =
                  element->Attribute(InterfaceSchema::LISTENER_TYPE_ATTR());
                checkError(listenerType != 0,
                           "constructInterfaces: no "
                           << InterfaceSchema::LISTENER_TYPE_ATTR()
                           << " attribute for listener XML:\n"
                           << *element);

                // load external library (linux only for now) if the listener is not registered
                // TODO: support mac os
                if (!ExecListenerFactory::isRegistered(listenerType)) {
                  const char* libCPath =
                    element->Attribute(InterfaceSchema::LIB_PATH_ATTR());
                  std::string *libPath;
                  if (libCPath == 0) {
                    debugMsg("true", "constructInterfaces: no "
                               << InterfaceSchema::LIB_PATH_ATTR()
                               << " attribute for adapter XML:\n"
                               << *element);
                    std::stringstream libStream;
                    libStream << "lib" << listenerType << LIB_EXT;
                    libPath = new std::string(libStream.str());
                  } else {
                    libPath = new std::string(libCPath);
                  }
                  std::string funcName = (std::string("init") + listenerType);
                  void (*func)();
                  *(void **)(&func) = DynamicLoader::getDynamicSymbol(libPath->c_str(), funcName.c_str());
                  checkError(func != 0, DynamicLoader::getError());
                  (*func)();
                }

                // Construct an ExecListener instance and attach it to the Exec
                ExecListenerId listener = 
                  ExecListenerFactory::createInstance(LabelStr(listenerType),
                                                      element);
                checkError(listener.isId(),
                           "constructInterfaces: failed to construct listener of type "
                           << listenerType);
                m_listeners.push_back(listener);
              }
            else
              {
                checkError(ALWAYS_FAIL,
                           "constructInterfaces: unrecognized XML element \""
                           << elementType << "\"");
              }

            element = element->NextSiblingElement();
          }

      }
    debugMsg("InterfaceManager:constructInterfaces", " done.");
  }

  /**
   * @brief Add an externally constructed interface adapter.
   * @param The adapter ID.
   */
  void InterfaceManager::addInterfaceAdapter(const InterfaceAdapterId& adapter)
  {
    if (m_adapters.find(adapter) != m_adapters.end())
      m_adapters.insert(adapter);
  }

  /**
   * @brief Add an externally constructed ExecListener.
   * @param listener The ExecListener ID.
   */
  void InterfaceManager::addExecListener(const ExecListenerId& listener)
  {
    m_listeners.push_back(listener);
  }

  /**
   * @brief Performs basic initialization of the interface and all adapters.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::initialize()
  {
    debugMsg("InterfaceManager:initialize", " initializing interface adapters");
    bool success = true;
    for (std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
         success && it != m_adapters.end();
         it++)
      success = (*it)->initialize();
    for (std::vector<ExecListenerId>::iterator it = m_listeners.begin();
         success && it != m_listeners.end();
         it++)
      success = (*it)->initialize();
    return success;
  }

  /**
   * @brief Prepares the interface and adapters for execution.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::start()
  {
    debugMsg("InterfaceManager:startInterface", " starting interface adapters");
    bool success = true;
    for (std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
         success && it != m_adapters.end();
         it++)
      success = (*it)->start();
    for (std::vector<ExecListenerId>::iterator it = m_listeners.begin();
         success && it != m_listeners.end();
         it++)
      {
        if (success = (*it)->start())
          m_exec->addListener(*it);
      }
    return success;
  }

  /**
   * @brief Halts all interfaces.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::stop()
  {
    // halt adapters
    bool success = true;
    for (std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
         success && it != m_adapters.end();
         it++)
      success = (*it)->stop();
    for (std::vector<ExecListenerId>::iterator it = m_listeners.begin();
         success && it != m_listeners.end();
         it++)
      success = (*it)->stop();
    return success;
  }

  /**
   * @brief Resets the interface prior to restarting.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::reset()
  {
    // reset queue etc. to freshly initialized state
    // *** NYI ***

    // clear adapter registry
    clearAdapterRegistry();

    bool success = true;
    for (std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
         success && it != m_adapters.end();
         it++)
      success = (*it)->reset();
    for (std::vector<ExecListenerId>::iterator it = m_listeners.begin();
         success && it != m_listeners.end();
         it++)
      success = (*it)->reset();
    return success;
  }

  /**
   * @brief Clears the interface adapter registry.
   */
  void InterfaceManager::clearAdapterRegistry() {
    m_lookupAdapterMap.clear();
    m_adapterConfig->clearAdapterRegistry();
  }

  /**
   * @brief Shuts down the interface.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::shutdown()
  {
    // clear adapter registry
    clearAdapterRegistry();

    bool success = true;
    for (std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
         success && it != m_adapters.end();
         it++)
      success = (*it)->shutdown();
    for (std::vector<ExecListenerId>::iterator it = m_listeners.begin();
         success && it != m_listeners.end();
         it++)
      success = (*it)->shutdown();

    // Clean up
    // *** NYI ***

    return success;
  }

  //
  // API for exec
  //

  /**
   * @brief Delete any entries in the queue.
   */
  void InterfaceManager::resetQueue()
  {
    debugMsg("InterfaceManager:resetQueue", " entered");
    while (!m_valueQueue.isEmpty())
      {
	m_valueQueue.pop();
      }
  }
    
    
  /**
   * @brief Updates the state cache from the items in the queue.
   * @return True if the Exec needs to be stepped, false otherwise.
   * @note Should only be called with exec locked by the current thread
   */
  bool InterfaceManager::processQueue()
  {
    debugMsg("InterfaceManager:processQueue",
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
	debugMsg("InterfaceManager:processQueue",
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
		debugMsg("InterfaceManager:processQueue",
			 " (" << pthread_self() << ") queue empty at entry, returning false");
		return false;
	      }
	    debugMsg("InterfaceManager:processQueue",
		     " (" << pthread_self() << ") reached end of queue without finding mark, returning true");
	    return true;
	    break;

	  case queueEntry_MARK:
	    // Exit loop now, whether or not queue is empty
	    debugMsg("InterfaceManager:processQueue",
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
		  debugMsg("InterfaceManager:processQueue",
			   " (" << pthread_self()
			   << ") Handling state change for '"
			   << getText(state)
			   << "', " << newStateValues.size()
			   << " new value(s)");

		  if (newStateValues.size() == 0)
		    {
		      debugMsg("InterfaceManager:processQueue",
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
			  debugMsg("InterfaceManager:processQueue",
				   " (" << pthread_self()
				   << ") Ignoring stale time update - new value "
				   << newStateValues[0] << " is not greater than cached value "
				   << m_currentTime);
			}
		      else
			{
			  debugMsg("InterfaceManager:processQueue",
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
		  debugMsg("InterfaceManager:processQueue", 
			   " (" << pthread_self()
			   << ") ignoring lookup for nonexistent state, key = "
			   << stateKey);
		}
	      break;
	    }

	  case queueEntry_RETURN_VALUE:
	    // Expression -- update the expression only
	    debugMsg("InterfaceManager:processQueue",
		     " (" << pthread_self()
		     << ") Updating expression " << exp
		     << ", new value is '" << valueToString(newExpValue) << "'");
	    this->releaseResourcesAtCommandTermination(exp);
	    exp->setValue(newExpValue);
	    break;

	  case queueEntry_PLAN:
	    // Plan -- add the plan
	    debugMsg("InterfaceManager:processQueue",
		     " (" << pthread_self() << ") Received plan");
	    getExec()->addPlan(plan, parent);
	    break;

	  case queueEntry_LIBRARY:
	    // Library -- add the library

	    debugMsg("InterfaceManager:processQueue",
		     " (" << pthread_self() << ") Received library");
	    // *** TODO: check here for duplicates ***
            getExec()->addLibraryNode(plan);
	    // no need to step here
	    break;

	  default:
	    // error
	    checkError(ALWAYS_FAIL,
		       "InterfaceManager:processQueue: Invalid entry type "
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
  InterfaceManager::registerChangeLookup(const LookupKey& source,
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
  InterfaceManager::registerChangeLookup(const LookupKey& source,
						  const StateKey& key, 
						  const std::vector<double>& tolerances)
  {
    // Extract state name and arglist
    State state;
    m_exec->getStateCache()->stateForKey(key, state);
    const LabelStr& stateName(state.first);

    InterfaceAdapterId adapter = getLookupInterface(stateName);
    assertTrueMsg(!adapter.isNoId(),
		  "registerChangeLookup: No interface adapter found for lookup '"
		  << stateName.toString() << "'");

    m_lookupAdapterMap.insert(std::pair<LookupKey, InterfaceAdapterId>(source, adapter));
    // for convenience of adapter implementors
    adapter->registerAsynchLookup(source, key);
    adapter->registerChangeLookup(source, key, tolerances);
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
  InterfaceManager::registerFrequencyLookup(const LookupKey& source,
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
  InterfaceManager::registerFrequencyLookup(const LookupKey& source,
						     const StateKey& key,
						     const double& lowFreq,
						     const double& highFreq)
  {
    // Extract state name and arglist
    State state;
    m_exec->getStateCache()->stateForKey(key, state);
    const LabelStr& stateName(state.first);

    InterfaceAdapterId adapter = getLookupInterface(stateName);
    assertTrueMsg(!adapter.isNoId(),
		  "registerFrequencyLookup: No interface adapter found for lookup '"
		  << stateName.toString() << "'");

    m_lookupAdapterMap.insert(std::pair<LookupKey, InterfaceAdapterId>(source, adapter));
    // for convenience of adapter implementors
    adapter->registerAsynchLookup(source, key);
    adapter->registerFrequencyLookup(source, key, lowFreq, highFreq);
  }

  /**
   * @brief Perform an immediate lookup on a new state.
   * @param state The state
   * @param key The key for the state to be used in future communications about the state.
   * @param dest The destination for the current values for the state.
   */

  // *** N.B. dest is stack allocated, therefore pointers to it should not be stored!
  void 
  InterfaceManager::lookupNow(const State& state,
				       const StateKey& key,
				       std::vector<double>& dest)
  {
    const LabelStr& stateName(state.first);
    debugMsg("InterfaceManager:lookupNow", " of '" << stateName.toString() << "'");
    InterfaceAdapterId adapter = getLookupInterface(stateName);
    assertTrueMsg(!adapter.isNoId(),
		  "lookupNow: No interface adapter found for lookup '"
		  << stateName.toString() << "'");

    adapter->lookupNow(key, dest);
    // update internal idea of time if required
    if (key == m_exec->getStateCache()->getTimeStateKey())
      {
	if (dest[0] <= m_currentTime)
	  {
	    debugMsg("InterfaceManager:lookupNow",
		     " Ignoring stale time update - new value "
		     << dest[0] << " is not greater than cached value "
		     << m_currentTime);
	  }
	else
	  {
	    debugMsg("InterfaceManager:lookupNow",
		     " setting current time to "
		     << valueToString(dest[0]));
	    m_currentTime = dest[0];
	  }
      }

    debugMsg("InterfaceManager:lookupNow", " of '" << stateName.toString() << "' complete");
  }

  /**
   * @brief Perform an immediate lookup on an existing state.
   * @param key The key for the state.
   * @param dest The destination for the current values for the state.
   */

  // *** N.B. dest is stack allocated, therefore pointers to it should not be stored!
  void 
  InterfaceManager::lookupNow(const StateKey& key, std::vector<double>& dest)
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
  InterfaceManager::unregisterChangeLookup(const LookupKey& dest)
  {
    LookupAdapterMap::iterator it = m_lookupAdapterMap.find(dest);
    if (it == m_lookupAdapterMap.end())
      {
	debugMsg("InterfaceManager:unregisterChangeLookup", 
		 " no lookup found for key " << dest);
	return;
      }

    InterfaceAdapterId adapter = (*it).second;
    assertTrueMsg(!adapter.isNoId(),
		  "unregisterChangeLookup: Internal Error: No interface adapter found for lookup key '"
		  << dest << "'");

    adapter->unregisterChangeLookup(dest);
    adapter->unregisterAsynchLookup(dest);
    m_lookupAdapterMap.erase(dest);
  }

  /**
   * @brief Inform the FL that a lookup should no longer receive updates.
   */ 
  void 
  InterfaceManager::unregisterFrequencyLookup(const LookupKey& dest)
  {
    LookupAdapterMap::iterator it = m_lookupAdapterMap.find(dest);
    if (it == m_lookupAdapterMap.end())
      {
	debugMsg("InterfaceManager:unregisterFrequencyLookup", 
		 " no lookup found for key " << dest);
	return;
      }

    InterfaceAdapterId adapter = (*it).second;
    assertTrueMsg(!adapter.isNoId(),
		  "unregisterFrequencyLookup: Internal Error: No interface adapter found for lookup key '"
		  << dest << "'");

    adapter->unregisterFrequencyLookup(dest);
    adapter->unregisterAsynchLookup(dest);
    m_lookupAdapterMap.erase(dest);
  }

  // this batches the set of commands from quiescence completion.

  void 
  InterfaceManager::batchActions(std::list<CommandId>& commands)
  {
    if (commands.empty())
      return;

    debugMsg("InterfaceManager:batchActions", " entered");

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
            debugMsg("InterfaceManager:batchActions", 
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
            debugMsg("InterfaceManager:batchActions ", 
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

    debugMsg("InterfaceManager:batchActions", " exited");
  }

  // this batches the set of function calls from quiescence completion.

  void 
  InterfaceManager::batchActions(std::list<FunctionCallId>& calls)
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
  InterfaceManager::updatePlanner(std::list<UpdateId>& updates)
  {
    InterfaceAdapterId intf = this->getPlannerUpdateInterface();
    if (intf.isNoId())
      {
	debugMsg("InterfaceManager:updatePlanner",
		 " no planner interface defined, not sending planner updates");
	return;
      }
    for (std::list<UpdateId>::const_iterator it = updates.begin();
	 it != updates.end();
	 it++)
      {
	UpdateId upd = *it;
	debugMsg("InterfaceManager:updatePlanner",
		 " sending planner update for node '"
		 << upd->getSource()->getNodeId().toString() << "'");
	intf->sendPlannerUpdate(upd->getSource(),
				upd->getPairs(),
				upd->getAck());
      }
  }

  // executes a command with the given arguments by looking up the command name 
  // and passing the information to the appropriate interface adapter

  // *** To do:
  //  - bookkeeping (i.e. tracking active commands), mostly for invokeAbort() below
  void
  InterfaceManager::executeCommand(const LabelStr& name,
					    const std::list<double>& args,
					    ExpressionId dest,
					    ExpressionId ack)
  {
    InterfaceAdapterId intf = getCommandInterface(name);
    assertTrueMsg(!intf.isNoId(),
		  "executeCommand: null interface adapter for command " << name.toString());
    intf->executeCommand(name, args, dest, ack);
  }

    // rejects a command due to non-availability of resources
  void 
  InterfaceManager::rejectCommand(const LabelStr& name,
                                           const std::list<double>& args,
                                           ExpressionId dest,
                                           ExpressionId ack)
  {
    this->handleValueChange(ack, CommandHandleVariable::COMMAND_DENIED());
  }

  // executes a function call with the given arguments by looking up the name 
  // and passing the information to the appropriate interface adapter

  // *** To do:
  //  - bookkeeping (i.e. tracking active calls), mostly for invokeAbort() below
  void
  InterfaceManager::executeFunctionCall(const LabelStr& name,
						 const std::list<double>& args,
						 ExpressionId dest,
						 ExpressionId ack)
  {
    InterfaceAdapterId intf = getFunctionInterface(name);
    assertTrueMsg(!intf.isNoId(),
		  "executeFunctionCall: null interface adapter for function " << name.toString());
    intf->executeFunctionCall(name, args, dest, ack);
  }

  //abort the given command with the given arguments.  store the abort-complete into dest
  void
  InterfaceManager::invokeAbort(const LabelStr& name,
					 const std::list<double>& args,
					 ExpressionId dest)
  {
    InterfaceAdapterId intf = getCommandInterface(name);
    assertTrueMsg(!intf.isNoId(),
		  "invokeAbort: null interface adapter for command " << name.toString());
    intf->invokeAbort(name, args, dest);
  }

  double 
  InterfaceManager::currentTime()
  {
    // *** punt for now
    return m_currentTime;
  }


  //
  // API to interface adapters
  //

  /**
   * @brief Register the given interface adapter for this command.  
   Returns true if successful.  Fails and returns false 
   iff the command name already has an adapter registered.
   * @param commandName The command to map to this adapter.
   * @param intf The interface adapter to handle this command.
   */
  bool
  InterfaceManager::registerCommandInterface(const LabelStr & commandName,
						      InterfaceAdapterId intf)
  {
    return m_adapterConfig->registerCommandInterface(commandName, intf);
  }

  /**
   * @brief Register the given interface adapter for this function.  
            Returns true if successful.  Fails and returns false 
            iff the function name already has an adapter registered.
   * @param functionName The function to map to this adapter.
   * @param intf The interface adapter to handle this function.
   */
  bool
  InterfaceManager::registerFunctionInterface(const LabelStr & functionName,
						       InterfaceAdapterId intf)
  {
    return m_adapterConfig->registerFunctionInterface(functionName, intf);
  }

  /**
   * @brief Register the given interface adapter for lookups to this state.
   Returns true if successful.  Fails and returns false 
   if the state name already has an adapter registered.
   * @param stateName The name of the state to map to this adapter.
   * @param intf The interface adapter to handle this lookup.
   */
  bool 
  InterfaceManager::registerLookupInterface(const LabelStr & stateName,
      const InterfaceAdapterId& intf)
  {
    return m_adapterConfig->registerLookupInterface(stateName, intf);
  }

  /**
   * @brief Register the given interface adapter for planner updates.
            Returns true if successful.  Fails and returns false 
	    iff an adapter is already registered.
   * @param intf The interface adapter to handle planner updates.
   */
  bool
  InterfaceManager::registerPlannerUpdateInterface(InterfaceAdapterId intf)
  {
    return m_adapterConfig->registerPlannerUpdateInterface(intf);
  }

  /**
   * @brief Register the given interface adapter as the default for all lookups and commands
   which do not have a specific adapter.  Returns true if successful.
   Fails and returns false if there is already a default adapter registered.
   * @param intf The interface adapter to use as the default.
   */
  bool 
  InterfaceManager::setDefaultInterface(InterfaceAdapterId intf)
  {
    return m_adapterConfig->setDefaultInterface(intf);
  }

  /**
   * @brief Register the given interface adapter as the default for all commands
   which do not have a specific adapter.  Returns true if successful.
   Fails and returns false if there is already a default adapter registered.
   * @param intf The interface adapter to use as the default.
   */
  bool 
  InterfaceManager::setDefaultCommandInterface(InterfaceAdapterId intf)
  {
    return m_adapterConfig->setDefaultCommandInterface(intf);
  }

  /**
   * @brief Register the given interface adapter as the default for all lookups
   which do not have a specific adapter.  Returns true if successful.
   Fails and returns false if there is already a default adapter registered.
   * @param intf The interface adapter to use as the default.
   */
  bool 
  InterfaceManager::setDefaultLookupInterface(InterfaceAdapterId intf)
  {
    return m_adapterConfig->setDefaultLookupInterface(intf);
  }


  /**
   * @brief Removes the adapter and deletes it iff nothing refers to it.
   */
  void InterfaceManager::deleteIfUnknown(InterfaceAdapterId intf)
  {
    if (m_adapterConfig->isKnown(intf))
    {
      deleteAdapter(intf);
    }
  }

  /**
   * @brief Retract registration of the previous interface adapter for this command.  
   * @param commandName The command.
   */
  void
  InterfaceManager::unregisterCommandInterface(const LabelStr & commandName)
  {
    return m_adapterConfig->unregisterFunctionInterface(commandName);
  }

  /**
   * @brief Retract registration of the previous interface adapter for this function.  
   * @param functionName The function.
   */
  void
  InterfaceManager::unregisterFunctionInterface(const LabelStr & functionName)
  {
    return m_adapterConfig->unregisterFunctionInterface(functionName);
  }

  /**
   * @brief Retract registration of the previous interface adapter for this state.
   * @param stateName The state name.
   */
  void
  InterfaceManager::unregisterLookupInterface(const LabelStr & stateName)
  {
    return m_adapterConfig->unregisterLookupInterface(stateName);
  }

  /**
   * @brief Retract registration of the previous interface adapter for planner updates.
   */
  void
  InterfaceManager::unregisterPlannerUpdateInterface()
  {
    return m_adapterConfig->unregisterPlannerUpdateInterface();
  }

  /**
   * @brief Retract registration of the previous default interface adapter.
   */
  void
  InterfaceManager::unsetDefaultInterface()
  {
    return m_adapterConfig->unsetDefaultInterface();
  }

  /**
   * @brief Retract registration of the previous default interface adapter for commands.
   */
  void
  InterfaceManager::unsetDefaultCommandInterface()
  {
    return m_adapterConfig->unsetDefaultCommandInterface();
  }

  /**
   * @brief Retract registration of the previous default interface adapter for lookups.
   */
  void
  InterfaceManager::unsetDefaultLookupInterface()
  {
    return m_adapterConfig->unsetDefaultLookupInterface();
  }

  /**
   * @brief Return the interface adapter in effect for this command, whether 
   specifically registered or default. May return NoId().
   * @param commandName The command.
   */
  InterfaceAdapterId
  InterfaceManager::getCommandInterface(const LabelStr & commandName)
  {
    return m_adapterConfig->getCommandInterface(commandName);
  }

  /**
   * @brief Return the interface adapter in effect for this function, whether 
   specifically registered or default. May return NoId().
   * @param functionName The function.
   */
  InterfaceAdapterId
  InterfaceManager::getFunctionInterface(const LabelStr & functionName)
  {
    return m_adapterConfig->getFunctionInterface(functionName);
  }

  /**
   * @brief Return the interface adapter in effect for lookups with this state name,
   whether specifically registered or default. May return NoId().
   * @param stateName The state.
   */
  InterfaceAdapterId
  InterfaceManager::getLookupInterface(const LabelStr & stateName)
  {
    return m_adapterConfig->getLookupInterface(stateName);
  }

  /**
   * @brief Return the current default interface adapter. May return NoId().
   */
  InterfaceAdapterId 
  InterfaceManager::getDefaultInterface()
  {
    return m_adapterConfig->getDefaultInterface();
  }

  /**
   * @brief Return the current default interface adapter for commands. May return NoId().
   */
  InterfaceAdapterId 
  InterfaceManager::getDefaultCommandInterface()
  {
    return m_adapterConfig->getDefaultCommandInterface();
  }

  /**
   * @brief Return the current default interface adapter for lookups. May return NoId().
   */
  InterfaceAdapterId 
  InterfaceManager::getDefaultLookupInterface()
  {
    return m_adapterConfig->getDefaultLookupInterface();
  }

  /**
   * @brief Return the interface adapter in effect for planner updates,
            whether specifically registered or default. May return NoId().
  */
  InterfaceAdapterId
  InterfaceManager::getPlannerUpdateInterface()
  {
    return m_adapterConfig->getPlannerUpdateInterface();
  }

  /**
   * @brief Register the given resource arbiter interface for all commands
   Returns true if successful.
   Fails and returns false if there is already an interface registered.
   * @param raIntf The resource arbiter interface to use as the default.
   */
  bool 
  InterfaceManager::setResourceArbiterInterface(ResourceArbiterInterfaceId raIntf)
  {
    if (m_raInterface.isId())
      {
	debugMsg("InterfaceManager:setResourceArbiterInterface",
		 " attempt to overwrite resource arbiter interface " << m_raInterface);
	return false;
      }
    m_raInterface = raIntf;
    debugMsg("InterfaceManager:setResourceArbiterInterface",
	     " setting resource arbiter interface " << raIntf);
    return true;
  }

  /**
   * @brief Retract registration of the previous resource arbiter interface.
   */
  void 
  InterfaceManager::unsetResourceArbiterInterface()
  {
    debugMsg("InterfaceManager:unsetResourceArbiterInterface",
	     " removing resource arbiter interface");
    m_raInterface = ResourceArbiterInterfaceId::noId();
  }

  /**
   * @brief Notify of the availability of new values for a lookup.
   * @param key The state key for the new values.
   * @param values The new values.
   */
  void
  InterfaceManager::handleValueChange(const StateKey& key, 
					       const std::vector<double>& values)
  {
    debugMsg("InterfaceManager:handleValueChange", " for lookup values entered");
    m_valueQueue.enqueue(key, values);
  }

  /**
   * @brief Notify of the availability of (e.g.) a command return or acknowledgement.
   * @param exp The expression whose value is being returned.
   * @param value The new value of the expression.
   */
  void 
  InterfaceManager::handleValueChange(const ExpressionId & exp,
					       double value)
  {
    debugMsg("InterfaceManager:handleValueChange", " for return value entered");
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
  InterfaceManager::registerCommandReturnValue(ExpressionId dest,
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
  InterfaceManager::registerFunctionReturnValue(ExpressionId dest,
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
  InterfaceManager::unregisterCommandReturnValue(ExpressionId dest)
  {
    assertTrue(ALWAYS_FAIL, "unregisterCommandReturnValue not yet implemented!");
  }

  /**
   * @brief Notify the external interface that this previously registered expression
            should not wait for a return value.
   * @param dest The expression whose value was to be returned.
   */
  void
  InterfaceManager::unregisterFunctionReturnValue(ExpressionId dest)
  {
    assertTrue(ALWAYS_FAIL, "unregisterFunctionReturnValue not yet implemented!");
  }

  /**
   * @brief Notify the executive of a new plan.
   * @param planXml The TinyXML representation of the new plan.
   * @param parent Label string naming the parent node.
   */
  void
  InterfaceManager::handleAddPlan(TiXmlElement * planXml,
					   const LabelStr& parent)
    throw(ParserException)
  {
    debugMsg("InterfaceManager:handleAddPlan(XML)", " entered");

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
  InterfaceManager::handleAddPlan(PlexilNodeId planStruct,
					   const LabelStr& parent)
  {
    debugMsg("InterfaceManager:handleAddPlan", " entered");
    m_valueQueue.enqueue(planStruct, parent);
  }

  /**
   * @brief Notify the executive of a new plan.
   * @param planStruct The PlexilNode representation of the new plan.
   */
  void
  InterfaceManager::handleAddLibrary(PlexilNodeId planStruct)
  {
    debugMsg("InterfaceManager:handleAddLibrary", " entered");
    m_valueQueue.enqueue(planStruct);
  }

  /**
   * @brief Notify the executive that it should run one cycle.  
            This should be sent after each batch of lookup, command
            return, and function return data.
  */
  void
  InterfaceManager::notifyOfExternalEvent()
  {
    debugMsg("InterfaceManager:notify",
	     " (" << pthread_self() << ") received external event");
    m_valueQueue.mark();
    m_application.notifyExec();
  }

  //
  // Utility accessors
  //

  StateCacheId
  InterfaceManager::getStateCache() const
  { 
    return m_exec->getStateCache(); 
  }

  /**
   * @brief Look up the unique key for a state.
   * @param state The state.
   * @param key The key associated with this state.
   * @return True if the key was found.
   */
  bool
  InterfaceManager::findStateKey(const State& state, StateKey& key)
  {
    return m_exec->getStateCache()->findStateKey(state, key);
  }

  /**
   * @brief Get a unique key for a state, creating a new key for a new state.
   * @param state The state.
   * @param key The key.
   * @return True if a new key had to be generated.
   */
  bool
  InterfaceManager::keyForState(const State& state, StateKey& key)
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
  InterfaceManager::stateForKey(const StateKey& key, State& state) const
  {
    return m_exec->getStateCache()->stateForKey(key, state);
  }

  /**
   * @brief update the resoruce arbiter interface that an ack or return value
   * has been received so that resources can be released.
   * @param ackOrDest The expression id for which a value has been posted.
   */
  void InterfaceManager::releaseResourcesAtCommandTermination(ExpressionId ackOrDest)
  {
    // Check if the expression is an ack or a return value
    std::map<ExpressionId, CommandId>::iterator iter;

    if ((iter = m_ackToCmdMap.find(ackOrDest)) != m_ackToCmdMap.end())
      {
        CommandId cmdId = iter->second;
        debugMsg("InterfaceManager:releaseResourcesAtCommandTermination",
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
        debugMsg("InterfaceManager:releaseResourcesForCommand",
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
      debugMsg("InterfaceManager:releaseResourcesForCommand:",
               " The expression is neither an acknowledgement"
               << " nor a return value for a command. Ignoring.");

  }

  /**
   * @brief Deletes the given adapter
   * @return true if the given adapter existed and was deleted. False if not found
   */
  bool InterfaceManager::deleteAdapter(InterfaceAdapterId intf) {
    int res = m_adapters.erase(intf);
    intf.release();
    return res != 0;
  }

  //
  // ValueQueue implementation
  //

  //
  // *** To do:
  //  - reimplement queue data structures to reduce copying
  //
  
  InterfaceManager::ValueQueue::ValueQueue()
    : m_queue(),
      m_mutex(new ThreadMutex())
  {
  }

  InterfaceManager::ValueQueue::~ValueQueue()
  {
    delete m_mutex;
  }

  void InterfaceManager::ValueQueue::enqueue(const ExpressionId & exp,
						      double newValue)
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.push(QueueEntry(exp, newValue));
  }

  void InterfaceManager::ValueQueue::enqueue(const StateKey& key, 
						      const std::vector<double> & newValues)
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.push(QueueEntry(key, newValues));
  }

  void InterfaceManager::ValueQueue::enqueue(PlexilNodeId newPlan,
						      const LabelStr & parent)
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.push(QueueEntry(newPlan, parent, queueEntry_PLAN));
  }

  void InterfaceManager::ValueQueue::enqueue(PlexilNodeId newLibraryNode)
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.push(QueueEntry(newLibraryNode, EMPTY_LABEL(), queueEntry_LIBRARY));
  }

  InterfaceManager::QueueEntryType
  InterfaceManager::ValueQueue::dequeue(StateKey& stateKey, std::vector<double>& newStateValues,
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
		   "InterfaceManager:dequeue: Invalid number of values for return value entry");
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
		   "InterfaceManager:dequeue: Invalid queue entry");
	break;
      }
    m_queue.pop();
    return e.type;
  }

  void InterfaceManager::ValueQueue::pop()
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.pop();
  }

  bool InterfaceManager::ValueQueue::isEmpty() const
  {
    ThreadMutexGuard guard(*m_mutex);
    return m_queue.empty();
  }
    
  void InterfaceManager::ValueQueue::mark()
  {
    ThreadMutexGuard guard(*m_mutex);
    m_queue.push(QueueEntry(queueEntry_MARK));
  }

}
