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

#include "ParserException.hh"
#include "pugixml.hpp"

#include <cctype>
#include <cstring>

namespace PLEXIL
{
  //
  // Internal error checking/reporting utilities
  //

  bool testTag(const char* t, const pugi::xml_node& e) {
    return e.type() == pugi::node_element && 0 == strcmp(t, e.name());
  }

  bool testTagPrefix(const char* prefix, const pugi::xml_node& e)
  {
    if (e.type() != pugi::node_element)
      return false;
    const char* valueStr = e.name();
    const size_t prefixLen = strlen(prefix);
    // I don't believe this is really an optimization.
    // if (strlen(valueStr) < prefixLen)
    //   return false;
    return 0 == strncmp(prefix, valueStr, prefixLen);
  }

  bool testTagSuffix(const char* suffix, const pugi::xml_node& e)
  {
    if (e.type() != pugi::node_element)
      return false;
    const char* valueStr = e.name();
    const size_t valueLen = strlen(valueStr);
    const size_t suffixLen = strlen(suffix);
    if (valueLen < suffixLen)
      return false;
    return 0 == strncmp(suffix, &(valueStr[valueLen - suffixLen]), suffixLen);
  }

  bool hasChildElement(const pugi::xml_node& e) 
  {
    return e && e.first_child() && e.first_child().type() == pugi::node_element;
  }

  void checkTag(const char* t, const pugi::xml_node& e) {
    checkParserExceptionWithLocation(testTag(t, e),
                                     e,
                                     "XML parsing error: Expected <" << t << "> element, but got <" << e.name() << "> instead.");
  }

  void checkAttr(const char* t, const pugi::xml_node& e) {
    checkParserExceptionWithLocation(e && e.type() == pugi::node_element && e.attribute(t),
                                     e,
                                     "XML parsing error: Expected an attribute named '" << t << "' in element <" << e.name() << ">");
  }

  void checkTagSuffix(const char* t, const pugi::xml_node& e) 
  {
    checkParserExceptionWithLocation(testTagSuffix(t, e),
                                     e,
                                     "XML parsing error: Expected an element ending in '" << t << "', but instead got <" << e.name() << ">");
  }

  // N.B. presumes e is not empty
  void checkNotEmpty(const pugi::xml_node& e) {
    checkParserExceptionWithLocation(e.first_child()
                                     && e.first_child().type() == pugi::node_pcdata
                                     && *(e.first_child().value()),
                                     e,
                                     "XML parsing error: Expected a non-empty text child of <" << e.name() << ">");
  }

  // N.B. presumes e is not empty
  void checkHasChildElement(const pugi::xml_node& e) {
    checkParserExceptionWithLocation(hasChildElement(e),
                                     e,
                                     "XML parsing error: Expected a child element of <" << e.name() << ">");
  }

  bool isBoolean(const char* initval)
  {
    if (initval == NULL)
      return false;

    switch (*initval) {
    case '0':
    case '1':
      if (*++initval)
        return false;
      else
        return true;

    case 'f':
      return (0 == strcmp(++initval, "alse"));

    case 't':
      return (0 == strcmp(++initval, "rue"));

    default:
      return false;
    }
  }

  bool isInteger(const char* initval)
  {
    if (initval == NULL || !*initval)
      return false;

    // Check against XML 'integer'
    // [\-+]?[0-9]+
    if ('+' == *initval || '-' == *initval) {
      if (!*++initval)
        return false; // sign w/ nothing after it
    }
    if (isdigit(*initval)) {
      while (*++initval && isdigit(*initval)) {
      }
    }
    if (*initval)
      return false; // junk after number

    // TODO: add range check
    return true;
  }

  bool isDouble(const char* initval)
  {
    if (initval == NULL || !*initval)
      return false;
      
    // Check against XML 'double'
    // (\+|-)?([0-9]+(\.[0-9]*)?|\.[0-9]+)([Ee](\+|-)?[0-9]+)?|(\+|-)?INF|NaN

    if (strcmp(initval, "NaN") == 0) 
      return true;

    if ('+' == *initval || '-' == *initval) {
      if (!*++initval)
        return false; // sign w/ nothing after it
    }

    if (*initval && strcmp(initval, "INF") == 0)
      return true;

    // ([0-9]+(\.[0-9]*)?|\.[0-9]+)([Ee](\+|-)?[0-9]+)?
    bool digitsSeen = false;
    if (*initval && isdigit(*initval)) {
      digitsSeen = true;
      while (*++initval && isdigit(*initval)) {
      }
    }
    if (*initval && '.' == *initval) {
      if (*++initval && isdigit(*initval)) {
        digitsSeen = true;        
        while (*++initval && isdigit(*initval)) {
        }
      }
    }
    if (!digitsSeen)
      return false;

    // Optional exponent
    if (*initval && ('E' == *initval || 'e' == *initval)) {
      if (!*++initval)
        return false; 
      if ('+' == *initval || '-' == *initval) {
        if (!*++initval)
          return false; 
      }
      if (isdigit(*initval)) {
        while (*++initval && isdigit(*initval)) {
        }
      }
    }

    if (*initval)
      return false; // junk after number

    // FIXME: add range check?
    return true;
  }

} // namespace PLEXIL
