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

#include "Assignable.hh"
#include "Assignment.hh"
#include "AssignmentNode.hh"
#include "createExpression.hh"
#include "parser-utils.hh"
#include "ParserException.hh"
#include "PlexilSchema.hh"

#include "pugixml.hpp"

#include <limits>

#include <cerrno>
#include <cstdlib>  // strtoul()

using pugi::xml_node;

namespace PLEXIL
{

  // Check pass
  void checkAssignmentBody(char const *nodeId, xml_node const assnXml)
  {
    checkTag(ASSN_TAG, assnXml);

    xml_node const varXml = assnXml.first_child();
    checkParserExceptionWithLocation(varXml,
                                     assnXml,
                                     "Assignment Node \"" << nodeId
                                     << "\": Malformed Assignment element");

    checkParserExceptionWithLocation(testTagSuffix(VAR_SUFFIX, varXml)
                                     || testTag(ARRAYELEMENT_TAG, varXml),
                                     varXml,
                                     "Assignment Node \"" << nodeId
                                     << "\": invalid left hand side for Assignment");
    ValueType lht = checkExpression(nodeId, varXml);

    xml_node const rhsXml = varXml.next_sibling();
    checkParserExceptionWithLocation(rhsXml,
                                     assnXml,
                                     "Assignment Node \"" << nodeId
                                     << "\": Malformed Assignment element");
    checkParserExceptionWithLocation(testTagSuffix(RHS_TAG, rhsXml),
                                     assnXml,
                                     "Assignment Node \"" << nodeId
                                     << "\": Invalid right hand side for Assignment");
    ValueType rht = checkExpression(nodeId, rhsXml.first_child(), lht);

    // Check type consistency between variable (or array elt) and RHS expression
    checkParserExceptionWithLocation(areTypesCompatible(lht, rht),
                                     assnXml,
                                     "Assignment Node \"" << nodeId
                                     << "\": Type error; variable has type " << valueTypeName(lht)
                                     << " but right hand side has type " << valueTypeName(rht));
  }

  // Second pass
  void constructAssignment(AssignmentNode *anode, xml_node const xml)
  {
    assertTrue_1(anode);
    // Just construct it, will be populated in third pass
    anode->setAssignment(new Assignment());
  }

  // Third pass
  void finalizeAssignment(AssignmentNode *anode, xml_node const assn)
  {
    assertTrue_1(anode);
    Assignment *assign = anode->getAssignment();
    assertTrue_2(anode, "finalizeAssignment: AssignmentNode without an Assignment");
    xml_node temp = assn.first_child();
    bool varGarbage = false;
    Assignable *var = createAssignable(temp, anode, varGarbage);
    assertTrue_2(var, "finalizeAssignment: Internal error: null LHS expression");
    ValueType varType = var->valueType();
    temp = temp.next_sibling().first_child();
    bool rhsGarbage = false;
    Expression *rhs = nullptr;
    try {
      rhs = createExpression(temp, anode, rhsGarbage, varType);
    }
    catch (ParserException &e) {
      if (varGarbage)
        delete var;
      throw;
    }
    assertTrue_2(rhs, "finalizeAssignment: Internal error: null RHS expression");
    ValueType rhsType = rhs->valueType();
    if (!areTypesCompatible(varType, rhsType)) {
      if (varGarbage)
        delete var;
      if (rhsGarbage)
        delete rhs;
      reportParserExceptionWithLocation(assn,
                                        "Assignment Node \"" << anode->getNodeId()
                                        << "\": Type error; variable has type " << valueTypeName(varType)
                                        << " but right hand side has type " << valueTypeName(rhsType));
    }
    assign->setVariable(var, varGarbage);
    assign->setExpression(rhs, rhsGarbage);
  }

} // namespace PLEXIL
