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
    const char *name = nodeRef.child_value();
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
    std::string const nameStr(nodeRef.child_value());
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

    Expression *allocate(pugi::xml_node const expr,
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

    Expression *allocate(pugi::xml_node const expr,
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

    Expression *allocate(pugi::xml_node const expr,
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

    Expression *allocate(pugi::xml_node const expr,
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

    Expression *allocate(pugi::xml_node const expr,
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
      NodeState state = parseNodeState(stateName.child_value());
      checkParserExceptionWithLocation(state != NO_NODE_STATE,
                                       stateName,
                                       "createExpression: Invalid NodeStateValue \""
                                       << stateName.child_value()
                                       << "\"");
      pugi::xml_node which = stateName.next_sibling();
      checkParserExceptionWithLocation(which && testTag(TIMEPOINT_TAG, which),
                                       expr,
                                       "createExpression: NodeTimepointValue has no Timepoint element");
      checkNotEmpty(which);
      char const *whichStr = which.child_value();
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
    // Default, copy, assign all prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(const ConcreteExpressionFactory &);
    ConcreteExpressionFactory &operator=(const ConcreteExpressionFactory &);
  };

  //
  // Specialized expression factories for above
  //

  template <class C>
  class NamedConstantExpressionFactory : public ExpressionFactory
  {
  public:
    NamedConstantExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    ~NamedConstantExpressionFactory()
    {
    }

    Expression *allocate(pugi::xml_node const expr,
                         NodeConnector *node,
                         bool &wasCreated) const;

  private:
    // Default, copy, assign all prohibited
    NamedConstantExpressionFactory();
    NamedConstantExpressionFactory(const NamedConstantExpressionFactory &);
    NamedConstantExpressionFactory &operator=(const NamedConstantExpressionFactory &);
  };

  //
  // Factory methods
  //

  template <>
  Expression *NamedConstantExpressionFactory<NodeStateConstant>::allocate(pugi::xml_node const expr,
                                                                          NodeConnector *node,
                                                                          bool &wasCreated) const
  {
    checkNotEmpty(expr);
    wasCreated = false;
    switch (parseNodeState(expr.child_value())) {
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
      checkParserExceptionWithLocation(ALWAYS_FAIL,
                                       expr.first_child(),
                                       "createExpression: Invalid NodeStateValue \"" << expr.child_value() << "\"");
      return NULL;
    }
  }

  template <>
  Expression *NamedConstantExpressionFactory<NodeOutcomeConstant>::allocate(pugi::xml_node const expr,
                                                                            NodeConnector *node,
                                                                            bool &wasCreated) const
  {
    checkNotEmpty(expr);
    wasCreated = false;
    switch (parseNodeOutcome(expr.child_value())) {
    case SUCCESS_OUTCOME:
      return SUCCESS_CONSTANT();

    case FAILURE_OUTCOME:
      return FAILURE_CONSTANT();

    case SKIPPED_OUTCOME:
      return SKIPPED_CONSTANT();

    case INTERRUPTED_OUTCOME:
      return INTERRUPTED_CONSTANT();

    default:
      checkParserExceptionWithLocation(ALWAYS_FAIL,
                                       expr.first_child(),
                                       "createExpression: Invalid NodeOutcomeValue \"" << expr.child_value() << "\"");
      return NULL;
    }
  }

  template <>
  Expression *NamedConstantExpressionFactory<FailureTypeConstant>::allocate(pugi::xml_node const expr,
                                                                            NodeConnector *node,
                                                                            bool &wasCreated) const
  {
    checkNotEmpty(expr);
    wasCreated = false;
    switch (parseFailureType(expr.child_value())) {
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
      checkParserExceptionWithLocation(ALWAYS_FAIL,
                                       expr.first_child(),
                                       "createExpression: Invalid FailureTypeValue \"" << expr.child_value() << "\"");
      return NULL;
    }
  }

  template <>
  Expression *NamedConstantExpressionFactory<CommandHandleConstant>::allocate(pugi::xml_node const expr,
                                                                              NodeConnector *node,
                                                                              bool &wasCreated) const
  {
    checkNotEmpty(expr);
    wasCreated = false;
    switch (parseCommandHandleValue(expr.child_value())) {
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
      checkParserExceptionWithLocation(ALWAYS_FAIL,
                                       expr.first_child(),
                                       "createExpression: Invalid CommandHandleValue \"" << expr.child_value() << "\"");
      return NULL;
    }
  }

// Convenience macros
#define ENSURE_NAMED_CONSTANT_FACTORY(CLASS) template class PLEXIL::NamedConstantExpressionFactory<CLASS >;
#define REGISTER_NAMED_CONSTANT_FACTORY(CLASS,NAME) {new PLEXIL::NamedConstantExpressionFactory<CLASS >(#NAME);}

  // Named constants
  ENSURE_NAMED_CONSTANT_FACTORY(NodeStateConstant);
  ENSURE_NAMED_CONSTANT_FACTORY(NodeOutcomeConstant);
  ENSURE_NAMED_CONSTANT_FACTORY(FailureTypeConstant);
  ENSURE_NAMED_CONSTANT_FACTORY(CommandHandleConstant);

  void registerInternalExpressionFactories()
  {
    static bool sl_inited = false;
    if (!sl_inited) {
      // Named constants
      REGISTER_NAMED_CONSTANT_FACTORY(NodeStateConstant, NodeStateValue);
      REGISTER_NAMED_CONSTANT_FACTORY(NodeOutcomeConstant, NodeOutcomeValue);
      REGISTER_NAMED_CONSTANT_FACTORY(FailureTypeConstant, NodeFailureValue);
      REGISTER_NAMED_CONSTANT_FACTORY(CommandHandleConstant, NodeCommandHandleValue);

      REGISTER_EXPRESSION(StateVariable, NodeStateVariable);
      REGISTER_EXPRESSION(OutcomeVariable, NodeOutcomeVariable);
      REGISTER_EXPRESSION(FailureVariable, NodeFailureVariable);
      REGISTER_EXPRESSION(CommandHandleVariable, NodeCommandHandleVariable);
      REGISTER_EXPRESSION(NodeTimepointValue, NodeTimepointValue);

      sl_inited = true;
    }
  }

} // namespace PLEXIL
