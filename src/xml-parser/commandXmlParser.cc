/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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
#include "Command.hh"
#include "CommandNode.hh"
#include "Error.hh"
#include "ExpressionFactory.hh"
#include "ExprVec.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"
#include "SymbolTable.hh"

#include "pugixml.hpp"

#include <cstring>

using pugi::xml_node;

namespace PLEXIL
{

  // First pass
  static void checkResource(char const *nodeId, xml_node const resourceElt)
    throw (ParserException)
  {
    checkTag(RESOURCE_TAG, resourceElt);
    xml_node nameXml, prioXml;

    // check the fields
    for (xml_node rtemp = resourceElt.first_child();
         rtemp;
         rtemp = rtemp.next_sibling()) {
      char const* tag = rtemp.name();
      size_t taglen = strlen(tag);
      xml_node lowerBoundXml, upperBoundXml, releaseAtTermXml;
      switch (taglen) {
      case 12: // ResourceName
        checkParserExceptionWithLocation(!strcmp(RESOURCE_NAME_TAG, tag),
                                         rtemp,
                                         "Invalid " << tag << " element in Command Resource");
        checkParserExceptionWithLocation(!nameXml,
                                         rtemp,
                                         "Duplicate " << RESOURCE_NAME_TAG
                                         << " element in Command Resource");
        nameXml = rtemp;
        break;

      case 16: // ResourcePriority
        checkParserExceptionWithLocation(!strcmp(RESOURCE_PRIORITY_TAG, tag),
                                         rtemp,
                                         "Invalid " << tag << " element in Command Resource");
        checkParserExceptionWithLocation(!prioXml,
                                         rtemp,
                                         "Duplicate " << RESOURCE_PRIORITY_TAG << " element in Command Resource");
        prioXml = rtemp;
        break;

      case 18: // ResourceLowerBound, ResourceUpperBound
        if (!strcmp(RESOURCE_LOWER_BOUND_TAG, tag)) {
          checkParserExceptionWithLocation(!lowerBoundXml,
                                           rtemp,
                                           "Duplicate " << RESOURCE_LOWER_BOUND_TAG << " element in Command Resource");
          lowerBoundXml = rtemp;
        }
        else {
          checkParserExceptionWithLocation(!strcmp(RESOURCE_UPPER_BOUND_TAG, tag),
                                           rtemp,
                                           "Invalid " << tag << " element in Command Resource");
          checkParserExceptionWithLocation(!upperBoundXml,
                                           rtemp,
                                           "Duplicate " << RESOURCE_UPPER_BOUND_TAG << " element in Command Resource");
          upperBoundXml = rtemp;
        }
        break;

      default:
        checkParserExceptionWithLocation(0 == strcmp(RESOURCE_RELEASE_AT_TERMINATION_TAG, tag),
                                         rtemp,
                                         "Invalid " << tag << " element in Command Resource");
        checkParserExceptionWithLocation(!releaseAtTermXml,
                                         rtemp,
                                         "Duplicate " << RESOURCE_RELEASE_AT_TERMINATION_TAG << " element in Command Resource");
        releaseAtTermXml = rtemp;
        break;
      }
    }
        
    // Check that name and priority were supplied
    checkParserExceptionWithLocation(nameXml,
                                     resourceElt,
                                     "Node \"" << nodeId
                                     << "\": No " << RESOURCE_NAME_TAG << " element for resource");
    checkParserExceptionWithLocation(prioXml,
                                     resourceElt,
                                     "Node \"" << nodeId
                                     << "\": No " << RESOURCE_PRIORITY_TAG << " element for resource");
  }

  // First pass
  static void checkResourceList(char const *nodeId, xml_node const resourceXml)
    throw (ParserException)
  {
    // Process resource list
    for (xml_node resourceElt = resourceXml.first_child(); 
         resourceElt;
         resourceElt = resourceElt.next_sibling())
      checkResource(nodeId, resourceElt);

    // Check for duplicate names
    for (xml_node resourceElt = resourceXml.first_child(); 
         resourceElt;
         resourceElt = resourceElt.next_sibling()) {
      xml_node rnameXml = resourceElt.child(RESOURCE_NAME_TAG).first_child(); 
      // Can only check if constant string supplied
      if (testTag(STRING_VAL_TAG, rnameXml)) {
        char const *rname = rnameXml.child_value();
        xml_node temp = resourceElt.next_sibling();
        while (temp) {
          xml_node tnameXml = temp.child(RESOURCE_NAME_TAG).first_child();
          // Can only check if constant string supplied
          if (testTag(STRING_VAL_TAG, tnameXml)) {
            checkParserExceptionWithLocation(strcmp(rname, tnameXml.child_value()),
                                             temp,
                                             "Node \"" << nodeId
                                             << "\": Duplicate resource name \"" << rname << '"');
          }
          temp = temp.next_sibling();
        }
      }
    }
  }

  // First pass: XML checks
  void checkCommandBody(char const *nodeId, pugi::xml_node const cmdXml)
    throw (ParserException)
  {
    checkHasChildElement(cmdXml);
    xml_node temp = cmdXml.first_child();

    // Optional ResourceList
    if (testTag(RESOURCE_LIST_TAG, temp)) {
      checkResourceList(nodeId, temp);
      temp = temp.next_sibling();
    }

    // Optional destination expression
    // Ensure it's a user variable reference or ArrayElement
    if (testTagSuffix(VAR_SUFFIX, temp) || testTag(ARRAYELEMENT_TAG, temp)) {
      temp = temp.next_sibling();
    }

    // Required command name expression
    checkTag(NAME_TAG, temp);
    checkHasChildElement(temp);

    // Optional arguments
    temp = temp.next_sibling();
    if (temp)
      checkTag(ARGS_TAG, temp);
  }

  // Pass 3
  static void finalizeResourceList(NodeConnector *node,
                                   Command *cmd,
                                   xml_node const rlist)
    throw (ParserException)
  {
    ResourceList *resources =
      new ResourceList(std::distance(rlist.begin(), rlist.end()));
    size_t n = 0;
    try {
      for (xml_node resourceElt = rlist.first_child(); 
           resourceElt;
           resourceElt = resourceElt.next_sibling()) {
        checkTag(RESOURCE_TAG, resourceElt);
        // Update ResourceSpec in place
        ResourceSpec &rspec = (*resources)[n++];
        for (xml_node rtemp = resourceElt.first_child();
             rtemp;
             rtemp = rtemp.next_sibling()) {
          char const* tag = rtemp.name();
          size_t taglen = strlen(tag);
          bool isGarbage = false;
          Expression *exp = NULL;
          switch (taglen) {
          case 12: // ResourceName
            assertTrueMsg(!strcmp(RESOURCE_NAME_TAG, tag),
                          "finalizeResourceList: unexpected tag \"" << tag << '"');
            exp = createExpression(rtemp.first_child(), node, isGarbage);
            checkParserExceptionWithLocation(exp->valueType() == STRING_TYPE || exp->valueType() == UNKNOWN_TYPE,
                                             rtemp.first_child(),
                                             RESOURCE_NAME_TAG << " expression is not String valued in Command Resource");
            rspec.setNameExpression(exp, isGarbage);
            break;

          case 16: // ResourcePriority
            assertTrueMsg(!strcmp(RESOURCE_PRIORITY_TAG, tag),
                          "finalizeResourceList: unexpected tag \"" << tag << '"');
            exp = createExpression(rtemp.first_child(), node, isGarbage);
            checkParserExceptionWithLocation(exp->valueType() == INTEGER_TYPE || exp->valueType() == UNKNOWN_TYPE,
                                             rtemp.first_child(),
                                             RESOURCE_PRIORITY_TAG << " expression is not Integer valued in Command Resource");
            rspec.setPriorityExpression(exp, isGarbage);
            break;

          case 18: // ResourceLowerBound, ResourceUpperBound
            if (!strcmp(RESOURCE_LOWER_BOUND_TAG, tag)) {
              exp = createExpression(rtemp.first_child(), node, isGarbage);
              checkParserExceptionWithLocation(isNumericType(exp->valueType()) || exp->valueType() == UNKNOWN_TYPE,
                                               rtemp.first_child(),
                                               RESOURCE_LOWER_BOUND_TAG << " expression is not a numeric expression in Command Resource");
              rspec.setLowerBoundExpression(exp, isGarbage);
            }
            else if (!strcmp(RESOURCE_UPPER_BOUND_TAG, tag)) {
              exp = createExpression(rtemp.first_child(), node, isGarbage);
              checkParserExceptionWithLocation(isNumericType(exp->valueType()) || exp->valueType() == UNKNOWN_TYPE,
                                               rtemp.first_child(),
                                               RESOURCE_UPPER_BOUND_TAG << " expression is not a numeric expression in Command Resource");
              rspec.setUpperBoundExpression(exp, isGarbage);
            }
            else {
              assertTrueMsg(ALWAYS_FAIL,
                            "finalizeResourceList: unexpected tag \"" << tag << '"');
            }
            break;

          default:
            assertTrueMsg(!strcmp(RESOURCE_RELEASE_AT_TERMINATION_TAG, tag),
                          "finalizeResourceList: unexpected tag \"" << tag << '"');
            exp = createExpression(rtemp.first_child(), node, isGarbage);
            checkParserExceptionWithLocation(exp->valueType() == BOOLEAN_TYPE || exp->valueType() == UNKNOWN_TYPE,
                                             rtemp.first_child(),
                                             RESOURCE_RELEASE_AT_TERMINATION_TAG << " expression is not a Boolean expression in Command");
            rspec.setReleaseAtTerminationExpression(exp, isGarbage);
            break;
          }
        }
      }
    }
    catch (ParserException const &e) {
      delete resources;
      throw;
    }

    cmd->setResourceList(resources);
  }

  // Pass 3
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
    xml_node destXml;
    Expression *dest = NULL;
    if (!testTag(NAME_TAG, temp)) {
      destXml = temp;
      bool destIsGarbage = false;
      dest = createAssignable(temp, node, destIsGarbage);
      cmd->setDestination(dest, destIsGarbage);
      temp = temp.next_sibling();
    }

    // Required command name expression
    checkTag(NAME_TAG, temp); // belt-and-suspenders check
    xml_node nameElt = temp.first_child();
    bool nameIsGarbage = false;
    Expression *nameExpr = createExpression(nameElt, node, nameIsGarbage);
    ValueType nameType = nameExpr->valueType();
    if (nameType != STRING_TYPE && nameType != UNKNOWN_TYPE) {
      if (nameIsGarbage)
        delete nameExpr;
      reportParserExceptionWithLocation(temp,
                                        "Command Name must be a String expression");
    }

    cmd->setNameExpr(nameExpr, nameIsGarbage);

    // Get symbol table entry, if name is constant and declared as command
    Symbol const *cmdSym = NULL;
    if (nameExpr->isConstant() && nameType == STRING_TYPE) {
      std::string cmdName = nameExpr->valueString();
      cmdSym = getCommandSymbol(cmdName.c_str());
    }

    if (cmdSym && dest) {
      // Check destination consistency with command declaration
      checkParserExceptionWithLocation(areTypesCompatible(dest->valueType(),
                                                          cmdSym->returnType()),
                                       destXml,
                                       "Command " << cmdSym->name() << " returns type "
                                       << cmdSym->returnType() << ", but result variable has type "
                                       << dest->valueType());
    }

    // Optional arguments
    temp = temp.next_sibling();
    if (temp) {
      size_t n = 0;
      xml_node arg;
      for (arg = temp.first_child(); arg; arg = arg.next_sibling())
        ++n;
      if (cmdSym) {
        // Check argument count against command declaration
        checkParserExceptionWithLocation(n == cmdSym->parameterCount()
                                         || (cmdSym->anyParameters() && n > cmdSym->parameterCount()),
                                         temp,
                                         "Command " << cmdSym->name() << " expects "
                                         << (cmdSym->anyParameters() ? "at least " : "")
                                         << cmdSym->parameterCount() << " arguments, but was supplied "
                                         << n);
      }
      if (n) {
        ExprVec *argVec = makeExprVec(n);
        cmd->setArgumentVector(argVec);

        size_t i = 0;
        for (arg = temp.first_child(); arg; arg = arg.next_sibling(), ++i) {
          bool wasCreated = false;
          Expression *thisArg = createExpression(arg, node, wasCreated);
          argVec->setArgument(i, thisArg, wasCreated);

          if (cmdSym && i < cmdSym->parameterCount()) {
            // Check argument type against declaration
            ValueType actual = thisArg->valueType();
            ValueType expected = cmdSym->parameterType(i);
            checkParserExceptionWithLocation(areTypesCompatible(expected, actual),
                                             arg,
                                             "Parameter " << i << " to command "
                                             << cmdSym->name() << " should be of type "
                                             << valueTypeName(expected)
                                             << ", but has type "
                                             << valueTypeName(actual));
          }
        }
      }
    }
  }


} // namespace PLEXIL
