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

//
// Implementation of the message handling system
//

#ifndef PLEXIL_MESSAGE_HH
#define PLEXIL_MESSAGE_HH

#include "State.hh"

namespace PLEXIL
{

  //! \struct Message
  //! \brief Represents one entry in the incoming message queue.
  struct Message final
  {
    //! \brief The message content, in the form of a State instance.
    State const message;

    //! \brief A string identifying the source of the message.
    std::string const sender;

    //! \brief The time the message was received, represented as a
    //! double precision floating point number.
    double const timestamp;

    Message(State const &msg, std::string const &from, double received)
      : message(msg),
        sender(from),
        timestamp(received)
    {}

    //! \brief Destructor.
    ~Message() = default;

    // Default, copy, move constructors, assignment operators not implemented.
    Message() = delete;
    Message(Message const &) = delete;
    Message(Message &&) = delete;
    Message &operator=(Message const &) = delete;
    Message &operator=(Message &&) = delete;
  };

} // namespace PLEXIL

#endif // PLEXIL_MESSAGE_HH
