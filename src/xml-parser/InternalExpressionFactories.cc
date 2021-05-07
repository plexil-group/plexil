/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "CommandImpl.hh"
#include "CommandNode.hh"
#include "ConcreteExpressionFactory.hh"
#include "NodeImpl.hh"
#include "NodeConstantExpressions.hh"
#include "NodeTimepointValue.hh"
#include "NodeVariables.hh"
#include "parseNodeReference.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"

#include "pugixml.hpp"

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

namespace PLEXIL
{
  //
  // Specializations for internal variables
  //

  template <>
  ValueType factoryCheck<StateVariable>(char const * /* nodeId */,
                                        pugi::xml_node const expr,
                                        ValueType /* desiredType*/)
  {
    checkHasChildElement(expr);
    checkNodeReference(expr.first_child());
    return NODE_STATE_TYPE;
  }    

  template <>
  Expression *factoryAllocate<StateVariable>(pugi::xml_node const expr,
                                             NodeConnector *node,
                                             bool &wasCreated,
                                             ValueType /* returnType */)
  {
    checkHasChildElement(expr);
    NodeImpl *impl = dynamic_cast<NodeImpl *>(node);
    assertTrueMsg(impl,
                  "StateVariable factory: internal error: argument is not a NodeImpl");
    NodeImpl *refNode = parseNodeReference(expr.first_child(), impl); // can throw ParserException
    wasCreated = false;
    return refNode->getStateVariable();
  }

  template <>
  ValueType factoryCheck<OutcomeVariable>(char const * /* nodeId */,
                                          pugi::xml_node const expr,
                                          ValueType /* desiredType*/)
  {
    checkHasChildElement(expr);
    checkNodeReference(expr.first_child());
    return OUTCOME_TYPE;
  }    
  
  template <>
  Expression *factoryAllocate<OutcomeVariable>(pugi::xml_node const expr,
                                               NodeConnector *node,
                                               bool &wasCreated,
                                               ValueType /* returnType */)
  {
    checkHasChildElement(expr);
    NodeImpl *impl = dynamic_cast<NodeImpl *>(node);
    assertTrueMsg(impl,
                  "OutcomeVariable factory: internal error: argument is not a NodeImpl");
    NodeImpl *refNode = parseNodeReference(expr.first_child(), impl); // can throw ParserException
    wasCreated = false;
    return refNode->getOutcomeVariable();
  }

  template <>
  ValueType factoryCheck<FailureVariable>(char const * /* nodeId */,
                                          pugi::xml_node const expr,
                                          ValueType /* desiredType*/)
  {
    checkHasChildElement(expr);
    checkNodeReference(expr.first_child());
    return FAILURE_TYPE;
  }    

  template <>
  Expression *factoryAllocate<FailureVariable>(pugi::xml_node const expr,
                                               NodeConnector *node,
                                               bool &wasCreated,
                                               ValueType /* returnType */)
  {
    checkHasChildElement(expr);
    NodeImpl *impl = dynamic_cast<NodeImpl *>(node);
    assertTrueMsg(impl,
                  "FailureVariable factory: internal error: argument is not a NodeImpl");
    NodeImpl *refNode = parseNodeReference(expr.first_child(), impl); // can throw ParserException
    wasCreated = false;
    return refNode->getFailureTypeVariable();
  }

  template <>
  ValueType factoryCheck<CommandHandleVariable>(char const * /* nodeId */,
                                                pugi::xml_node const expr,
                                                ValueType /* desiredType*/)
  {
    checkHasChildElement(expr);
    checkNodeReference(expr.first_child());
    return COMMAND_HANDLE_TYPE;
  }    

  template <>
  Expression *factoryAllocate<CommandHandleVariable>(pugi::xml_node const expr,
                                                     NodeConnector *node,
                                                     bool &wasCreated,
                                                     ValueType /* returnType */)
  {
    checkHasChildElement(expr);
    NodeImpl *impl = dynamic_cast<NodeImpl *>(node);
    assertTrueMsg(impl,
                  "CommandHandleVariable factory: internal error: argument is not a NodeImpl");
    pugi::xml_node nodeRef = expr.first_child();
    NodeImpl *refNode = parseNodeReference(nodeRef, impl); // can throw ParserException
    checkParserExceptionWithLocation(refNode->getType() == NodeType_Command,
                                     expr.first_child(),
                                     "createExpression: Node " << refNode->getNodeId()
                                     << " is not a Command node");
    CommandNode *cnode = dynamic_cast<CommandNode *>(refNode);
    assertTrue_1(cnode);
    wasCreated = false;
    return cnode->getCommand()->getAck();
  }
  
  // Specialization for node timepoint references
  template <>
  ValueType factoryCheck<NodeTimepointValue>(char const * /* nodeId */,
                                             pugi::xml_node const expr,
                                             ValueType /* desiredType*/)
  {
    checkHasChildElement(expr);
    checkNodeReference(expr.first_child());
    return DATE_TYPE;
  }    

  template <>
  Expression *factoryAllocate<NodeTimepointValue>(pugi::xml_node const expr,
                                                  NodeConnector *node,
                                                  bool &wasCreated,
                                                  ValueType /* returnType */)
  {
    checkHasChildElement(expr);
    NodeImpl *impl = dynamic_cast<NodeImpl *>(node);
    assertTrueMsg(impl,
                  "NodeTimepointValue factory: internal error: argument is not a NodeImpl");
    pugi::xml_node nodeRef = expr.first_child();
    NodeImpl *refNode = parseNodeReference(nodeRef, impl); // can throw ParserException
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
    else {
      reportParserExceptionWithLocation(which,
                                        "createExpression: Invalid Timepoint value \""
                                        << whichStr << "\"");
      return nullptr;
    }
    wasCreated = false;
    return refNode->ensureTimepoint(state, isEnd);
  }

  ENSURE_EXPRESSION_FACTORY(StateVariable);
  ENSURE_EXPRESSION_FACTORY(OutcomeVariable);
  ENSURE_EXPRESSION_FACTORY(FailureVariable);
  ENSURE_EXPRESSION_FACTORY(CommandHandleVariable);
  ENSURE_EXPRESSION_FACTORY(NodeTimepointValue);

} // namespace PLEXIL
