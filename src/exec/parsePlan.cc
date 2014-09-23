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
#include "CommandNode.hh"
#include "commandXmlParser.hh"
#include "Expression.hh"
#include "ExpressionFactory.hh"
#include "ListNode.hh"
#include "LibraryCallNode.hh"
//#include "Node.hh" // redundant
#include "parser-utils.hh"
#include "PlexilSchema.hh"
#include "UpdateNode.hh"
#include "updateXmlParser.hh"

#include "pugixml.hpp"

using pugi::xml_attribute;
using pugi::xml_document;
using pugi::xml_node;
using pugi::node_element;
using pugi::node_pcdata;

#include <cstring>

namespace PLEXIL
{
  static void parseGlobalDeclarations(xml_node declXml)
  {
    // NYI
  }

  static void parseVariableDeclarations(Node *node, xml_node decls)
  {
    xml_node decl = decls.first_child();
    while (decl) {
      checkHasChildElement(decl);
      Expression *var = createExpression(decl, node);
      checkParserExceptionWithLocation(node->addVariable(decl.child_value("Name"), var),
                                       decl.child("Name"),
                                       "Node " << node->getNodeId()
                                       << ": Duplicate variable name "
                                       << decl.child_value("Name"));
      decl = decl.next_sibling();
    }
  }

  // First pass parsing of In interfaces
  static void parseIn(Node *node, xml_node inXml)
  {
    // TODO
  }

  // First pass parsing of InOut interfaces
  static void parseInOut(Node *node, xml_node inOutXml)
  {
    // TODO
  }

  // First pass
  static void parseInterface(Node *node, xml_node iface)
  {
    xml_node elt = iface.first_child();
    while (elt) {
      if (0 == strcmp(IN_TAG, elt.name()))
        parseIn(node, elt);
      else if (0 == strcmp(INOUT_TAG, elt_name()))
        parseInOut(node, elt);
      else
        checkParserExceptionWithLocation(ALWAYS_FAIL,
                                         elt,
                                         "Node " << node->getNodeId()
                                         << ": Illegal " << elt.name() << " element inside " << INTERFACE_TAG);
      elt = elt.next_sibling();
    }
  }

  static Assignment *assignmentXmlParser(xml_node assn, Node *node)
  {
    checkHasChildElement(assn);
    xml_node varXml = assn.first_child();
    checkNotEmpty(varXml);
    bool varGarbage, rhsGarbage;
    Assignable *var = createAssignable(assn.first_child(), node, varGarbage);
    xml_node rhsXml = varXml.next_sibling();
    checkTagSuffix("RHS", rhsXml);
    checkHasChildElement(rhsXml);
    rhsXml = rhsXml.first_child();
    bool rhsGarbage;
    Expression *rhs = createExpression(rhsXml, node, rhsGarbage);
    checkParserExceptionWithLocation(areTypesCompatible(var->valueType(), rhs->valueType()),
                                     rhsXml,
                                     "Assignment Node " << node->getNodeId()
                                     << ": Expression type mismatch with assignment variable");
    return new Assignment(var, rhs, varGarbage, rhsGarbage, node->getNodeId());
  }

  static Node *parseNode(xml_node xml, Node *parent)
  {
    xml_attribute const typeAttr = xml.attribute(NODETYPE_ATTR);
    checkParserExceptionWithLocation(typeAttr,
                                     xml,
                                     "Node has no " << NODETYPE_ATTR << " attribute");
    PlexilNodeType nodeType = parseNodeType(typeAttr.value());
    checkParserExceptionWithLocation(nodeType >= NodeType_NodeList && nodeType <= NodeType_LibraryNodeCall,
                                     xml, // should be attribute
                                     "Invalid node type \"" << typeAttr.value << "\"");

    // Where to put the things we parse on the first pass
    xml_node id;
    xml_node prio;
    xml_node iface;
    xml_node varDecls;
    xml_node body;

    xml_node temp = xml.first_child();
    while (temp) {
      checkParserExceptionWithLocation(temp.type() == node_element,
                                       temp,
                                       "Non-element found at top level of node");
      char const *tag = temp.name();
      size_t taglen = strlen(tag);
      switch (taglen) {
      case 6: // NodeId
        checkParserExceptionWithLocation(0 == strcmp(NODEID_TAG, tag),
                                         temp, 
                                         "Illegal element \"" << tag << "\" in Node");
        checkParserExceptionWithLocation(!id,
                                         temp, 
                                         "Duplicate " << tag << " element in Node");
        checkNotEmpty(temp);
        id = temp;
        break;

      case 7: // Comment
        checkParserExceptionWithLocation(0 == strcmp(COMMENT_TAG, tag),
                                         temp, 
                                         "Illegal element \"" << tag << "\" in Node");
        break;

      case 8: // NodeBody, Priority
        if (0 == strcmp(BODY_TAG, tag)) {
          checkParserExceptionWithLocation(!body,
                                           temp, 
                                           "Duplicate " << tag << " element in Node");
          body = temp;
          break;
        }
        else if (0 == strcmp(PRIORITY_TAG, tag)) {
          checkParserExceptionWithLocation(!prio,
                                           temp, 
                                           "Duplicate " << tag << " element in Node");
          checkParserExceptionWithLocation(nodeType == NodeType_Assignment,
                                           temp,
                                           "Only Assignment nodes may have a Priority");
          prio = temp;
          break;
        }
        checkParserExceptionWithLocation(ALWAYS_FAIL,
                                         temp,
                                         "Illegal element \"" << tag << "\" in Node");
        break;

      case 9: // Interface
        checkParserExceptionWithLocation(0 == strcmp(INTERFACE_TAG, tag),
                                         temp, 
                                         "Illegal element \"" << tag << "\" in Node");
        checkParserExceptionWithLocation(!iface,
                                         temp, 
                                         "Duplicate " << tag << " element in Node");
        iface = temp;
        break;

      case 12: // EndCondition, PreCondition
        if (0 == strcmp(END_CONDITION_TAG, tag)
            || 0 == strcmp(PRE_CONDITION_TAG, tag)) {
          break;
        }
        checkParserExceptionWithLocation(ALWAYS_FAIL,
                                         temp,
                                         "Illegal element \"" << tag << "\" in Node");
        break;

      case 13: // ExitCondition, PostCondition, SkipCondition
        if (0 == strcmp(EXIT_CONDITION_TAG, tag)
            || 0 == strcmp(POST_CONDITION_TAG, tag)
            || 0 == strcmp(SKIP_CONDITION_TAG, tag)) {
          break;
        }
        checkParserExceptionWithLocation(ALWAYS_FAIL,
                                         temp,
                                         "Illegal element \"" << tag << "\" in Node");
        break;

      case 14: // StartCondition
        checkParserExceptionWithLocation(0 == strcmp(START_CONDITION_TAG, tag),
                                         temp, 
                                         "Illegal element \"" << tag << "\" in Node");
        break;

      case 15: // RepeatCondition
        checkParserExceptionWithLocation(0 == strcmp(REPEAT_CONDITION_TAG, tag),
                                         temp, 
                                         "Illegal element \"" << tag << "\" in Node");
        break;

      case 18: // InvariantCondition
        checkParserExceptionWithLocation(0 == strcmp(INVARIANT_CONDITION_TAG, tag),
                                         temp, 
                                         "Illegal element \"" << tag << "\" in Node");
        break;

      case 20: // VariableDeclarations
        checkParserExceptionWithLocation(0 == strcmp(VAR_DECLS_TAG, tag),
                                         temp, 
                                         "Illegal element \"" << tag << "\" in Node");
        checkParserExceptionWithLocation(!varDecls,
                                         temp, 
                                         "Duplicate " << tag << " element in Node");
        varDecls = temp;
        break;

      default:
        checkParserExceptionWithLocation(ALWAYS_FAIL,
                                         temp,
                                         "Illegal element \"" << tag << "\" in Node");
        break;
      }

      temp = temp.next_sibling();
    }

    checkParserExceptionWithLocation(id,
                                     xml,
                                     "Node has no " << NODEID_TAG << " element");
    char const *name = id.child_value();

    // Superficial checks of node body before we construct node
    if (nodeBody) {
      checkParserExceptionWithLocation(nodeType != NodeType_Empty,
                                       nodeBody,
                                       "Empty Node \"" << name << "\" may not have a NodeBody element");
      checkHasChildElement(nodeBody);
      nodeBody = nodeBody.first_child(); // strip away NodeBody wrapper
      char const *bodyName = nodeBody.name();
      switch (nodeType) {
      case NodeType_Assignment:
        checkParserExceptionWithLocation(0 == strcmp(ASSIGNMENT_TAG, bodyName),
                                         nodeBody,
                                         "Assignment Node \"" << name << " missing Assignment body");
        break;

      case NodeType_Command:
        checkParserExceptionWithLocation(0 == strcmp(COMMAND_TAG, bodyName),
                                         nodeBody,
                                         "Command Node \"" << name << " missing Command body");
        break;

      case NodeType_LibraryNodeCall:
        checkParserExceptionWithLocation(0 == strcmp(LIBRARYNODECALL_TAG, bodyName),
                                         nodeBody,
                                         "LibraryNodeCall Node \"" << name << " missing LibraryNodeCall body");
        break;

      case NodeType_NodeList:
        checkParserExceptionWithLocation(0 == strcmp(NODELIST_TAG, bodyName),
                                         nodeBody,
                                         "NodeList Node \"" << name << " missing NodeList body");
        checkHasChildElement(nodeBody);
        break;

      case NodeType_Update:
        checkParserExceptionWithLocation(0 == strcmp(UPDATE_TAG, bodyName),
                                         nodeBody,
                                         "Update Node \"" << name << " missing Update body");
        break;

      default: // appease compiler
        break;
      }
    }
    else {
      checkParserExceptionWithLocation(nodeType == NodeType_Empty,
                                       xml,
                                       "Node \"" << name << "\" has no NodeBody element");
    }

    Node *result = NodeFactory::createNode(nodeType, name, parent);

    try {
      // Populate local variables
      if (varDecls)
        parseVariableDeclarations(node, varDecls);

      // Get interface variables
      // *** TODO: separate alias linking from construction of default initializer,
      // move construction of initializer to post-init phase in most cases
      if (iface)
        parseInterface(node, iface);

      // Construct body for NodeList, LibraryNodeCall nodes
      switch (nodeType) {
      case NodeType_NodeList:
        constructChildNodes(node, body);
        break;

      case NodeType_LibraryNodeCall:
        // TODO
        break;

      default:
        break;
      }
    }
    catch (std::exception const & exc) {
      delete result;
      throw;
    }
    return result;
  }

  // XML has already been checked for gross errors
  static void postInitNode(Node *node, xml_node const &xml)
  {
    // Where to put the things we parse on the second pass
    xml_node iface;
    xml_node varDecls;
    xml_node body;
    std::vector<xml_node> conditions;

    xml_node temp = xml.first_child();
    while (temp) {
      char const *tag = temp.name();
      size_t taglen = strlen(tag);
      switch (taglen) {
      case 8: // NodeBody, Priority
        if (0 == strcmp(BODY_TAG, tag))
          body = temp.first_child(); // strip off <NodeBody> element
        break;

      case 9: // Interface
        iface = temp;
        break;

      case 12: // EndCondition, PreCondition
      case 13: // ExitCondition, PostCondition, SkipCondition
      case 14: // StartCondition
      case 15: // RepeatCondition
      case 18: // InvariantCondition
        conditions.push_back(temp);
        break;

      case 20: // VariableDeclarations
        varDecls = temp;
        break;

      case 6: // NodeId
      case 7: // Comment
      default:
        break;
      }

      temp = temp.next_sibling();
    }

    // Construct body for Assignment, Command, Update nodes
    if (body)
      switch (node->getType()) {
      case NodeType_Assignment:
        ((AssignmentNode *)node)->setAssignment(assignmentXmlParser(body, node));
        break;

      case NodeType_Command:
        ((CommandNode *)node)->setCommand(commandXmlParser(body, node));
        break;

      case NodeType_Update:
        ((UpdateNode *)node)->setUpdate(updateXmlParser(body, node));
        break;

      default: // ignore
        break;
      }

    // Construct variable initializers here
    // It is only here after all child nodes and node bodies have been constructed
    // that all variables which could be referenced are accessible.
    if (varDecls) {
      xml_node decl = varDecls.first_child();
      while (decl) {
        xml_node initXml = decl.child("InitialValue");
        if (initXml) {
          char const *varName = decl.child_value("Name");
          Expression *var = node.findLocalVariable(std::string(varName));
          assertTrueMsg(var,
                        "postInitNode: Internal error: variable " << varName
                        << " not found in node " << node->getNodeId());
          // FIXME: is this needed/possible?
          checkParserExceptionWithLocation(var->isAssignable(),
                                           initXml,
                                           "This variable may not take an initializer");
          bool garbage;
          Expression *init = createExpression(initXml, node, garbage);
          checkParserExceptionWithLocation(areTypesCompatible(var->valueType(), init->valueType()),
                                           initXml,
                                           "Node " << node->getNodeId()
                                           << ": Initialization type mismatch for variable "
                                           << varName);
          var->asAssignable()->setInitializer(init, garbage);
        }
      }
    }

    // TODO: Construct alias default initializers here, if required
    if (iface) {
      // TODO
    }

    // Instantiate user conditions
    for (std::vector<xml_node>::const_iterator it = conditions.begin();
         it != conditions.end();
         ++it) {
      xml_node elt = *it;
      checkHasChildElement(elt);
      // Check that condition name is valid, get index
      Node::ConditionIndex which = Node::getConditionIndex(std::string(elt.name()));
      checkParserExceptionWithLocation(which >= Node::skip_idx && which <= Node::repeatIdx,
                                       "Node " << node->getNodeId()
                                       << ": Illegal condition name \"" << elt.name() << "\"");
      bool garbage;
      Expression *cond = createExpression(elt.first_child(), node, garbage);
      checkParserExceptionWithLocation(cond->valueType() == BOOLEAN_TYPE || cond->valueType() == UNKNOWN_TYPE,
                                       cond.first_child(),
                                       "Node " << node->getNodeId() << ": Expression for "
                                       << elt.name() << " is not Boolean");
      node->addUserCondition(which, cond, garbage);
    }

    // finalize conditions
    node->finalizeConditions();

    // recurse on children
    if (node->getType() == NodeType_NodeList || node->getType == NodeType_LibraryNodeCall) {
      std::vector<Node *> kids = node->getChildren();
      for (std::vector<Node *>::iterator kid = kids.begin(),
             xml_node kidXml = body.first_child();
           kid != kids.end() && kidXml;
           ++kid, kidXml = kidXml.next_sibling())
        postInitNode(kid, kidXml);
    }
  }

  Node *parsePlan(xml_node const &xml)
    throw(ParserException)
  {
    checkTag(PLEXIL_PLAN_TAG, xml);
    checkHasChildElement(xml);

    xml_node elt = xml.first_child();

    // Handle global declarations
    if (testTag(GLOBAL_DECLARATIONS_TAG, elt)) {
      parseGlobalDeclarations(elt);
      elt = elt.next_sibling();
    }

    checkTag(NODE_TAG, elt);
    Node *result = parseNode(xml, NULL);
    postInitNode(result, xml);
    return result;
  }

} // namespace PLEXIL
