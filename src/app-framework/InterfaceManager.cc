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

//
// TODO:
//  - implement tracking of active commands (?)
//  - implement un/registerCommandReturnValue()
//  - utilities for adapters?
//

#include "InterfaceManager.hh"

#include <plexil-config.h>

#include "AdapterConfiguration.hh"
#include "AdapterConfigurationFactory.hh"
#include "AdapterFactory.hh"
#include "BooleanVariable.hh"
#include "ControllerFactory.hh"
#include "Command.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "DefaultAdapterConfiguration.hh"
#include "DummyAdapter.hh"
#include "Error.hh"
#include "ExecApplication.hh"
#include "ExecController.hh"
#include "ExecListener.hh"
#include "ExecListenerFactory.hh"
#include "ExecListenerFilterFactory.hh"
#include "ExecListenerHub.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceSchema.hh"
#include "ListenerFilters.hh"
#include "Node.hh"
#include "PlexilExec.hh"
#include "PlexilXmlParser.hh"
#include "ResourceArbiterInterface.hh"
#include "StateCache.hh"
#include "TimeAdapter.hh"
#include "Update.hh"
#include "UtilityAdapter.hh"

#if HAVE_LUV_LISTENER
#include "LuvListener.hh"
#endif

#if HAVE_DEBUG_LISTENER
#include "PlanDebugListener.hh"
#endif

#if defined(_POSIX_TIMERS) && ((_POSIX_TIMERS - 200112L) >= 0L || defined(PLEXIL_ANDROID))
#include "PosixTimeAdapter.hh"
#elif defined(HAVE_SETITIMER)
#include "DarwinTimeAdapter.hh"
//#else
//#error "No time adapter implementation class for this environment"
#endif

#include <cstring>
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
      m_interfaceManagerId(this, ExternalInterface::getId()),
      m_application(app),
      m_adapterConfig(),
      m_valueQueue(),
      m_listenerHub((new ExecListenerHub())->getId()),
      m_adapters(),
      m_ackToCmdMap(),
      m_destToCmdMap(),
      m_raInterface(),
      m_execController(),
      m_currentTime(std::numeric_limits<double>::min()),
      m_lastMark(0)
  {
    // Every application has access to the dummy and utility adapters
    REGISTER_ADAPTER(DummyAdapter, "Dummy");
    REGISTER_ADAPTER(UtilityAdapter, "Utility");

#ifdef TIME_ADAPTER_CLASS
    // Every application has access to the OS-native time adapter
    REGISTER_ADAPTER(TIME_ADAPTER_CLASS, "OSNativeTime");
#endif
    // Every application has access to the NodeState filter
    REGISTER_EXEC_LISTENER_FILTER(NodeStateFilter, "NodeState")

#if HAVE_DEBUG_LISTENER
      // Every application should have access to the Plan Debug Listener
      REGISTER_EXEC_LISTENER(PlanDebugListener, "PlanDebugListener");
#endif

#if HAVE_LUV_LISTENER
    // Every application should have access to the Plexil Viewer (formerly LUV) Listener
    REGISTER_EXEC_LISTENER(LuvListener, "LuvListener");
#endif

    // Every application has access to the default adapter configuration
    REGISTER_ADAPTER_CONFIGURATION(DefaultAdapterConfiguration, "default");
  }

  /**
   * @brief Destructor.
   */
  InterfaceManager::~InterfaceManager()
  {
    // unregister and delete listeners
    m_exec->setExecListenerHub(ExecListenerHubId::noId());
    delete (ExecListenerHub*) m_listenerHub; // deletes listeners too

    // unregister and delete adapters
    // *** kludge for buggy std::set template ***
    std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
    while (it != m_adapters.end()) {
      InterfaceAdapterId ia = *it;
      m_adapters.erase(it); // these two lines should be:
      it = m_adapters.begin(); // it = m_adapters.erase(it)
      delete (InterfaceAdapter*) ia;
    }
    
    // we may not have initialized these!
    if (m_adapterConfig.isId())
      delete m_adapterConfig.operator->();

    if (m_execController.isId()) {
      // shut it down
      m_execController->controllerShutdown();
      delete m_execController.operator->();
    }
  }

  void InterfaceManager::setExec(const PlexilExecId& exec)
  {
    ExternalInterface::setExec(exec);
    exec->setExecListenerHub(m_listenerHub);
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
    assertTrue(m_adapterConfig.isId());
    debugMsg("InterfaceManager:defaultRegisterAdapter", " for adapter " << adapter);
    m_adapterConfig->defaultRegisterAdapter(adapter);
  }

  /**
   * @brief Constructs interface adapters from the provided XML.
   * @param configXml The XML element used for interface configuration.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::constructInterfaces(const pugi::xml_node& configXml)
  {
    if (configXml.empty()) {
      debugMsg("InterfaceManager:constructInterfaces",
               " empty configuration, nothing to construct");
      m_adapterConfig = AdapterConfigurationFactory::createInstance(LabelStr("default"), this);
      return true;
    }

    debugMsg("InterfaceManager:verboseConstructInterfaces", " parsing configuration XML");
    const char* elementType = configXml.name();
    if (!strcmp(elementType, InterfaceSchema::INTERFACES_TAG()) == 0) {
      debugMsg("InterfaceManager:constructInterfaces",
               " invalid configuration XML: no " << InterfaceSchema::INTERFACES_TAG() << " element");
      return false;
    }
    const char* configType =
      configXml.attribute(InterfaceSchema::CONFIGURATION_TYPE_ATTR()).value();
    if (*configType == '\0') {
      m_adapterConfig = AdapterConfigurationFactory::createInstance(LabelStr("default"), this);
    } 
    else {
      m_adapterConfig = AdapterConfigurationFactory::createInstance(LabelStr(configType), this);
    }

    // Walk the children of the configuration XML element
    // and register the adapter according to the data found there
    pugi::xml_node element = configXml.first_child();
    while (!element.empty()) {
      debugMsg("InterfaceManager:verboseConstructInterfaces", " found element " << element.name());
      const char* elementType = element.name();
      if (strcmp(elementType, InterfaceSchema::ADAPTER_TAG()) == 0) {
        // Construct the adapter
        debugMsg("InterfaceManager:constructInterfaces",
                 " constructing adapter type \""
                 << element.attribute(InterfaceSchema::ADAPTER_TYPE_ATTR()).value()
                 << "\"");
        InterfaceAdapterId adapter = 
          AdapterFactory::createInstance(element,
                                         *((AdapterExecInterface*)this));
        if (!adapter.isId()) {
          debugMsg("InterfaceManager:constructInterfaces",
                   " failed to construct adapter type \""
                   << element.attribute(InterfaceSchema::ADAPTER_TYPE_ATTR()).value()
                   << "\"");
          return false;
        }
        m_adapters.insert(adapter);
      }
      else if (strcmp(elementType, InterfaceSchema::LISTENER_TAG()) == 0) {
        // Construct an ExecListener instance and attach it to the Exec
        debugMsg("InterfaceManager:constructInterfaces",
                 " constructing listener type \""
                 << element.attribute(InterfaceSchema::LISTENER_TYPE_ATTR()).value()
                 << "\"");
        ExecListenerId listener = 
          ExecListenerFactory::createInstance(element);
        if (!listener.isId()) {
          debugMsg("InterfaceManager:constructInterfaces",
                   " failed to construct listener from XML");
          return false;
        }
        m_listenerHub->addListener(listener);
      }
      else if (strcmp(elementType, InterfaceSchema::CONTROLLER_TAG()) == 0) {
        // Construct an ExecController instance and attach it to the application
        ExecControllerId controller = 
          ControllerFactory::createInstance(element, m_application);
        if (!controller.isId()) {
          debugMsg("InterfaceManager:constructInterfaces", 
                   " failed to construct controller from XML");
          return false;
        }
        m_execController = controller;
      }
      else if (strcmp(elementType, InterfaceSchema::LIBRARY_NODE_PATH_TAG()) == 0) {
        // Add to library path
        const char* pathstring = element.child_value();
        if (*pathstring != '\0') {
          std::vector<std::string> * path = InterfaceSchema::parseCommaSeparatedArgs(pathstring);
          for (std::vector<std::string>::const_iterator it = path->begin();
               it != path->end();
               ++it)
            m_libraryPath.push_back(*it);
          delete path;
        }
      }
      else if (strcmp(elementType, InterfaceSchema::PLAN_PATH_TAG()) == 0) {
        // Add to plan path
        const char* pathstring = element.child_value();
        if (pathstring != '\0') {
          std::vector<std::string> * path = InterfaceSchema::parseCommaSeparatedArgs(pathstring);
          for (std::vector<std::string>::const_iterator it = path->begin();
               it != path->end();
               ++it)
            m_planPath.push_back(*it);
          delete path;
        }
      }
      else {
        debugMsg("InterfaceManager:constructInterfaces",
                 " ignoring unrecognized XML element \""
                 << elementType << "\"");
      }

      element = element.next_sibling();
    }

    debugMsg("InterfaceManager:verboseConstructInterfaces", " done.");
    return true;
  }

  /**
   * @brief Add an externally constructed interface adapter.
   * @param The adapter ID.
   */
  void InterfaceManager::addInterfaceAdapter(const InterfaceAdapterId& adapter)
  {
    if (m_adapters.find(adapter) == m_adapters.end())
      m_adapters.insert(adapter);
  }

  /**
   * @brief Add an externally constructed ExecListener.
   * @param listener The ExecListener ID.
   */
  void InterfaceManager::addExecListener(const ExecListenerId& listener)
  {
    m_listenerHub->addListener(listener);
  }

  /**
   * @brief Get the search path for library nodes.
   * @return A reference to the library search path.
   */
  const std::vector<std::string>& InterfaceManager::getLibraryPath() const
  {
    return m_libraryPath;
  }

  /**
   * @brief Get the search path for plans.
   * @return A reference to the plan search path.
   */
  const std::vector<std::string>& InterfaceManager::getPlanPath() const
  {
    return m_planPath;
  }

  /**
   * @brief Add the specified directory name to the end of the library node loading path.
   * @param libdir The directory name.
   */
  void InterfaceManager::addLibraryPath(const std::string& libdir)
  {
    m_libraryPath.push_back(libdir);
  }

  /**
   * @brief Add the specified directory names to the end of the library node loading path.
   * @param libdirs The vector of directory names.
   */
  void InterfaceManager::addLibraryPath(const std::vector<std::string>& libdirs)
  {
    for (std::vector<std::string>::const_iterator it = libdirs.begin();
         it != libdirs.end();
         ++it) {
      m_libraryPath.push_back(*it);
    }
  }

  /**
   * @brief Add the specified directory name to the end of the plan loading path.
   * @param libdir The directory name.
   */
  void InterfaceManager::addPlanPath(const std::string& libdir)
  {
    m_planPath.push_back(libdir);
  }

  /**
   * @brief Add the specified directory names to the end of the plan loading path.
   * @param libdirs The vector of directory names.
   */
  void InterfaceManager::addPlanPath(const std::vector<std::string>& libdirs)
  {
    for (std::vector<std::string>::const_iterator it = libdirs.begin();
         it != libdirs.end();
         ++it) {
      m_planPath.push_back(*it);
    }
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
         ++it) {
      InterfaceAdapterId a = *it;
      success = a->initialize();
      if (!success) {
        const pugi::xml_node& adapterXml = a->getXml();
        const char* adapterType = adapterXml.attribute(InterfaceSchema::ADAPTER_TYPE_ATTR()).value();        
        debugMsg("InterfaceManager:initialize",
                 " adapter initialization failed for type \"" << adapterType << "\", returning false");
        m_adapters.erase(it);
        delete (InterfaceAdapter*) a;
        return false;
      }
    }
    success = m_listenerHub->initialize();
    if (!success) {
      debugMsg("InterfaceManager:initialize", " failed to initialize all Exec listeners, returning false");
      return false;
    }

    if (m_execController.isId()) {
      success = m_execController->initialize();
      if (!success) {
        debugMsg("InterfaceManager:initialize", " failed to initialize exec controller, returning false");
        return false;
      }
    }

    return success;
  }

  /**
   * @brief Prepares the interface and adapters for execution.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::start()
  {
    debugMsg("InterfaceManager:start", " starting interface adapters");
    bool success = true;
    for (std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
         success && it != m_adapters.end();
         ++it) {
      success = (*it)->start();
      if (!success) {
        const pugi::xml_node& adapterXml = (*it)->getXml();
        const char* adapterType = adapterXml.attribute(InterfaceSchema::ADAPTER_TYPE_ATTR()).value();        
        debugMsg("InterfaceManager:initialize",
                 " adapter start failed for type \"" << adapterType << "\", returning false");
        return false;
      }
    }

    success = m_listenerHub->start();
    condDebugMsg(!success, 
                 "InterfaceManager:start", " failed to start all Exec listeners, returning false");
    return success;
  }

  /**
   * @brief Halts all interfaces.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::stop()
  {
    debugMsg("InterfaceManager:stop", " entered");

    // halt adapters
    bool success = true;
    for (std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
         it != m_adapters.end();
         ++it)
      success = (*it)->stop() && success;

    success = m_listenerHub->stop() && success;

    debugMsg("InterfaceManager:stop", " completed");
    return success;
  }

  /**
   * @brief Resets the interface prior to restarting.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::reset()
  {
    debugMsg("InterfaceManager:reset", " entered");

    // reset queue etc. to freshly initialized state
    // *** NYI ***

    // clear adapter registry
    clearAdapterRegistry();

    bool success = true;
    for (std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
         it != m_adapters.end();
         ++it)
      success = (*it)->reset() && success;

    success = m_listenerHub->reset() && success;
    debugMsg("InterfaceManager:reset", " completed");
    return success;
  }

  /**
   * @brief Clears the interface adapter registry.
   */
  void InterfaceManager::clearAdapterRegistry() {
    assertTrue(m_adapterConfig.isId());
    m_adapterConfig->clearAdapterRegistry();
  }

  /**
   * @brief Shuts down the interface.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::shutdown()
  {
    debugMsg("InterfaceManager:shutdown", " entered");
    // clear adapter registry
    clearAdapterRegistry();

    bool success = true;
    for (std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
         it != m_adapters.end();
         ++it)
      success = (*it)->shutdown() && success;
    success = m_listenerHub->shutdown() && success;

    // Clean up
    // *** NYI ***

    debugMsg("InterfaceManager:shutdown", " completed");
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
      m_valueQueue.pop();
  }
    
    
  /**
   * @brief Updates the state cache from the items in the queue.
   * @return True if the Exec needs to be stepped, false otherwise.
   * @note Should only be called with exec locked by the current thread
   */
  bool InterfaceManager::processQueue()
  {
    debugMsg("InterfaceManager:processQueue", " entered");

    // Potential optimization (?): these could be member variables
    // Can't use static as that would cause collisions between multiple instances
    Value newValue;
    State state;
    ExpressionId exp;
    PlexilNodeId plan;
    LabelStr parent;
    unsigned int sequence;
    QueueEntryType typ;

    bool needsStep = false;

    while (true) {
      // get next entry
      debugMsg("InterfaceManager:processQueue", " Fetch next queue entry");
      typ = m_valueQueue.dequeue(newValue, 
                                 state,
                                 exp,
                                 plan,
                                 parent,
                                 sequence);
      switch (typ) {
      case queueEntry_EMPTY:
        debugMsg("InterfaceManager:processQueue",
                 " Queue empty, returning " << (needsStep ? "true" : "false"));
        return needsStep;
        break;

      case queueEntry_MARK:
        // Store sequence number and notify application
        m_lastMark = sequence;
        m_application.markProcessed();
        debugMsg("InterfaceManager:processQueue",
                 " Received mark, returning " << (needsStep ? "true" : "false"));
        return needsStep;
        break;

      case queueEntry_LOOKUP_VALUES:
        // State -- update all listeners
        {
          debugMsg("InterfaceManager:processQueue",
                   " Handling state change for " << StateCache::toString(state));

          // If this is a time state update message, check if it's stale
          if (state == m_exec->getStateCache()->getTimeState()) {
            if (newValue.getDoubleValue() <= m_currentTime) {
              debugMsg("InterfaceManager:processQueue",
                       " Ignoring stale time update - new value "
                       << newValue << " is not greater than cached value "
                       << Value::valueToString(m_currentTime));
            }
            else {
              debugMsg("InterfaceManager:processQueue", " setting current time to " << newValue);
              m_currentTime = newValue.getDoubleValue();
              m_exec->getStateCache()->updateState(state, newValue);
            }
          }
          else {
            // General case, update state cache
            m_exec->getStateCache()->updateState(state, newValue);
          }
          needsStep = true;
          break;
        }

      case queueEntry_RETURN_VALUE:
        // Expression -- update the expression only.  Note that this could
        // be either an assignment OR command return value.
        debugMsg("InterfaceManager:processQueue",
                 " Updating expression " << exp
                 << ", new value is '" << newValue << "'");

        // Handle potential command return value.
        this->releaseResourcesAtCommandTermination(exp);

        exp->setValue(newValue);
        needsStep = true;
        break;

      case queueEntry_PLAN:
        // Plan -- add the plan
        debugMsg("InterfaceManager:processQueue", " Received plan");
        if (!getExec()->addPlan(plan, parent)) {
          debugMsg("InterfaceManager:processQueue", " addPlan failed!");
          // TODO: report back to whoever enqueued it
        }
        delete (PlexilNode*) plan;
        needsStep = true;
        break;

      case queueEntry_LIBRARY:
        // Library -- add the library

        debugMsg("InterfaceManager:processQueue",
                 " Received library");
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
    }
  }

  /**
   * @brief Perform an immediate lookup on a new state.
   * @param key The key for the state to be used in future communications about the state.
   */

  Value
  InterfaceManager::lookupNow(const State& state)
  {
    const LabelStr& stateName(state.first);
    debugMsg("InterfaceManager:lookupNow", " of " << StateCache::toString(state));
    InterfaceAdapterId adapter = getLookupInterface(stateName);
    assertTrueMsg(!adapter.isNoId(),
                  "lookupNow: No interface adapter found for lookup '"
                  << stateName.toString() << "'");

    Value result = adapter->lookupNow(state);
    // update internal idea of time if required
    if (state == m_exec->getStateCache()->getTimeState()) {
      if (result.getDoubleValue() <= m_currentTime) {
        debugMsg("InterfaceManager:verboseLookupNow",
                 " Ignoring stale time update - new value "
                 << result << " is not greater than cached value "
                 << Value::valueToString(m_currentTime));
      }
      else {
        debugMsg("InterfaceManager:verboseLookupNow",
                 " setting current time to " << result);
        m_currentTime = result.getDoubleValue();
      }
    }

    debugMsg("InterfaceManager:lookupNow", " of '" << stateName.toString() << "' returning " << result);
    return result;
  }

  /**
   * @brief Inform the interface that it should report changes in value of this state.
   * @param state The state.
   */
  void InterfaceManager::subscribe(const State& state)
  {
    const LabelStr stateName(state.first);
    debugMsg("InterfaceManager:subscribe", " to state " << StateCache::toString(state));
    InterfaceAdapterId adapter = getLookupInterface(stateName);
    assertTrueMsg(!adapter.isNoId(),
                  "subscribe: No interface adapter found for lookup '"
                  << stateName.toString() << "'");
    adapter->subscribe(state);
  }

  /**
   * @brief Inform the interface that a lookup should no longer receive updates.
   */
  void InterfaceManager::unsubscribe(const State& state)
  {
    const LabelStr stateName(state.first);
    debugMsg("InterfaceManager:unsubscribe", " to state " << StateCache::toString(state));
    InterfaceAdapterId adapter = getLookupInterface(stateName);
    assertTrueMsg(!adapter.isNoId(),
                  "unsubscribe: No interface adapter found for lookup '"
                  << stateName.toString() << "'");
    adapter->unsubscribe(state);
  }

  /**
   * @brief Advise the interface of the current thresholds to use when reporting this state.
   * @param state The state.
   * @param hi The upper threshold, at or above which to report changes.
   * @param lo The lower threshold, at or below which to report changes.
   */
  void InterfaceManager::setThresholds(const State& state, double hi, double lo)
  {
    const LabelStr stateName(state.first);
    debugMsg("InterfaceManager:setThresholds", " for state " << StateCache::toString(state));
    InterfaceAdapterId adapter = getLookupInterface(stateName);
    assertTrueMsg(!adapter.isNoId(),
                  "setThresholds: No interface adapter found for lookup '"
                  << stateName.toString() << "'");
    adapter->setThresholds(state, hi, lo);
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
         ++it) {
      CommandId cmd = *it;

      if (!resourceArbiterExists || (acceptCmds.find(cmd) != acceptCmds.end())) {
        condDebugMsg(resourceArbiterExists,
                     "InterfaceManager:batchActions", 
                     " Permission to execute " << cmd->getName()
                     << " has been granted by the resource arbiter.");
        // Maintain a <acks, cmdId> map of commands
        m_ackToCmdMap[cmd->getAck()] = cmd;
        // Maintain a <dest, cmdId> map
        m_destToCmdMap[cmd->getDest()] = cmd;
            
        executeCommand(cmd);
      }
      else {
        commandRejected = true;
        debugMsg("InterfaceManager:batchActions ", 
                 "Permission to execute " << cmd->getName()
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

  // *** To do:
  //  - bookkeeping (i.e. tracking non-acked updates) ?

  void
  InterfaceManager::updatePlanner(std::list<UpdateId>& updates)
  {
    if (updates.empty()) 
      {
        debugMsg("InterfaceManager:updatePlanner", " update list is empty, returning");
        return;
      }
    InterfaceAdapterId intf = this->getPlannerUpdateInterface();
    if (intf.isNoId())
      {
        // Must acknowledge updates if no interface for them
        debugMsg("InterfaceManager:updatePlanner",
                 " no planner update interface defined, acknowledging updates");
        for (std::list<UpdateId>::const_iterator it = updates.begin();
             it != updates.end();
             ++it)
          handleValueChange((ExpressionId) (*it)->getAck(),
                            BooleanVariable::TRUE_VALUE());
        notifyOfExternalEvent();
      }
    else
      {
        for (std::list<UpdateId>::const_iterator it = updates.begin();
             it != updates.end();
             ++it)
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
  }

  // executes a command with the given arguments by looking up the command name 
  // and passing the information to the appropriate interface adapter

  // *** To do:
  //  - bookkeeping (i.e. tracking active commands), mostly for invokeAbort() below
  void
  InterfaceManager::executeCommand(const CommandId& cmd)
  {
    LabelStr name(cmd->getName());
    InterfaceAdapterId intf = getCommandInterface(cmd->getName());
    assertTrueMsg(!intf.isNoId(),
                  "executeCommand: null interface adapter for command " << name.toString());
    intf->executeCommand(cmd);
  }

  // rejects a command due to non-availability of resources
  void 
  InterfaceManager::rejectCommand(const LabelStr& /* name */,
                                  const std::vector<Value>& /* args */,
                                  ExpressionId /* dest */,
                                  ExpressionId ack)
  {
    this->handleValueChange(ack, CommandHandleVariable::COMMAND_DENIED());
  }

  /**
   * @brief Abort the pending command with the supplied name and arguments.
   * @param cmd The command.
   */

  void InterfaceManager::invokeAbort(const CommandId& cmd)
  {
    InterfaceAdapterId intf = getCommandInterface(cmd->getName());
    assertTrueMsg(!intf.isNoId(),
                  "invokeAbort: null interface adapter for command " << cmd->getName());
    intf->invokeAbort(cmd);
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
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->registerCommandInterface(commandName, intf);
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
    assertTrue(m_adapterConfig.isId());
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
    assertTrue(m_adapterConfig.isId());
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
    assertTrue(m_adapterConfig.isId());
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
    assertTrue(m_adapterConfig.isId());
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
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->setDefaultLookupInterface(intf);
  }


  /**
   * @brief Removes the adapter and deletes it iff nothing refers to it.
   */
  void InterfaceManager::deleteIfUnknown(InterfaceAdapterId intf)
  {
    assertTrue(m_adapterConfig.isId());
    if (!m_adapterConfig->isKnown(intf))
      deleteAdapter(intf);
  }

  /**
   * @brief Retract registration of the previous interface adapter for this command.  
   * @param commandName The command.
   */
  void
  InterfaceManager::unregisterCommandInterface(const LabelStr & commandName)
  {
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->unregisterCommandInterface(commandName);
  }

  /**
   * @brief Retract registration of the previous interface adapter for this state.
   * @param stateName The state name.
   */
  void
  InterfaceManager::unregisterLookupInterface(const LabelStr & stateName)
  {
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->unregisterLookupInterface(stateName);
  }

  /**
   * @brief Retract registration of the previous interface adapter for planner updates.
   */
  void
  InterfaceManager::unregisterPlannerUpdateInterface()
  {
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->unregisterPlannerUpdateInterface();
  }

  /**
   * @brief Retract registration of the previous default interface adapter.
   */
  void
  InterfaceManager::unsetDefaultInterface()
  {
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->unsetDefaultInterface();
  }

  /**
   * @brief Retract registration of the previous default interface adapter for commands.
   */
  void
  InterfaceManager::unsetDefaultCommandInterface()
  {
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->unsetDefaultCommandInterface();
  }

  /**
   * @brief Retract registration of the previous default interface adapter for lookups.
   */
  void
  InterfaceManager::unsetDefaultLookupInterface()
  {
    assertTrue(m_adapterConfig.isId());
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
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->getCommandInterface(commandName);
  }

  /**
   * @brief Return the interface adapter in effect for lookups with this state name,
   whether specifically registered or default. May return NoId().
   * @param stateName The state.
   */
  InterfaceAdapterId
  InterfaceManager::getLookupInterface(const LabelStr & stateName)
  {
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->getLookupInterface(stateName);
  }

  /**
   * @brief Return the current default interface adapter. May return NoId().
   */
  InterfaceAdapterId 
  InterfaceManager::getDefaultInterface()
  {
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->getDefaultInterface();
  }

  /**
   * @brief Return the current default interface adapter for commands. May return NoId().
   */
  InterfaceAdapterId 
  InterfaceManager::getDefaultCommandInterface()
  {
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->getDefaultCommandInterface();
  }

  /**
   * @brief Return the current default interface adapter for lookups. May return NoId().
   */
  InterfaceAdapterId 
  InterfaceManager::getDefaultLookupInterface()
  {
    assertTrue(m_adapterConfig.isId());
    return m_adapterConfig->getDefaultLookupInterface();
  }

  /**
   * @brief Return the interface adapter in effect for planner updates,
   whether specifically registered or default. May return NoId().
  */
  InterfaceAdapterId
  InterfaceManager::getPlannerUpdateInterface()
  {
    assertTrue(m_adapterConfig.isId());
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
   * @brief Notify of the availability of a new value for a lookup.
   * @param state The state for the new value.
   * @param value The new value.
   */
  void
  InterfaceManager::handleValueChange(const State& state, const Value& value)
  {
    debugMsg("InterfaceManager:handleValueChange",
             " for state " << state.first.toString()
             << ", new value = " << value);
    m_valueQueue.enqueue(state, value);
  }

  /**
   * @brief Notify of the availability of (e.g.) a command return or acknowledgement.
   * @param exp The expression whose value is being returned.
   * @param value The new value of the expression.
   */
  void 
  InterfaceManager::handleValueChange(const ExpressionId & exp,
                                      const Value& value)
  {
    debugMsg("InterfaceManager:handleValueChange", " for return value entered");
    m_valueQueue.enqueue(exp, value);
  }

  /**
   * @brief Notify the executive of a new plan.
   * @param planXml The XML representation of the new plan.
   * @param parent Label string naming the parent node.
   * @return False if the plan references unloaded libraries, true otherwise.
   */
  bool
  InterfaceManager::handleAddPlan(const pugi::xml_node& planXml,
                                  const LabelStr& parent)
    throw(ParserException)
  {
    debugMsg("InterfaceManager:handleAddPlan", " (XML) entered");

    // check that the plan actually *has* a Node element!
    // Assumes we are starting from the PlexilPlan element.
    checkParserException(!planXml.first_child().empty()
                         && *(planXml.first_child().name()) != '\0'
                         && planXml.child("Node") != NULL,
                         "<" << planXml.name() << "> is not a valid Plexil XML plan");

    // parse the plan
    PlexilNodeId root =
      PlexilXmlParser::parse(planXml.child("Node")); // can also throw ParserException

    return this->handleAddPlan(root, parent);
  }

  /**
   * @brief Notify the executive of a new plan.
   * @param planStruct The PlexilNode representation of the new plan.
   * @param parent The node which is the parent of the new node.
   * @return False if the plan references unloaded libraries, true otherwise.
   */
  bool
  InterfaceManager::handleAddPlan(PlexilNodeId planStruct,
                                  const LabelStr& parent)
  {
    checkError(planStruct.isId(),
               "InterfaceManager::handleAddPlan: Invalid PlexilNodeId");

    debugMsg("InterfaceManager:handleAddPlan", " entered");

    // Check for null
    if (planStruct.isNoId()) {
      debugMsg("InterfaceManager:handleAddPlan", 
               " failed; PlexilNodeId is null");
      return false;
    }

    // Check whether plan is a library w/o a caller
    PlexilInterfaceId interface = planStruct->interface();
    if (interface.isId()) {
      debugMsg("InterfaceManager:handleAddPlan", 
               " for " << planStruct->nodeId() << " failed; root node may not have interface variables");
      return false;
    }

    // Determine if there are any unloaded libraries
    bool result = true;

    // Check whether all libraries for this plan are loaded
    // and try to load those that aren't
    std::vector<std::string> libs = planStruct->getLibraryReferences();
    // N.B. libs is likely growing during this operation, 
    // so we can't use a traditional iterator.
    for (unsigned int i = 0; i < libs.size(); i++) {
      // COPY the string because its location may change out from under us!
      const std::string libname(libs[i]);
      PlexilNodeId libroot = m_exec->getLibrary(libname);
      if (libroot.isNoId()) {
        // Try to load the library
        libroot = PlexilXmlParser::findLibraryNode(libname, m_libraryPath);
        if (libroot.isNoId()) {
          debugMsg("InterfaceManager:handleAddPlan", 
                   " Plan references unloaded library node \"" << libname << "\"");
          delete (PlexilNode*) planStruct;
          return false;
        }
        
        // add the library node
        handleAddLibrary(libroot);
      }

      // Make note of any dependencies in the library itself
      if (libroot.isId())
        libroot->getLibraryReferences(libs);
    }

    if (result) {
      m_valueQueue.enqueue(planStruct, parent);
      debugMsg("InterfaceManager:handleAddPlan", " plan enqueued for loading");
    }
    return result;
  }

  /**
   * @brief Notify the executive of a new plan.
   * @param planStruct The PlexilNode representation of the new plan.
   */
  void
  InterfaceManager::handleAddLibrary(PlexilNodeId planStruct)
  {
    checkError(planStruct.isId(),
               "InterfaceManager::handleAddLibrary: Invalid PlexilNodeId");
    debugMsg("InterfaceManager:handleAddLibrary", " entered");
    m_valueQueue.enqueue(planStruct);
  }

  /**
   * @brief Determine whether the named library is loaded.
   * @return True if loaded, false otherwise.
   */
  bool
  InterfaceManager::isLibraryLoaded(const std::string& libName) const {
    return m_exec->hasLibrary(libName);
  }

  /**
   * @brief Notify the executive that it should run one cycle.  
   */
  void
  InterfaceManager::notifyOfExternalEvent()
  {
    debugMsg("InterfaceManager:notify", " received external event");
    m_application.notifyExec();
  }

#ifdef PLEXIL_WITH_THREADS
  /**
   * @brief Notify the executive that it should run one cycle.  
   */
  void
  InterfaceManager::notifyAndWaitForCompletion()
  {
    m_application.notifyAndWaitForCompletion();
  }
#endif

  //
  // Utility accessors
  //

  StateCacheId
  InterfaceManager::getStateCache() const
  { 
    return m_exec->getStateCache(); 
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
                 << " for the command: " << cmdId->getName());
        
        // Check if the command has a return value. If not, release resources
        // otherwise ignore
        if (cmdId->getDest().isNoId())
          {
            if (getResourceArbiterInterface().isId())
              getResourceArbiterInterface()->releaseResourcesForCommand(cmdId->getName());
            // remove the ack expression from the map
            m_ackToCmdMap.erase(iter);
          }
      }
    else if ((iter = m_destToCmdMap.find(ackOrDest)) != m_destToCmdMap.end())
      {
        CommandId cmdId = iter->second;
        debugMsg("InterfaceManager:releaseResourcesForCommand",
                 " The expression that was received is a valid return value"
                 << " for the command: " << cmdId->getName());

        //Release resources
        if (getResourceArbiterInterface().isId())
          getResourceArbiterInterface()->releaseResourcesForCommand(cmdId->getName());
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


  /**
   * @brief Associate an arbitrary object with a string.
   * @param name The string naming the property.
   * @param thing The property value as an untyped pointer.
   */
  void InterfaceManager::setProperty(const std::string& name, void * thing)
  {
    m_propertyMap[name] = thing;
  }

  /**
   * @brief Fetch the named property.
   * @param name The string naming the property.
   * @return The property value as an untyped pointer.
   */
  void* InterfaceManager::getProperty(const std::string& name)
  {
    PropertyMap::const_iterator it = m_propertyMap.find(name);
    if (it == m_propertyMap.end())
      return NULL;
    else
      return it->second;
  }

}
