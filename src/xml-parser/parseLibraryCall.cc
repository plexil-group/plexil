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

#include "createExpression.hh"
#include "Debug.hh"
#include "Error.hh"
#include "LibraryCallNode.hh"
#include "parseNode.hh"
#include "parsePlan.hh"
#include "parser-utils.hh"
#include "planLibrary.hh"
#include "PlexilSchema.hh"
#include "SymbolTable.hh"

#include "pugixml.hpp"

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

using pugi::xml_node;
using pugi::node_element;
using pugi::node_pcdata;

namespace PLEXIL
{

  //
  // First pass
  //

  static void checkAlias(std::string const &callerId, xml_node const aliasXml)
  {
    checkTag(ALIAS_TAG, aliasXml);
    xml_node nameXml = aliasXml.first_child();
    checkTag(NODE_PARAMETER_TAG, nameXml);
    char const *name = nameXml.child_value();
    checkParserExceptionWithLocation(*name,
                                     nameXml,
                                     "NodeParameter element is empty in LibraryNodeCall node "
                                     << callerId);

    // Check for duplicate parameter names here
    for (xml_node sib = aliasXml.previous_sibling(); sib; sib = sib.previous_sibling()) {
      checkParserExceptionWithLocation(strcmp(name, sib.first_child().child_value()),
                                       aliasXml,
                                       "Multiple aliases for \""
                                       << name
                                       << "\" in LibraryNodeCall node "
                                       << callerId);
    }
    
    // Basic checks to see that we have something that could be an expression
    xml_node temp = nameXml.next_sibling();
    checkParserExceptionWithLocation(temp,
                                     aliasXml,
                                     "Alias for \"" << name
                                     << "\" without value expression in LibraryNodeCall node "
                                     << callerId);
    // Don't need to have a child if is of string type, otherwise do
    checkParserExceptionWithLocation(temp.type() == node_element &&
                                     (temp.first_child() || !strcmp(temp.name(),"StringValue")),
                                      
                                     temp,
                                     "Alias for \"" << name
                                     << "\" has malformed value expression in LibraryNodeCall node " 
                                     << callerId);
  }

  void checkLibraryCall(char const *callerId, xml_node const callXml)
  {
    checkTag(LIBRARYNODECALL_TAG, callXml);
    xml_node temp = callXml.first_child();
    checkTag(NODEID_TAG, temp);
    char const *name = temp.child_value();
    checkParserExceptionWithLocation(*name,
                                     temp,
                                     "Empty NodeId for called library in LibraryNodeCall node "
                                     << callerId);
    // Check aliases
    while ((temp = temp.next_sibling()))
      checkAlias(callerId, temp);
  }

  // Simply count the # of aliases in the call
  size_t estimateAliasSpace(pugi::xml_node const callXml)
  {
    xml_node alias = callXml.first_child().next_sibling();
    if (!alias)
      return 0;
    // checkTag(ALIAS_TAG, alias); // done in checkAlias()
    size_t result = 1;
    while ((alias = alias.next_sibling())) // tag checked in checkAlias()
      result++;
    return result;
  }

  static void allocateAliases(LibraryCallNode *node, xml_node const callXml)
  {
    // Preallocate, but don't populate, aliases
    node->allocateAliasMap(estimateAliasSpace(callXml));
  }

  void constructLibraryCall(LibraryCallNode *node, xml_node const callXml)
  {
    assertTrue_1(node);
    debugMsg("constructLibraryCall", " caller " << node->getNodeId());

    allocateAliases(node, callXml);

    Library const *l = getLibraryNode(callXml.first_child().child_value());
    checkParserExceptionWithLocation(l,
                                     callXml,
                                     "Library node "
                                     << callXml.first_child().child_value()
                                     << " not found while expanding LibraryNodeCall node "
                                     << node->getNodeId());
    // Construct call
    // Template was checked before it was added to library
    node->addChild(constructPlan(l->doc->document_element(), l->symtab, node));
  }

  // Second pass
  static void finalizeAliases(LibraryCallNode *node, xml_node const callXml)
  {
    debugMsg("finalizeAliases", " caller " << node->getNodeId());
    // Skip over NodeId
    xml_node aliasXml = callXml.first_child();
    while ((aliasXml = aliasXml.next_sibling())) {
      xml_node const nameXml = aliasXml.first_child();
      debugMsg("finalizeAliases", " constructing alias " << nameXml.child_value());
             
      // Add the alias
      bool isGarbage = false;
      Expression *exp = createExpression(nameXml.next_sibling(), node, isGarbage);
      node->addAlias(nameXml.child_value(), exp, isGarbage);
    }
  }

  // Second pass
  void finalizeLibraryCall(LibraryCallNode *node, xml_node const callXml)
  {
    assertTrue_1(node);
    debugMsg("finalizeLibraryCall", " caller " << node->getNodeId());

    finalizeAliases(node, callXml);

    Library const *l = getLibraryNode(callXml.first_child().child_value());
    assertTrue_2(l,
                 "finalizeLibraryCall: Internal error: can't find library");
    xml_node const calleeXml = l->doc->document_element().child(NODE_TAG);

    // should never happen, but...
    assertTrue_2(!node->getChildren().empty(),
                 "finalizeLibraryCall: Internal error: LibraryNodeCall node missing called node");

    pushSymbolTable(l->symtab);
    try {
      finalizeNode(node->getChildren().front(), calleeXml);
    }
    catch (...) {
      popSymbolTable();
      throw;
    }
    popSymbolTable();
  }

} // namespace PLEXIL
