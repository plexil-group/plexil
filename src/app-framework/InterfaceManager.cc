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
#include "AdapterFactory.hh"
#include "CachedValue.hh"
#include "Command.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExecApplication.hh"
#include "ExecListener.hh"
#include "ExecListenerFactory.hh"
#include "ExecListenerFilter.hh"
#include "ExecListenerHub.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceSchema.hh"
#include "Node.hh"
#include "PlexilExec.hh"
#include "PlexilXmlParser.hh"
#include "QueueEntry.hh"
#include "ResourceArbiterInterface.hh"
#include "SimpleInputQueue.hh" // FIXME: add lockable variety
#include "StateCacheEntry.hh"
#include "StateCacheMap.hh"
#include "Update.hh"

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
      m_listenerHub((new ExecListenerHub())->getId()),
      m_adapters(),
      m_raInterface(),
      // *** FIXME *** Allow other types to be specified
      m_inputQueue(new SimpleInputQueue()),
      m_currentTime(std::numeric_limits<double>::min()),
      m_lastMark(0)
  {
  }

  /**
   * @brief Destructor.
   */
  InterfaceManager::~InterfaceManager()
  {
    // unregister and delete adapters
    // *** kludge for buggy std::set template ***
    std::set<InterfaceAdapterId>::iterator it = m_adapters.begin();
    while (it != m_adapters.end()) {
      InterfaceAdapterId ia = *it;
      m_adapters.erase(it); // these two lines should be:
      it = m_adapters.begin(); // it = m_adapters.erase(it)
      delete (InterfaceAdapter*) ia;
    }

    delete m_inputQueue;
  }

  //
  // Top-level loop
  //

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
      return true;
    }

    assertTrue_2(g_configuration.isNoId(),
                 "No AdapterConfiguration instance");

    debugMsg("InterfaceManager:verboseConstructInterfaces", " parsing configuration XML");
    const char* elementType = configXml.name();
    if (strcmp(elementType, InterfaceSchema::INTERFACES_TAG()) != 0) {
      debugMsg("InterfaceManager:constructInterfaces",
               " invalid configuration XML: no " << InterfaceSchema::INTERFACES_TAG() << " element");
      return false;
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
  void InterfaceManager::addLibraryPath(const std::string &libdir)
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
  void InterfaceManager::addPlanPath(const std::string &libdir)
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
    assertTrue_1(g_configuration.isId());
    g_configuration->clearAdapterRegistry();
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
    m_inputQueue->flush();
  }
    
    
  /**
   * @brief Updates the state cache from the items in the queue.
   * @return True if the Exec needs to be stepped, false otherwise.
   * @note Should only be called with exec locked by the current thread
   */
  bool InterfaceManager::processQueue()
  {
    assertTrue_1(m_inputQueue);
    if (m_inputQueue->isEmpty())
      return false;

    bool needsStep = false;
    QueueEntry *entry;
    while ((entry = m_inputQueue->get())) {
      switch (entry->type) {
      case Q_MARK:
        debugMsg("InterfaceManager:processQueue", " Received mark");
        // Store sequence number and notify application
        m_lastMark = entry->sequence; // ???
        m_application.markProcessed();
        break;

      case Q_LOOKUP:
        assertTrue_1(entry->state);
        debugMsg("InterfaceManager:processQueue",
                 " Received new value " << entry->value << " for " << *(entry->state));

        // If this is a time state update message, grab it
        if (*(entry->state) == State::timeState()) {
          // FIXME: assumes time is a double
          double newValue;
          bool known = entry->value.getValue(newValue);
          assertTrue_2(known, "Time cannot be unknown");
#if PARANOID_ABOUT_TIME_DIRECTION
          assertTrue_2(newValue >= m_currentTime, "Time is going backwards!");
#endif
          debugMsg("InterfaceManager:processQueue", " setting current time to " << newValue);
          m_currentTime = newValue;
        }

        g_interface->lookupReturn(*(entry->state), entry->value);
        break;

      case Q_COMMAND_ACK:
        assertTrue_1(entry->command);
        assertTrue_1(entry->value.valueType() == COMMAND_HANDLE_TYPE);
        {
          uint16_t handle;
          bool known = entry->value.getValue(handle);
          assertTrue_1(known);
          assertTrue_1(handle > NO_COMMAND_HANDLE && handle < COMMAND_HANDLE_MAX);

          debugMsg("InterfaceManager:processQueue",
                   " received command handle value "
                   << commandHandleValueName((CommandHandleValue) handle)
                   << " for command " << entry->command->getCommand());
          g_interface->commandHandleReturn(entry->command, (CommandHandleValue) handle);
        }
        needsStep = true;
        break;

      case Q_COMMAND_RETURN:
        assertTrue_1(entry->command);
        debugMsg("InterfaceManager:processQueue",
                 " received return value " << entry->value
                 << " for command " << entry->command->getCommand());
        g_interface->commandReturn(entry->command, entry->value);
        needsStep = true;
        break;

      case Q_COMMAND_ABORT:
        assertTrue_1(entry->command);
        {
          bool ack;
          bool known = entry->value.getValue(ack);
          assertTrue_1(known);
          debugMsg("InterfaceManager:processQueue",
                   " received command abort ack " << (ack ? "true" : "false")
                   << " for command " << entry->command->getCommand());
          g_interface->commandAbortAcknowledge(entry->command, ack);
        }
        needsStep = true;
        break;

      case Q_UPDATE_ACK:
        assertTrue_1(entry->update);
        {
          bool ack;
          bool known = entry->value.getValue(ack);
          assertTrue_1(known);
          debugMsg("InterfaceManager:processQueue",
                   " received update ack " << (ack ? "true" : "false")
                   << " for node "
                   << entry->update->getSource()->getNodeId());
          g_interface->acknowledgeUpdate(entry->update, ack);
        }

      case Q_ADD_PLAN:
        // Plan -- add the plan
        assertTrue_1(entry->plan);
        {
          PlexilNodeId pid = entry->plan->getId();
          assertTrue_1(pid.isValid());
          debugMsg("InterfaceManager:processQueue",
                   " adding plan " << entry->plan->nodeId());
          g_exec->addPlan(pid);
          delete (PlexilNode *)pid;
        }
        entry->plan = NULL;
        needsStep = true;
        break;

      case Q_ADD_LIBRARY:
        // Library -- add the library
        assertTrue_1(entry->plan);
        {
          PlexilNodeId pid = entry->plan->getId();
          assertTrue_1(pid.isValid());
          debugMsg("InterfaceManager:processQueue",
                   " adding library " << entry->plan->nodeId());
          g_exec->addLibraryNode(pid);
        }
        // no need to step here
        break;

      default:
        // error
        checkError(ALWAYS_FAIL,
                   "InterfaceManager:processQueue: Invalid entry type "
                   << entry->type);
        break;
      }

      // Recycle the queue entry
      m_inputQueue->release(entry);
    }

    debugMsg("InterfaceManager:processQueue",
             " Queue empty, returning " << (needsStep ? "true" : "false"));
    return needsStep;
  }

  /**
   * @brief Perform an immediate lookup on a new state.
   * @param key The key for the state to be used in future communications about the state.
   */
  void 
  InterfaceManager::lookupNow(State const &state, StateCacheEntry &cacheEntry)
  {
    debugMsg("InterfaceManager:lookupNow", " of " << state);
    InterfaceAdapterId adapter = g_configuration->getLookupInterface(state.name());
    if (!adapter.isNoId()) {
      warn("lookupNow: No interface adapter found for lookup "
           << state.name() << ", returning UNKNOWN");
      return;
    }
    adapter->lookupNow(state, cacheEntry);
    // update internal idea of time if required
    if (state == State::timeState()) {
      CachedValue const *val = cacheEntry.cachedValue();
      assertTrue_2(val, "Time is unknown");
      double newTime; // FIXME
      assertTrue_2(val->getValue(newTime), "Time is unknown");
#if PARANOID_ABOUT_TIME_DIRECTION
      assertTrue_2(newTime >= m_currentTime, "Time is going backwards!");
#endif
      debugMsg("InterfaceManager:lookupNow",
               " setting current time to " << newTime);
      m_currentTime = newTime;
    }
  }

  /**
   * @brief Inform the interface that it should report changes in value of this state.
   * @param state The state.
   */
  void InterfaceManager::subscribe(const State& state)
  {
    debugMsg("InterfaceManager:subscribe", " to state " << state);
    InterfaceAdapterId adapter = g_configuration->getLookupInterface(state.name());
    if (adapter.isNoId()) {
      warn("subscribe: No interface adapter found for lookup " << state);
      return;
    }
    adapter->subscribe(state);
  }

  /**
   * @brief Inform the interface that a lookup should no longer receive updates.
   */
  void InterfaceManager::unsubscribe(const State& state)
  {
    debugMsg("InterfaceManager:unsubscribe", " to state " << state);
    InterfaceAdapterId adapter = g_configuration->getLookupInterface(state.name());
    if (adapter.isNoId()) {
      warn("unsubscribe: No interface adapter found for lookup " << state);
      return;
    }
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
    debugMsg("InterfaceManager:setThresholds", " for state " << state);
    InterfaceAdapterId adapter = g_configuration->getLookupInterface(state.name());
    if (!adapter.isNoId()) {
      warn("setThresholds: No interface adapter found for lookup "
           << state);
      return;
    }
    adapter->setThresholds(state, hi, lo);
  }

  void InterfaceManager::setThresholds(const State& state, int32_t hi, int32_t lo)
  {
    debugMsg("InterfaceManager:setThresholds", " for state " << state);
    InterfaceAdapterId adapter = g_configuration->getLookupInterface(state.name());
    if (!adapter.isNoId()) {
      warn("setThresholds: No interface adapter found for lookup "
           << state);
      return;
    }
    adapter->setThresholds(state, hi, lo);
  }

  // *** To do:
  //  - bookkeeping (i.e. tracking non-acked updates) ?

  void
  InterfaceManager::executeUpdate(Update *update)
  {
    assertTrue_1(update);
    InterfaceAdapterId intf = g_configuration->getPlannerUpdateInterface();
    if (intf.isNoId()) {
      // Fake the ack
      g_interface->acknowledgeUpdate(update, true);
      return;
    }
    debugMsg("InterfaceManager:updatePlanner",
             " sending planner update for node "
             << update->getSource()->getNodeId());
    intf->sendPlannerUpdate(update);
  }

  // executes a command with the given arguments by looking up the command name 
  // and passing the information to the appropriate interface adapter

  // *** To do:
  //  - bookkeeping (i.e. tracking active commands), mostly for invokeAbort() below
  void
  InterfaceManager::executeCommand(Command *cmd)
  {
    InterfaceAdapterId intf = g_configuration->getCommandInterface(cmd->getName());
    if (intf.isId()) {
      intf->executeCommand(cmd);
    }
    else {
      // return failed status
      warn("executeCommand: null interface adapter for command " << cmd->getName());
      g_interface->commandHandleReturn(cmd, COMMAND_FAILED);
    }
  }

  // rejects a command due to non-availability of resources
  void 
  InterfaceManager::rejectCommand(Command *cmd)
  {
    g_interface->commandHandleReturn(cmd, COMMAND_DENIED);
  }

  /**
   * @brief Abort the pending command with the supplied name and arguments.
   * @param cmd The command.
   */
  void InterfaceManager::invokeAbort(Command *cmd)
  {
    InterfaceAdapterId intf = g_configuration->getCommandInterface(cmd->getName());
    if (intf.isId()) {
      intf->invokeAbort(cmd);
    }
    else {
      warn("invokeAbort: null interface adapter for command " << cmd->getCommand());
      g_interface->commandAbortAcknowledge(cmd, false);
    }
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
   * @brief Removes the adapter and deletes it iff nothing refers to it.
   */
  void InterfaceManager::deleteIfUnknown(InterfaceAdapterId intf)
  {
    assertTrue_1(g_configuration.isId());
    if (!g_configuration->isKnown(intf))
      deleteAdapter(intf);
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
   * @brief Notify of the availability of a new value for a lookup.
   * @param state The state for the new value.
   * @param value The new value.
   */
  void
  InterfaceManager::handleValueChange(const State& state, const Value& value)
  {
    debugMsg("InterfaceManager:handleValueChange",
             " for state " << state << ", new value = " << value);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    entry->initForLookup(state, value);
    m_inputQueue->put(entry);
  }

  void
  InterfaceManager::handleCommandAck(Command * cmd, CommandHandleValue value)
  {
    assertTrue_1(cmd);
    assertTrue_1(value > NO_COMMAND_HANDLE && value < COMMAND_HANDLE_MAX);
    debugMsg("InterfaceManager:handleCommandAck",
             " for command " << cmd->getCommand()
             << ", handle = " << commandHandleValueName(value));
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    entry->initForCommandAck(cmd, value);
    m_inputQueue->put(entry);
  }

  void
  InterfaceManager::handleCommandReturn(Command * cmd, Value const &value)
  {
    assertTrue_1(cmd);
    debugMsg("InterfaceManager:handleCommandReturn",
             " for command " << cmd->getCommand()
             << ", value = " << value);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    entry->initForCommandReturn(cmd, value);
    m_inputQueue->put(entry);
  }

  void
  InterfaceManager::handleCommandAbortAck(Command * cmd, bool ack)
  {
    assertTrue_1(cmd);
    debugMsg("InterfaceManager:handleCommandAbortAck",
             " for command " << cmd->getCommand()
             << ", ack = " << (ack ? "true" : "false"));
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    entry->initForCommandAbort(cmd, ack);
    m_inputQueue->put(entry);
  }

  void
  InterfaceManager::handleUpdateAck(Update * upd, bool ack)
  {
    assertTrue_1(upd);
    debugMsg("InterfaceManager:handleUpdateAck",
             " for node " << upd->getSource()->getNodeId()
             << ", ack = " << (ack ? "true" : "false"));
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    entry->initForUpdateAck(upd, ack);
    m_inputQueue->put(entry);
  }

  /**
   * @brief Notify the executive of a new plan.
   * @param planXml The XML representation of the new plan.
   * @return False if the plan references unloaded libraries, true otherwise.
   */
  bool
  InterfaceManager::handleAddPlan(const pugi::xml_node& planXml)
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

    return this->handleAddPlan(root);
  }

  /**
   * @brief Notify the executive of a new plan.
   * @param planStruct The PlexilNode representation of the new plan.
   * @return False if the plan references unloaded libraries, true otherwise.
   */
  bool
  InterfaceManager::handleAddPlan(PlexilNodeId planStruct)
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
      PlexilNodeId libroot = g_exec->getLibrary(libname);
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
      QueueEntry *entry = m_inputQueue->allocate();
      assertTrue_1(entry);
      entry->initForAddPlan((PlexilNode *) planStruct);
      m_inputQueue->put(entry);
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
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    entry->initForAddPlan((PlexilNode *) planStruct);
    m_inputQueue->put(entry);
    debugMsg("InterfaceManager:handleAddLibrary", " library node enqueued");
  }

  /**
   * @brief Determine whether the named library is loaded.
   * @return True if loaded, false otherwise.
   */
  bool
  InterfaceManager::isLibraryLoaded(const std::string &libName) const {
    return g_exec->getLibrary(libName).isId();
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

  // *************
  // *** FIXME ***
  // *************

  /**
   * @brief update the resoruce arbiter interface that an ack or return value
   * has been received so that resources can be released.
   * @param ackOrDest The expression id for which a value has been posted.
   */
  // void InterfaceManager::releaseResourcesAtCommandTermination(ExpressionId ackOrDest)
  // {
  //   // Check if the expression is an ack or a return value
  //   std::map<ExpressionId, CommandId>::iterator iter;

  //   if ((iter = m_ackToCmdMap.find(ackOrDest)) != m_ackToCmdMap.end())
  //     {
  //       CommandId cmdId = iter->second;
  //       debugMsg("InterfaceManager:releaseResourcesAtCommandTermination",
  //                " The expression that was received is a valid acknowledgement"
  //                << " for the command: " << cmdId->getName());
        
  //       // Check if the command has a return value. If not, release resources
  //       // otherwise ignore
  //       if (cmdId->getDest().isNoId())
  //         {
  //           if (getResourceArbiterInterface().isId())
  //             getResourceArbiterInterface()->releaseResourcesForCommand(cmdId->getName());
  //           // remove the ack expression from the map
  //           m_ackToCmdMap.erase(iter);
  //         }
  //     }
  //   else if ((iter = m_destToCmdMap.find(ackOrDest)) != m_destToCmdMap.end())
  //     {
  //       CommandId cmdId = iter->second;
  //       debugMsg("InterfaceManager:releaseResourcesForCommand",
  //                " The expression that was received is a valid return value"
  //                << " for the command: " << cmdId->getName());

  //       //Release resources
  //       if (getResourceArbiterInterface().isId())
  //         getResourceArbiterInterface()->releaseResourcesForCommand(cmdId->getName());
  //       //remove the ack from the map        
  //       m_ackToCmdMap.erase(m_ackToCmdMap.find(cmdId->getAck()));

  //       //remove the dest from the map
  //       m_destToCmdMap.erase(iter);
  //     }
  //   else
  //     debugMsg("InterfaceManager:releaseResourcesForCommand:",
  //              " The expression is neither an acknowledgement"
  //              << " nor a return value for a command. Ignoring.");

  // }

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
  void InterfaceManager::setProperty(const std::string &name, void * thing)
  {
    m_propertyMap[name] = thing;
  }

  /**
   * @brief Fetch the named property.
   * @param name The string naming the property.
   * @return The property value as an untyped pointer.
   */
  void* InterfaceManager::getProperty(const std::string &name)
  {
    PropertyMap::const_iterator it = m_propertyMap.find(name);
    if (it == m_propertyMap.end())
      return NULL;
    else
      return it->second;
  }

}
