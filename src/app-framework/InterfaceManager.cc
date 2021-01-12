/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "AdapterConfiguration.hh"
#include "CachedValue.hh"
#include "Command.hh"
#include "Debug.hh"
#include "ExecApplication.hh"
#include "ExecListenerHub.hh"
#include "InputQueue.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceError.hh"
#include "LookupReceiver.hh"
#include "NodeImpl.hh"
#include "parsePlan.hh"
#include "parser-utils.hh"
#include "planLibrary.hh"
#include "PlexilExec.hh"
#include "PlexilSchema.hh"
#include "QueueEntry.hh"
#include "State.hh"
#include "Update.hh"

#include <iomanip>
#include <limits>
#include <sstream>

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

namespace PLEXIL
{

  //! Constructor.
  InterfaceManager::InterfaceManager(ExecApplication *app,
                                     AdapterConfiguration *config)
    : ExternalInterface(),
      AdapterExecInterface(),
      m_application(app),
      m_configuration(config),
      m_inputQueue(),
      m_lastMark(0),
      m_markCount(0)
  {
  }

  //
  // Top-level loop
  //

  //! Performs basic initialization of the interface manager.
  // @return true if successful, false otherwise.
  bool InterfaceManager::initialize()
  {
    m_inputQueue.reset(m_configuration->makeInputQueue());
    return (bool) m_inputQueue;
  }

  //
  // API for exec
  //
    
  //! Updates the Exec's knowledge of the outside world from the items
  //! in the queue.
  //! @return True if the Exec needs to be stepped, false otherwise.
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
        m_application->markProcessed();
        break;

      case Q_LOOKUP:
        assertTrue_1(entry->state);

        debugMsg("InterfaceManager:processQueue",
                 " Received new value " << entry->value << " for " << *(entry->state));

        lookupReturn(*(entry->state), entry->value);
        needsStep = true;
        break;

      case Q_COMMAND_ACK:
        assertTrue_1(entry->command);

        {
          CommandHandleValue handle = NO_COMMAND_HANDLE;
          entry->value.getValue(handle);
          assertTrue_1(handle != NO_COMMAND_HANDLE);
          debugMsg("InterfaceManager:processQueue",
                   " received command handle value "
                   << commandHandleValueName((CommandHandleValue) handle)
                   << " for command " << entry->command->getCommand());
          commandHandleReturn(entry->command, (CommandHandleValue) handle);
        }
        needsStep = true;
        break;

      case Q_COMMAND_RETURN:
        assertTrue_1(entry->command);
        debugMsg("InterfaceManager:processQueue",
                 " received return value " << entry->value
                 << " for command " << entry->command->getCommand());
        commandReturn(entry->command, entry->value);
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
          commandAbortAcknowledge(entry->command, ack);
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
          acknowledgeUpdate(entry->update, ack);
        }
        needsStep = true;
        break;

      case Q_ADD_PLAN:
        {
          NodeImpl *pid = entry->plan;
          assertTrue_1(pid);
          debugMsg("InterfaceManager:processQueue",
                   " adding plan " << entry->plan->getNodeId());
          g_exec->addPlan(pid);
        }
        entry->plan = nullptr;
        needsStep = true;
        break;

      default:
        // Internal error
        errorMsg("InterfaceManager:processQueue: Invalid entry type "
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

  //! Perform an immediate lookup for a state.
  //! @param state The state.
  void 
  InterfaceManager::lookupNow(State const &state, LookupReceiver *rcvr)
  {
    debugMsg("InterfaceManager:lookupNow", " of " << state);
    LookupHandler *handler = m_configuration->getLookupHandler(state.name());
    try {
      handler->lookupNow(state, rcvr);
    }
    catch (InterfaceError const &e) {
      warn("lookupNow: Error performing lookup of " << state << ":\n"
           << e.what() << "\n Returning UNKNOWN");
      rcvr->setUnknown();
    }
  }

  //! Advise the interface of the current thresholds to use when reporting this state.
  //! @param state The state.
  //! @param hi The upper threshold, at or above which to report changes.
  //! @param lo The lower threshold, at or below which to report changes.
  void InterfaceManager::setThresholds(const State& state, Real hi, Real lo)
  {
    debugMsg("InterfaceManager:setThresholds", " for state " << state);
    LookupHandler *handler = m_configuration->getLookupHandler(state.name());
    handler->setThresholds(state, hi, lo);
  }

  void InterfaceManager::setThresholds(const State& state, Integer hi, Integer lo)
  {
    debugMsg("InterfaceManager:setThresholds", " for state " << state);
    LookupHandler *handler = m_configuration->getLookupHandler(state.name());
    handler->setThresholds(state, hi, lo);
  }

  //! Tell the interface that thresholds are no longer in effect for
  //! this state.
  //! @param state The state.
  void InterfaceManager::clearThresholds(const State& state)
  {
    debugMsg("InterfaceManager:clearThresholds", " for state " << state);
    LookupHandler *handler = m_configuration->getLookupHandler(state.name());
    handler->clearThresholds(state);
  }

  // *** To do:
  //  - bookkeeping (i.e. tracking non-acked updates) ?

  //! Pass information from the plan to an outside recipient.
  //! @param update The list of name-value pairs to send.
  void InterfaceManager::executeUpdate(Update *update)
  {
    assertTrue_1(update);
    PlannerUpdateHandler handler = m_configuration->getPlannerUpdateHandler();
    if (!handler) {
      // Fake the ack
      warn("executeUpdate: no handler for updates");
      handleUpdateAck(update, true);
      notifyOfExternalEvent(); // possibility of reentrancy?
      return;
    }
    debugMsg("InterfaceManager:updatePlanner",
             " sending planner update for node "
             << update->getSource()->getNodeId());
    (handler)(update, this);
  }

  //! Issue the given command to the appropriate interface.
  //! @param cmd The comamnd being executed.
  void InterfaceManager::executeCommand(Command *cmd)
  {
    CommandHandler *handler = m_configuration->getCommandHandler(cmd->getName()); 
    try {
      handler->executeCommand(cmd, this);
    }
    catch (InterfaceError const &e) {
      // return error status
      warn("executeCommand: Error executing command " << cmd->getName()
           << ":\n" << e.what());
      commandHandleReturn(cmd, COMMAND_INTERFACE_ERROR);
    }
  }

  //! Report arbitration failure from a command.
  //! @param The command for which arbitiration failed.
  void InterfaceManager::reportCommandArbitrationFailure(Command *cmd)
  {
    this->handleCommandAck(cmd, COMMAND_DENIED);
  }

  //! Abort one command in execution.
  //! @param cmd The command.
  void InterfaceManager::invokeAbort(Command *cmd)
  {
    CommandHandler *handler = m_configuration->getCommandHandler(cmd->getName());
    try {
      handler->abortCommand(cmd, this);
    }
    catch (InterfaceError const &e) {
      warn("invokeAbort: error aborting command " << cmd->getCommand()
           << ":\n" << e.what());
      commandAbortAcknowledge(cmd, false);
    }
  }

  //
  // API to handlers
  //

  //! Notify of the availability of a new value for a lookup.
  // @param state The state for the new value.
  // @param value The new value.
  void
  InterfaceManager::handleValueChange(const State &state, const Value &value)
  {
    debugMsg("InterfaceManager:handleValueChange",
             " for state " << state << ", new value = " << value);

    assertTrue_1(m_inputQueue);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);

    entry->initForLookup(state, value);
    m_inputQueue->put(entry);
  }

  void
  InterfaceManager::handleValueChange(const State &state, Value &&value)
  {
    debugMsg("InterfaceManager:handleValueChange",
             " for state " << state << ", new value = " << value);

    assertTrue_1(m_inputQueue);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);

    entry->initForLookup(state, value);
    m_inputQueue->put(entry);
  }

  void
  InterfaceManager::handleValueChange(State &&state, const Value &value)
  {
    debugMsg("InterfaceManager:handleValueChange",
             " for state " << state << ", new value = " << value);

    assertTrue_1(m_inputQueue);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);

    entry->initForLookup(state, value);
    m_inputQueue->put(entry);
  }

  void
  InterfaceManager::handleValueChange(State &&state, Value &&value)
  {
    debugMsg("InterfaceManager:handleValueChange",
             " for state " << state << ", new value = " << value);

    assertTrue_1(m_inputQueue);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);

    entry->initForLookup(state, value);
    m_inputQueue->put(entry);
  }

  //! Receive a command handle value for a command in execution.
  //! @param cmd Pointer to the Command instance.
  //! @param value The new value.
  void
  InterfaceManager::handleCommandAck(Command * cmd, CommandHandleValue value)
  {
    if (!cmd) {
      warn("handleCommandAck: null command");
      return;
    }

    if (value <= NO_COMMAND_HANDLE || value >= COMMAND_HANDLE_MAX) {
      warn("handleCommandAck: invalid command handle value");
      value = COMMAND_INTERFACE_ERROR;
    }
    debugMsg("InterfaceManager:handleCommandAck",
             " for command " << cmd->getCommand()
             << ", handle = " << commandHandleValueName(value));

    assertTrue_1(m_inputQueue);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);

    entry->initForCommandAck(cmd, value);
    m_inputQueue->put(entry);
  }

  //! Receive a return value from a command.
  //! @param cmd Pointer to the Command instance.
  //! @param value The new value.
  void
  InterfaceManager::handleCommandReturn(Command * cmd, Value const &value)
  {
    if (!cmd) {
      warn("handleCommandReturn: null command");
      return;
    }
    debugMsg("InterfaceManager:handleCommandReturn",
             " for command " << cmd->getCommand()
             << ", value = " << value);

    assertTrue_1(m_inputQueue);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);

    entry->initForCommandReturn(cmd, value);
    m_inputQueue->put(entry);
  }

  void
  InterfaceManager::handleCommandReturn(Command * cmd, Value &&value)
  {
    if (!cmd) {
      warn("handleCommandReturn: null command");
      return;
    }
    debugMsg("InterfaceManager:handleCommandReturn",
             " for command " << cmd->getCommand()
             << ", value = " << value);

    assertTrue_1(m_inputQueue);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);

    entry->initForCommandReturn(cmd, value);
    m_inputQueue->put(entry);
  }

  //! Receive acknowledgement of a command abort.
  //! @param cmd The command being aborted.
  //! @param ack The acknowledgment value.
  void
  InterfaceManager::handleCommandAbortAck(Command * cmd, bool ack)
  {
    if (!cmd) {
      warn("handleCommandAbortAck: null command");
      return;
    }

    debugMsg("InterfaceManager:handleCommandAbortAck",
             " for command " << cmd->getCommand()
             << ", ack = " << (ack ? "true" : "false"));

    assertTrue_1(m_inputQueue);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);

    entry->initForCommandAbort(cmd, ack);
    m_inputQueue->put(entry);
  }

  //! Receive acknowledgement of a planner update.
  //! @param upd Pointer to the Update instance.
  //! @param ack The acknowledgment value.
  void
  InterfaceManager::handleUpdateAck(Update * upd, bool ack)
  {
    if (!upd) {
      warn("handleUpdateAck: null update");
      return;
    }
    
    debugMsg("InterfaceManager:handleUpdateAck",
             " for node " << upd->getSource()->getNodeId()
             << ", ack = " << (ack ? "true" : "false"));

    assertTrue_1(m_inputQueue);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);

    entry->initForUpdateAck(upd, ack);
    m_inputQueue->put(entry);
  }

  //! Place a mark in the input queue.
  //! @return The sequence number of the mark.
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

  //! Receive a new plan and give it to the Exec.
  //! @param planXml The XML representation of the new plan.
  void
  InterfaceManager::handleAddPlan(pugi::xml_node const planXml)
  {
    debugMsg("InterfaceManager:handleAddPlan", " entered");

    // parse the plan
    NodeImpl *root = parsePlan(planXml); // can throw ParserException

    assertTrue_1(m_inputQueue);
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);

    entry->initForAddPlan(root);
    m_inputQueue->put(entry);
    if (m_configuration->getListenerHub())
      m_configuration->getListenerHub()->notifyOfAddPlan(planXml);
    debugMsg("InterfaceManager:handleAddPlan", " plan enqueued for loading");
  }

  //! Receive a new or updated library node.
  //! @param doc The XML document containing the library node.
  //! @return True if successful, false otherwise.
  bool
  InterfaceManager::handleAddLibrary(pugi::xml_document *doc)
  {
    assertTrue_1(m_inputQueue);
    checkError(doc,
               "InterfaceManager::handleAddLibrary: Null plan document");

    // Hand off to librarian
    Library const *l = loadLibraryDocument(doc);
    if (l) {
      pugi::xml_node const node = l->doc->document_element().child(NODE_TAG);
      char const *name = node.child_value(NODEID_TAG);
      if (m_configuration->getListenerHub())
        m_configuration->getListenerHub()->notifyOfAddLibrary(node);
      debugMsg("InterfaceManager:handleAddLibrary", " library node " << name << " added");
      return true;
    }
    else {
      debugMsg("InterfaceManager:handleAddLibrary", " failed");
      return false;
    }
  }

  //! Load the named library from the library path.
  //! @param libname Name of the library node.
  //! @return True if successful, false if not found.
  //
  bool
  InterfaceManager::handleLoadLibrary(std::string const &libName)
  {
    if (loadLibraryNode(libName.c_str()))
      return true;
    return PLEXIL::isLibraryLoaded(libName.c_str());
  }

  //! Determine whether the named library is loaded.
  //! @param libName Name of the library.
  //! @return True if the named library has been loaded, false otherwise.
  bool
  InterfaceManager::isLibraryLoaded(const std::string &libName) const
  {
    return PLEXIL::isLibraryLoaded(libName.c_str());
  }

  //! Notify the application that the executive should run one cycle.  
  void
  InterfaceManager::notifyOfExternalEvent()
  {
    debugMsg("InterfaceManager:notify", " received external event");
    m_application->notifyExec();
  }

#ifdef PLEXIL_WITH_THREADS
  //! Notify the executive that it should run one cycle. Block the
  //! calling thread until all the items in the input queue at the
  //! time of the call have been processed.
  void
  InterfaceManager::notifyAndWaitForCompletion()
  {
    m_application->notifyAndWaitForCompletion();
  }
#endif

}
