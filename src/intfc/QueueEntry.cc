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

#include "QueueEntry.hh"

#include "State.hh"

namespace PLEXIL
{

  QueueEntry::QueueEntry()
    : next(nullptr),
      command(nullptr),
      value(),
      type(Q_UNINITED)
  {
  }

  void QueueEntry::reset()
  {
    next = nullptr;
    if (type == Q_LOOKUP)
      delete state;
    state = nullptr;
    value.setUnknown();
    type = Q_UNINITED;
  }

  void QueueEntry::initForLookup(State const &stat, Value const &val)
  {
    state = new State(stat); // have to copy 
    value = val;
    type = Q_LOOKUP;
  }

  void QueueEntry::initForLookup(State const &stat, Value &&val)
  {
    state = new State(stat); // have to copy 
    value = std::move(val);
    type = Q_LOOKUP;
  }

  void QueueEntry::initForLookup(State &&stat, Value const &val)
  {
    state = new State(stat); // have to copy 
    value = val;
    type = Q_LOOKUP;
  }

  void QueueEntry::initForLookup(State &&stat, Value &&val)
  {
    state = new State(stat); // have to copy 
    value = std::move(val);
    type = Q_LOOKUP;
  }

  void QueueEntry::initForCommandAck(Command *cmd, CommandHandleValue val)
  {
    command = cmd;
    value = val;
    type = Q_COMMAND_ACK;
  }

  void QueueEntry::initForCommandReturn(Command *cmd, Value const &val)
  {
    command = cmd;
    value = val;
    type = Q_COMMAND_RETURN;
  }

  void QueueEntry::initForCommandReturn(Command *cmd, Value &&val)
  {
    command = cmd;
    value = std::move(val);
    type = Q_COMMAND_RETURN;
  }

  void QueueEntry::initForCommandAbort(Command *cmd, bool ack)
  {
    command = cmd;
    value = ack;
    type = Q_COMMAND_ABORT;
  }

  void QueueEntry::initForUpdateAck(Update *upd, bool ack)
  {
    update = upd;
    value = ack;
    type = Q_UPDATE_ACK;
  }

  void QueueEntry::initForAddPlan(NodeImpl *node)
  {
    plan = node;
    type = Q_ADD_PLAN;
  }

  void QueueEntry::initForReceiveMessage(Message *msg)
  {
    message = msg;
    type = Q_RECEIVE_MSG;
  }

  void QueueEntry::initForAcceptMessage(Message *msg, std::string const &handle)
  {
    message = msg;
    type = Q_ACCEPT_MSG;
  }

  void QueueEntry::initForReleaseMessageHandle(std::string const &handle)
  {
    value = handle;
    type = Q_RELEASE_MSG_HANDLE;
  }

  void QueueEntry::initForMessageQueueEmpty()
  {
    type = Q_MSG_QUEUE_EMPTY;
  }

  void QueueEntry::initForMark(unsigned int seq)
  {
    sequence = seq;
    type = Q_MARK;
  }

} // namespace PLEXIL
