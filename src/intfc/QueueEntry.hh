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

  enum QueueEntryType {
    Q_UNINITED = 0,
    Q_LOOKUP,
    Q_COMMAND_ACK,
    Q_COMMAND_RETURN,
    Q_COMMAND_ABORT,
    Q_UPDATE_ACK,
    Q_ADD_PLAN,
    Q_RECEIVE_MSG,
    Q_ACCEPT_MSG,
    Q_RELEASE_MSG_HANDLE,
    Q_MSG_QUEUE_EMPTY,
    Q_MARK,

    Q_INVALID
  };

  struct QueueEntry final
  {
    QueueEntry *next;
    union {
      Command *command;
      Message *message;
      NodeImpl *plan;
      State *state;
      Update *update;
      unsigned int sequence;
    };
    Value value;
    QueueEntryType type;

    QueueEntry();
    QueueEntry(QueueEntry const &) = default;
    QueueEntry(QueueEntry &&) = default;
    QueueEntry &operator=(QueueEntry const &) = default;
    QueueEntry &operator=(QueueEntry &&) = default;

    ~QueueEntry() = default;

    void reset();

    void initForLookup(State const &st, Value const &val);
    void initForLookup(State const &st, Value &&val);
    void initForLookup(State &&st, Value const &val);
    void initForLookup(State &&st, Value &&val);

    void initForCommandAck(Command *cmd, CommandHandleValue val);
    void initForCommandReturn(Command *cmd, Value const &val);
    void initForCommandReturn(Command *cmd, Value &&val);
    void initForCommandAbort(Command *cmd, bool ack);

    void initForUpdateAck(Update *upd, bool ack);

    void initForAddPlan(NodeImpl *p);

    void initForReceiveMessage(Message *msg);
    void initForAcceptMessage(Message *msg, std::string const &handle);
    void initForReleaseMessageHandle(std::string const &handle);
    void initForMessageQueueEmpty();

    void initForMark(unsigned int seq);

  };

} // namespace PLEXIL

#endif // PLEXIL_QUEUE_ENTRY_HH
