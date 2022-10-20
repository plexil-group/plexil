/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_COMMAND_HANDLE_HH
#define PLEXIL_COMMAND_HANDLE_HH

#include <string>

namespace PLEXIL
{

  //! \brief Command handle state enumeration.
  //! \note Order must be consistent with ALL_COMMAND_HANDLE_NAMES array.
  //! \see ALL_COMMAND_HANDLE_NAMES
  //! \ingroup Values
  enum CommandHandleValue : uint8_t
    {
      NO_COMMAND_HANDLE = 48,
      COMMAND_SENT_TO_SYSTEM,
      COMMAND_ACCEPTED,
      COMMAND_RCVD_BY_SYSTEM,
      COMMAND_FAILED,
      COMMAND_DENIED,
      COMMAND_SUCCESS,
      COMMAND_ABORTED,
      COMMAND_ABORT_FAILED,
      COMMAND_INTERFACE_ERROR,
      COMMAND_HANDLE_MAX
    };

  //! \brief Parse the given string as a command handle name.
  //! \param name Const reference to the string.
  //! \return The CommandHandle value, or NO_COMMAND_HANDLE if not found.
  //! \ingroup Values
  extern CommandHandleValue parseCommandHandleValue(std::string const &name);

  //! \brief Table of command handle value names.
  //! \note Order must be consistent with CommandHandleValue enum.
  //! \see CommandHandleValue
  //! \ingroup Values
  extern std::string const ALL_COMMAND_HANDLE_NAMES[];

  //! \brief Get the name of this command handle value.
  //! \return Const reference to the name string.
  //! \ingroup Values
  extern std::string const &commandHandleValueName(CommandHandleValue c);

  //! \brief Test whether the value is a valid CommandHandle value.
  //! \param val The value to check.
  //! \return True if valid, false otherwise.
  //! \ingroup Values
  extern bool isCommandHandleValid(unsigned int val);

} // namespace PLEXIL

#endif // PLEXIL_COMMAND_HANDLE_HH
