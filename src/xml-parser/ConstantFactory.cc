/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#include "ConcreteExpressionFactory.hh"

#include "Constant.hh"
#include "createExpression.hh"
#include "ExpressionConstants.hh"
#include "findDeclarations.hh"
#include "NodeConnector.hh"
#include "NodeConstantExpressions.hh"
#include "parser-utils.hh"
#include "ParserException.hh"
#include "PlexilSchema.hh"
#include "UserVariable.hh"

#include "pugixml.hpp"

#include <cstring>

using pugi::xml_node;

namespace PLEXIL
{

  //
  // Constant factories
  //

  // (What should be the) General case.
  // For all but string types, the value string may not be empty.

  // If partial function template specialization were allowed, we wouldn't have to
  // repeat this function definition for each flavor of Constant.
  // C++ sucks at abstraction.
  template <>
  ValueType factoryCheck<Constant<Boolean>>(char const *nodeId,
                                            pugi::xml_node const expr,
                                            ValueType desiredType)
  {
    checkParserExceptionWithLocation(expr.first_child() && *(expr.child_value()),
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Empty value is not valid for \"" << expr.name() << "\"");
    Boolean dummy;
    parseValue(expr.child_value(), dummy); // for effect
    return BOOLEAN_TYPE;
  }

  template <>
  ValueType factoryCheck<Constant<Integer>>(char const *nodeId,
                                      pugi::xml_node const expr,
                                      ValueType desiredType)
  {
    checkParserExceptionWithLocation(expr.first_child() && *(expr.child_value()),
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Empty value is not valid for \"" << expr.name() << "\"");
    Integer dummy;
    parseValue(expr.child_value(), dummy); // for effect
    return INTEGER_TYPE;
  }

  template <>
  ValueType factoryCheck<Constant<Real>>(char const *nodeId,
                                         pugi::xml_node const expr,
                                         ValueType desiredType)
  {
    checkParserExceptionWithLocation(expr.first_child() && *(expr.child_value()),
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Empty value is not valid for \"" << expr.name() << "\"");
    Real dummy;
    parseValue(expr.child_value(), dummy); // for effect
    return REAL_TYPE;
  }

  // String can be empty, don't care about contents
  template <>
  ValueType factoryCheck<Constant<String>>(char const * /* nodeId */,
                                           pugi::xml_node const /* expr */,
                                           ValueType /* desiredType */)
  {
    return STRING_TYPE;
  }

  //
  // Constant allocators
  //

  // Since there are exactly 3 possible Boolean constants, return references to them,
  // instead of constructing them anew.
  template <>
  Expression * factoryAllocate<Constant<Boolean>>(xml_node const expr,
                                                  NodeConnector * /* node */,
                                                  bool &wasCreated,
                                                  ValueType /* returnType */) 
  {
    bool value;
    bool known = parseValue(expr.child_value(), value);
    // if we got here, there was no parsing exception
    wasCreated = false;
    if (!known)
      return UNKNOWN_BOOLEAN_EXP();
    else if (value)
      return TRUE_EXP();
    else
      return FALSE_EXP();
  }

  // Look for common Integer values, e.g. 1, 0, -1
  template <>
  Expression *factoryAllocate<Constant<Integer>>(xml_node const expr,
                                                 NodeConnector * /* node */,
                                                 bool &wasCreated,
                                                 ValueType /* returnType */)
  {
    // check for empty value
    Integer value;
    if (!parseValue<Integer>(expr.child_value(), value)) {
      // Unknown
      wasCreated = true;
      return new Constant<Integer>();
    }

    // Known
    switch (value) {
    case 1:
      wasCreated = false;
      return INT_ONE_EXP();

    case 0:
      wasCreated = false;
      return INT_ZERO_EXP();

    case -1:
      wasCreated = false;
      return INT_MINUS_ONE_EXP();

    default:
      wasCreated = true;
      return new Constant<Integer>(value);
    }
  }

  // Look for common Real values, e.g. 1, 0, -1
  template <>
  Expression *factoryAllocate<Constant<Real>>(xml_node const expr,
                                              NodeConnector * /* node */,
                                              bool &wasCreated,
                                              ValueType /* returnType */)
  {
    // check for empty value
    Real value;
    if (!parseValue<Real>(expr.child_value(), value)) {
      // Unknown
      wasCreated = true;
      return new Constant<Real>();
    }

    // Known
    if (value == 1) {
      wasCreated = false;
      return REAL_ONE_EXP();
    }
    else if (value == 0) {
      wasCreated = false;
      return REAL_ZERO_EXP();
    }
    else if (value == -1) {
      wasCreated = false;
      return REAL_MINUS_ONE_EXP();
    }
    else {
      wasCreated = true;
      return new Constant<Real>(value);
    }
  }

  template <>
  Expression *factoryAllocate<Constant<String>>(xml_node const expr,
                                                NodeConnector * /* node */,
                                                bool &wasCreated,
                                                ValueType /* returnType */)
  {
    wasCreated = true;
    return new Constant<String>(expr.child_value());
  }

  // Explicit instantiations
  ENSURE_EXPRESSION_FACTORY(Constant<Boolean>);
  ENSURE_EXPRESSION_FACTORY(Constant<Integer>);
  ENSURE_EXPRESSION_FACTORY(Constant<Real>);
  ENSURE_EXPRESSION_FACTORY(Constant<String>);


  //
  // Named constant methods
  //

  template <>
  ValueType factoryCheck<NodeStateConstant>(char const * /* nodeId */,
                                            pugi::xml_node const expr,
                                            ValueType /* desiredType*/)
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
  Expression *factoryAllocate<NodeStateConstant>(pugi::xml_node const expr,
                                                 NodeConnector * /* node */,
                                                 bool &wasCreated,
                                                 ValueType /* returnType */)
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
      return nullptr;
    }
  }

  template <>
  ValueType factoryCheck<NodeOutcomeConstant>(char const * /* nodeId */,
                                              pugi::xml_node const expr,
                                              ValueType /* desiredType*/)
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
  Expression *factoryAllocate<NodeOutcomeConstant>(pugi::xml_node const expr,
                                                   NodeConnector * /* node */,
                                                   bool &wasCreated,
                                                   ValueType /* returnType */)
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
      return nullptr;
    }
  }

  template <>
  ValueType factoryCheck<FailureTypeConstant>(char const * /* nodeId */,
                                              pugi::xml_node const expr,
                                              ValueType /* desiredType*/)
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
  Expression *factoryAllocate<FailureTypeConstant>(pugi::xml_node const expr,
                                                   NodeConnector * /* node */,
                                                   bool &wasCreated,
                                                   ValueType /* returnType */)
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
      return nullptr;
    }
  }

  template <>
  ValueType factoryCheck<CommandHandleConstant>(char const * /* nodeId */,
                                                pugi::xml_node const expr,
                                                ValueType /* desiredType*/)
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
  Expression *factoryAllocate<CommandHandleConstant>(pugi::xml_node const expr,
                                                     NodeConnector * /* node */,
                                                     bool &wasCreated,
                                                     ValueType /* returnType */)
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
      return nullptr;
    }
  }

  // Named constants
  ENSURE_EXPRESSION_FACTORY(NodeStateConstant);
  ENSURE_EXPRESSION_FACTORY(NodeOutcomeConstant);
  ENSURE_EXPRESSION_FACTORY(FailureTypeConstant);
  ENSURE_EXPRESSION_FACTORY(CommandHandleConstant);

} // namespace PLEXIL
