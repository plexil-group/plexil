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

#include "Debug.hh"
#include "NodeImpl.hh"
#include "parseGlobalDeclarations.hh"
#include "parseNode.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"
#include "SymbolTable.hh"

#include "pugixml.hpp"

using pugi::xml_document;
using pugi::xml_node;
using pugi::xml_parse_result;

namespace PLEXIL
{
  // Initialize globals
  unsigned int const PUGI_PARSE_OPTIONS = pugi::parse_default | pugi::parse_ws_pcdata_single;

  // Load a file and extract the top-level XML element from it.
  xml_document *loadXmlFile(std::string const &filename)
  {
    debugMsg("loadXmlFile", ' ' << filename);
    xml_document *doc = new xml_document;
    xml_parse_result parseResult = doc->load_file(filename.c_str(), PUGI_PARSE_OPTIONS);
    if (parseResult.status == pugi::status_file_not_found) {
      delete doc;
      return NULL;
    }
    if (parseResult.status != pugi::status_ok) {
      delete doc;
      doc = NULL;
      checkParserException(false,
                           "Error reading XML file " << filename
                           << ": " << parseResult.description());
    }
    return doc;
  }

  // First pass: surface check of XML
  SymbolTable *checkPlan(xml_node const xml)
  {
    debugMsg("checkPlan", " entered");
    checkTag(PLEXIL_PLAN_TAG, xml);
    checkHasChildElement(xml);

    xml_node elt = xml.first_child();
    SymbolTable *result = NULL;
    if (testTag(GLOBAL_DECLARATIONS_TAG, elt)) {
      checkGlobalDeclarations(elt);
      result = parseGlobalDeclarations(elt);

      elt = elt.next_sibling();
    }
    else {
      // Create an empty symbol table
      result = makeSymbolTable();
    }

    // Check the node using the context of the global declarations
    pushSymbolTable(result);
    try {
      checkNode(elt);
    }
    catch (...) {
      delete result;
      popSymbolTable();
      throw;
    }
    popSymbolTable();

    return result;
  }

  NodeImpl *constructPlan(xml_node const xml, SymbolTable *symtab, NodeImpl *parent)
  {
    xml_node const root = xml.child(NODE_TAG);
    debugMsg("constructPlan", ' ' << root.child_value(NODEID_TAG));
    pushSymbolTable(symtab);
    NodeImpl *result = NULL;
    try {
      // Construct the plan
      try {
        result = constructNode(root, parent);
      }
      catch (...) {
        delete result;
        result = NULL;
        throw;
      }
    }
    catch (...) {
      popSymbolTable();
      throw;
    }

    popSymbolTable();

    // Normal return
    return result;
  }

  NodeImpl *parsePlan(xml_node const xml)
  {
    debugMsg("parsePlan", "entered");
    // Perform surface checks & log global symbols
    SymbolTable *symtab = checkPlan(xml);
    NodeImpl *result = NULL;
    result = constructPlan(xml, symtab, NULL); // can throw ParserException
    pushSymbolTable(symtab);
    try {
      finalizeNode(result, xml.child(NODE_TAG));
    }
    catch (...) {
      popSymbolTable();
      delete symtab;
      throw;
    }

    popSymbolTable();
    delete symtab;

    // Normal return
    return result;
  }

} // namespace PLEXIL
