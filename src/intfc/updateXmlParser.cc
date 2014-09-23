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
#include "NodeConnector.hh"
#include "parser-utils.hh"
#include "Update.hh"

#include "pugixml.hpp"

using pugi::xml_attribute;
using pugi::xml_node;

namespace PLEXIL
{
  Update *updateXmlParser(pugi::xml_node const &upd,
                          NodeConnector *node)
    throw (ParserException)
  {
    checkTag(UPDATE_TAG, upd);
    Update::PairExpressionMap pairs;
    std::vector<Expression *> garbage;
    xml_node pr = upd.first_child();
    while (pr) {
      checkTag(PAIR_TAG, pr);
      checkTag(NAME_TAG, pr.first_child());
      checkNotEmpty(pr.first_child());
      std::string pairName(pr.first_child().child_value());
      checkParserExceptionWithLocation(pairs.find(pairName) == pairs.end(),
                                       pr,
                                       "Duplicate pairs with name \"" << pairName << "\"")
      bool wasCreated;
      Expression *exp = createExpression(pr.first_child().next_sibling(), node, wasCreated);
      pairs.insert(std::pair<std::string, Expression *>(pairName, exp));
      if (wasCreated)
        garbage.push_back(exp);
      pr = pr.next_sibling();
    }

    return new Update(node, pairs, garbage);
  }

} // namespace PLEXIL
