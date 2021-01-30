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
#include "Command.hh"
#include "commandUtils.hh"
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
#include "StateCache.hh"
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
    : AdapterExecInterface(),
      m_application(app),
      m_configuration(config),
      m_inputQueue(),
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
  // API to handlers
  //

  //
  // Lookups
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

  //
  // Command API
  //

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

  //
  // Update API
  //

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

  //
  // Message API
  //


  //! Notify the executive that a message has been received.
  //! @param message The message.
  void
  InterfaceManager::notifyMessageReceived(Message *message)
  {
    if (!message) {
      warn("notifyMessageReceived: null message");
      return;
    }
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);

    entry->initForReceiveMessage(message);
    m_inputQueue->put(entry);
  }

  //! Notify the executive that the message queue is empty.
  void
  InterfaceManager::notifyMessageQueueEmpty()
  {
    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    entry->initForMessageQueueEmpty();
    m_inputQueue->put(entry);
  }

  //! Notify the executive that a message has been accepted.
  //! @param message The message
  //! @param handle The message handle.
  void
  InterfaceManager::notifyMessageAccepted(Message *message, std::string const &handle)
  {
    if (!message) {
      warn("notifyMessageAccepted: null message");
      return;
    }
    if (handle.empty()) {
      warn("notifyMessageAccepted: empty handle");
      return;
    }

    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    entry->initForAcceptMessage(message, handle);
    m_inputQueue->put(entry);
  }

  //! Notify the executive that a message handle has been released.
  //! @param handle The message handle.
  void
  InterfaceManager::notifyMessageHandleReleased(std::string const &handle)
  {
    if (handle.empty()) {
      warn("notifyMessageAccepted: empty handle");
      return;
    }

    QueueEntry *entry = m_inputQueue->allocate();
    assertTrue_1(entry);
    entry->initForReleaseMessageHandle(handle);
    m_inputQueue->put(entry);
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
    m_application->listenerHub()->notifyOfAddPlan(planXml);
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
      m_application->listenerHub()->notifyOfAddLibrary(node);
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

  //
  // Notify API
  //

  //! Notify the application that the executive should run one cycle.  
  void
  InterfaceManager::notifyOfExternalEvent()
  {
    debugMsg("InterfaceManager:notify", " received external event");
    m_application->notifyExec();
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
        m_application->markProcessed(entry->sequence);
        break;

      case Q_LOOKUP:
        assertTrue_1(entry->state);

        debugMsg("InterfaceManager:processQueue",
                 " Received new value " << entry->value << " for " << *(entry->state));

        StateCache::instance().lookupReturn(*(entry->state), entry->value);
        needsStep = true;
        break;

      case Q_COMMAND_ACK:
        assertTrue_1(entry->command);

        {
          CommandHandleValue handle = NO_COMMAND_HANDLE;
          assertTrue_1(entry->value.getValue(handle));
          debugMsg("InterfaceManager:processQueue",
                   " received command handle value "
                   << commandHandleValueName((CommandHandleValue) handle)
                   << " for command " << entry->command->getCommand());
          commandHandleReturn(entry->command, handle);
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
          assertTrue_1(entry->value.getValue(ack));
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
          assertTrue_1(entry->value.getValue(ack));
          debugMsg("InterfaceManager:processQueue",
                   " received update ack " << (ack ? "true" : "false")
                   << " for node "
                   << entry->update->getSource()->getNodeId());
          entry->update->acknowledge(ack);
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

      case Q_RECEIVE_MSG:
        StateCache::instance().messageReceived(entry->message);
        needsStep = true;
        break;

      case Q_MSG_QUEUE_EMPTY:
        StateCache::instance().messageQueueEmpty();
        needsStep = true;
        break;
        
      case Q_ACCEPT_MSG:
        {
          std::string handle;
          assertTrue_2(entry->value.getValue(handle),
                       "InterfaceManager::processQueue: message handle is unknown or wrong type");
          StateCache::instance().assignMessageHandle(entry->message, handle);
        }
        entry->message = nullptr;
        needsStep = true;
        break;

      case Q_RELEASE_MSG_HANDLE:
        {
          std::string handle;
          assertTrue_2(entry->value.getValue(handle),
                       "InterfaceManager::processQueue: message handle is unknown or wrong type");
          StateCache::instance().releaseMessageHandle(handle);
        }
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

}
