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

#ifndef PLEXIL_COMMAND_HH
#define PLEXIL_COMMAND_HH

#include "State.hh"
#include "Value.hh"

namespace PLEXIL
{

  //! \class Command
  //! \brief Abstract base class representing the Command API to
  //!        external interfaces.
  //! \note From outside the Exec, Command is intended to be a
  //!       read-only object.
  class Command
  {
  public:

    //! \brief Virtual destructor.
    virtual ~Command() = default;

    //! \brief Accessor for a command's name and parameters, as a
    //!        State instance.
    //! \return Const reference to the State.
    virtual State const &getCommand() const = 0;

    //! \brief Accessor for the command's name.
    //! \return Const reference to the name as a std::string.
    virtual std::string const &getName() const = 0;

    //! \brief Accessor for the command's parameters, if any.
    //! \return Const reference to a std::vector of Value instances.
    virtual std::vector<Value> const &getArgValues() const = 0;

    //! \brief Is this command expected to return a value?
    //! \return true if a return value is expected, false if not.
    //! \note For the benefit of TestExec.
    virtual bool isReturnExpected() const = 0;

  };

  //
  // Public functions for returning command status and values
  //

  //! \brief Return a value from a command
  //! \param cmd Pointer to the Command.
  //! \param value Const reference to the value.
  void commandReturn(Command *cmd, Value const &value);

  //! \brief Return a command handle value for a command.
  //! \param cmd Pointer to the Command.
  //! \param val The command handle value.
  void commandHandleReturn(Command *cmd, CommandHandleValue val);

  //! \brief Return an abort-acknowledge value for a command.
  //! \param cmd Pointer to the Command.
  //! \param ack The acknowledgement value.
  void commandAbortAcknowledge(Command *cmd, bool ack);

}

#endif // PLEXIL_COMMAND_HH
