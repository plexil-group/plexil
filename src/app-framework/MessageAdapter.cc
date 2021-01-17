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
// New unified interface for receiving messages and commands
// from external agents.
//

#include "AdapterConfiguration.hh"
#include "AdapterExecInterface.hh"
#include "AdapterFactory.hh"
#include "Command.hh"
#include "CommandHandler.hh"
#include "InterfaceAdapter.hh"
#include "Message.hh"
#include "MessageAdapter.hh"
#include "Timebase.hh"

#include <queue>

namespace PLEXIL
{

  //! @class MessageAdapter
  //! A generalization and rationalization of the functionality
  //! provided by the old IpcAdapter and UdpAdapter.

  //! The MessageAdapter maintains an input queue. Arriving messages
  //! are enqueued along with sender ID (if available) and timestamp.

  class MessageAdapterImpl : virtual public InterfaceAdapter, public MessageAdapter
  {
  public:

    //! Constructor.
    MessageAdapterImpl(AdapterExecInterface &intf, AdapterConf *conf)
      : InterfaceAdapter(intf, conf)
    {
    }

    //! Virtual destructor.
    virtual ~MessageAdapterImpl()
    {
      // Clean up message queue
      while (!m_queue.empty()) {
        Message *msg = m_queue.front();
        m_queue.pop();
        delete msg;
      }
    }

    virtual bool initialize(AdapterConfiguration *config)
    {
      // Register command handlers
      config->registerCommandHandlerFunction("GetMessageHandle",
                                             getMessageHandleHandler);
      config->registerCommandHandlerFunction("ReleaseMessageHandle",
                                             releaseMessageHandleHandler);
      config->registerCommandHandlerFunction("PublishMessage",
                                             publishMessageHandler);
      config->registerCommandHandlerFunction("SendMessage",
                                             sendMessageHandler);
      return true;
    }

    virtual bool start()
    {
      // Begin accepting incoming messages
      // TODO

      return true;
    }

    virtual void stop()
    {
      // Stop accepting incoming messages
      // TODO
    }

    //
    // API to transport adapter
    //

    void receiveMessage(std::string const &sender, State const &message)
    {
      enqueueMessage(message, sender, Timebase::queryTime());
    }

    void receiveMessageAnonymously(State const &message)
    {
      enqueueMessage(message, std::string(), Timebase::queryTime());
    }

    //
    // API to handlers
    //

    //! Pop the message at the head of the queue and send it to the Exec.
    void acceptMessage(Command *cmd, AdapterExecInterface *intf)
    {
      if (m_queue.empty()) {
        // Empty queue -> handle unknown
        intf->handleCommandReturn(cmd, Value());
      }
      else {
        // Generate a handle
        std::string handle;
        // TODO - generate UID?
        intf->notifyMessageAccepted(m_queue.front(), handle);
        m_queue.pop();
        intf->handleCommandReturn(cmd, Value(handle));
        // Post the new state of the queue.
        if (m_queue.empty()) {
          intf->notifyMessageQueueEmpty();
        } else {
          intf->notifyMessageReceived(m_queue.front());
        }
      }
      // In either case, we've done our job
      intf->handleCommandAck(cmd, COMMAND_SUCCESS);
      intf->notifyOfExternalEvent();
    }

    //! Tell the Exec the plan no longer needs that message
    //! @param cmd The command.
    //! @param intf Pointer to the AdapterExecInterface.
    //! @note Command handler has already vetted the parameter.
    void releaseMessageHandle(Command *cmd, AdapterExecInterface *intf)
    {
      intf->notifyMessageHandleReleased(cmd->getCommand().parameter(0).valueToString());
      intf->handleCommandAck(cmd, COMMAND_SUCCESS);
      intf->notifyOfExternalEvent();
    }

    //! Publish the message.
    //! @param cmd The command.
    //! @param intf Pointer to the AdapterExecInterface.
    //! @note Command handler has already vetted the parameter.
    void publishMessage(Command *cmd, AdapterExecInterface *intf)
    {
      // TODO
      intf->handleCommandAck(cmd, COMMAND_SUCCESS);
      intf->notifyOfExternalEvent();
    }

    //! Send the message.
    //! @param cmd The command.
    //! @param intf Pointer to the AdapterExecInterface.
    //! @note Command handler has already vetted the parameters.
    void sendMessage(Command *cmd, AdapterExecInterface *intf)
    {
      // TODO
      intf->handleCommandAck(cmd, COMMAND_SUCCESS);
      intf->notifyOfExternalEvent();
    }

  private:

    //
    // Command handlers
    //

    ExecuteCommandHandler getMessageHandleHandler =
      [this] (Command *cmd, AdapterExecInterface *intf) -> void
      {
        // No parameters, just call the 
        this->acceptMessage(cmd, intf);
      };

    ExecuteCommandHandler releaseMessageHandleHandler =
      [this] (Command *cmd, AdapterExecInterface *intf) -> void
      {
        // Check that there is at least one parameter and that it is a string
        State const &command = cmd->getCommand();
        if (command.parameterCount() < 1) {
          // report arg-count error
          warn("ReleaseMessageHandle command requires exactly one argument");
          intf->handleCommandAck(cmd, COMMAND_FAILED);
          intf->notifyOfExternalEvent();
        }
        if (command.parameterType(0) != STRING_TYPE) {
          // report arg-type error
          warn("ReleaseMessageHandle command: first argument must be a String");
          intf->handleCommandAck(cmd, COMMAND_FAILED);
          intf->notifyOfExternalEvent();
        }
        if (!command.parameter(0).isKnown()) {
          // do nothing, but report success
          intf->handleCommandAck(cmd, COMMAND_SUCCESS);
          intf->notifyOfExternalEvent();
        }

        // All good? Hand it off for implementation.
        this->releaseMessageHandle(cmd, intf);
      };

    ExecuteCommandHandler publishMessageHandler =
      [this] (Command *cmd, AdapterExecInterface *intf) -> void
      {
        // Check that there is at least one parameter and that it is a string
        // TODO
        State const &command = cmd->getCommand();
        if (command.parameterCount() < 1) {
          // report arg-count error
          warn("PublishMessage command requires exactly one argument");
          intf->handleCommandAck(cmd, COMMAND_FAILED);
          intf->notifyOfExternalEvent();
        }
        if (command.parameterType(0) != STRING_TYPE) {
          // report arg-type error
          warn("PublishMessage command: first argument must be a String");
          intf->handleCommandAck(cmd, COMMAND_FAILED);
          intf->notifyOfExternalEvent();
        }
        if (!command.parameter(0).isKnown()) {
          // report arg-unknown error
          warn("PublishMessage command: first argument, message, is unknown");
          intf->handleCommandAck(cmd, COMMAND_FAILED);
          intf->notifyOfExternalEvent();
        }

        // Then hand it off for implementation
        this->publishMessage(cmd, intf);
      };

    ExecuteCommandHandler sendMessageHandler =
      [this] (Command *cmd, AdapterExecInterface *intf) -> void
      {
        // Check that there are at least two parameters and that they are strings
        // Check that there is at least one parameter and that it is a string
        State const &command = cmd->getCommand();
        if (command.parameterCount() < 1) {
          // report arg-count error
          warn("SendMessage command requires at least two arguments");
          intf->handleCommandAck(cmd, COMMAND_FAILED);
          intf->notifyOfExternalEvent();
        }
        if (command.parameterType(0) != STRING_TYPE) {
          // report arg-type error
          warn("SendMessage command: first argument, recipient, must be a String");
          intf->handleCommandAck(cmd, COMMAND_FAILED);
          intf->notifyOfExternalEvent();
        }
        if (!command.parameter(0).isKnown()) {
          // report arg-unknown error
          warn("SendMessage command: first argument, recipient, is unknown");
          intf->handleCommandAck(cmd, COMMAND_FAILED);
          intf->notifyOfExternalEvent();
        }
        if (command.parameterType(1) != STRING_TYPE) {
          // report arg-type error
          warn("SendMessage command: second argument, message, must be a String");
          intf->handleCommandAck(cmd, COMMAND_FAILED);
          intf->notifyOfExternalEvent();
        }
        if (!command.parameter(1).isKnown()) {
          // report arg-unknown error
          warn("SendMessage command: second argument, message, is unknown");
          intf->handleCommandAck(cmd, COMMAND_FAILED);
          intf->notifyOfExternalEvent();
        }

        // Then hand it off for implementation
        this->sendMessage(cmd, intf);
      };

    //
    // Transport implementation
    //

    // Do we need a move variant?
    void enqueueMessage(State const &state, std::string const &sender, double timestamp)
    {
      // check if started?
      bool wasEmpty = m_queue.empty();
      m_queue.push(new Message(state, sender, timestamp));
      if (wasEmpty) {
        // Tell the Exec we have a message for them
        // (populates the PeekAtMessage and PeekAtMessageSender lookups)
        getInterface().notifyMessageReceived(m_queue.front());
        getInterface().notifyOfExternalEvent();
      }
    }
    
    //
    // Member data
    //

    using MessageQueue = std::queue<Message *>;

    MessageQueue m_queue;
  }; // class MessageAdapterImpl

} // namespace PLEXIL

extern "C"
void initMessageAdapter()
{
  REGISTER_ADAPTER(PLEXIL::MessageAdapterImpl, "Message");
}
