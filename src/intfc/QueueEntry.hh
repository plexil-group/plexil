// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_QUEUE_ENTRY_HH
#define PLEXIL_QUEUE_ENTRY_HH

#include "Value.hh"

namespace PLEXIL
{
  // Forward declarations
  class Command;
  struct Message;
  class NodeImpl;
  class State;
  class Update;

  //! \brief Enumeration representing the purpose of an item in the queue.
  //! \ingroup External-Interface
  enum QueueEntryType {
    Q_UNINITED = 0,         //!< Value to mark an uninitialized QueueEntryType value.
    Q_LOOKUP,               //!< A Lookup return value.
    Q_COMMAND_ACK,          //!< A command handle (status) value.
    Q_COMMAND_RETURN,       //!< A command return value.
    Q_COMMAND_ABORT,        //!< A command abort acknowledgement value.
    Q_UPDATE_ACK,           //!< A planner update acknowledgement value.
    Q_ADD_PLAN,             //!< A plan to be executed.
    Q_RECEIVE_MSG,          //!< A message to receive.
    Q_ACCEPT_MSG,           //!< A message being accepted.
    Q_RELEASE_MSG_HANDLE,   //!< A message handle which can now be released.
    Q_MSG_QUEUE_EMPTY,      //!< The message queue is empty.
    Q_MARK,                 //!< A marked position in the queue.

    Q_INVALID               //!< The first value above the valid range of QueueEntryType values.
  };

  //! \struct QueueEntry
  //! \brief Discriminated union representing external state to send to the Exec.
  //! \see InputQueue
  //! \ingroup External-Interface
  struct QueueEntry final
  {
    QueueEntry *next;           //!< Pointer to the next item in the queue.
    union {
      Command *command;         //!< Only valid if type is one of Q_COMMAND_ACK, Q_COMMAND_RETURN, Q_COMMAND_ABORT.
      Message *message;         //!< Only valid if type is one of Q_RECEIVE_MSG, Q_ACCEPT_MSG.
      NodeImpl *plan;           //!< Only valid if type is Q_ADD_PLAN.
      State *state;             //!< Only valid if type is Q_LOOKUP.
      Update *update;           //!< Only valid if type is Q_UPDATE_ACK.
      unsigned int sequence;    //!< Only valid if type is Q_MARK.
    };

    //! \brief The value associated with the command, update, state, or message handle.
    //!        Not valid if type is Q_ADD_PLAN, Q_MARK, or Q_MSG_QUEUE_EMPTY.
    Value value;
    QueueEntryType type;        //!< The type of this entry.

    //! \brief Default constructor.
    QueueEntry();

    //! \brief Copy constructor.
    QueueEntry(QueueEntry const &) = default;

    //! \brief Move constructor.
    QueueEntry(QueueEntry &&) = default;

    //! \brief Copy assignment operator.
    QueueEntry &operator=(QueueEntry const &) = default;

    //! \brief Move assignment operator.
    QueueEntry &operator=(QueueEntry &&) = default;

    //! \brief Destructor.
    ~QueueEntry() = default;

    //
    // Member functions.
    //

    //! \brief Reset the entry to a blank state.
    //!        Type is set to Q_UNINITED, value to unknown, the union to NULL.
    void reset();

    ///@{
    //! \brief Prepare the entry for a lookup value return.
    //! \param st The State whose value is being returned.
    //! \param val The return value.
    void initForLookup(State const &st, Value const &val);
    void initForLookup(State const &st, Value &&val);
    void initForLookup(State &&st, Value const &val);
    void initForLookup(State &&st, Value &&val);
    ///@}

    //! \brief Prepare the entry for a command handle (acknowledgement) return.
    //! \param st The Command.
    //! \param val The return value.
    void initForCommandAck(Command *cmd, CommandHandleValue val);

    //! \brief Prepare the entry for a command return value.
    //! \param st The Command.
    //! \param val The return value.
    void initForCommandReturn(Command *cmd, Value const &val);

    //! \brief Prepare the entry for a command return value.
    //! \param st The Command.
    //! \param val The return value.
    void initForCommandReturn(Command *cmd, Value &&val);

    //! \brief Prepare the entry for a command abort acknowledgement return value.
    //! \param st The Command.
    //! \param ack The return value.
    void initForCommandAbort(Command *cmd, bool ack);

    //! \brief Prepare the entry for a planner update acknowledgement return value.
    //! \param st The Update.
    //! \param ack The return value.
    void initForUpdateAck(Update *upd, bool ack);

    //! \brief Prepare the entry for a plan to be added.
    //! \param p Pointer to the root Node of the plan.
    void initForAddPlan(NodeImpl *p);

    //! \brief Prepare the entry for a message being received.
    //! \param msg Pointer to the Message.
    void initForReceiveMessage(Message *msg);

    //! \brief Prepare the entry for a message being accepted.
    //! \param msg Pointer to the Message.
    //! \param handle Const reference to a string value used as the "handle".
    void initForAcceptMessage(Message *msg, std::string const &handle);

    //! \brief Prepare the entry for a message handle to release.
    //! \param handle Const reference to a string value used as the "handle".
    void initForReleaseMessageHandle(std::string const &handle);

    //! \brief Prepare the entry for the message queue being empty.
    void initForMessageQueueEmpty();

    //! \brief Prepare the entry for a queue marker.
    //! \param seq The sequence number of the marker.
    void initForMark(unsigned int seq);

  };

} // namespace PLEXIL

#endif // PLEXIL_QUEUE_ENTRY_HH
