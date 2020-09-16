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
#include "ExpressionFactory.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"
#include "Update.hh"

#include "pugixml.hpp"

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

using pugi::node_element;
using pugi::xml_node;

namespace PLEXIL
{

  void checkUpdateBody(char const *nodeId, pugi::xml_node const updXml)
  {
    checkTag(UPDATE_TAG, updXml);
    for (xml_node pr = updXml.first_child(); pr; pr = pr.next_sibling()) {
      checkTag(PAIR_TAG, pr);
      xml_node temp = pr.first_child();
      checkTag(NAME_TAG, temp);
      checkParserExceptionWithLocation(*(temp.child_value()),
                                       temp,
                                       "Node \"" << nodeId
                                       << "\": Update " << NAME_TAG << " element empty or malformed");
      temp = temp.next_sibling();
      checkParserExceptionWithLocation(temp,
                                       pr,
                                       "Node \"" << nodeId
                                       << "\": Update pair without a value expression");
      checkExpression(nodeId, temp);
    }

    // Check for duplicates
    for (xml_node pr = updXml.first_child(); pr; pr = pr.next_sibling()) {
      char const *prName = pr.child_value(NAME_TAG);
      xml_node temp = pr.next_sibling();
      while (temp) {
        checkParserExceptionWithLocation(strcmp(prName, temp.child_value(NAME_TAG)),
                                         temp,
                                         "Node \"" << nodeId
                                         << "\": Duplicate Update " << NAME_TAG
                                         << " \"" << prName << '"');
        temp = temp.next_sibling();
      }
    }
  }

  // Pass 2
  Update *constructUpdate(NodeConnector *node, pugi::xml_node const updXml)
  {
    Update *result = new Update(node);
    result->reservePairs(std::distance(updXml.begin(), updXml.end()));
    return result;
  }

  // Unit test entry point

  void finalizeUpdate(Update *update, NodeConnector *node, pugi::xml_node const updXml)
  {
    xml_node pr = updXml.last_child();
    while (pr) {
      xml_node temp = pr.first_child();
      std::string pairName(temp.child_value());
      temp = temp.next_sibling();
      bool wasCreated;
      Expression *exp = createExpression(temp, node, wasCreated);
      update->addPair(pairName, exp, wasCreated);
      pr = pr.previous_sibling();
    }
  }

} // namespace PLEXIL
