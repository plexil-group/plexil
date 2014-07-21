/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#include "CommandHandleVariable.hh"

#include "Command.hh"

namespace PLEXIL
{

  //
  // CommandHandleVariable
  //

  CommandHandleVariable::CommandHandleVariable(Command const &cmd)
    : NotifierImpl(),
      ExpressionImpl<uint16_t>(),
      m_command(cmd)
  {
  }

  CommandHandleVariable::~CommandHandleVariable()
  {
  }

  void CommandHandleVariable::setName(std::string const &name)
  {
    m_name = name;
  }

  const std::string& CommandHandleVariable::getName() const
  {
    return m_name;
  }

  bool CommandHandleVariable::isKnown() const
  {
    return NO_COMMAND_HANDLE != m_command.getCommandHandle();
  }

  bool CommandHandleVariable::getValueImpl(uint16_t &result) const
  {
    if (!isActive())
      return false;
    uint16_t handle = m_command.getCommandHandle();
    if (handle == NO_COMMAND_HANDLE)
      return false;
    result = handle;
    return true;
  }

  void CommandHandleVariable::valueChanged()
  {
    this->publishChange(static_cast<Expression const *>(this));
  }
  
  void CommandHandleVariable::printValue(std::ostream &s) const
  {
    uint16_t handle;
    if (!isActive()
        || (handle = m_command.getCommandHandle()) == NO_COMMAND_HANDLE)
      s << "UNKNOWN";
    else
      s << commandHandleValueName((CommandHandleValue) handle);
  }

} // namespace PLEXIL
