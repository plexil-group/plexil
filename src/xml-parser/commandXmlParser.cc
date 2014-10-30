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

#include "Command.hh"
#include "CommandNode.hh"
#include "Error.hh"
#include "ExpressionFactory.hh"
#include "ExprVec.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"
#include "resource-tags.hh"

#include "pugixml.hpp"

#include <cstring>

using pugi::xml_node;

namespace PLEXIL
{
  // First pass
  Command *constructCommand(NodeConnector *node, xml_node const cmdXml)
    throw (ParserException)
  {
    checkHasChildElement(cmdXml);
    ResourceList resources;
    xml_node temp = cmdXml.first_child();

    // Optional ResourceList
    size_t n_resources = 0;
    if (testTag(RESOURCE_LIST_TAG, temp)) {
      // Process resource list
      for (xml_node resourceElt = temp.first_child(); 
           resourceElt;
           resourceElt = resourceElt.next_sibling()) {
        checkTag(RESOURCE_TAG, resourceElt);
        // check that the resource has a name and a priority
        checkParserExceptionWithLocation(resourceElt.child(RESOURCE_NAME_TAG),
                                         resourceElt,
                                         "No " << RESOURCE_NAME_TAG << " element for resource");
        checkParserExceptionWithLocation(resourceElt.child(RESOURCE_PRIORITY_TAG),
                                         resourceElt,
                                         "No " << RESOURCE_PRIORITY_TAG << " element for resource");
        // save rest for 2nd pass
        ++n_resources;
      }

      temp = temp.next_sibling();
    }

    // Optional destination expression
    if (!testTag(NAME_TAG, temp)) {
      // TODO: ensure it's a user variable reference or ArrayElement
      temp = temp.next_sibling();
    }

    // Required command name expression
    checkTag(NAME_TAG, temp);
    checkHasChildElement(temp);

    // Optional arguments
    temp = temp.next_sibling();
    if (temp)
      checkTag(ARGS_TAG, temp);

    Command *result = new Command(node->getNodeId());
    if (n_resources)
      result->getResourceList().resize(n_resources);
    return result;
  }

  void finalizeResourceList(NodeConnector *node, Command *cmd, xml_node const rlist)
  throw (ParserException)
  {
    ResourceList &resources = cmd->getResourceList();
    size_t n = 0;
    for (xml_node resourceElt = rlist.first_child(); 
         resourceElt;
         resourceElt = resourceElt.next_sibling()) {
      checkTag(RESOURCE_TAG, resourceElt);
      // Update ResourceSpec in place
      ResourceSpec &rspec = resources[n++];
      for (xml_node rtemp = resourceElt.first_child();
           rtemp;
           rtemp = rtemp.next_sibling()) {
        char const* tag = rtemp.name();
        size_t taglen = strlen(tag);
        bool isGarbage = false;
        Expression *exp = NULL;
        switch (taglen) {
        case 12: // ResourceName
          checkParserExceptionWithLocation(0 == strcmp(RESOURCE_NAME_TAG, tag),
                                           rtemp,
                                           "Invalid " << tag << " element in Command Resource");
          checkParserExceptionWithLocation(rspec.nameExp == NULL,
                                           rtemp,
                                           "Duplicate " << RESOURCE_NAME_TAG << " element in Command Resource");
          exp = createExpression(rtemp.first_child(), node, isGarbage);
          checkParserExceptionWithLocation(exp->valueType() == STRING_TYPE || exp->valueType() == UNKNOWN_TYPE,
                                           rtemp.first_child(),
                                           RESOURCE_NAME_TAG << " expression is not String valued in Command Resource");
          rspec.nameExp = exp;
          if (isGarbage)
            cmd->addGarbageExpression(exp);
          break;

        case 16: // ResourcePriority
          checkParserExceptionWithLocation(0 == strcmp(RESOURCE_PRIORITY_TAG, tag),
                                           rtemp,
                                           "Invalid " << tag << " element in Command Resource");
          checkParserExceptionWithLocation(rspec.priorityExp == NULL,
                                           rtemp,
                                           "Duplicate " << RESOURCE_PRIORITY_TAG << " element in Command Resource");
          exp = createExpression(rtemp.first_child(), node, isGarbage);
          checkParserExceptionWithLocation(exp->valueType() == INTEGER_TYPE || exp->valueType() == UNKNOWN_TYPE,
                                           rtemp.first_child(),
                                           RESOURCE_PRIORITY_TAG << " expression is not Integer valued in Command Resource");
          rspec.priorityExp = exp;
          if (isGarbage)
            cmd->addGarbageExpression(exp);
          break;

        case 18: // ResourceLowerBound, ResourceUpperBound
          if (0 == strcmp(RESOURCE_LOWER_BOUND_TAG, tag)) {
            checkParserExceptionWithLocation(rspec.lowerBoundExp == NULL,
                                             rtemp,
                                             "Duplicate " << RESOURCE_LOWER_BOUND_TAG << " element in Command Resource");
            exp = createExpression(rtemp.first_child(), node, isGarbage);
            checkParserExceptionWithLocation(isNumericType(exp->valueType()) || exp->valueType() == UNKNOWN_TYPE,
                                             rtemp.first_child(),
                                             RESOURCE_LOWER_BOUND_TAG << " expression is not a numeric expression in Command Resource");
            rspec.lowerBoundExp = exp;
            if (isGarbage)
              cmd->addGarbageExpression(exp);
          }
          else {
            checkParserExceptionWithLocation(0 == strcmp(RESOURCE_UPPER_BOUND_TAG, tag),
                                             rtemp,
                                             "Invalid " << tag << " element in Command Resource");
            checkParserExceptionWithLocation(rspec.upperBoundExp == NULL,
                                             rtemp,
                                             "Duplicate " << RESOURCE_UPPER_BOUND_TAG << " element in Command Resource");
            exp = createExpression(rtemp.first_child(), node, isGarbage);
            checkParserExceptionWithLocation(isNumericType(exp->valueType()) || exp->valueType() == UNKNOWN_TYPE,
                                             rtemp.first_child(),
                                             RESOURCE_UPPER_BOUND_TAG << " expression is not a numeric expression in Command Resource");
            rspec.upperBoundExp = exp;
            if (isGarbage)
              cmd->addGarbageExpression(exp);
          }
          break;

        default:
          checkParserExceptionWithLocation(0 == strcmp(RESOURCE_RELEASE_AT_TERMINATION_TAG, tag),
                                           rtemp,
                                           "Invalid " << tag << " element in Command Resource");
          checkParserExceptionWithLocation(rspec.releaseAtTermExp == NULL,
                                           rtemp,
                                           "Duplicate " << RESOURCE_RELEASE_AT_TERMINATION_TAG << " element in Command Resource");
          exp = createExpression(rtemp.first_child(), node, isGarbage);
          checkParserExceptionWithLocation(exp->valueType() == BOOLEAN_TYPE || exp->valueType() == UNKNOWN_TYPE,
                                           rtemp.first_child(),
                                           RESOURCE_RELEASE_AT_TERMINATION_TAG << " expression is not a Boolean expression in Command");
          rspec.releaseAtTermExp = exp;
          if (isGarbage)
            cmd->addGarbageExpression(exp);
          break;
        }
      }
    }
  }

  void finalizeCommand(Command *cmd, NodeConnector *node, xml_node const cmdXml)
    throw (ParserException)
  {
    xml_node temp = cmdXml.first_child();

    // Optional ResourceList needs expressions parsed
    if (testTag(RESOURCE_LIST_TAG, temp)) {
      finalizeResourceList(node, cmd, temp);
      temp = temp.next_sibling();
    }

    // Optional destination expression
    if (!testTag(NAME_TAG, temp)) {
      bool destIsGarbage = false;
      Assignable *dest = createAssignable(temp, node, destIsGarbage);
      cmd->setDestination(dest, destIsGarbage);
      temp = temp.next_sibling();
    }

    // Required command name expression
    checkTag(NAME_TAG, temp); // belt-and-suspenders check
    bool nameIsGarbage = false;
    Expression *nameExpr = createExpression(temp.first_child(), node, nameIsGarbage);
    ValueType nameType = nameExpr->valueType();
    if (nameType != STRING_TYPE && nameType != UNKNOWN_TYPE) {
      if (nameIsGarbage)
        delete nameExpr;
      checkParserExceptionWithLocation(ALWAYS_FAIL,
                                       temp,
                                       "Command Name must be a String expression");
    }
    cmd->setNameExpr(nameExpr, nameIsGarbage);

    // Optional arguments
    temp = temp.next_sibling();
    if (temp) {
      size_t n = 0;
      xml_node arg;
      for (arg = temp.first_child(); arg; arg = arg.next_sibling())
        ++n;
      if (n) {
        ExprVec *argVec = makeExprVec(n);
        size_t i = 0;
        for (arg = temp.first_child(); arg; arg = arg.next_sibling(), ++i) {
          bool wasCreated = false;
          Expression *thisArg = createExpression(arg, node, wasCreated);
          argVec->setArgument(i, thisArg, wasCreated);
        }
        cmd->setArgumentVector(argVec);
      }
    }
  }

} // namespace PLEXIL
