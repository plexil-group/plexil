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

#include "LookupFactory.hh"

#include "createExpression.hh"
#include "ExprVec.hh"
#include "Lookup.hh"
#include "ParserException.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"
#include "SymbolTable.hh"

#include "pugixml.hpp"

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

using pugi::xml_node;

namespace PLEXIL
{

  LookupFactory::LookupFactory(std::string const &name)
    : ExpressionFactory(name)
  {
  }

  LookupFactory::~LookupFactory()
  {
  }

  ValueType LookupFactory::check(char const *nodeId, xml_node const expr) const
  {
    xml_node stateNameXml = expr.first_child();
    checkParserExceptionWithLocation(testTag(NAME_TAG, stateNameXml),
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": " << expr.name()
                                     << " without a " << NAME_TAG << " element");
    checkParserExceptionWithLocation(stateNameXml.first_child().type() == pugi::node_element,
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Malformed " << NAME_TAG
                                     << " element in " << expr.name());

    // Name can be any legal String expression
    xml_node nameXml = stateNameXml.first_child();
    ValueType nameType = checkExpression(nodeId, nameXml);
    checkParserExceptionWithLocation(nameType == STRING_TYPE || nameType == UNKNOWN_TYPE,
                                     stateNameXml,
                                     "Node \"" << nodeId
                                     << "\": " << NAME_TAG
                                     << " is not a String expression in " << expr.name());

    // If name is a literal, look up result type
    ValueType resultType = UNKNOWN_TYPE;
    Symbol const *lkup = NULL;
    if (testTag(STRING_VAL_TAG, nameXml)) {
        lkup = getLookupSymbol(nameXml.child_value());
        if (lkup)
          resultType = lkup->returnType();
    }
    
    xml_node temp = stateNameXml.next_sibling();
    if (!temp)
      return resultType; // everything past here is optional

    if (testTag(TOLERANCE_TAG, temp)) {
      checkParserExceptionWithLocation(testTag(LOOKUPCHANGE_TAG, expr),
                                       temp,
                                       "Node \"" << nodeId
                                       << "\": " << temp.name()
                                       << " may not appear in a " << expr.name());

      // Can be any valid numeric expression
      ValueType tolType = checkExpression(nodeId, temp.first_child());
      checkParserExceptionWithLocation(isNumericType(tolType) || tolType == UNKNOWN_TYPE,
                                       temp,
                                       "Node \"" << nodeId
                                       << "\": " << temp.name()
                                       << " is not a numeric expression");
      temp = temp.next_sibling();
      if (!temp)
        return resultType;
    }

    // Check arguments
    checkParserExceptionWithLocation(testTag(ARGS_TAG, temp),
                                     temp,
                                     "Node \"" << nodeId
                                     << "\": " << temp.name()
                                     << " may not appear in a " << expr.name());
    
    xml_node args = temp;
    temp = args.first_child();
    if (lkup) {
      // Check count and types of args if name is known and declared
      size_t i;
      size_t reqd = lkup->parameterCount();
      for (i = 0; i < reqd && temp; ++i) {
        // Check required parameters
        ValueType reqType = lkup->parameterType(i);
        ValueType hasType = checkExpression(nodeId, temp);
        checkParserExceptionWithLocation(areTypesCompatible(reqType, hasType),
                                         temp,
                                         "Node \"" << nodeId
                                         << "\": Argument type error for lookup "
                                         << nameXml.child_value()
                                         << "; argument " << i + 1
                                         << " expects " << valueTypeName(reqType)
                                         << " but expression has type " << valueTypeName(hasType));
        temp = temp.next_sibling();
      }

      // Did we get enough?
      checkParserExceptionWithLocation(i == reqd,
                                       args,
                                       "Node \"" << nodeId
                                       << "\": Not enough arguments for lookup "
                                       << nameXml.child_value()
                                       << "; expected "
                                       << (lkup->anyParameters() ? "at least" : "")
                                       << reqd
                                       << " arguments, but " << i << " were supplied");

      // Any unexpected leftovers?
      checkParserExceptionWithLocation(lkup->anyParameters() || !temp,
                                       args,
                                       "Node \"" << nodeId
                                       << "\": Too many arguments for lookup "
                                       << nameXml.child_value()
                                       << "; expected " << reqd
                                       << " arguments, but more were supplied");
    }
    // Fall through to here if known lookup allows any parameters
    while (temp) {
      checkExpression(nodeId, temp); // for effect
      temp = temp.next_sibling();
    }

    // TODO
    return resultType;
  }

  Expression *LookupFactory::allocate(xml_node const expr,
                                      NodeConnector *node,
                                      bool & wasCreated,
                                      ValueType /* returnType */) const
  {
    // Syntactic checking has been done already
    xml_node stateNameXml = expr.first_child();
    bool stateNameGarbage = false;
    Expression *stateName = createExpression(stateNameXml.first_child(), node, stateNameGarbage);
    ValueType stateNameType = stateName->valueType();
    checkParserExceptionWithLocation(stateNameType == STRING_TYPE || stateNameType == UNKNOWN_TYPE,
                                     stateNameXml.first_child(),
                                     "createExpression: Lookup name must be a string expression");

    // Type checking support
    Symbol const *lkup = NULL;
    ValueType returnType = UNKNOWN_TYPE;
    if (stateName->isConstant()) {
      // Check whether it's known
      std::string const *nameStr;
      if (stateName->getValuePointer(nameStr)) {
        // Check whether it's declared
        lkup = getLookupSymbol(nameStr->c_str());
        if (lkup)
          returnType = lkup->returnType();
      }
    }

    // TODO Warn if undeclared (future)

    // Parse tolerance, arguments if supplied
    xml_node argsXml = stateNameXml.next_sibling();
    xml_node tolXml;
    if (testTag(TOLERANCE_TAG, argsXml)) {
      tolXml = argsXml;
      argsXml = argsXml.next_sibling();
    }
    
    // Count args, then build ExprVec of appropriate size
    ExprVec *argVec = NULL;
    try {
      size_t nargs = std::distance(argsXml.begin(), argsXml.end());
      if (lkup) {
        // Check argument count against command declaration
        checkParserExceptionWithLocation(nargs == lkup->parameterCount()
                                         || (lkup->anyParameters() && nargs > lkup->parameterCount()),
                                         expr,
                                         "Lookup " << lkup->name() << " expects "
                                         << (lkup->anyParameters() ? "at least " : "")
                                         << lkup->parameterCount() << " arguments, but was supplied "
                                         << nargs);
      }
      if (nargs) {
        argVec = makeExprVec(nargs);
        size_t i = 0;
        for (xml_node arg = argsXml.first_child();
             arg;
             arg = arg.next_sibling(), ++i) {
          bool garbage = false;
          Expression *expr = createExpression(arg, node, garbage);
          argVec->setArgument(i, expr, garbage);

          // Check parameter type against declaration
          if (lkup && i < lkup->parameterCount()) {
            ValueType actual = expr->valueType();
            ValueType expected = lkup->parameterType(i);
            checkParserExceptionWithLocation(areTypesCompatible(expected, actual),
                                             arg,
                                             "Parameter " << i << " to lookup " << lkup->name()
                                             << " should be of type " << valueTypeName(expected)
                                             << ", but has type " << valueTypeName(actual));
          }
        }
      }
      wasCreated = true;
      if (tolXml) {
        bool tolGarbage = false;
        Expression *tol = createExpression(tolXml.first_child(), node, tolGarbage);
        try {
          ValueType tolType = tol->valueType();
          checkParserException(isNumericType(tolType) || tolType == UNKNOWN_TYPE,
                               "createExpression: LookupOnChange tolerance expression must be numeric");
          return new LookupOnChange(stateName, stateNameGarbage, returnType,
                                    tol, tolGarbage,
                                    argVec);
        }
        catch (ParserException &e) {
          if (tolGarbage)
            delete tol;
          throw;
        }
      }
      else
        return new Lookup(stateName, stateNameGarbage, returnType, argVec);
    }
    catch (ParserException &e) {
      delete argVec;
      throw;
    }
  }

} // namespace PLEXIL
