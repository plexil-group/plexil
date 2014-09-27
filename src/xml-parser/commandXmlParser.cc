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
#include "interface-schema.hh"
#include "parser-utils.hh"
#include "resource-tags.hh"

#include "pugixml.hpp"

using pugi::xml_node;

namespace PLEXIL
{
  // First pass
  void constructCommand(Node *node, xml_node const &cmd)
    throw (ParserException)
  {
    checkHasChildElement(cmd);
    ResourceList resources;
    xml_node temp = cmd.first_child();

    // Optional ResourceList
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

        resources.push_back(ResourceMap());
        ResourceMap &map = resources.back();
        for (xml_node child3 = resourceElt.first_child(); 
             child3; 
             child3 = child3.next_sibling()) {
          bool wasCreated;
          Expression *valueExpr = createExpression(child3.first_child(), node, wasCreated);
          map.insert(std::pair<std::string, Expression *>(child3.name(), valueExpr));
          if (wasCreated)
            garbage.push_back(valueExpr);
        }
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
   
    CommandNode *cnode = dynamic_cast<CommandNode *>(node);
    assertTrue_2(cnode, "Node is not a CommandNode");
    cnode->setCommand(new Command(resources, node->getNodeId()));
  }


  // Second pass
  void finalizeCommand(Node *node, xml_node const &cmd)
    throw (ParserException)
  {
    CommandNode *cnode = dynamic_cast<CommandNode *>(node);
    assertTrue_2(cnode, "Node is not a CommandNode");

    xml_node temp = cmd.first_child();

    // Optional ResourceList was parsed in first pass
    if (testTag(RESOURCE_LIST_TAG, temp))
      temp = temp.next_sibling();

    // Optional destination expression
    if (!testTag(NAME_TAG, temp)) {
      bool destIsGarbage = false;
      Assignable *dest = createAssignable(temp, node, destIsGarbage);
      cnode->setDestination(dest, destIsGarbage);
      temp = temp.next_sibling();
    }

    // Required command name expression
    checkTag(NAME_TAG, temp);
    bool nameIsGarbage = false;
    Expression *nameExpr = createExpression(temp.first_child(), node, nameIsGarbage);
    ValueType nameType = nameExpr->valueType();
    checkParserExceptionWithLocation(nameType == STRING_TYPE || nameType == UNKNOWN_TYPE,
                                     temp,
                                     "Command Name must be a String expression");
    cnode->setNameExpr(nameExpr, nameIsGarbage);

    // Optional arguments
    temp = temp.next_sibling();
    if (temp) {
      xml_node arg = temp.first_child();
      while (arg) {
        bool wasCreated = false;
        Expression *thisArg = createExpression(arg, node, wasCreated);
        cnode->addArgument(thisArg, wasCreated);
        arg = arg.next_sibling();
      }
    }
  }

} // namespace PLEXIL
