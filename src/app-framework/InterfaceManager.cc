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
//  - utilities for adapters?
//

#include "InterfaceManager.hh"

#include <plexil-config.h>

#include "AdapterConfiguration.hh"
#include "CachedValue.hh"
#include "Command.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExecApplication.hh"
#include "InputQueue.hh"
#include "InterfaceAdapter.hh"
#include "Node.hh"
#include "parsePlan.hh"
#include "parser-utils.hh"
#include "planLibrary.hh"
#include "PlexilExec.hh"
#include "PlexilSchema.hh"
#include "QueueEntry.hh"
#include "StateCacheEntry.hh"
#include "StateCacheMap.hh"
#include "Update.hh"

#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>

namespace PLEXIL
{

  /**
   * @brief Default constructor.
   */
  InterfaceManager::InterfaceManager(ExecApplication &app)
    : ExternalInterface(),
      AdapterExecInterface(),
      m_application(app),
      m_inputQueue(NULL), // configurable
      m_currentTime(std::numeric_limits<double>::min()),
      m_lastMark(0),
      m_markCount(0)
  {
  }

  /**
   * @brief Destructor.
   */
  InterfaceManager::~InterfaceManager()
  {
    delete m_inputQueue;
  }

  //
  // Top-level loop
  //

  /**
   * @brief Performs basic initialization of the interface and all adapters.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::initialize()
  {
    if (!g_configuration)
      return false;
    bool result = g_configuration->initialize();
    m_inputQueue = g_configuration->getInputQueue();
    if (!m_inputQueue)
      return false;
    return result;
  }

  /**
   * @brief Prepares the interface and adapters for execution.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::start()
  {
    assertTrue_1(g_configuration);
    return g_configuration->start();
  }

  /**
   * @brief Halts all interfaces.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::stop()
  {
    assertTrue_1(g_configuration);
    return g_configuration->stop();
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

    assertTrue_1(g_configuration);
    return g_configuration->reset();
  }

  /**
   * @brief Shuts down the interface.
   * @return true if successful, false otherwise.
   */
  bool InterfaceManager::shutdown()
  {
    assertTrue_1(g_configuration);
    bool success = g_configuration->stop();

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
    assertTrue_1(m_inputQueue);
    m_inputQueue->flush();
  }
    
    
  /**
   * @brief Updates the state cache from the items in the queue.
   * @return True if the Exec needs to be stepped, false otherwise.
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
        m_lastMark = entry->sequence;
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
          debugMsg("InterfaceManager:processQueue",
                   " setting current time to " << std::setprecision(15) << newValue);
          m_currentTime = newValue;
        }

        g_interface->lookupReturn(*(entry->state), entry->value);
        needsStep = true;
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
          Node *pid = entry->plan;
          assertTrue_1(pid);
          debugMsg("InterfaceManager:processQueue",
                   " adding plan " << entry->plan->getNodeId());
          g_exec->addPlan(pid);
        }
        entry->plan = NULL;
        needsStep = true;
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
    InterfaceAdapter *adapter = g_configuration->getLookupInterface(state.name());
    if (!adapter) {
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
               " setting current time to " << std::setprecision(15) << newTime);
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
    InterfaceAdapter *adapter = g_configuration->getLookupInterface(state.name());
    if (!adapter) {
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
    InterfaceAdapter *adapter = g_configuration->getLookupInterface(state.name());
    if (!adapter) {
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
    InterfaceAdapter *adapter = g_configuration->getLookupInterface(state.name());
    if (!adapter) {
      warn("setThresholds: No interface adapter found for lookup "
           << state);
      return;
    }
    adapter->setThresholds(state, hi, lo);
  }

  void InterfaceManager::setThresholds(const State& state, int32_t hi, int32_t lo)
  {
    debugMsg("InterfaceManager:setThresholds", " for state " << state);
    InterfaceAdapter *adapter = g_configuration->getLookupInterface(state.name());
    if (!adapter) {
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
    InterfaceAdapter *intf = g_configuration->getPlannerUpdateInterface();
    if (!intf) {
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
    InterfaceAdapter *intf = g_configuration->getCommandInterface(cmd->getName());
    if (intf) {
      intf->executeCommand(cmd);
    }
    else {
      // return failed status
      warn("executeCommand: null interface adapter for command " << cmd->getName());
      g_interface->commandHandleReturn(cmd, COMMAND_FAILED);
    }
  }

  /**
   * @brief Report the failure in the appropriate way for the application.
   */
  void InterfaceManager::reportCommandArbitrationFailure(Command *cmd)
  {
    this->handleCommandAck(cmd, COMMAND_DENIED);
  }

  /**
   * @brief Abort one command in execution.
   * @param cmd The command.
   */
  void InterfaceManager::invokeAbort(Command *cmd)
  {
    InterfaceAdapter *intf = g_configuration->getCommandInterface(cmd->getName());
    if (intf) {
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
    return m_currentTime;
  }

  double
  InterfaceManager::queryTime()
  {
    assertTrue_1(g_configuration);
    debugMsg("InterfaceManager:queryTime", " called");
    StateCacheEntry *cacheEntry = 
      StateCacheMap::instance().ensureStateCacheEntry(State::timeState());
    this->lookupNow(State::timeState(), *cacheEntry); // sets m_current time as side effect
    return m_currentTime;
  }

  //
  // API to interface adapters
  //

  /**
   * @brief Notify of the availability of a new value for a lookup.
   * @param state The state for the new value.
   * @param value The new value.
   */
  void
  InterfaceManager::handleValueChange(const State& state, const Value& value)
  {
    assertTrue_1(m_inputQueue);
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
    assertTrue_1(m_inputQueue);
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
    assertTrue_1(m_inputQueue);
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
    assertTrue_1(m_inputQueue);
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
    assertTrue_1(m_inputQueue);
    debugMsg("InterfaceManager:handleUpdateAck",
             " for node " << upd->getSource()->getNodeId()
             << ", ack = " << (ack ? "true" : "false"));
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    entry->initForUpdateAck(upd, ack);
    m_inputQueue->put(entry);
  }

  unsigned int InterfaceManager::markQueue()
  {
    assertTrue_1(m_inputQueue);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    unsigned int sequence = ++m_markCount;
    entry->initForMark(sequence);
    m_inputQueue->put(entry);
    debugMsg("InterfaceManager:markQueue",
             " sequence # " << sequence);
    return sequence;
  }

  /**
   * @brief Notify the executive of a new plan.
   * @param planXml The XML representation of the new plan.
   */
  void
  InterfaceManager::handleAddPlan(pugi::xml_node const planXml)
    throw (ParserException)
  {
    assertTrue_1(m_inputQueue);
    debugMsg("InterfaceManager:handleAddPlan", " entered");

    // check that the plan actually *has* a Node element!
    // Assumes we are starting from the PlexilPlan element.
    checkParserException(planXml && hasChildElement(planXml),
                         "Plan is empty or malformed");
    checkParserExceptionWithLocation(testTag(PLEXIL_PLAN_TAG, planXml),
                                     planXml,
                                     "Not a PLEXIL Plan");

    // parse the plan
    Node *root = parsePlan(planXml); // can also throw ParserException

    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    entry->initForAddPlan(root);
    m_inputQueue->put(entry);
    debugMsg("InterfaceManager:handleAddPlan", " plan enqueued for loading");
  }

  /**
   * @brief Notify the executive of a new library node.
   * @param doc The XML document containing the library node.
   */
  void
  InterfaceManager::handleAddLibrary(pugi::xml_document *doc)
    throw (ParserException)
  {
    assertTrue_1(m_inputQueue);
    checkError(doc,
               "InterfaceManager::handleAddLibrary: Null plan document");
    
    // Parse just far enough to extract name
    pugi::xml_node plan = doc->document_element();
    pugi::xml_node node;
    checkParserExceptionWithLocation(testTag(PLEXIL_PLAN_TAG, plan)
                                     && (node = plan.child(NODE_TAG)),
                                     plan,
                                     "handleAddLibrary: Input is not a PLEXIL plan");
    pugi::xml_node nodeIdElt = node.child(NODEID_TAG);
    checkParserExceptionWithLocation(nodeIdElt,
                                     node,
                                     "handleAddLibrary: Root node lacks " << NODEID_TAG << " element");
    const char *name = nodeIdElt.child_value();
    checkParserExceptionWithLocation(*name,
                                     nodeIdElt,
                                     "handleAddLibrary: " << NODEID_TAG << " element is empty");
    addLibraryNode(name, doc);

    debugMsg("InterfaceManager:handleAddLibrary", " library node " << name << " added");
  }

  /**
   * @brief Load the named library from the library path.
   * @param libname Name of the library node.
   * @return True if successful, false if not found.
   */
  bool
  InterfaceManager::handleLoadLibrary(std::string const &libName)
      throw (ParserException) 
  {
    if (loadLibraryNode(libName.c_str()))
      return true;
    return getLibraryNode(libName.c_str(), false);
  }


  /**
   * @brief Determine whether the named library is loaded.
   * @return True if loaded, false otherwise.
   */
  bool
  InterfaceManager::isLibraryLoaded(const std::string &libName) const
  {
    return getLibraryNode(libName.c_str(), false);
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

  // Initialize global variable
  InterfaceManager *g_manager = NULL;

}
