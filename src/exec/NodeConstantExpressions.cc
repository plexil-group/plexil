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

#include "NodeConstantExpressions.hh"

#include "Error.hh"

namespace PLEXIL
{
  NodeStateConstant::NodeStateConstant(NodeState value)
    : Constant<uint16_t>((uint16_t) value)
  {
    assertTrue_2(isNodeStateValid(value), "NodeStateConstant constuctor: Invalid NodeState value");
  }

  NodeStateConstant::~NodeStateConstant()
  {
  }

  const ValueType NodeStateConstant::valueType() const
  {
    return NODE_STATE_TYPE;
  }

  char const *NodeStateConstant::exprName() const
  {
    return "NodeStateValue";
  }

  NodeOutcomeConstant::NodeOutcomeConstant(NodeOutcome value)
    : Constant<uint16_t>((uint16_t) value)
  {
    assertTrue_2(isNodeOutcomeValid(value), "NodeOutcomeConstant constuctor: Invalid NodeOutcome value");
  }

  NodeOutcomeConstant::~NodeOutcomeConstant()
  {
  }

  const ValueType NodeOutcomeConstant::valueType() const
  {
    return OUTCOME_TYPE;
  }

  char const *NodeOutcomeConstant::exprName() const
  {
    return "NodeOutcomeValue";
  }

  FailureTypeConstant::FailureTypeConstant(FailureType value)
    : Constant<uint16_t>((uint16_t) value)
  {
    assertTrue_2(isFailureTypeValid(value), "FailureTypeConstant constuctor: Invalid FailureType value");
  }

  FailureTypeConstant::~FailureTypeConstant()
  {
  }

  const ValueType FailureTypeConstant::valueType() const
  {
    return FAILURE_TYPE;
  }

  char const *FailureTypeConstant::exprName() const
  {
    return "FailureTypeValue";
  }

  CommandHandleConstant::CommandHandleConstant(CommandHandleValue value)
    : Constant<uint16_t>((uint16_t) value)
  {
  }

  CommandHandleConstant::~CommandHandleConstant()
  {
  }

  const ValueType CommandHandleConstant::valueType() const
  {
    return COMMAND_HANDLE_TYPE;
  }

  char const *CommandHandleConstant::exprName() const
  {
    return "CommandHandleValue";
  }

} // namespace PLEXIL
