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
#include "PlexilPlan.hh"

namespace PLEXIL
{
  //
  // Constants - specializations of Constant<uint16_t>

  template <>
  ExpressionId ConcreteExpressionFactory<NodeStateConstant>::create(const PlexilExprId& expr,
                                                                    const NodeConnectorId& node) const
  {
    PlexilValue const * valex = dynamic_cast<PlexilValue const *>((PlexilExpr const *) expr);
    assertTrue_2(valex, "createExpression: Not a PlexilValue");
    assertTrue_2(valex->type() == NODE_STATE_TYPE, "createExpression: not a NodeStateValue");
    NodeState val = parseNodeState(valex->value());
    assertTrue_2(val >= INACTIVE_STATE && val < NO_NODE_STATE, "createExpression: Invalid NodeStateValue");
    return (new NodeStateConstant(val))->getId();
  }

  template <>
  ExpressionId ConcreteExpressionFactory<NodeOutcomeConstant>::create(const PlexilExprId& expr,
                                                                      const NodeConnectorId& node) const
  {
    PlexilValue const * valex = dynamic_cast<PlexilValue const *>((PlexilExpr const *) expr);
    assertTrue_2(valex, "createExpression: Not a PlexilValue");
    assertTrue_2(valex->type() == OUTCOME_TYPE, "createExpression: not a NodeOutcomeValue");
    NodeOutcome val = parseNodeOutcome(valex->value());
    assertTrue_2(val > NO_OUTCOME && val < OUTCOME_MAX, "createExpression: Invalid NodeOutcomeValue");
    return (new NodeOutcomeConstant(val))->getId();
  }

  template <>
  ExpressionId ConcreteExpressionFactory<FailureTypeConstant>::create(const PlexilExprId& expr,
                                                                      const NodeConnectorId& node) const
  {
    PlexilValue const * valex = dynamic_cast<PlexilValue const *>((PlexilExpr const *) expr);
    assertTrue_2(valex, "createExpression: Not a PlexilValue");
    assertTrue_2(valex->type() == FAILURE_TYPE, "createExpression: not a FailureTypeValue");
    FailureType val = parseFailureType(valex->value());
    assertTrue_2(val > NO_FAILURE && val < FAILURE_TYPE_MAX, "createExpression: Invalid FailureTypeValue");
    return (new FailureTypeConstant(val))->getId();
  }

  //
  // Node variables
  //

  template <>
  ExpressionId ConcreteExpressionFactory<StateVariable>::allocate(const PlexilExprId& expr,
                                                                  const NodeConnectorId& node,
                                                                  bool &wasCreated) const
  {
    PlexilStateVar const *var = dynamic_cast<PlexilStateVar const *>((Expression const *) expr);
    assertTrue_2(var, "createExpression: not a PlexilStateVar");
    NodeId target = node->findNodeRef(var->ref());
    assertTrue_2(target.isId(), "createExpression: Can't find node for StateVariable");
    wasCreated = false;
    return target->getStateVariable();
  }

  template <>
  ExpressionId ConcreteExpressionFactory<StateVariable>::create(const PlexilExprId& expr,
                                                                const NodeConnectorId& node) const
  {
    return ExpressionId::noId();
  }

  template <>
  ExpressionId ConcreteExpressionFactory<OutcomeVariable>::allocate(const PlexilExprId& expr,
                                                                    const NodeConnectorId& node,
                                                                    bool &wasCreated) const
  {
    PlexilOutcomeVar const *var = dynamic_cast<PlexilOutcomeVar const *>((Expression const *) expr);
    assertTrue_2(var, "createExpression: not a PlexilOutcomeVar");
    NodeId target = node->findNodeRef(var->ref());
    assertTrue_2(target.isId(), "createExpression: Can't find node for OutcomeVariable");
    wasCreated = false;
    return target->getOutcomeVariable();
  }

  template <>
  ExpressionId ConcreteExpressionFactory<OutcomeVariable>::create(const PlexilExprId& expr,
                                                                  const NodeConnectorId& node) const
  {
    return ExpressionId::noId();
  }

  template <>
  ExpressionId ConcreteExpressionFactory<FailureVariable>::allocate(const PlexilExprId& expr,
                                                                    const NodeConnectorId& node,
                                                                    bool &wasCreated) const
  {
    PlexilFailureVar const *var = dynamic_cast<PlexilFailureVar const *>((Expression const *) expr);
    assertTrue_2(var, "createExpression: not a PlexilFailureVar");
    NodeId target = node->findNodeRef(var->ref());
    assertTrue_2(target.isId(), "createExpression: Can't find node for FailureTypeVariable");
    wasCreated = false;
    return target->getFailureTypeVariable();
  }

  template <>
  ExpressionId ConcreteExpressionFactory<FailureVariable>::create(const PlexilExprId& expr,
                                                                  const NodeConnectorId& node) const
  {
    return ExpressionId::noId();
  }

  //
  // CommandHandleVariable
  //

  template <>
  ExpressionId ConcreteExpressionFactory<CommandHandleVariable>::allocate(const PlexilExprId& expr,
                                                                          const NodeConnectorId& node,
                                                                          bool &wasCreated) const
  {
    PlexilCommandHandleVar const *var = dynamic_cast<PlexilCommandHandleVar const *>((Expression const *) expr);
    assertTrue_2(var, "createExpression: not a PlexilCommandHandleVar");
    NodeId target = node->findNodeRef(var->ref());
    assertTrue_2(target.isId(), "createExpression: Can't find node for CommandHandleVariable");
    CommandNode const * cnode = dynamic_cast<CommandNode const *>((Node const *) target);
    assertTrue_2(cnode, "createExpression: Node for CommandHandleVariable not a Command node");
    CommandId cmd = cnode->getCommand();
    assertTrue_2(cmd, "createExpression: Internal error: Command node has no Command");
    wasCreated = false;
    return cmd->getAck();
  }
  
  template <>
  ExpressionId ConcreteExpressionFactory<CommandHandleVariable>::create(const PlexilExprId& expr,
                                                                        const NodeConnectorId& node) const
  {
    return ExpressionId::noId();
  }

  //
  // Node timepoint reference
  //

  template <>
  ExpressionId ConcreteExpressionFactory<NodeTimepointValue>::create(const PlexilExprId& expr,
                                                                     const NodeConnectorId& node) const
  {
    PlexilTimepointVar const *var = dynamic_cast<PlexilTimepointVar const *>((Expression const *) expr);
    assertTrue_2(var, "createExpression: not a PlexilTimepointVar");
    return (new NodeTimepointValue(node->findNodeRef(var->ref()),
                                   parseNodeState(var->state()),
                                   ("END" == var->timepoint())))->getId();
  }

  // Not sure why this is required... shouldn't it use the general case?
  template <>
  ExpressionId ConcreteExpressionFactory<NodeTimepointValue>::allocate(const PlexilExprId& expr,
                                                                       const NodeConnectorId& node,
                                                                       bool &wasCreated) const
  {
    wasCreated = true;
    return this->create(expr, node);
  }

  
  ENSURE_EXPRESSION_FACTORY(StateVariable);
  ENSURE_EXPRESSION_FACTORY(OutcomeVariable);
  ENSURE_EXPRESSION_FACTORY(FailureVariable);
  ENSURE_EXPRESSION_FACTORY(CommandHandleVariable);
  ENSURE_EXPRESSION_FACTORY(NodeTimepointValue);

} // namespace PLEXIL
