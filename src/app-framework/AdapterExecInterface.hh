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

#ifndef PLEXIL_ADAPTER_EXEC_INTERFACE_HH
#define PLEXIL_ADAPTER_EXEC_INTERFACE_HH

#include "CommandHandle.hh"
#include "ValueType.hh" // Date typedef

#include <memory>

// forward reference
namespace pugi
{
  class xml_node;
  class xml_document;
}

namespace PLEXIL
{
  // forward references
  class Command;
  struct Message;
  class State;
  class Update;
  class Value;

  //!
  // @brief An abstract base class representing the PLEXIL Exec API
  //        from the interface implementor's point of view.
  //

  class AdapterExecInterface
  {
  public:

    // Virtual destructor
    virtual ~AdapterExecInterface() = default;

    //
    // Lookup API
    //

    //!
    // @brief Notify of the availability of a new value for a lookup.
    // @param state The state for the new value.
    // @param value The new value.
    //
    virtual void handleValueChange(State const &state, const Value &value) = 0;
    virtual void handleValueChange(State const &state, Value &&value) = 0;
    virtual void handleValueChange(State &&state, const Value &value) = 0;
    virtual void handleValueChange(State &&state, Value &&value) = 0;

    //
    // Command API
    //

    //!
    // @brief Notify of the availability of a command handle value for a command.
    // @param cmd Pointer to the Command instance.
    // @param value The new value.
    //
    virtual void handleCommandAck(Command *cmd, CommandHandleValue value) = 0;

    //!
    // @brief Notify of the availability of a return value for a command.
    // @param cmd Pointer to the Command instance.
    // @param value The new value.
    //
    virtual void handleCommandReturn(Command *cmd, Value const &value) = 0;
    virtual void handleCommandReturn(Command *cmd, Value &&value) = 0;

    //!
    // @brief Notify of the availability of a command abort acknowledgment.
    // @param cmd Pointer to the Command instance.
    // @param ack The acknowledgment value.
    //
    virtual void handleCommandAbortAck(Command *cmd, bool ack) = 0;

    //
    // Update API
    //

    //!
    // @brief Notify of the availability of a planner update acknowledgment.
    // @param upd Pointer to the Update instance.
    // @param ack The acknowledgment value.
    //
    virtual void handleUpdateAck(Update *upd, bool ack) = 0;

    //
    // Message API
    //

    //! Notify the executive that a message has been received.
    //! @param message The message.
    virtual void notifyMessageReceived(Message *message) = 0;

    //! Notify the executive that the message queue is empty.
    virtual void notifyMessageQueueEmpty() = 0;

    //! Notify the executive that a message has been accepted.
    //! @param message The message
    //! @param handle The message handle.
    virtual void notifyMessageAccepted(Message *message, std::string const &handle) = 0;

    //! Notify the executive that a message handle has been released.
    //! @param handle The message handle.
    virtual void notifyMessageHandleReleased(std::string const &handle) = 0;

    //
    // Plan API
    //

    //!
    // @brief Notify the executive of a new plan.
    // @param planXml The pugixml representation of the new plan.
    //
    virtual void handleAddPlan(pugi::xml_node const planXml) = 0;

    //! Notify the executive of a new library node.
    //! @param planXml The XML document containing the new library node
    //! @return true if successful, false otherwise.
    //! @note The interface takes ownership of the plan document.
    virtual bool handleAddLibrary(pugi::xml_document *planXml) = 0;

    //
    // Notify API
    //

    //! Notify the executive that it should run one cycle.
    virtual void notifyOfExternalEvent() = 0;

#ifdef PLEXIL_WITH_THREADS
    //!
    // @brief Run the exec and wait until all events in the queue have been processed.
    // 
    // @note Why is this *here*?
    //
    virtual void notifyAndWaitForCompletion() = 0;
#endif
  };

}

#endif // PLEXIL_ADAPTER_EXEC_INTERFACE_HH
