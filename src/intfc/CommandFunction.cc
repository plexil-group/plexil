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

#include "CommandFunction.hh"

#include "CommandImpl.hh"
#include "CommandOperator.hh"

namespace PLEXIL
{
  CommandFunction::CommandFunction(CommandOperator const *op, CommandImpl &command)
    : Propagator(),
      m_op(op),
      m_command(command)
  {
  }

  const char *CommandFunction::exprName() const
  {
    return m_op->getName().c_str();
  }

  ValueType CommandFunction::valueType() const
  {
    return m_op->valueType();
  }

  bool CommandFunction::isKnown() const
  {
    // Delegate to operator
    return m_op->isKnown(&m_command);
  }

  void CommandFunction::printValue(std::ostream &s) const
  {
    m_op->printValue(s, &m_command);
  }

  Value CommandFunction::toValue() const
  {
    return m_op->toValue(&m_command);
  }

  void CommandFunction::doSubexprs(ListenableUnaryOperator const &oper)
  {
    m_op->doPropagationSources(&m_command, oper);
  }

  bool CommandFunction::getValue(Boolean &result) const
  {
    return (*m_op)(result, &m_command);
  }

} // namespace PLEXIL
