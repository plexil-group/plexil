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

#include "Assignable.hh"
#include "Assignment.hh"
#include "AssignmentNode.hh"
#include "Error.hh"
#include "ExpressionFactory.hh"
#include "parser-utils.hh"

#include "pugixml.hpp"

#include <cerrno>
#include <cstdlib> // for strtoul()

using pugi::xml_node;

namespace PLEXIL
{

  // First pass
  void parsePriority(Node *node, xml_node prio)
  throw (ParserException)
  {
    char const *prioString = prio.child_value();
    char *endptr = NULL;
    unsigned long prioValue = strtoul(prioString, &endptr, 10);
    checkParserExceptionWithLocation(endptr != prioString && !*endptr,
                                     prio,
                                     "Priority element does not contain a non-negative integer");
    checkParserExceptionWithLocation(!errno,
                                     prio,
                                     "Priority element contains negative or out-of-range integer");
    checkParserExceptionWithLocation(prioValue < INT32_MAX,
                                     prio,
                                     "Priority element contains out-of-range integer");
    AssignmentNode *anode = dynamic_cast<AssignmentNode *>(node);
    assertTrue_2(anode, "parsePriority: Not an AssignmentNode");
    anode->setPriority((int32_t) prioValue);
  }

  // First pass
  void constructAssignment(Node *node, xml_node assn)
  throw (ParserException)
  {
    checkHasChildElement(assn);
    xml_node varXml = assn.first_child();
    // TODO: check that varXml is a variable reference or ArrayElement
    xml_node rhsXml = varXml.next_sibling();
    checkParserExceptionWithLocation(rhsXml,
                                     assn,
                                     "Assignment Node " << node->getNodeId()
                                     << ": Assignment missing value expression");
    checkTagSuffix("RHS", rhsXml);
    checkHasChildElement(rhsXml);
    rhsXml = rhsXml.first_child();
    AssignmentNode *anode = dynamic_cast<AssignmentNode *>(node);
    assertTrue_2(anode, "constructAssignment: Not an AssignmentNode");
    anode->setAssignment(new Assignment(node->getNodeId()));
  }

  // Second pass
  void finalizeAssignment(Node *node, xml_node assn)
  throw (ParserException)
  {
    AssignmentNode *anode = dynamic_cast<AssignmentNode *>(node);
    assertTrue_2(anode, "finalizeAssignment: Not an AssignmentNode");
    Assignment *assign = anode->getAssignment();
    assertTrue_2(anode, "finalizeAssignment: AssignmentNode without an Assignment");
    xml_node temp = assn.first_child();
    bool varGarbage = false;
    Expression *exp = createExpression(temp, node, varGarbage);
    Assignable *var = exp->asAssignable();
    checkParserExceptionWithLocation(var,
                                     temp,
                                     "Assignment Node " << node->getNodeId()
                                     << ": Destination expression is read-only");
    temp = temp.next_sibling().first_child();
    bool rhsGarbage;
    Expression *rhs = createExpression(temp, node, rhsGarbage);
    checkParserExceptionWithLocation(areTypesCompatible(var->valueType(), rhs->valueType()),
                                     temp,
                                     "Assignment Node " << node->getNodeId()
                                     << ": Expression type mismatch with assignment variable");
    assign->setVariable(var, varGarbage);
    assign->setExpression(rhs, rhsGarbage);
  }

} // namespace PLEXIL
