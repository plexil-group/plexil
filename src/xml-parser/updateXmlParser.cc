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

#include "Error.hh"
#include "ExpressionFactory.hh"
#include "interface-schema.hh"
#include "parser-utils.hh"
#include "Update.hh"
#include "UpdateNode.hh"

#include "pugixml.hpp"

using pugi::node_element;
using pugi::xml_node;

namespace PLEXIL
{

  void constructUpdate(Node *node, pugi::xml_node const &upd)
    throw (ParserException)
  {
    checkTag(UPDATE_TAG, upd);
    xml_node pr = upd.first_child();
    while (pr) {
      checkTag(PAIR_TAG, pr);
      xml_node temp = pr.first_child();
      checkTag(NAME_TAG, temp);
      checkNotEmpty(temp);
      temp = temp.next_sibling();
      checkParserExceptionWithLocation(temp && temp.type() == node_element,
                                       pr,
                                       "Update pair without a value expression");
      pr = pr.next_sibling();
    }

    UpdateNode unode = dynamic_cast<UpdateNode *>(node);
    assertTrue_2(unode, "Not an UpdateNode");
    unode->setUpdate(new Update(node));
  }

  void finalizeUpdate(Node *node, pugi::xml_node const &upd)
    throw (ParserException)
  {
    UpdateNode unode = dynamic_cast<UpdateNode *>(node);
    assertTrue_2(unode, "Not an UpdateNode");
    Update *update = unode->getUpdate();

    xml_node pr = upd.first_child();
    while (pr) {
      checkTag(PAIR_TAG, pr);
      xml_node temp = pr.first_child();
      std::string pairName(temp.child_value());
      temp = temp.next_sibling();
      bool wasCreated;
      Expression *exp = createExpression(temp, node, wasCreated);
      checkParserExceptionWithLocation(update->addPair(pairName, exp, wasCreated),
                                       pr,
                                       "Duplicate Update pair name " << pairName);
      pr = pr.next_sibling();
    }
  }

} // namespace PLEXIL
