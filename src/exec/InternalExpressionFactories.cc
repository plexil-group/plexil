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
#include "parser-utils.hh"
#include "PlexilPlan.hh"
#include "PlexilSchema.hh"
#include "pugixml.hpp"

namespace PLEXIL
{
  // Utility routines
  static Node *parseNodeRef(pugi::xml_node nodeRef, NodeConnector *node)
  {
    // parse directional reference
    checkAttr(DIR_ATTR, nodeRef);
    const char* dirValue = nodeRef.attribute(DIR_ATTR).value();

    if (0 == strcmp(dirValue, SELF_VAL))
      return dynamic_cast<Node *>(node);

    Node *result = NULL;
    if (0 == strcmp(dirValue, PARENT_VAL)) {
      result = node->getParent();
      checkParserExceptionWithLocation(result,
                                       nodeRef,
                                       "createExpression: Parent node reference in root node "
                                       << node->getNodeId());
      return result;
    }

    checkNotEmpty(nodeRef);
    const char *name = nodeRef.first_child().value();
    if (0 == strcmp(dirValue, CHILD_VAL)) {
      result = node->findChild(std::string(name));
      checkParserExceptionWithLocation(result,
                                       nodeRef,
                                       "createExpression: No child node named " << name 
                                       << " in node " << node->getNodeId());
      return result;
    }
    if (0 == strcmp(dirValue, SIBLING_VAL)) {
      Node *parent = node->getParent();
      checkParserExceptionWithLocation(parent,
                                       nodeRef,
                                       "createExpression: Sibling node reference from root node "
                                       << node->getNodeId());
      result = parent->findChild(std::string(name));
      checkParserExceptionWithLocation(result,
                                       nodeRef,
                                       "createExpression: No sibling node named " << name 
                                       << " for node " << node->getNodeId());
      return result;
    }
    else {
      checkParserExceptionWithLocation(ALWAYS_FAIL,
                                       nodeRef,
                                       "XML parsing error: Invalid value for " << DIR_ATTR << " attibute \""
                                       << dirValue << "\"");
    }
  }

  static Node *findLocalNodeId(std::string const &nameStr, NodeConnector *node)
  {
    // search for node ID
    if (nameStr == node->getNodeId())
      return dynamic_cast<Node *>(node);
    // Check children, if any
    Node *result = node->findChild(nameStr);
    if (result)
      return result;
    return NULL;
  }

  static Node *parseNodeId(pugi::xml_node nodeRef, NodeConnector *node)
  {
    // search for node ID
    checkNotEmpty(nodeRef);
    std::string const nameStr(nodeRef.first_child().value());
    Node *result = findLocalNodeId(nameStr, node);
    if (result)
      return result;

    Node *parent = node->getParent();
    while (parent) {
      result = findLocalNodeId(nameStr, parent);
      if (result)
        return result;
      parent = parent->getParent();
    }
    checkParserExceptionWithLocation(ALWAYS_FAIL,
                                     nodeRef.first_child(),
                                     "createExpression: No node named "
                                     << nameStr
                                     << " reachable from node " << node->getNodeId());
    return NULL;
  }

  static Node *parseNodeReference(pugi::xml_node nodeRef, NodeConnector *node)
  {
    checkParserExceptionWithLocation(nodeRef.type() == pugi::node_element,
                                     nodeRef,
                                     "createExpression: Node reference is not an element");
    const char* tag = nodeRef.name();
    if (0 == strcmp(tag, NODEREF_TAG))
      return parseNodeRef(nodeRef, node);
    else if (0 == strcmp(tag, NODEID_TAG))
      return parseNodeId(nodeRef, node);
    else 
      checkParserExceptionWithLocation(ALWAYS_FAIL,
                                       nodeRef,
                                       "createExpression: Invalid node reference");
  }
  
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
      checkHasChildElement(expr);
      Node *refNode = parseNodeReference(expr.first_child(), node); // can throw ParserException
      wasCreated = false;
      return refNode->getStateVariable();
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
      checkHasChildElement(expr);
      Node *refNode = parseNodeReference(expr.first_child(), node); // can throw ParserException
      wasCreated = false;
      return refNode->getOutcomeVariable();
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
      checkHasChildElement(expr);
      Node *refNode = parseNodeReference(expr.first_child(), node); // can throw ParserException
      wasCreated = false;
      return refNode->getFailureTypeVariable();
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
      checkHasChildElement(expr);
      pugi::xml_node nodeRef = expr.first_child();
      Node *refNode = parseNodeReference(nodeRef, node); // can throw ParserException
      checkParserExceptionWithLocation(refNode->getType() == NodeType_Command,
                                       expr.first_child(),
                                       "createExpression: Node " << refNode->getNodeId()
                                       << " is not a Command node");
      CommandNode *cnode = dynamic_cast<CommandNode *>(refNode);
      assertTrue_1(cnode);
      wasCreated = false;
      return cnode->getCommand()->getAck();
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
      checkHasChildElement(expr);
      pugi::xml_node nodeRef = expr.first_child();
      Node *refNode = parseNodeReference(nodeRef, node); // can throw ParserException
      pugi::xml_node stateName = nodeRef.next_sibling();
      checkParserExceptionWithLocation(stateName && testTag(STATEVAL_TAG, stateName),
                                       expr,
                                       "createExpression: NodeTimepointValue has no NodeStateValue element");
      checkNotEmpty(stateName);
      NodeState state = parseNodeState(stateName.first_child().value());
      checkParserExceptionWithLocation(state != NO_NODE_STATE,
                                       stateName,
                                       "createExpression: Invalid NodeStateValue \""
                                       << stateName.first_child().value()
                                       << "\"");
      pugi::xml_node which = stateName.next_sibling();
      checkParserExceptionWithLocation(which && testTag(TIMEPOINT_TAG, which),
                                       expr,
                                       "createExpression: NodeTimepointValue has no Timepoint element");
      checkNotEmpty(which);
      char const *whichStr = which.first_child().value();
      bool isEnd;
      if (0 == strcmp(START_VAL, whichStr))
        isEnd = false;
      else if (0 == strcmp(END_VAL, whichStr))
        isEnd = true;
      else
        checkParserExceptionWithLocation(ALWAYS_FAIL,
                                         which,
                                         "createExpression: Invalid Timepoint value \""
                                         << whichStr << "\"");
      wasCreated = true;
      return new NodeTimepointValue(refNode, state, isEnd);
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
