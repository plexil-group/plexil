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

#include "InternalExpressionFactories.hh"

#include "Command.hh"
#include "CommandHandleVariable.hh"
#include "CommandNode.hh"
#include "Node.hh"
#include "NodeConstantExpressions.hh"
#include "NodeTimepointValue.hh"
#include "NodeVariables.hh"
#include "ParserException.hh"
#include "PlexilPlan.hh"

namespace PLEXIL
{
  //
  // Constants - specializations of Constant<uint16_t>

  template <>
  Expression *ConcreteExpressionFactory<NodeStateConstant>::allocate(PlexilExprId const &expr,
                                                                     const NodeConnectorId& node,
                                                                     bool &wasCreated) const
  {
    PlexilValue const *valex = dynamic_cast<PlexilValue const *>((Expression const *) expr);
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
  Expression *ConcreteExpressionFactory<NodeOutcomeConstant>::allocate(PlexilExprId const &expr,
                                                                       const NodeConnectorId& node,
                                                                       bool &wasCreated) const
  {
    PlexilValue const *valex = dynamic_cast<PlexilValue const *>((Expression const *) expr);
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
  Expression *ConcreteExpressionFactory<FailureTypeConstant>::allocate(PlexilExprId const &expr,
                                                                       const NodeConnectorId& node,
                                                                       bool &wasCreated) const
  {
    PlexilValue const *valex = dynamic_cast<PlexilValue const *>((Expression const *) expr);
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
  Expression *ConcreteExpressionFactory<CommandHandleConstant>::allocate(PlexilExprId const &expr,
                                                                         const NodeConnectorId& node,
                                                                         bool &wasCreated) const
  {
    PlexilValue const *valex = dynamic_cast<PlexilValue const *>((Expression const *) expr);
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

  //
  // Internal node variables
  //

  template <>
  Expression *ConcreteExpressionFactory<StateVariable>::allocate(const PlexilExprId& expr,
                                                                 const NodeConnectorId& node,
                                                                 bool &wasCreated) const
  {
    PlexilStateVar const *var = dynamic_cast<PlexilStateVar const *>((Expression const *) expr);
    checkParserException(var, "createExpression: not a PlexilStateVar");
    NodeId target = node->findNodeRef(var->ref());
    checkParserException(target.isId(), "createExpression: Can't find node for StateVariable");
    wasCreated = false;
    return target->getStateVariable();
  }

  template <>
  Expression *ConcreteExpressionFactory<OutcomeVariable>::allocate(const PlexilExprId& expr,
                                                                   const NodeConnectorId& node,
                                                                   bool &wasCreated) const
  {
    PlexilOutcomeVar const *var = dynamic_cast<PlexilOutcomeVar const *>((Expression const *) expr);
    checkParserException(var, "createExpression: not a PlexilOutcomeVar");
    NodeId target = node->findNodeRef(var->ref());
    checkParserException(target.isId(), "createExpression: Can't find node for OutcomeVariable");
    wasCreated = false;
    return target->getOutcomeVariable();
  }

  template <>
  Expression *ConcreteExpressionFactory<FailureVariable>::allocate(const PlexilExprId& expr,
                                                                   const NodeConnectorId& node,
                                                                   bool &wasCreated) const
  {
    PlexilFailureVar const *var = dynamic_cast<PlexilFailureVar const *>((Expression const *) expr);
    checkParserException(var, "createExpression: not a PlexilFailureVar");
    NodeId target = node->findNodeRef(var->ref());
    checkParserException(target.isId(), "createExpression: Can't find node for FailureTypeVariable");
    wasCreated = false;
    return target->getFailureTypeVariable();
  }

  //
  // CommandHandleVariable
  //

  template <>
  Expression *ConcreteExpressionFactory<CommandHandleVariable>::allocate(const PlexilExprId& expr,
                                                                         const NodeConnectorId& node,
                                                                         bool &wasCreated) const
  {
    PlexilCommandHandleVar const *var = dynamic_cast<PlexilCommandHandleVar const *>((Expression const *) expr);
    checkParserException(var, "createExpression: not a PlexilCommandHandleVar");
    NodeId target = node->findNodeRef(var->ref());
    checkParserException(target.isId(), "createExpression: Can't find node for CommandHandleVariable");
    CommandNode const * cnode = dynamic_cast<CommandNode const *>((Node const *) target);
    checkParserException(cnode, "createExpression: Node for CommandHandleVariable not a Command node");
    CommandId cmd = cnode->getCommand();
    checkParserException(cmd, "createExpression: Internal error: Command node has no Command");
    wasCreated = false;
    return cmd->getAck();
  }

  //
  // Node timepoint reference
  //

  ConcreteExpressionFactory<NodeTimepointValue>::ConcreteExpressionFactory(const std::string& name)
    : ExpressionFactory(name) 
  {
  }

  ConcreteExpressionFactory<NodeTimepointValue>::~ConcreteExpressionFactory()
  {
  }

  Expression *ConcreteExpressionFactory<NodeTimepointValue>::allocate(const PlexilExprId& expr,
                                                                      const NodeConnectorId& node,
                                                                      bool &wasCreated) const
  {
    PlexilTimepointVar const *var = dynamic_cast<PlexilTimepointVar const *>((Expression const *) expr);
    checkParserException(var, "createExpression: not a PlexilTimepointVar");
    wasCreated = true;
    return create(var, node);
  }

  Expression *ConcreteExpressionFactory<NodeTimepointValue>::create(PlexilTimepointVar const *var,
                                                                    NodeConnectorId const &node) const
  {
    NodeId refNode = node->findNodeRef(var->ref());
    checkParserException(refNode.isId(), "createExpression: Timepoint node reference not found");
    NodeState state = parseNodeState(var->state());
    checkParserException(isNodeStateValid(state), "createExpression: Invalid NodeState value \"" << var->state() << "\"");
    return new NodeTimepointValue(refNode,
                                  state,
                                  ("END" == var->timepoint()));
  }
  
  ENSURE_EXPRESSION_FACTORY(NodeStateConstant);
  ENSURE_EXPRESSION_FACTORY(NodeOutcomeConstant);
  ENSURE_EXPRESSION_FACTORY(FailureTypeConstant);
  ENSURE_EXPRESSION_FACTORY(CommandHandleConstant);

  ENSURE_EXPRESSION_FACTORY(StateVariable);
  ENSURE_EXPRESSION_FACTORY(OutcomeVariable);
  ENSURE_EXPRESSION_FACTORY(FailureVariable);
  ENSURE_EXPRESSION_FACTORY(CommandHandleVariable);
  ENSURE_EXPRESSION_FACTORY(NodeTimepointValue);

} // namespace PLEXIL
