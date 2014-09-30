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

#include "Node.hh"
#include "parseNode.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"

#include "pugixml.hpp"

using pugi::xml_document;
using pugi::xml_node;

#include <cstring>

namespace PLEXIL
{

  // Place to store library nodes
  static std::map<std::string, xml_node> libraryMap;

  void addLibraryNode(std::string const &name, xml_node const xml)
  {
    libraryMap[name] = xml;
  }

  xml_node getLibraryNode(std::string const &name)
  {
    std::map<std::string, xml_node>::iterator it = libraryMap.find(name);
    if (it == libraryMap.end())
      return xml_node();
    else
      return it->second;
  }

  static void parseGlobalDeclarations(xml_node declXml)
  {
    // TODO
  }

  Node *parsePlan(xml_node const xml)
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
    finalizeNode(result, xml);
    return result;
  }

} // namespace PLEXIL
