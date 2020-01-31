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

#ifndef PLEXIL_PARSER_UTILS_HH
#define PLEXIL_PARSER_UTILS_HH

#include "Error.hh" // PLEXIL_NORETURN macro
#include "ParserException.hh"
#include "ValueType.hh"

//
// General purpose xml parsing utilities
//

namespace pugi
{
  class xml_node;
}

namespace PLEXIL
{
  bool testPrefix(char const *prefix, char const *str);
  bool testSuffix(char const *suffix, char const *str);
  bool testTag(const char* t, pugi::xml_node const e);
  bool testTagPrefix(const char* prefix, pugi::xml_node const e);
  bool testTagSuffix(const char* suffix, pugi::xml_node const e);
  bool hasChildElement(pugi::xml_node const e);
  void checkTag(const char* t, pugi::xml_node const e);
  void checkAttr(const char* t, pugi::xml_node const e);
  void checkTagSuffix(const char* t, pugi::xml_node const e);
  void checkNotEmpty(pugi::xml_node const e);
  void checkHasChildElement(pugi::xml_node const e);
  bool isBoolean(const char* initval);
  bool isInteger(const char* initval);
  bool isDouble(const char* initval);

  char const *typeNameAsValue(ValueType ty);

  // Helper for checkParserExceptionWithLocation
  PLEXIL_NORETURN void throwParserException(std::string const &msg, pugi::xml_node location);

} // namespace PLEXIL

/**
 * @def reportParserExceptionWithLocation
 * @brief Throw a ParserException unconditionally
 * @param loc A pugi::xml_node with the location of the exception
 * @param msg An expression which writes the required message to a stream
 */
#define reportParserExceptionWithLocation(loc, msg) { \
  std::ostringstream whatstr; \
  whatstr << msg; \
  throwParserException(whatstr.str().c_str(), loc); \
}

/**
 * @def checkParserExceptionWithLocation
 * @brief If the condition is false, throw a ParserException
 * @param cond The condition to test; if false, throw the exception
 * @param loc A pugi::xml_node with the location of the exception
 * @param msg An expression which writes the required message to a stream
 */
#define checkParserExceptionWithLocation(cond, loc, msg) { \
  if (!(cond)) { \
    reportParserExceptionWithLocation(loc, msg); \
  } \
}

#endif // PLEXIL_PARSER_UTILS_HH
