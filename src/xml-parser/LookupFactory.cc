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

#include "LookupFactory.hh"

#include "Error.hh"
#include "Lookup.hh"
#include "ParserException.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"

namespace PLEXIL
{

  LookupFactory::LookupFactory(std::string const &name)
    : ExpressionFactory(name)
  {
  }

  LookupFactory::~LookupFactory()
  {
  }

  Expression *LookupFactory::allocate(pugi::xml_node const expr,
                                      NodeConnector *node,
                                      bool & wasCreated) const
  {
    checkHasChildElement(expr);
    pugi::xml_node stateNameXml = expr.first_child();
    checkTag(NAME_TAG, stateNameXml);
    checkHasChildElement(stateNameXml);
    pugi::xml_node argsXml = stateNameXml.next_sibling();
    pugi::xml_node tolXml;
    if (argsXml) {
      // Tolerance tag is supposed to come between name and args, sigh
      if (0 == strcmp(TOLERANCE_TAG, argsXml.name())) {
        tolXml = argsXml;
        if (0 == strcmp(LOOKUPNOW_TAG, expr.name())) {
          checkParserExceptionWithLocation(tolXml,
                                           tolXml,
                                           "LookupNow may not have a Tolerance element");
        }
        checkHasChildElement(tolXml);
        argsXml = tolXml.next_sibling();
      }
    }
    if (argsXml) {
      checkTag(ARGS_TAG, argsXml);
    }

    bool stateNameGarbage = false;
    Expression *stateName = createExpression(stateNameXml.first_child(), node, stateNameGarbage);
    ValueType stateNameType = stateName->valueType();
    checkParserException(stateNameType == STRING_TYPE || stateNameType == UNKNOWN_TYPE,
                         "createExpression: Lookup name must be a string expression");

    std::vector<Expression* > params;
    std::vector<bool> paramsGarbage;
    pugi::xml_node arg = argsXml.first_child();
    while (arg) {
      bool garbage = false;
      params.push_back(createExpression(arg, node, garbage));
      paramsGarbage.push_back(garbage);
      arg = arg.next_sibling();
    }
    wasCreated = true;
    if (tolXml) {
      bool tolGarbage = false;
      Expression *tol = createExpression(tolXml.first_child(), node, tolGarbage);
      ValueType tolType = tol->valueType();
      checkParserException(isNumericType(tolType) || tolType == UNKNOWN_TYPE,
                           "createExpression: LookupOnChange tolerance expression must be numeric");
      return new LookupOnChange(stateName, stateNameGarbage,
                                params, paramsGarbage,
                                tol, tolGarbage);
    }
    else
      return new Lookup(stateName, stateNameGarbage,
                        params, paramsGarbage);
  }

} // namespace PLEXIL