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

#include "parseNode.hh"

#include "Alias.hh"
#include "Array.hh"
#include "ArrayLiteralFactory.hh"
#include "Assignable.hh"
#include "AssignmentNode.hh"
#include "Command.hh"
#include "CommandNode.hh"
#include "commandXmlParser.hh"
#include "createExpression.hh"
#include "Debug.hh"
#include "Error.hh"
#include "LibraryCallNode.hh"
#include "ListNode.hh"
#include "NodeFactory.hh"
#include "parseAssignment.hh"
#include "parseLibraryCall.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"
#include "UpdateNode.hh"
#include "updateXmlParser.hh"

#include "pugixml.hpp"

#if defined(HAVE_CSTRING)
#include <cstring> // strcmp()
#elif defined(HAVE_STRING_H)
#include <string.h> // strcmp()
#endif

using pugi::xml_attribute;
using pugi::xml_node;
using pugi::node_element;

namespace PLEXIL
{

  //
  // First pass: check the XML for obvious oopsies
  //

  PlexilNodeType checkNodeTypeAttr(xml_node const xml)
  {
    xml_attribute const typeAttr = xml.attribute(NODETYPE_ATTR);
    checkParserExceptionWithLocation(typeAttr,
                                     xml,
                                     "Node has no " << NODETYPE_ATTR << " attribute");

    PlexilNodeType nodeType = parseNodeType(typeAttr.value());
    checkParserExceptionWithLocation(nodeType < NodeType_error,
                                     xml, // should be attribute
                                     "Invalid " << NODETYPE_ATTR << " \"" << typeAttr.value() << "\"");
    return nodeType;
  }

  // Used for VariableDeclarations and In, InOut interface declarations
  static void checkVariableDeclaration(char const *nodeId, xml_node const decl)
  {
    checkParserExceptionWithLocation(testTag(DECL_VAR_TAG, decl)
                                     || testTag(DECL_ARRAY_TAG, decl),
                                     decl,
                                     "Node \"" << nodeId
                                     << "\": \"" << decl.name()
                                     << "\" is not a valid variable declaration element");
  
    // Common checking for DeclareVariable and DeclareArray
    xml_node temp = decl.first_child();
    checkParserExceptionWithLocation(temp,
                                     decl,
                                     "Node \"" << nodeId
                                     << "\": Empty " << decl.name() << " element in " << decl.name());
    checkTag(NAME_TAG, temp);
    char const *name = temp.child_value();
    checkParserExceptionWithLocation(*name,
                                     temp,
                                     "Node \"" << nodeId
                                     << "\": Empty " << temp.name() << " element in " << decl.name());
    temp = temp.next_sibling();
    checkParserExceptionWithLocation(temp,
                                     decl,
                                     "Node \"" << nodeId
                                     << "\": " << decl.name() << " missing " << TYPE_TAG
                                     << " element in "
                                     << decl.name() << ' ' << decl.child_value(NAME_TAG));
    checkTag(TYPE_TAG, temp);
    checkParserExceptionWithLocation(*temp.child_value(),
                                     temp,
                                     "Node \"" << nodeId
                                     << "\": Empty " << temp.name() << " element in "
                                     << decl.name() << ' ' << decl.child_value(NAME_TAG));
    ValueType typ = parseValueType(temp.child_value());
    checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                     temp,
                                     "Node \"" << nodeId
                                     << "\": Unknown type name " << temp.child_value()
                                     << " in " << decl.name() << ' ' << decl.child_value(NAME_TAG));
    checkParserExceptionWithLocation(isScalarType(typ),
                                     temp,
                                     "Node \"" << nodeId
                                     << "\": Invalid type name " << temp.child_value()
                                     << " in " << decl.name() << ' ' << decl.child_value(NAME_TAG));

    // Dispatch to specific checks
    // See UserVariableFactory.cc and ArrayVariableFactory.cc
    checkExpression(nodeId, decl); // for effect; can throw ParserException
  }

  // Non-error-checking variant of above
  static char const *getVarDeclName(xml_node const decl)
  {
    // Name is always the first element
    return decl.first_child().child_value();
  }

  static void checkVariableDeclarations(char const *nodeId, xml_node const decls)
  {
    for (xml_node decl = decls.first_child(); decl; decl = decl.next_sibling()) {
      checkVariableDeclaration(nodeId, decl);

      // Check for duplicate names
      char const *name = getVarDeclName(decl);
      for (xml_node sib = decl.previous_sibling(); sib; sib = sib.previous_sibling()) {
        checkParserExceptionWithLocation(strcmp(name, getVarDeclName(sib)),
                                         decl,
                                         "Node \"" << nodeId
                                         << "\": Multiple variables named \""
                                         << name << '"');
      }
    }
  }

  // Early (superficial) interface checking
  static void checkInterface(char const *nodeId, xml_node const iface)
  {
    for (xml_node elt = iface.first_child(); elt; elt = elt.next_sibling()) {
      char const *name = elt.name();
      if (!strcmp(IN_TAG, name) || !strcmp(INOUT_TAG, name)) {
        for (xml_node decl = elt.first_child(); decl; decl = decl.next_sibling())
          checkVariableDeclaration(nodeId, decl);
        // Check for duplicate names
        // TODO
      }
      else
        reportParserExceptionWithLocation(elt,
                                          "Node " << nodeId
                                          << ": Illegal " << name << " element inside " << INTERFACE_TAG);
    }
  }

  static void checkChildNodes(char const *parentId, xml_node const kidsXml)
  {
    std::vector<char const *> nodeIds;
    xml_node kidXml = kidsXml.first_child();
    while (kidXml) { // empty list node is legal
      // Basic checks on child
      checkNode(kidXml);

      // Check that parent and child don't have same name
      char const *kidId = kidXml.child_value(NODEID_TAG);
      checkParserExceptionWithLocation(parentId != kidId,
                                       kidXml,
                                       "List Node " << parentId
                                       << " has a child node with the same NodeId");

      // Check that none of siblings has same name
      for (std::vector<char const *>::const_iterator iter = nodeIds.begin();
           iter != nodeIds.end(); 
           ++iter) {
        checkParserExceptionWithLocation(strcmp(kidId, *iter),
                                         kidXml,
                                         "List Node " << parentId
                                         << " has multiple child nodes with the same NodeId "
                                         << kidId);
      }

      nodeIds.push_back(kidId);
      kidXml = kidXml.next_sibling();
    }
  }

  static void checkNodeBody(char const *nodeId, xml_node const bodyXml, PlexilNodeType nodeType)
  {
    xml_node const child = bodyXml.first_child();
    checkParserExceptionWithLocation(child,
                                     bodyXml,
                                     "Node \"" << nodeId
                                     << "\" has an empty " << BODY_TAG << " element");

    switch (nodeType) {
    case NodeType_Assignment:
      checkAssignmentBody(nodeId, child);
      return;

    case NodeType_Command:
      checkCommandBody(nodeId, child);
      return;

    case NodeType_Empty:
      return;

    case NodeType_LibraryNodeCall:
      checkLibraryCall(nodeId, child);
      return;

    case NodeType_NodeList:
      checkChildNodes(nodeId, child);
      return;

    case NodeType_Update:
      checkUpdateBody(nodeId, child);
      return;

    default:
      // Internal error
      errorMsg("checkNodeBody: Internal error: invalid PlexilNodeType value");
      return;
    }
  }

  static void checkCondition(char const *nodeId, xml_node const xml)
  {
    xml_node const expr = xml.first_child();
    checkParserExceptionWithLocation(expr && expr.type() == node_element,
                                     xml,
                                     "Node \"" << nodeId
                                     << "\": " << xml.name()
                                     << " element is malformed");
    
    ValueType t = checkExpression(nodeId, expr);
    checkParserExceptionWithLocation(areTypesCompatible(BOOLEAN_TYPE, t),
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Type error for " << xml.name()
                                     << "; expected a Boolean expression but a "
                                     << valueTypeName(t)
                                     << " expression was supplied");
  }

  void checkNode(xml_node const xml)
  {
    checkTag(NODE_TAG, xml);

    PlexilNodeType nodeType = checkNodeTypeAttr(xml);

    // For error reporting purposes
    char const *nodeId = xml.child_value(NODEID_TAG);

    // Temps for duplicate checking
    xml_node idXml;
    xml_node ifaceXml;
    xml_node bodyXml;
    xml_node prioXml;
    xml_node varDeclsXml;
    xml_node endXml;
    xml_node exitXml;
    xml_node invariantXml;
    xml_node postXml;
    xml_node preXml;
    xml_node repeatXml;
    xml_node skipXml;
    xml_node startXml;

    // Scan all children in order
    for (xml_node temp = xml.first_child(); temp; temp = temp.next_sibling()) {
      char const *tag = temp.name();
      checkParserExceptionWithLocation(*tag,
                                       xml,
                                       "Non-element found at top level of node");
      bool validElt = false;
      debugMsg("checkNode", " parsing element " << tag);
      switch (*tag) {

      case 'A': // Assume - annotation for analysis, ignored
        if (!strcmp(ASSUME_TAG, tag))
          validElt = true;
        break;
        
      case 'C': // Comment
        if (!strcmp(COMMENT_TAG, tag))
          validElt = true;
        break;

      case 'D': // Desire - annotation for analysis, ignored
        if (!strcmp(DESIRE_TAG, tag))
          validElt = true;
        break;
        
      case 'E': // EndCondition, ExitCondition, Expect
        switch (strlen(tag)) {
        case 6: // Expect - annotation for analysis, ignored
          if (!strcmp(EXPECT_TAG, tag))
            validElt = true;
          break;

        case 12: // EndCondition
          if (!strcmp(END_CONDITION_TAG, tag)) {
            validElt = true;
            checkParserExceptionWithLocation(endXml.empty(),
                                             temp,
                                             "Duplicate " << tag << " element in Node");
            checkCondition(nodeId, temp);
            endXml = temp;
          }
          break;

        case 13: // ExitCondition
          if (!strcmp(EXIT_CONDITION_TAG, tag)) {
            validElt = true;
            checkParserExceptionWithLocation(exitXml.empty(),
                                             temp,
                                             "Duplicate " << tag << " element in Node");
            checkCondition(nodeId, temp);
            exitXml = temp;
          }
          break;

        default:
          break;
        }
        break;

      case 'I': // Interface, InvariantCondition
        if (!strcmp(INVARIANT_CONDITION_TAG, tag)) {
          validElt = true;
          checkParserExceptionWithLocation(invariantXml.empty(),
                                           temp,
                                           "Duplicate " << tag << " element in Node");
          checkCondition(nodeId, temp);
          invariantXml = temp;
          break;
        }
        if (!strcmp(INTERFACE_TAG, tag)) {
          validElt = true;
          checkParserExceptionWithLocation(!ifaceXml,
                                           temp, 
                                           "Duplicate " << tag << " element in Node");
          ifaceXml = temp;
        }
        break;

      case 'N': // NodeId, NodeBody
        if (!strcmp(NODEID_TAG, tag)) {
          validElt = true;
          checkParserExceptionWithLocation(!idXml,
                                           temp, 
                                           "Duplicate " << tag << " element in Node");
          checkParserExceptionWithLocation(temp.first_child().type() == pugi::node_pcdata
                                           && *temp.child_value(),
                                           temp,
                                           "Empty or invalid " << tag << " element in Node");
          idXml = temp;
          break;
        }
        if (!strcmp(BODY_TAG, tag)) {
          validElt = true;
          checkParserExceptionWithLocation(!bodyXml,
                                           temp, 
                                           "Duplicate " << tag << " element in Node");
          bodyXml = temp;
        }
        break;

      case 'P': // PostCondition, Priority, PreCondition
        switch (strlen(tag)) {
        case 8: // Priority
          if (!strcmp(PRIORITY_TAG, tag)) {
            validElt = true;
            checkParserExceptionWithLocation(nodeType == NodeType_Assignment,
                                             temp,
                                             "Only Assignment nodes may have a Priority element");
            checkParserExceptionWithLocation(!prioXml,
                                             temp, 
                                             "Duplicate " << tag << " element in Node");
            // TODO: check for non-negative integer
            prioXml = temp;
          }
          break;
          
        case 12: // PreCondition
          if (!strcmp(PRE_CONDITION_TAG, tag)) {
            validElt = true;
            checkParserExceptionWithLocation(preXml.empty(),
                                             temp,
                                             "Duplicate " << tag << " element in Node");
            checkCondition(nodeId, temp);
            preXml = temp;
          }
          break;

        case 13: // PostCondition
          if (!strcmp(POST_CONDITION_TAG, tag)) {
            validElt = true;
            checkParserExceptionWithLocation(postXml.empty(),
                                             temp,
                                             "Duplicate " << tag << " element in Node");
            checkCondition(nodeId, temp);
            postXml = temp;
          }
          break;

        default:
          break;
        }
        break;

      case 'R': // RepeatCondition
        if (!strcmp(REPEAT_CONDITION_TAG, tag)) {
          validElt = true;
          checkParserExceptionWithLocation(repeatXml.empty(),
                                           temp,
                                           "Duplicate " << tag << " element in Node");
          checkCondition(nodeId, temp);
          repeatXml = temp;
        }
        break;

      case 'S': // SkipCondition, StartCondition
        if (!strcmp(START_CONDITION_TAG, tag)) {
          validElt = true;
          checkParserExceptionWithLocation(startXml.empty(),
                                           temp,
                                           "Duplicate " << tag << " element in Node");
          checkCondition(nodeId, temp);
          startXml = temp;
        }
        else if (!strcmp(SKIP_CONDITION_TAG, tag)) {
          validElt = true;
          checkParserExceptionWithLocation(skipXml.empty(),
                                           temp,
                                           "Duplicate " << tag << " element in Node");
          checkCondition(nodeId, temp);
          skipXml = temp;
        }
        break;

      case 'V': // VariableDeclarations
        if (!strcmp(VAR_DECLS_TAG, tag)) {
          validElt = true;
          checkParserExceptionWithLocation(!varDeclsXml,
                                           temp, 
                                           "Duplicate " << tag << " element in Node");
          varDeclsXml = temp;
        }
        break;

      default:
        break;
      }
      // Report all illegal-element errors here
      checkParserExceptionWithLocation(validElt,
                                       temp,
                                       "Illegal element \"" << tag << "\" in Node");
    }

    // Empty NodeId check
    checkParserExceptionWithLocation(idXml,
                                     xml,
                                     "Node missing " << NODEID_TAG << " element");
    checkParserExceptionWithLocation(*nodeId,
                                     idXml,
                                     "Empty " << NODEID_TAG << " element in Node");

    // Check variable declarations (if supplied)
    if (varDeclsXml)
      checkVariableDeclarations(nodeId, varDeclsXml);

    // Check interface (if supplied)
    if (ifaceXml)
      checkInterface(nodeId, ifaceXml);

    // Check priority (if supplied)
    if (prioXml)
      checkPriority(nodeId, prioXml);

    // Check body
    if (nodeType == NodeType_Empty) {
      checkParserExceptionWithLocation(!bodyXml,
                                       bodyXml,
                                       "Empty Node \"" << nodeId
                                       << "\" may not have a " << BODY_TAG << " element");
    }
    else {
      checkParserExceptionWithLocation(bodyXml,
                                       xml,
                                       "Node \"" << nodeId
                                       << "\" missing " << BODY_TAG << " element");

      checkNodeBody(nodeId, bodyXml, nodeType);
    }
  }

  //
  // Second pass: begin constructing the node
  //

  // For Interface specs; may have other uses.
  static ValueType getVarDeclType(xml_node const decl)
  {
    ValueType typ = parseValueType(decl.child_value(TYPE_TAG));
    if (testTag(DECL_ARRAY_TAG, decl))
      return arrayType(typ);
    else 
      return typ;
  }

  //
  // N.B. There is a limited amount of checking we can do on interface variables in the second pass.
  // LibraryNodeCall aliases can't be expanded because some of the variables they can reference
  // (e.g. child node internal vars) may not exist yet. Same with default values.

  // Estimate symbol table space for variables
  static size_t estimateVariableSpace(xml_node const decls)
  {
    return std::distance(decls.begin(), decls.end());
  }

  // Estimate symbol table space for interface variables
  static size_t estimateInterfaceSpace(xml_node const iface)
  {
    size_t n = 0;
    for (xml_node elt = iface.first_child(); elt; elt = elt.next_sibling())
      n += std::distance(elt.begin(), elt.end());
    return n;
  }

  // Second pass checking of one In interface variable
  static void parseInDecl(NodeImpl *node, xml_node const inXml, bool isCall)
  {
    char const *name = getVarDeclName(inXml);
    checkParserExceptionWithLocation(!node->findLocalVariable(name),
                                     inXml,
                                     "In interface variable " << name
                                     << " shadows another variable of same name in this node");
  }

  // Second pass checking of one InOut interface
  static void parseInOutDecl(NodeImpl *node, xml_node const inOutXml, bool isCall)
  {
    char const *name = getVarDeclName(inOutXml);
    checkParserExceptionWithLocation(!node->findLocalVariable(name),
                                     inOutXml,
                                     "InOut interface variable " << name
                                     << " shadows another variable of same name in this node");
  }

  // Second pass
  static void parseInterface(NodeImpl *node, xml_node const iface)
  {
    // Figure out if this is a library node expansion
    NodeImpl *parent = node->getParentNode();
    bool isCall = (parent && parent->getType() == NodeType_LibraryNodeCall);
    for (xml_node elt = iface.first_child(); elt; elt = elt.next_sibling()) {
      if (testTag(IN_TAG, elt)) {
        for (xml_node decl = elt.first_child(); decl; decl = decl.next_sibling())
          parseInDecl(node, decl, isCall);
      }
      else /* if (testTag(INOUT_TAG, elt)) */ {
        for (xml_node decl = elt.first_child(); decl; decl = decl.next_sibling())
          parseInOutDecl(node, decl, isCall);
      }
    }
  }

  static void parseVariableDeclarations(NodeImpl *node, xml_node const decls)
  {
    for (xml_node decl = decls.first_child(); decl; decl = decl.next_sibling()) {
      // Variables are always created here, no need for "garbage" flag.
      node->addLocalVariable(getVarDeclName(decl),
                             createExpression(decl, node));
    }
  }

  static void initializeNodeVariables(NodeImpl *node, xml_node const xml)
  {
    xml_node const varDecls = xml.child(VAR_DECLS_TAG);
    xml_node const iface = xml.child(INTERFACE_TAG);

    // By now we have an upper bound on how many entries are required.
    // Reserve space for them. 
    // This saves us from reallocating and copying the whole table as it grows.
    if (varDecls || iface) {
      size_t nVariables = 0;
      if (node->getType() == NodeType_LibraryNodeCall)
        nVariables += estimateAliasSpace(xml.child(BODY_TAG).first_child());
      if (varDecls)
        nVariables += estimateVariableSpace(varDecls);
      if (iface)
        nVariables += estimateInterfaceSpace(iface);
      node->allocateVariables(nVariables);
    }

    // Check interface variables
    if (iface) {
      debugMsg("parseNode", " parsing interface declarations");
      parseInterface(node, iface);
    }

    // Populate local variables
    if (varDecls) {
      debugMsg("parseNode", " parsing variable declarations");
      parseVariableDeclarations(node, varDecls);
    }
  }

  static void constructChildNodes(ListNode *node, xml_node const kidsXml)
  {
    assertTrue_1(node);

    xml_node kidXml = kidsXml.first_child();
    if (!kidXml)
      return; // empty list

    size_t n = 1;
    while ((kidXml = kidXml.next_sibling()))
      ++n;

    node->reserveChildren(n);

    // Construct the children.
    kidXml = kidsXml.first_child();
    do {
      node->addChild(constructNode(kidXml, node));
    } while ((kidXml = kidXml.next_sibling()));
  }

  NodeImpl *constructNode(xml_node const xml, NodeImpl *parent)
  {
    xml_attribute attr = xml.attribute(NODETYPE_ATTR);
    PlexilNodeType nodeType = parseNodeType(attr.value());
    checkParserExceptionWithLocation(nodeType < NodeType_error,
                                     xml, // should really be the attribute
                                     "Invalid " << attr.name()
                                     << " value \"" << attr.value() << "\"");

    debugMsg("parseNode", " constructing node");
    NodeImpl *node =
      NodeFactory::createNode(xml.child(NODEID_TAG).child_value(),
                              nodeType,
                              parent);
    debugMsg("parseNode", " Node " << node->getNodeId()  << " created");

    try {
      // Populate interface and local variables.
      initializeNodeVariables(node, xml);

      // Construct body
      debugMsg("parseNode", " constructing body");
      switch (nodeType) {
      case NodeType_Assignment:
        constructAssignment(dynamic_cast<AssignmentNode *>(node), xml);
        break;

      case NodeType_Command:
        dynamic_cast<CommandNode *>(node)->setCommand(new Command(node->getNodeId()));
        break;

      case NodeType_LibraryNodeCall:
        constructLibraryCall(dynamic_cast<LibraryCallNode *>(node),
                             xml.child(BODY_TAG).first_child());
        break;

      case NodeType_NodeList:
        constructChildNodes(dynamic_cast<ListNode *>(node),
                            xml.child(BODY_TAG).first_child());
        break;

      case NodeType_Update:
        dynamic_cast<UpdateNode *>(node)->setUpdate(constructUpdate(node, 
                                                                    xml.child(BODY_TAG).first_child()));
        break;

      case NodeType_Empty:
        // no-op
        break;

      default:
        errorMsg("Internal error: bad node type");
        break;
      }
    }
    catch (std::exception const & exc) {
      debugMsg("parseNode", " recovering from parse error, deleting node "
               << node->getNodeId());
      delete node;
      throw;
    }
    debugMsg("parseNode", " first pass done.");
    return node;
  }

  //
  // Third pass: finalize the node
  //
  // The node is partially built and most XML checking has been done.
  // All nodes and their declared and internal variables have been constructed,
  // but aliases and interface variables may not have been.
  // Expressions (including LHS variable references) have NOT been constructed.
  // Finish populating the node and its children.
  // 

  static void parseVariableInitializer(NodeImpl *node, xml_node const decl)
  {
    xml_node initXml = decl.child(INITIALVAL_TAG);
    if (initXml) {
      char const *varName = decl.child_value(NAME_TAG);
      Expression *var = node->findLocalVariable(varName);
      assertTrueMsg(var,
                    "finalizeNode: Internal error: variable " << varName
                    << " not found in node " << node->getNodeId());
      // FIXME: is this needed/possible?
      checkParserExceptionWithLocation(var->isAssignable(),
                                       initXml,
                                       "This variable may not take an initializer");
      checkHasChildElement(initXml);
      Expression *init;
      ValueType varType = var->valueType();
      bool garbage;
      if (isArrayType(varType)
          && testTag(typeNameAsValue(arrayElementType(varType)), initXml.first_child())) {
        // Handle old style initializer
        garbage = true; // always constructed
        switch (varType) {
        case BOOLEAN_ARRAY_TYPE:
          init = createArrayLiteral<bool>("Boolean", initXml);
          break;

        case INTEGER_ARRAY_TYPE:
          init = createArrayLiteral<int32_t>("Integer", initXml);
          break;

        case REAL_ARRAY_TYPE:
          init = createArrayLiteral<double>("Real", initXml);
          break;

        case STRING_ARRAY_TYPE:
          init = createArrayLiteral<std::string>("String", initXml);
          break;

        default:
          reportParserExceptionWithLocation(initXml,
                                            "Can't parse initial value for unimplemented or illegal type "
                                            << valueTypeName(varType));
          return;
        }
      }
      else {
        // Simply parse whatever's inside the <InitialValue>
        initXml = initXml.first_child();
        init = createExpression(initXml, node, garbage);
      }
      ValueType initType = init->valueType();
      if (!areTypesCompatible(varType, initType)) {
        if (garbage)
          delete init;
        reportParserExceptionWithLocation(initXml,
                                          "Node " << node->getNodeId()
                                          << ": Initialization type mismatch for variable "
                                          << varName << ", variable is " << valueTypeName(varType)
                                          << ", initializer is " << valueTypeName(initType));
        return; // make cppcheck happy
      }
      if (isArrayType(varType)) {
        // Check whether initial value is larger than declared size
        int sizeSpec = decl.child(MAX_SIZE_TAG).text().as_int(-1);
        if (sizeSpec >= 0) {
          Array const *initArray = NULL;
          assertTrueMsg(init->getValuePointer(initArray),
                        "Internal error: array initial value is unknown");
          if (initArray->size() > (size_t) sizeSpec) {
            if (garbage)
              delete init;
            reportParserExceptionWithLocation(decl,
                                              "Node " << node->getNodeId()
                                              << ": initial value for array variable "
                                              << varName << " exceeds declared array size");
            return; // make cppcheck happy
          }
        }
      }
      var->asAssignable()->setInitializer(init, garbage);
    }
  }

  // Process variable declarations, if any
  static void constructVariableInitializers(NodeImpl *node, xml_node const xml)
  {
    xml_node temp = xml.child(VAR_DECLS_TAG);
    if (!temp)
      return;

    debugMsg("finalizeNode",
             " constructing variable initializers for " << node->getNodeId());
    for (xml_node decl = temp.first_child();
         decl;
         decl = decl.next_sibling()) 
      parseVariableInitializer(node, decl);
  }

  static void linkInVar(NodeImpl *node, xml_node const inXml, bool isCall)
  {
    char const *name = getVarDeclName(inXml);

    debugMsg("linkInVar",
             " node " << node->getNodeId() << ", In variable " << name);
    // Find the variable, if it exists
    // If a library call, should be in caller's alias list.
    // If not, should have been declared by an ancestor.
    Expression *exp = node->findVariable(name);
    ValueType typ = getVarDeclType(inXml);
    if (exp) {
      debugMsg("linkInVar", " found ancestor variable");
      checkParserExceptionWithLocation(areTypesCompatible(typ, exp->valueType()),
                                       inXml,
                                       "In interface variable " << name
                                       << ": Type " << valueTypeName(typ)
                                       << " expected, but expression of type "
                                       << valueTypeName(exp->valueType()) << " was provided");
      if (exp->isAssignable()) {
        // Construct read-only alias.
        debugMsg("linkInVar",
                 " constructing read-only alias for ancestor variable " << name);

        // Ancestor owns the aliased expression, so we can't delete it.
        Expression *alias = new Alias(name, exp, false);
        if (!node->addLocalVariable(name, alias)) {
          delete alias;
          reportParserExceptionWithLocation(inXml,
                                            "In interface variable " << name
                                            << " shadows existing local variable of same name");
        }
        
        // else nothing to do - "variable" already accessible and read-only
      }
    }
    else {
      debugMsg("linkInVar", " no ancestor variable found");

      // No such variable/alias - use default initial value
      xml_node initXml = inXml.child(INITIALVAL_TAG);
      checkParserExceptionWithLocation(initXml,
                                       inXml,
                                       "In variable " << name << " not found and no default InitialValue provided");

      debugMsg("linkInVar", " constructing default value");
      bool garbage;
      exp = createExpression(initXml.first_child(), node, garbage);
      if (!areTypesCompatible(typ, exp->valueType())) {
        ValueType expType = exp->valueType();
        if (garbage)
          delete exp;
        reportParserExceptionWithLocation(initXml,
                                          "In interface variable " << name
                                          << " has type " << valueTypeName(typ)
                                          << " but default InitialValue is of incompatible type "
                                          << valueTypeName(expType));
        return; // make cppcheck happy
      }
      // If exp is writable or is not something we can delete, 
      // wrap it in an Alias
      if (exp->isAssignable() || !garbage) {
        debugMsg("linkInVar", " constructing read-only alias for default value");
        exp = new Alias(name, exp, garbage);
      }
      if (!node->addLocalVariable(name, exp)) {
        delete exp;
        reportParserExceptionWithLocation(inXml,
                                          "In interface variable " << name
                                          << " shadows local variable of same name");
        return; // make cppcheck happy
      }
    }
  }

  static void linkInOutVar(NodeImpl *node, xml_node const inOutXml, bool isCall)
  {
    char const *name = getVarDeclName(inOutXml);
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
      bool garbage, initGarbage;
      Expression *initExp =
        createExpression(initXml.first_child(), node, initGarbage);
      ValueType initExpType = initExp->valueType(); 
      if (!areTypesCompatible(typ, initExpType)) {
        if (initGarbage)
          delete initExp;
        reportParserExceptionWithLocation(initXml,
                                          "InOut variable " << name
                                          << " has type " << valueTypeName(typ)
                                          << " but default InitialValue is of incompatible type "
                                          << valueTypeName(initExpType));
      }
      Expression *var = createAssignable(inOutXml, node, garbage);
      assertTrue_1(garbage); // better be something we can delete!
      if (!node->addLocalVariable(name, var)) {
        delete var;
        reportParserExceptionWithLocation(inOutXml,
                                          "InOut interface variable " << name
                                          << " shadows local variable of same name");
      }
      var->asAssignable()->setInitializer(initExp, initGarbage);
    }
  }

  static void linkAndInitializeInterfaceVars(NodeImpl *node, xml_node const nodeXml)
  {
    xml_node const iface = nodeXml.child(INTERFACE_TAG);
    if (!iface)
      return;
    
    debugMsg("linkAndInitializeInterface", " node " << node->getNodeId());
    NodeImpl *parent = node->getParentNode();
    bool isCall = (parent && parent->getType() == NodeType_LibraryNodeCall);
    for (xml_node temp = iface.first_child(); temp; temp = temp.next_sibling()) {
      if (testTag(IN_TAG, temp)) {
        for (xml_node decl = temp.first_child(); decl; decl = decl.next_sibling())
          linkInVar(node, decl, isCall);
      }
      else if (testTag(INOUT_TAG, temp)) {
        for (xml_node decl = temp.first_child(); decl; decl = decl.next_sibling())
          linkInOutVar(node, decl, isCall);
      }
    }
  }

  static void createConditions(NodeImpl *node, xml_node const xml)
  {
    for (xml_node elt = xml.first_child(); elt; elt = elt.next_sibling()) {
      char const *tag = elt.name();
      if (testSuffix(CONDITION_SUFFIX, tag)) {
        debugMsg("finalizeNode", " processing condition " << tag);
        bool garbage;
        Expression *cond = createExpression(elt.first_child(), node, garbage);
        ValueType condType = cond->valueType();
        if (condType != BOOLEAN_TYPE && condType != UNKNOWN_TYPE) {
          if (garbage)
            delete cond;
          reportParserExceptionWithLocation(elt.first_child(),
                                            "Node " << node->getNodeId() << ": "
                                            << tag << " expression is not Boolean");
        }
        node->addUserCondition(tag, cond, garbage);
      }
    }

    node->finalizeConditions();
  }

  static void finalizeListNode(ListNode *node, xml_node const listXml)
  {
    assertTrue_1(node);
    std::vector<NodeImpl *> &kids = node->getChildren();
    std::vector<NodeImpl *>::iterator kid = kids.begin();
    xml_node kidXml = listXml.first_child();
    while (kid != kids.end() && kidXml) {
      finalizeNode(*kid, kidXml);
      ++kid;
      kidXml = kidXml.next_sibling();
    }
  }

  void finalizeNode(NodeImpl *node, xml_node const xml)
  {
    debugMsg("finalizeNode", " node " << node->getNodeId());
    linkAndInitializeInterfaceVars(node, xml);
    constructVariableInitializers(node, xml);
    createConditions(node, xml);

    // Process body
    switch (node->getType()) {
    case NodeType_Assignment:
      finalizeAssignment(dynamic_cast<AssignmentNode *>(node),
                         xml.child(BODY_TAG).first_child());
      break;
      
    case NodeType_Command:
      finalizeCommand(dynamic_cast<CommandNode *>(node)->getCommand(),
                      node,
                      xml.child(BODY_TAG).first_child());
      break;

    case NodeType_LibraryNodeCall:
      finalizeLibraryCall(dynamic_cast<LibraryCallNode *>(node),
                          xml.child(BODY_TAG).first_child());
      break;

    case NodeType_NodeList:
      finalizeListNode(dynamic_cast<ListNode *>(node),
                       xml.child(BODY_TAG).first_child());
      break;

    case NodeType_Update:
      finalizeUpdate(dynamic_cast<UpdateNode *>(node)->getUpdate(),
                     node,
                     xml.child(BODY_TAG).first_child());
      break;

      // No-op for empty.
      // Invalid type should have been caught in first pass.
    default:
      break;
    }
  }

} // namespace PLEXIL
