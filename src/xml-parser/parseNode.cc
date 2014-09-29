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

#include "parseNode.hh"

#include "Alias.hh"
#include "Assignable.hh"
#include "CommandNode.hh"
#include "commandXmlParser.hh"
#include "Error.hh"
#include "ExpressionFactory.hh"
#include "ListNode.hh"
#include "NodeFactory.hh"
#include "parseAssignment.hh"
#include "parseLibraryCall.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"
#include "UpdateNode.hh"
#include "updateXmlParser.hh"

#include "pugixml.hpp"

#include <cstdlib> // for strtoul()

using pugi::xml_attribute;
using pugi::xml_node;
using pugi::node_element;

namespace PLEXIL
{
  //
  // First pass
  //

  static void parseVariableDeclarations(Node *node, xml_node const &decls)
  {
    xml_node decl = decls.first_child();
    while (decl) {
      checkHasChildElement(decl);
      char const *name = decl.child_value("Name");
      if (node->findLocalVariable(std::string(name))) {
        checkParserExceptionWithLocation(ALWAYS_FAIL,
                                         decl.child("Name"),
                                         "Node " << node->getNodeId()
                                         << ": Duplicate variable name "
                                         << name);
      }
      // Variables are always created here, no need for "garbage" flag.
      node->addVariable(name, createExpression(decl, node));
      decl = decl.next_sibling();
    }
  }

  // For Interface specs; may have other uses.
  static ValueType getVarDeclType(xml_node const &decl)
  {
    xml_node typeElt = decl.child(TYPE_TAG);
    checkParserExceptionWithLocation(typeElt,
                                     decl,
                                     "Variable declaration lacks " << TYPE_TAG << " element");
    checkNotEmpty(typeElt);
    ValueType typ = parseValueType(typeElt.child_value());
    checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                     typeElt,
                                     "Unknown variable type name " << typeElt.child_value());
    checkParserExceptionWithLocation(isScalarType(typ),
                                     typeElt,
                                     "Invalid type name " << typeElt.child_value() << " for " << decl.name());
    if (testTag(DECL_ARRAY_TAG, decl))
      typ = arrayType(typ);
    else 
      checkParserExceptionWithLocation(testTag(DECL_VAR_TAG, decl),
                                       decl,
                                       "Only " << DECL_VAR_TAG << " and " << DECL_ARRAY_TAG << " are valid in this context");
    return typ;
  }

  static Expression *getInterfaceVar(Node *node, char const *name)
  {
    assertTrue_1(node && name);
    Node *parent = node->getParent();
    if (!parent)
      return NULL;
    return parent->findVariable(std::string(name),
                                (parent->getType() == NodeType_LibraryNodeCall));
  }

  static char const *getVarDeclName(xml_node const &decl)
  {
    checkHasChildElement(decl);
    xml_node nameXml = decl.first_child();
    checkTag(NAME_TAG, nameXml);
    char const *name = nameXml.child_value();
    checkParserExceptionWithLocation(*name,
                                     nameXml,
                                     "Empty " << NAME_TAG << " element in " << decl.name());
    return name;
  }

  //
  // N.B. There is a limited amount of checking we can do on interface variables in the first pass.
  // LibraryNodeCall aliases can't be expanded because some of the variables they can reference
  // (e.g. child node internal vars) may not exist yet. Same with default values.

  // First pass checking of one In interface variable
  static void checkInDecl(Node *node, xml_node const &inXml, bool isCall)
    throw (ParserException)
  {
    char const *name = getVarDeclName(inXml);
    checkParserExceptionWithLocation(!node->findLocalVariable(std::string(name)),
                                     inXml,
                                     "In interface variable " << name
                                     << " shadows another variable of same name in this node");
    getVarDeclType(inXml); // for effect
  }

  // First pass checking of one InOut interface
  static void checkInOutDecl(Node *node, xml_node const &inOutXml, bool isCall)
    throw (ParserException)
  {
    std::string const name(getVarDeclName(inOutXml));
    checkParserExceptionWithLocation(!node->findLocalVariable(name),
                                     inOutXml,
                                     "InOut interface variable " << name
                                     << " shadows another variable of same name in this node");
    getVarDeclType(inOutXml); // for effect
  }

  // First pass
  static void parseInterface(Node *node, xml_node const &iface)
    throw (ParserException)
  {
    // Figure out if this is a library node expansion
    Node *parent = node->getParent();
    bool isCall = (parent && parent->getType() == NodeType_LibraryNodeCall);
    
    xml_node elt = iface.first_child();
    while (elt) {
      if (0 == strcmp(IN_TAG, elt.name())) {
        xml_node decl = elt.first_child();
        while (decl) {
          checkInDecl(node, decl, isCall);
          decl = decl.next_sibling();
        }
      }
      else if (0 == strcmp(INOUT_TAG, elt.name())) {
        xml_node decl = elt.first_child();
        while (decl) {
          checkInOutDecl(node, decl, isCall);
          decl = decl.next_sibling();
        }
      }
      else
        checkParserExceptionWithLocation(ALWAYS_FAIL,
                                         elt,
                                         "Node " << node->getNodeId()
                                         << ": Illegal " << elt.name() << " element inside " << INTERFACE_TAG);
      elt = elt.next_sibling();
    }
  }

  void constructChildNodes(Node *node, xml_node const &kidsXml)
  throw (ParserException)
  {
    ListNode *lnode = dynamic_cast<ListNode *>(node);
    assertTrue_2(lnode, "Not a ListNode");
    for (xml_node kidXml = kidsXml.first_child();
         kidXml;
         kidXml = kidXml.next_sibling()) {
      Node *kid = parseNode(kidXml, node);
      // Check name is not a duplicate
      std::string const &kidId = kid->getNodeId();
      checkParserExceptionWithLocation(node->getNodeId() != kidId,
                                       kidXml,
                                       "List Node " << node->getNodeId() << " cannot have a child node with the same NodeId");
      checkParserExceptionWithLocation(!node->findChild(kidId),
                                       kidXml,
                                       "List Node " << node->getNodeId()
                                       << " cannot have multiple child nodes with the same NodeId "
                                       << kidId);
      lnode->addChild(kid);
    }
  }

  Node *parseNode(xml_node const &xml, Node *parent)
    throw (ParserException)
  {
    xml_attribute const typeAttr = xml.attribute(NODETYPE_ATTR);
    checkParserExceptionWithLocation(typeAttr,
                                     xml,
                                     "Node has no " << NODETYPE_ATTR << " attribute");
    PlexilNodeType nodeType = parseNodeType(typeAttr.value());
    checkParserExceptionWithLocation(nodeType >= NodeType_NodeList && nodeType <= NodeType_LibraryNodeCall,
                                     xml, // should be attribute
                                     "Invalid node type \"" << typeAttr.value() << "\"");

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
                                           "Only Assignment nodes may have a Priority element");
          checkNotEmpty(temp);
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
    if (body) {
      checkParserExceptionWithLocation(nodeType != NodeType_Empty,
                                       body,
                                       "Empty Node \"" << name << "\" may not have a NodeBody element");
      checkHasChildElement(body);
      body = body.first_child(); // strip away NodeBody wrapper
    }
    else
      checkParserExceptionWithLocation(nodeType == NodeType_Empty,
                                       xml,
                                       "Node \"" << name << "\" has no NodeBody element");

    Node *node = NodeFactory::createNode(name, nodeType, parent);

    try {
      // Populate local variables
      if (varDecls)
        parseVariableDeclarations(node, varDecls);

      // Check interface variables
      if (iface)
        parseInterface(node, iface);

      // Construct body including all associated variables
      switch (nodeType) {
      case NodeType_Assignment:
        if (prio) 
          parsePriority(node, prio);
        constructAssignment(node, body);
        break;

      case NodeType_Command: {
        CommandNode *cnode = dynamic_cast<CommandNode *>(node);
        assertTrue_2(cnode, "Node is not a CommandNode");
        cnode->setCommand(constructCommand(node, body));
      }
        break;

      case NodeType_LibraryNodeCall:
        constructLibraryCall(node, body);
        break;

      case NodeType_NodeList:
        constructChildNodes(node, body);
        break;

      case NodeType_Update: {
        UpdateNode *unode = dynamic_cast<UpdateNode *>(node);
        assertTrue_2(unode, "Not an UpdateNode");
        unode->setUpdate(constructUpdate(node, body));
      }
          break;

        default:
          assertTrue_2(ALWAYS_FAIL, "Internal error: bad node type");
          break;
      }
    }
    catch (std::exception const & exc) {
      delete node;
      throw;
    }
    return node;
  }

  //
  // Second pass
  //
  // The node is partially built and some XML checking has been done.
  // All nodes and their declared and internal variables have been constructed,
  // but aliases and interface variables may not have been.
  // Expressions (including LHS variable references) have NOT been constructed.
  // Finish populating the node and its children.
  // 

  static void linkInVar(Node *node, xml_node const &inXml, bool isCall)
    throw (ParserException)
  {
    std::string const name(getVarDeclName(inXml));
    // Find the variable, if it exists
    // If a library call, should be in caller's alias list.
    // If not, should have been declared by an ancestor.
    Expression *exp = node->findVariable(name);
    ValueType typ = getVarDeclType(inXml);
    if (exp) {
      checkParserExceptionWithLocation(areTypesCompatible(typ, exp->valueType()),
                                       inXml,
                                       "In interface variable " << name
                                       << ": Type " << valueTypeName(typ)
                                       << " expected, but expression of type "
                                       << valueTypeName(exp->valueType()) << " was provided");
      if (exp->isAssignable()) 
        // Construct read-only alias.
        // Ancestor owns the aliased expression, so we can't delete it.
        checkParserExceptionWithLocation(node->addLocalVariable(name.c_str(), new Alias(node, name, exp, false)),
                                         inXml,
                                         "In interface variable " << name
                                         << " shadows existing local variable of same name");
      // else nothing to do - "variable" already accessible and read-only
    }
    else {
      // No such variable/alias - use default initial value
      xml_node initXml = inXml.child(INITIALVAL_TAG);
      checkParserExceptionWithLocation(initXml,
                                       inXml,
                                       "In variable " << name << " not found and no default InitialValue provided");
      bool garbage;
      exp = createExpression(initXml, node, garbage);
      checkParserExceptionWithLocation(areTypesCompatible(typ, exp->valueType()),
                                       initXml,
                                       "In interface variable " << name
                                       << " has type " << valueTypeName(typ)
                                       << " but default InitialValue is of incompatible type "
                                       << valueTypeName(exp->valueType()));
      // If exp is writable or is not something we can delete, 
      // wrap it in an Alias
      if (exp->isAssignable() || !garbage)
        exp = new Alias(node, name, exp, garbage);
      checkParserExceptionWithLocation(node->addLocalVariable(name.c_str(), exp),
                                       inXml,
                                       "In interface variable " << name
                                       << " shadows local variable of same name");
    }
  }

  static void linkInOutVar(Node *node, xml_node const &inOutXml, bool isCall)
    throw (ParserException)
  {
    std::string const name(getVarDeclName(inOutXml));
    ValueType typ = getVarDeclType(inOutXml);
    // Find the variable, if it exists
    // If a library call, should be in caller's alias list.
    // If not, should have been declared by an ancestor.
    Expression *exp = node->findVariable(name);
    if (exp) {
      checkParserExceptionWithLocation(areTypesCompatible(typ, exp->valueType()),
                                       inOutXml,
                                       "InOut interface variable " << name
                                       << ": Type " << valueTypeName(typ)
                                       << " expected, but expression of type "
                                       << valueTypeName(exp->valueType()) << " was provided");
      checkParserExceptionWithLocation(exp->isAssignable(),
                                       inOutXml,
                                       "InOut interface variable " << name
                                       << " is read-only");
    }
    else {
      // No such variable/alias - use default initial value
      xml_node initXml = inOutXml.child(INITIALVAL_TAG);
      checkParserExceptionWithLocation(initXml,
                                       inOutXml,
                                       "InOut variable " << name << " not found and no default InitialValue provided");
      bool garbage;
      Expression *initExp = createExpression(initXml, node, garbage);
      checkParserExceptionWithLocation(areTypesCompatible(typ, initExp->valueType()),
                                       initXml,
                                       "InOut variable " << name
                                       << " has type " << valueTypeName(typ)
                                       << " but default InitialValue is of incompatible type "
                                       << valueTypeName(initExp->valueType()));
      Assignable *var = createAssignable(inOutXml, node, garbage);
      assertTrue_1(garbage); // better be something we can delete!
      checkParserExceptionWithLocation(node->addLocalVariable(name.c_str(), var),
                                       inOutXml,
                                       "InOut interface variable " << name
                                       << " shadows local variable of same name");
      var->setInitializer(initExp, garbage);
    }
  }

  static void linkAndInitializeInterfaceVars(Node *node, xml_node const &iface)
    throw (ParserException)
  {
    Node *parent = node->getParent();
    bool isCall = (parent && parent->getType() == NodeType_LibraryNodeCall);
    xml_node temp = iface.first_child();
    while (temp) {
      checkHasChildElement(temp);
      if (0 == strcmp(IN_TAG, temp.name()))
        linkInVar(node, temp, isCall);
      else if (0 == strcmp(INOUT_TAG, temp.name()))
        linkInOutVar(node, temp, isCall);
      else
        assertTrueMsg(ALWAYS_FAIL,
                      "Internal error: Found " << temp.name() << " element in Interface during second pass");
      temp = temp.next_sibling();
    }
  }

  void finalizeNode(Node *node, xml_node const &xml)
    throw (ParserException)
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

    // Construct variable initializers here
    // It is only here after all child nodes and node bodies have been constructed
    // that all variables which could be referenced are accessible.
    if (varDecls) {
      xml_node decl = varDecls.first_child();
      while (decl) {
        xml_node initXml = decl.child("InitialValue");
        if (initXml) {
          char const *varName = decl.child_value("Name");
          Expression *var = node->findLocalVariable(std::string(varName));
          assertTrueMsg(var,
                        "finalizeNode: Internal error: variable " << varName
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

    // Link aliases and construct interface default initializers
    if (iface)
      // TODO
      linkAndInitializeInterfaceVars(node, iface);

    // Instantiate user conditions
    for (std::vector<xml_node>::const_iterator it = conditions.begin();
         it != conditions.end();
         ++it) {
      xml_node elt = *it;
      checkHasChildElement(elt);
      // Check that condition name is valid, get index
      Node::ConditionIndex which = Node::getConditionIndex(std::string(elt.name()));
      checkParserExceptionWithLocation(which >= Node::skipIdx && which <= Node::repeatIdx,
                                       elt,
                                       "Node " << node->getNodeId()
                                       << ": Illegal condition name \"" << elt.name() << "\"");
      bool garbage;
      Expression *cond = createExpression(elt.first_child(), node, garbage);
      checkParserExceptionWithLocation(cond->valueType() == BOOLEAN_TYPE || cond->valueType() == UNKNOWN_TYPE,
                                       elt.first_child(),
                                       "Node " << node->getNodeId() << ": Expression for "
                                       << elt.name() << " is not Boolean");
      node->addUserCondition(which, cond, garbage);
    }

    // finalize conditions
    node->finalizeConditions();

    // finalize node bodies
    switch (node->getType()) {
    case NodeType_Assignment:
      finalizeAssignment(node, body);
      break;

    case NodeType_Command: {
      CommandNode *cnode = dynamic_cast<CommandNode *>(node);
      assertTrue_2(cnode, "Node is not a CommandNode");
      finalizeCommand(node, cnode->getCommand(), body);
    }
      break;

    case NodeType_LibraryNodeCall:
      finalizeLibraryCall(node, xml);
      // fall through to NodeList case

    case NodeType_NodeList: {
      std::vector<Node *> kids = node->getChildren();
      std::vector<Node *>::iterator kid = kids.begin();
      xml_node kidXml = body.first_child();
      while (kid != kids.end() && kidXml) {
        finalizeNode(*kid, kidXml);
        ++kid;
        kidXml = kidXml.next_sibling();
      }
    }
      break;

    case NodeType_Update: {
      UpdateNode *unode = dynamic_cast<UpdateNode *>(node);
      assertTrue_2(unode, "Not an UpdateNode");
      finalizeUpdate(unode->getUpdate(), node, body);
    }
      break;

    default:
      assertTrue_2(ALWAYS_FAIL, "Bad NodeType");
      break;
    }
  }

} // namespace PLEXIL
