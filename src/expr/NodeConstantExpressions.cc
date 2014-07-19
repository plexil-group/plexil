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
#include "PlexilExpr.hh"

#define DEFINE_EXPRESSION_CONSTANT(TYPE, NAME, VALUE)\
  Expression *NAME() { static TYPE sl_ ## NAME(VALUE); return &sl_ ## NAME; }

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
  DEFINE_EXPRESSION_CONSTANT(NodeStateConstant, INACTIVE_CONSTANT, INACTIVE_STATE);
  DEFINE_EXPRESSION_CONSTANT(NodeStateConstant, WAITING_CONSTANT, WAITING_STATE);
  DEFINE_EXPRESSION_CONSTANT(NodeStateConstant, EXECUTING_CONSTANT, EXECUTING_STATE);
  DEFINE_EXPRESSION_CONSTANT(NodeStateConstant, ITERATION_ENDED_CONSTANT, ITERATION_ENDED_STATE);
  DEFINE_EXPRESSION_CONSTANT(NodeStateConstant, FINISHED_CONSTANT, FINISHED_STATE);
  DEFINE_EXPRESSION_CONSTANT(NodeStateConstant, FAILING_CONSTANT, FAILING_STATE);
  DEFINE_EXPRESSION_CONSTANT(NodeStateConstant, FINISHING_CONSTANT, FINISHING_STATE);

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

  DEFINE_EXPRESSION_CONSTANT(NodeOutcomeConstant, SUCCESS_CONSTANT, SUCCESS_OUTCOME);
  DEFINE_EXPRESSION_CONSTANT(NodeOutcomeConstant, FAILURE_CONSTANT, FAILURE_OUTCOME);
  DEFINE_EXPRESSION_CONSTANT(NodeOutcomeConstant, SKIPPED_CONSTANT, SKIPPED_OUTCOME);
  DEFINE_EXPRESSION_CONSTANT(NodeOutcomeConstant, INTERRUPTED_CONSTANT, INTERRUPTED_OUTCOME);

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

  DEFINE_EXPRESSION_CONSTANT(FailureTypeConstant, PRE_CONDITION_FAILED_CONSTANT, PRE_CONDITION_FAILED);
  DEFINE_EXPRESSION_CONSTANT(FailureTypeConstant, POST_CONDITION_FAILED_CONSTANT, POST_CONDITION_FAILED);
  DEFINE_EXPRESSION_CONSTANT(FailureTypeConstant, INVARIANT_CONDITION_FAILED_CONSTANT, INVARIANT_CONDITION_FAILED);
  DEFINE_EXPRESSION_CONSTANT(FailureTypeConstant, PARENT_FAILED_CONSTANT, PARENT_FAILED);
  DEFINE_EXPRESSION_CONSTANT(FailureTypeConstant, EXITED_CONSTANT, EXITED);
  DEFINE_EXPRESSION_CONSTANT(FailureTypeConstant, PARENT_EXITED_CONSTANT, PARENT_EXITED);

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

  DEFINE_EXPRESSION_CONSTANT(CommandHandleConstant, COMMAND_SENT_TO_SYSTEM_CONSTANT, COMMAND_SENT_TO_SYSTEM);
  DEFINE_EXPRESSION_CONSTANT(CommandHandleConstant, COMMAND_ACCEPTED_CONSTANT, COMMAND_ACCEPTED);
  DEFINE_EXPRESSION_CONSTANT(CommandHandleConstant, COMMAND_RCVD_BY_SYSTEM_CONSTANT, COMMAND_RCVD_BY_SYSTEM);
  DEFINE_EXPRESSION_CONSTANT(CommandHandleConstant, COMMAND_FAILED_CONSTANT, COMMAND_FAILED);
  DEFINE_EXPRESSION_CONSTANT(CommandHandleConstant, COMMAND_DENIED_CONSTANT, COMMAND_DENIED);
  DEFINE_EXPRESSION_CONSTANT(CommandHandleConstant, COMMAND_SUCCESS_CONSTANT, COMMAND_SUCCESS);

  //
  // Factory methods
  //

  template <>
  Expression *NamedConstantExpressionFactory<NodeStateConstant>::allocate(PlexilExpr const *expr,
                                                                          NodeConnector *node,
                                                                          bool &wasCreated) const
  {
    PlexilValue const *valex = dynamic_cast<PlexilValue const *>(expr);
    checkParserException(valex, "createExpression: not a PlexilValue");
    checkParserException(valex->type() == NODE_STATE_TYPE, "createExpression: not a NodeStateValue");
    wasCreated = false;
    switch (parseNodeState(valex->value())) {
    case INACTIVE_STATE:
      return INACTIVE_CONSTANT();

    case WAITING_STATE:
      return WAITING_CONSTANT();

    case EXECUTING_STATE:
      return EXECUTING_CONSTANT();

    case ITERATION_ENDED_STATE:
      return ITERATION_ENDED_CONSTANT();

    case FINISHED_STATE:
      return FINISHED_CONSTANT();

    case FAILING_STATE:
      return FAILING_CONSTANT();

    case FINISHING_STATE:
      return FINISHING_CONSTANT();

    default:
      checkParserException(ALWAYS_FAIL, "createExpression: Invalid NodeStateValue \"" << valex->value() << "\"");
      return NULL;
    }
  }

  template <>
  Expression *NamedConstantExpressionFactory<NodeOutcomeConstant>::allocate(PlexilExpr const *expr,
                                                                            NodeConnector *node,
                                                                            bool &wasCreated) const
  {
    PlexilValue const *valex = dynamic_cast<PlexilValue const *>(expr);
    checkParserException(valex, "createExpression: not a PlexilValue");
    checkParserException(valex->type() == OUTCOME_TYPE, "createExpression: not a NodeOutcomeValue");
    wasCreated = false;
    switch (parseNodeOutcome(valex->value())) {
    case SUCCESS_OUTCOME:
      return SUCCESS_CONSTANT();

    case FAILURE_OUTCOME:
      return FAILURE_CONSTANT();

    case SKIPPED_OUTCOME:
      return SKIPPED_CONSTANT();

    case INTERRUPTED_OUTCOME:
      return INTERRUPTED_CONSTANT();

    default:
      checkParserException(ALWAYS_FAIL, "createExpression: Invalid NodeOutcomeValue \"" << valex->value() << "\"");
      return NULL;
    }
  }

  template <>
  Expression *NamedConstantExpressionFactory<FailureTypeConstant>::allocate(PlexilExpr const *expr,
                                                                            NodeConnector *node,
                                                                            bool &wasCreated) const
  {
    PlexilValue const *valex = dynamic_cast<PlexilValue const *>(expr);
    checkParserException(valex, "createExpression: not a PlexilValue");
    checkParserException(valex->type() == FAILURE_TYPE, "createExpression: not a FailureTypeValue");
    wasCreated = false;
    switch (parseFailureType(valex->value())) {
    case PRE_CONDITION_FAILED:
      return PRE_CONDITION_FAILED_CONSTANT();

    case POST_CONDITION_FAILED:
      return POST_CONDITION_FAILED_CONSTANT();

    case INVARIANT_CONDITION_FAILED:
      return INVARIANT_CONDITION_FAILED_CONSTANT();

    case PARENT_FAILED:
      return PARENT_FAILED_CONSTANT();

    case EXITED:
      return EXITED_CONSTANT();

    case PARENT_EXITED:
      return PARENT_EXITED_CONSTANT();

    default:
      checkParserException(ALWAYS_FAIL, "createExpression: Invalid FailureTypeValue \"" << valex->value() << "\"");
      return NULL;
    }
  }

  template <>
  Expression *NamedConstantExpressionFactory<CommandHandleConstant>::allocate(PlexilExpr const *expr,
                                                                              NodeConnector *node,
                                                                              bool &wasCreated) const
  {
    PlexilValue const *valex = dynamic_cast<PlexilValue const *>(expr);
    checkParserException(valex, "createExpression: not a PlexilValue");
    checkParserException(valex->type() == COMMAND_HANDLE_TYPE, "createExpression: not a CommandHandleValue");
    wasCreated = false;
    switch (parseCommandHandleValue(valex->value())) {
    case COMMAND_SENT_TO_SYSTEM:
      return COMMAND_SENT_TO_SYSTEM_CONSTANT();

    case COMMAND_ACCEPTED:
      return COMMAND_ACCEPTED_CONSTANT();

    case COMMAND_RCVD_BY_SYSTEM:
      return COMMAND_RCVD_BY_SYSTEM_CONSTANT();

    case COMMAND_FAILED:
      return COMMAND_FAILED_CONSTANT();

    case COMMAND_DENIED:
      return COMMAND_DENIED_CONSTANT();

    case COMMAND_SUCCESS:
      return COMMAND_SUCCESS_CONSTANT();

    default:
      checkParserException(ALWAYS_FAIL, "createExpression: Invalid CommandHandleValue \"" << valex->value() << "\"");
      return NULL;
    }
  }

} // namespace PLEXIL
