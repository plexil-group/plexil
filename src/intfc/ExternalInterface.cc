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

#include "ExternalInterface.hh"

#include "CommandImpl.hh"
#include "Debug.hh"
#include "Error.hh"
#include "LookupReceiver.hh"
#include "StateCache.hh"
#include "Update.hh"

namespace PLEXIL
{
  // Define global variable
  ExternalInterface *g_interface = nullptr;

  void ExternalInterface::lookupReturn(State const &state, Value const &value)
  {
    debugMsg("ExternalInterface:lookupReturn", '(' << state << ", " << value << ')');
    StateCache::instance().lookupReturn(state, value);
  }

  void ExternalInterface::commandReturn(Command *cmd, Value const &value)
  {
    CommandImpl *impl = dynamic_cast<CommandImpl *>(cmd);
    assertTrue_1(impl);
    impl->returnValue(value);
  }

  void ExternalInterface::commandHandleReturn(Command *cmd, CommandHandleValue val)
  {
    CommandImpl *impl = dynamic_cast<CommandImpl *>(cmd);
    assertTrue_1(impl);
    impl->setCommandHandle(val);
  }

  void ExternalInterface::commandAbortAcknowledge(Command *cmd, bool ack)
  {
    CommandImpl *impl = dynamic_cast<CommandImpl *>(cmd);
    assertTrue_1(impl);
    impl->acknowledgeAbort(ack);
  }

  void ExternalInterface::acknowledgeUpdate(Update *upd, bool val)
  {
    upd->acknowledge(val);
  }

  //! Receive notification of a message becoming available.
  //! @param msg Const pointer to the new message.
  //! @note Populates the PeekAtMessage and PeekAtMessageSender states.
  void ExternalInterface::messageReceived(Message const *msg)
  {
    StateCache::instance().messageReceived(msg);
  }

  //! Receive notification that the message queue is empty.
  void ExternalInterface::messageQueueEmpty()
  {
    StateCache::instance().messageQueueEmpty();
  }

  //! Accept an incoming message and associate it with the handle.
  //! @param msg Pointer to the message. StateCache takes ownership of the message.
  //! @param handle String used as a handle for the message.
  void ExternalInterface::assignMessageHandle(Message *msg, std::string const &handle)
  {
    StateCache::instance().assignMessageHandle(msg, handle);
  }

  //! Release the message handle, and clear the message data
  //! associated with that handle.
  //! @param handle The handle being released.
  void ExternalInterface::releaseMessageHandle(std::string const &handle)
  {
    StateCache::instance().releaseMessageHandle(handle);
  }

}
