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
  //
  // NodeStateConstant
  //

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

  // Instantiations
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(NodeStateConstant, INACTIVE_CONSTANT, INACTIVE_STATE);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(NodeStateConstant, WAITING_CONSTANT, WAITING_STATE);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(NodeStateConstant, EXECUTING_CONSTANT, EXECUTING_STATE);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(NodeStateConstant, ITERATION_ENDED_CONSTANT, ITERATION_ENDED_STATE);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(NodeStateConstant, FINISHED_CONSTANT, FINISHED_STATE);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(NodeStateConstant, FAILING_CONSTANT, FAILING_STATE);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(NodeStateConstant, FINISHING_CONSTANT, FINISHING_STATE);

  //
  // NodeOutcomeConstant
  //

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

  // Instantiations

  DEFINE_GLOBAL_CONST_WITH_CLEANUP(NodeOutcomeConstant, SUCCESS_CONSTANT, SUCCESS_OUTCOME);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(NodeOutcomeConstant, FAILURE_CONSTANT, FAILURE_OUTCOME);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(NodeOutcomeConstant, SKIPPED_CONSTANT, SKIPPED_OUTCOME);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(NodeOutcomeConstant, INTERRUPTED_CONSTANT, INTERRUPTED_OUTCOME);

  //
  // FailureTypeConstant
  //

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

  // Instantiations

  DEFINE_GLOBAL_CONST_WITH_CLEANUP(FailureTypeConstant, PRE_CONDITION_FAILED_CONSTANT, PRE_CONDITION_FAILED);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(FailureTypeConstant, POST_CONDITION_FAILED_CONSTANT, POST_CONDITION_FAILED);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(FailureTypeConstant, INVARIANT_CONDITION_FAILED_CONSTANT, INVARIANT_CONDITION_FAILED);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(FailureTypeConstant, PARENT_FAILED_CONSTANT, PARENT_FAILED);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(FailureTypeConstant, EXITED_CONSTANT, EXITED);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(FailureTypeConstant, PARENT_EXITED_CONSTANT, PARENT_EXITED);

  //
  // CommandHandleConstant
  //

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

  // Instantiations

  DEFINE_GLOBAL_CONST_WITH_CLEANUP(CommandHandleConstant, COMMAND_SENT_TO_SYSTEM_CONSTANT, COMMAND_SENT_TO_SYSTEM);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(CommandHandleConstant, COMMAND_ACCEPTED_CONSTANT, COMMAND_ACCEPTED);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(CommandHandleConstant, COMMAND_RCVD_BY_SYSTEM_CONSTANT, COMMAND_RCVD_BY_SYSTEM);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(CommandHandleConstant, COMMAND_FAILED_CONSTANT, COMMAND_FAILED);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(CommandHandleConstant, COMMAND_DENIED_CONSTANT, COMMAND_DENIED);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(CommandHandleConstant, COMMAND_SUCCESS_CONSTANT, COMMAND_SUCCESS);

} // namespace PLEXIL
