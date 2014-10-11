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

#include "parser-utils.hh" // for checkParserExceptionWithLocation macro

#include "pugixml.hpp"

#include <cctype>
#include <cstring>

using pugi::node_element;
using pugi::node_pcdata;
using pugi::xml_node;

namespace PLEXIL
{
  //
  // Internal parsing utilities
  //

  bool testPrefix(char const *prefix, char const *str)
  {
    return 0 == strncmp(prefix, str, strlen(prefix));
  }

  bool testSuffix(char const* suffix, char const *str)
  {
    size_t const valueLen = strlen(str);
    size_t const suffixLen = strlen(suffix);
    int offset = valueLen - suffixLen;
    if (offset < 0)
      return false;
    return 0 == strncmp(suffix, &(str[offset]), suffixLen);
  }

  bool testTag(const char* t, xml_node const e) {
    return e.type() == node_element && 0 == strcmp(t, e.name());
  }

  bool testTagPrefix(const char* prefix, xml_node const e)
  {
    if (e.type() != node_element)
      return false;
    return testPrefix(prefix, e.name());
  }

  bool testTagSuffix(const char* suffix, xml_node const e)
  {
    if (e.type() != node_element)
      return false;
    return testSuffix(suffix, e.name());
  }

  bool hasChildElement(xml_node const e) 
  {
    if (!e)
      return false;
    xml_node temp = e.first_child();
    return temp && temp.type() == node_element;
  }

  void checkTag(const char* t, xml_node const e)
    throw (ParserException)
  {
    checkParserExceptionWithLocation(testTag(t, e),
                                     e,
                                     "XML parsing error: Expected <" << t << "> element, but got <" << e.name() << "> instead.");
  }

  void checkAttr(const char* t, xml_node const e)
    throw (ParserException)
  {
    checkParserExceptionWithLocation(e && e.type() == node_element && e.attribute(t),
                                     e,
                                     "XML parsing error: Expected an attribute named '" << t << "' in element <" << e.name() << ">");
  }

  void checkTagSuffix(const char* t, xml_node const e)
    throw (ParserException)
  {
    checkParserExceptionWithLocation(testTagSuffix(t, e),
                                     e,
                                     "XML parsing error: Expected an element ending in '" << t << "', but instead got <" << e.name() << ">");
  }

  // N.B. presumes e is not empty
  void checkNotEmpty(xml_node const e)
    throw (ParserException)
  {
    xml_node temp = e.first_child();
    checkParserExceptionWithLocation(temp
                                     && temp.type() == node_pcdata
                                     && *(temp.value()),
                                     e,
                                     "XML parsing error: Expected a non-empty text child of <" << e.name() << ">");
  }

  // N.B. presumes e is not empty
  void checkHasChildElement(xml_node const e)
    throw (ParserException)
  {
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
