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

#ifndef PLEXIL_MESSAGE_ADAPTER_HH
#define PLEXIL_MESSAGE_ADAPTER_HH

#include "InterfaceAdapter.hh"

namespace PLEXIL
{

  //! @class MessageAdapter
  //! A generalization and rationalization of the functionality
  //! provided by the old IpcAdapter and UdpAdapter.

  //! The MessageAdapter maintains an input queue. Arriving messages
  //! are enqueued along with sender ID (if available) and timestamp.

  // This file defines the additional member functions transport
  // implementations can use to pass messages into the PLEXIL Exec.

  class MessageAdapter
  {
  public:
    virtual ~MessageAdapter() = default;

    //
    // API to transport implementation
    //

    virtual void receiveMessage(std::string const &senderId, State const &message) = 0;

    virtual void receiveMessageAnonymously(State const &message) = 0;
  };
  
} // namespace PLEXIL

extern "C"
void initMessageAdapter();

#endif // PLEXIL_MESSAGE_ADAPTER_HH
