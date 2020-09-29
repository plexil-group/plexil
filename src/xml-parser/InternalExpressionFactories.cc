/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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
#include "NodeImpl.hh"
#include "NodeConstantExpressions.hh"
#include "NodeTimepointValue.hh"
#include "NodeVariables.hh"
#include "parseNodeReference.hh"
#include "ParserException.hh"
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

  ValueType ConcreteExpressionFactory<StateVariable>::check(char const * nodeId,
                                                            pugi::xml_node const expr) const
  {
    // TODO
    return NODE_STATE_TYPE;
  }    

  Expression *ConcreteExpressionFactory<StateVariable>::allocate(pugi::xml_node const expr,
                                                                 NodeConnector *node,
                                                                 bool &wasCreated,
                                                                 ValueType /* returnType */) const
  {
    checkHasChildElement(expr);
    NodeImpl *impl = dynamic_cast<NodeImpl *>(node);
    assertTrueMsg(impl,
                  "StateVariable factory: internal error: argument is not a NodeImpl");
    NodeImpl *refNode = parseNodeReference(expr.first_child(), impl); // can throw ParserException
    wasCreated = false;
    return refNode->getStateVariable();
  }

  ValueType ConcreteExpressionFactory<OutcomeVariable>::check(char const *nodeId,
                                                              pugi::xml_node const expr) const
  {
    // TODO
    return OUTCOME_TYPE;
  }    
  
  Expression *ConcreteExpressionFactory<OutcomeVariable>::allocate(pugi::xml_node const expr,
                                                                   NodeConnector *node,
                                                                   bool &wasCreated,
                                                                   ValueType /* returnType */) const
  {
    checkHasChildElement(expr);
    NodeImpl *impl = dynamic_cast<NodeImpl *>(node);
    assertTrueMsg(impl,
                  "OutcomeVariable factory: internal error: argument is not a NodeImpl");
    NodeImpl *refNode = parseNodeReference(expr.first_child(), impl); // can throw ParserException
    wasCreated = false;
    return refNode->getOutcomeVariable();
  }


  ValueType ConcreteExpressionFactory<FailureVariable>::check(char const *nodeId,
                                                              pugi::xml_node const expr) const
  {
    // TODO
    return FAILURE_TYPE;
  }    

  Expression *ConcreteExpressionFactory<FailureVariable>::allocate(pugi::xml_node const expr,
                                                                   NodeConnector *node,
                                                                   bool &wasCreated,
                                                                   ValueType /* returnType */) const
  {
    checkHasChildElement(expr);
    NodeImpl *impl = dynamic_cast<NodeImpl *>(node);
    assertTrueMsg(impl,
                  "FailureVariable factory: internal error: argument is not a NodeImpl");
    NodeImpl *refNode = parseNodeReference(expr.first_child(), impl); // can throw ParserException
    wasCreated = false;
    return refNode->getFailureTypeVariable();
  }

  ValueType ConcreteExpressionFactory<CommandHandleVariable>::check(char const *nodeId,
                                                                    pugi::xml_node const expr) const
  {
    // TODO
    return COMMAND_HANDLE_TYPE;
  }    

  Expression *ConcreteExpressionFactory<CommandHandleVariable>::allocate(pugi::xml_node const expr,
                                                                         NodeConnector *node,
                                                                         bool &wasCreated,
                                                                         ValueType /* returnType */) const
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

  ValueType ConcreteExpressionFactory<NodeTimepointValue>::check(char const *nodeId,
                                                                 pugi::xml_node const expr) const
  {
    // TODO
    return DATE_TYPE;
  }    

  Expression *ConcreteExpressionFactory<NodeTimepointValue>::allocate(pugi::xml_node const expr,
                                                                      NodeConnector *node,
                                                                      bool &wasCreated,
                                                                      ValueType /* returnType */) const
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
      return NULL;
    }
    wasCreated = false;
    return refNode->ensureTimepoint(state, isEnd);
  }

  //
  // Named constant methods
  //

  template <>
  ValueType NamedConstantExpressionFactory<NodeStateConstant>::check(char const *nodeId,
                                                                     pugi::xml_node const expr) const
  {
    checkNotEmpty(expr);
    switch (parseNodeState(expr.child_value())) {
    case INACTIVE_STATE:
    case WAITING_STATE:
    case EXECUTING_STATE:
    case ITERATION_ENDED_STATE:
    case FINISHED_STATE:
    case FAILING_STATE:
    case FINISHING_STATE:
      return NODE_STATE_TYPE; // is OK

    default:
      reportParserExceptionWithLocation(expr.first_child(),
                                        "Invalid NodeStateValue");
      return UNKNOWN_TYPE;
    }
  }

  template <>
  Expression *NamedConstantExpressionFactory<NodeStateConstant>::allocate(pugi::xml_node const expr,
                                                                          NodeConnector * /* node */,
                                                                          bool &wasCreated,
                                                                          ValueType /* returnType */) const
  {
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
      reportParserExceptionWithLocation(expr,
                                        "Invalid NodeStateValue");
      return NULL;
    }
  }

  template <>
  ValueType NamedConstantExpressionFactory<NodeOutcomeConstant>::check(char const *nodeId,
                                                                       pugi::xml_node const expr) const
  {
    checkNotEmpty(expr);
    switch (parseNodeOutcome(expr.child_value())) {
    case SUCCESS_OUTCOME:
    case FAILURE_OUTCOME:
    case SKIPPED_OUTCOME:
    case INTERRUPTED_OUTCOME:
      return OUTCOME_TYPE; // is OK

    default:
      reportParserExceptionWithLocation(expr,
                                        "Invalid NodeOutcomeValue");
      return UNKNOWN_TYPE;
    }
  }

  template <>
  Expression *NamedConstantExpressionFactory<NodeOutcomeConstant>::allocate(pugi::xml_node const expr,
                                                                            NodeConnector * /* node */,
                                                                            bool &wasCreated,
                                                                            ValueType /* returnType */) const
  {
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
      reportParserExceptionWithLocation(expr,
                                        "Invalid NodeOutcomeValue");
      return NULL;
    }
  }

  template <>
  ValueType NamedConstantExpressionFactory<FailureTypeConstant>::check(char const *nodeId,
                                                                       pugi::xml_node const expr) const
  {
    checkNotEmpty(expr);
    switch (parseFailureType(expr.child_value())) {
    case PRE_CONDITION_FAILED:
    case POST_CONDITION_FAILED:
    case INVARIANT_CONDITION_FAILED:
    case PARENT_FAILED:
    case EXITED:
    case PARENT_EXITED:
      return FAILURE_TYPE; // is OK

    default:
      reportParserExceptionWithLocation(expr,
                                        "Invalid FailureTypeValue");
      return UNKNOWN_TYPE;
    }
  }

  template <>
  Expression *NamedConstantExpressionFactory<FailureTypeConstant>::allocate(pugi::xml_node const expr,
                                                                            NodeConnector * /* node */,
                                                                            bool &wasCreated,
                                                                            ValueType /* returnType */) const
  {
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
      reportParserExceptionWithLocation(expr,
                                        "createExpression: Invalid FailureTypeValue");
      return NULL;
    }
  }

  template <>
  ValueType NamedConstantExpressionFactory<CommandHandleConstant>::check(char const *nodeId,
                                                                         pugi::xml_node const expr) const
  {
    checkNotEmpty(expr);
    switch (parseCommandHandleValue(expr.child_value())) {
    case COMMAND_SENT_TO_SYSTEM:
    case COMMAND_ACCEPTED:
    case COMMAND_RCVD_BY_SYSTEM:
    case COMMAND_FAILED:
    case COMMAND_DENIED:
    case COMMAND_SUCCESS:
      return COMMAND_HANDLE_TYPE; // is OK

    default:
      reportParserExceptionWithLocation(expr,
                                        "Invalid CommandHandleValue");
      return UNKNOWN_TYPE;
    }
  }

  template <>
  Expression *NamedConstantExpressionFactory<CommandHandleConstant>::allocate(pugi::xml_node const expr,
                                                                              NodeConnector * /* node */,
                                                                              bool &wasCreated,
                                                                              ValueType /* returnType */) const
  {
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
      reportParserExceptionWithLocation(expr,
                                        "createExpression: Invalid CommandHandleValue");
      return NULL;
    }
  }

// Convenience macros
#define ENSURE_NAMED_CONSTANT_FACTORY(CLASS) template class PLEXIL::NamedConstantExpressionFactory<CLASS >;

  // Named constants
  ENSURE_NAMED_CONSTANT_FACTORY(NodeStateConstant);
  ENSURE_NAMED_CONSTANT_FACTORY(NodeOutcomeConstant);
  ENSURE_NAMED_CONSTANT_FACTORY(FailureTypeConstant);
  ENSURE_NAMED_CONSTANT_FACTORY(CommandHandleConstant);

} // namespace PLEXIL
