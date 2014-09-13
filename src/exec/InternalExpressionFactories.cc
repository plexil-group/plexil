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
#include "ConcreteExpressionFactory.hh"
#include "Error.hh"
#include "Node.hh"
#include "NodeConstantExpressions.hh"
#include "NodeTimepointValue.hh"
#include "NodeVariables.hh"
#include "ParserException.hh"
#include "PlexilPlan.hh"
#include "pugixml.hpp"

namespace PLEXIL
{
  
  // Specialization for internal variables
  template <>
  class ConcreteExpressionFactory<StateVariable> : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    ~ConcreteExpressionFactory()
    {
    }

    Expression *allocate(PlexilExpr const * expr,
                         NodeConnector *node,
                         bool &wasCreated) const
    {
      PlexilStateVar const *var = dynamic_cast<PlexilStateVar const *>(expr);
      checkParserException(var, "createExpression: not a PlexilStateVar");
      Node *target = node->findNodeRef(var->ref());
      checkParserException(target, "createExpression: Can't find node for StateVariable");
      wasCreated = false;
      return target->getStateVariable();
    }

    Expression *allocate(pugi::xml_node const &expr,
                         NodeConnector *node,
                         bool &wasCreated) const
    {
      assertTrue_2(ALWAYS_FAIL, "Not yet implemented");
      return NULL;
    }

  private:
    // Default, copy, assign all prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(const ConcreteExpressionFactory &);
    ConcreteExpressionFactory &operator=(const ConcreteExpressionFactory &);
  };

  template <>
  class ConcreteExpressionFactory<OutcomeVariable> : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    ~ConcreteExpressionFactory()
    {
    }

    Expression *allocate(PlexilExpr const * expr,
                         NodeConnector *node,
                         bool &wasCreated) const
    {
      PlexilOutcomeVar const *var = dynamic_cast<PlexilOutcomeVar const *>(expr);
      checkParserException(var, "createExpression: not a PlexilOutcomeVar");
      Node *target = node->findNodeRef(var->ref());
      checkParserException(target, "createExpression: Can't find node for OutcomeVariable");
      wasCreated = false;
      return target->getOutcomeVariable();
    }

    Expression *allocate(pugi::xml_node const &expr,
                         NodeConnector *node,
                         bool &wasCreated) const
    {
      assertTrue_2(ALWAYS_FAIL, "Not yet implemented");
      return NULL;
    }

  private:
    // Default, copy, assign all prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(const ConcreteExpressionFactory &);
    ConcreteExpressionFactory &operator=(const ConcreteExpressionFactory &);
  };

  template <>
  class ConcreteExpressionFactory<FailureVariable> : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    ~ConcreteExpressionFactory()
    {
    }

    Expression *allocate(PlexilExpr const * expr,
                         NodeConnector *node,
                         bool &wasCreated) const
    {
      PlexilFailureVar const *var = dynamic_cast<PlexilFailureVar const *>(expr);
      checkParserException(var, "createExpression: not a PlexilFailureVar");
      Node *target = node->findNodeRef(var->ref());
      checkParserException(target, "createExpression: Can't find node for FailureTypeVariable");
      wasCreated = false;
      return target->getFailureTypeVariable();
    }

    Expression *allocate(pugi::xml_node const &expr,
                         NodeConnector *node,
                         bool &wasCreated) const
    {
      assertTrue_2(ALWAYS_FAIL, "Not yet implemented");
      return NULL;
    }

  private:
    // Default, copy, assign all prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(const ConcreteExpressionFactory &);
    ConcreteExpressionFactory &operator=(const ConcreteExpressionFactory &);
  };

  template <>
  class ConcreteExpressionFactory<CommandHandleVariable> : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    ~ConcreteExpressionFactory()
    {
    }

    Expression *allocate(PlexilExpr const * expr,
                         NodeConnector *node,
                         bool &wasCreated) const
    {
      PlexilCommandHandleVar const *var = dynamic_cast<PlexilCommandHandleVar const *>(expr);
      checkParserException(var, "createExpression: not a PlexilCommandHandleVar");
      Node *target = node->findNodeRef(var->ref());
      checkParserException(target, "createExpression: Can't find node for CommandHandleVariable");
      CommandNode *cnode = dynamic_cast<CommandNode *>(target);
      checkParserException(cnode, "createExpression: Node for CommandHandleVariable not a Command node");
      Command *cmd = cnode->getCommand();
      checkParserException(cmd, "createExpression: Internal error: Command node has no Command");
      wasCreated = false;
      return cmd->getAck();
    }

    Expression *allocate(pugi::xml_node const &expr,
                         NodeConnector *node,
                         bool &wasCreated) const
    {
      assertTrue_2(ALWAYS_FAIL, "Not yet implemented");
      return NULL;
    }

  private:
    // Default, copy, assign all prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(const ConcreteExpressionFactory &);
    ConcreteExpressionFactory &operator=(const ConcreteExpressionFactory &);
  };
  
  // Specialization for node timepoint references
  template <>
  class ConcreteExpressionFactory<NodeTimepointValue> : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    ~ConcreteExpressionFactory()
    {
    }

    Expression *allocate(PlexilExpr const * expr,
                         NodeConnector *node,
                         bool &wasCreated) const
    {
      PlexilTimepointVar const *var = dynamic_cast<PlexilTimepointVar const *>(expr);
      checkParserException(var, "createExpression: not a PlexilTimepointVar");
      wasCreated = true;
      return create(var, node);
    }

    Expression *allocate(pugi::xml_node const &expr,
                         NodeConnector *node,
                         bool &wasCreated) const
    {
      assertTrue_2(ALWAYS_FAIL, "Not yet implemented");
      return NULL;
    }

  private:
    Expression *create(PlexilTimepointVar const *var,
                       NodeConnector *node) const
    {
      Node *refNode = node->findNodeRef(var->ref());
      checkParserException(refNode, "createExpression: Timepoint node reference not found");
      NodeState state = parseNodeState(var->state());
      checkParserException(isNodeStateValid(state), "createExpression: Invalid NodeState value \"" << var->state() << "\"");
      return new NodeTimepointValue(refNode,
                                    state,
                                    ("END" == var->timepoint()));
    }

    // Default, copy, assign all prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(const ConcreteExpressionFactory &);
    ConcreteExpressionFactory &operator=(const ConcreteExpressionFactory &);
  };

  void registerInternalExpressionFactories()
  {
    static bool sl_inited = false;
    if (!sl_inited) {
      REGISTER_EXPRESSION(StateVariable, NodeStateVariable);
      REGISTER_EXPRESSION(OutcomeVariable, NodeOutcomeVariable);
      REGISTER_EXPRESSION(FailureVariable, NodeFailureVariable);
      REGISTER_EXPRESSION(CommandHandleVariable, NodeCommandHandleVariable);
      REGISTER_EXPRESSION(NodeTimepointValue, NodeTimepointValue);

      sl_inited = true;
    }
  }

} // namespace PLEXIL
