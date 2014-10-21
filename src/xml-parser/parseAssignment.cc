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
#include "PlexilSchema.hh"

#include "pugixml.hpp"

#include <cerrno>
#include <cstdlib> // for strtoul()
#include <limits>

using pugi::xml_node;

namespace PLEXIL
{

  // First pass
  void parsePriority(Node *node, xml_node prio)
  throw (ParserException)
  {
    AssignmentNode *anode = dynamic_cast<AssignmentNode *>(node);
    assertTrue_2(anode, "parsePriority: Not an AssignmentNode");

    char const *prioString = prio.child_value();
    checkParserExceptionWithLocation(*prioString,
                                     prio,
                                     "Priority element is empty");
    char *endptr = NULL;
    errno = 0;
    unsigned long prioValue = strtoul(prioString, &endptr, 10);
    checkParserExceptionWithLocation(endptr != prioString && !*endptr,
                                     prio,
                                     "Priority element does not contain a non-negative integer");
    checkParserExceptionWithLocation(!errno,
                                     prio,
                                     "Priority element contains negative or out-of-range integer");
    checkParserExceptionWithLocation(prioValue < std::numeric_limits<int32_t>::max(),
                                     prio,
                                     "Priority element contains out-of-range integer");
    anode->setPriority((int32_t) prioValue);
  }

  // First pass
  void constructAssignment(Node *node, xml_node assn)
  throw (ParserException)
  {
    xml_node varXml = assn.first_child();
    xml_node rhsXml = varXml.next_sibling();
    checkParserExceptionWithLocation(rhsXml,
                                     assn,
                                     "Assignment Node " << node->getNodeId()
                                     << ": Malformed Assignment element");
    checkTagSuffix(RHS_TAG, rhsXml);
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
    Assignable *var = createAssignable(temp, node, varGarbage);
    assertTrue_2(var, "finalizeAssignment: Internal error: null LHS expression");
    temp = temp.next_sibling().first_child();
    bool rhsGarbage;
    Expression *rhs = NULL;
    try {
      rhs = createExpression(temp, node, rhsGarbage);
    }
    catch (ParserException &e) {
      if (varGarbage)
        delete var;
      throw;
    }
    assertTrue_2(rhs, "finalizeAssignment: Internal error: null RHS expression");
    if (!areTypesCompatible(var->valueType(), rhs->valueType())) {
      if (varGarbage)
        delete var;
      if (rhsGarbage)
        delete rhs;
      checkParserExceptionWithLocation(ALWAYS_FAIL,
                                       temp,
                                       "Assignment Node " << node->getNodeId()
                                       << ": Expression type mismatch with assignment variable");
    }
    assign->setVariable(var, varGarbage);
    assign->setExpression(rhs, rhsGarbage);
  }

} // namespace PLEXIL
