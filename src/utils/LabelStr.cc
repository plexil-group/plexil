/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

#include "LabelStr.hh"
#include <cstring> // for strcmp()

namespace PLEXIL
{

  // define the empty label
  DEFINE_GLOBAL_CONST(LabelStr, EMPTY_LABEL, "");

  /**
   * Zero argument constructor.
   * @note Should only be used indirectly, e.g., via std::list.
   */
  LabelStr::LabelStr()
    : LabelStr_item_t()
#ifndef PLEXIL_FAST
    , m_string(getItem().c_str())
#endif
  {
  }

  /**
   * @brief Constructor
   * @param label The symbolic value as a string
   */
  LabelStr::LabelStr(const std::string& label)
    : LabelStr_item_t(label)
#ifndef PLEXIL_FAST
    , m_string(getItem().c_str())
#endif
  {
  }

  /**
   * @brief Constructor
   * @param str A null terminated string
   */
  LabelStr::LabelStr(const char* label)
    : LabelStr_item_t(std::string(label))
#ifndef PLEXIL_FAST
    , m_string(getItem().c_str())
#endif
  {
  }

  /**
   * @brief Constructor from encoded key
   * @param key the key value for a previously created LabelStr instance.
   */
  LabelStr::LabelStr(double key)
    : LabelStr_item_t(key)
#ifndef PLEXIL_FAST
    , m_string(getItem().c_str())
#endif
  {
  }

  /**
   * @brief Copy constructor.
   * @param org The source LabelStr.
   */
  LabelStr::LabelStr(const LabelStr& org)
    : LabelStr_item_t(org)
#ifndef PLEXIL_FAST
    , m_string(org.m_string)
#endif
  {
  }

  /**
   * @brief Assignment operator
   * @param org LabelStr.
   * @return Reference to this LabelStr.
   */
  LabelStr& LabelStr::operator=(const LabelStr& org)
  {
    LabelStr_item_t::operator=(org);
#ifndef PLEXIL_FAST
    m_string = org.m_string;
#endif
    return *this;
  }

  /**
   * @brief Assignment operator
   * @param string The new value as a std::string.
   * @return Reference to this LabelStr.
   */
  LabelStr& LabelStr::operator=(const std::string& string)
  {
    LabelStr_item_t::operator=(string);
#ifndef PLEXIL_FAST
    m_string = getItem().c_str();
#endif
    return *this;
  }

  /**
   * @brief Assignment operator from char*
   * @param chars The new value as a const reference to const char*.
   * @return Reference to this LabelStr.
   */
  LabelStr& LabelStr::operator=(const char* chars)
  {
    return this->operator=(std::string(chars));
  }

  /**
   * @brief Assignment operator from key type
   * @param key The key from another existing LabelStr.
   * @return Reference to this LabelStr.
   */
  LabelStr& LabelStr::operator=(LabelStr_key_t key)
  {
    LabelStr_item_t::operator=(key);
#ifndef PLEXIL_FAST
    m_string = getItem().c_str();
#endif
    return *this;
  }

  /**
   * @brief Equality operator for std::string.
   * @param other A string for comparison.
   * @return true if equal, false otherwise.
   */
  bool LabelStr::operator==(const std::string& other) const
  {
    return getItem() == other;
  }

  /**
   * @brief Equality operator for const char*.
   * @param other A char* constant for comparison.
   * @return true if equal, false otherwise.
   */
  bool LabelStr::operator==(const char* other) const
  {
    return 0 == strcmp(c_str(), other);
  }

  bool LabelStr::operator<(const LabelStr& lbl) const
  {
    return toString() < lbl.toString();
  }

  bool LabelStr::operator>(const LabelStr& lbl) const
  {
    return toString() > lbl.toString();
  }

  /**
   * @brief Tests if a given string is contained within this string.
   * @param lblStr The string to test for.
   * @return true if present, otherwise false.
   */
  bool LabelStr::contains(const LabelStr& lblStr) const
  {
    return std::string::npos != toString().find(lblStr.toString());
  }

  /**
   * @brief Tests if a given string is contained within this string.
   * @param str The string to test for.
   * @return true if present, otherwise false.
   */
  bool LabelStr::contains(const std::string& str) const
  {
    return std::string::npos != toString().find(str);
  }

  /**
   * @brief Tests if a given string is contained within this string.
   * @param str The const char* to test for.
   * @return true if present, otherwise false.
   */
  bool LabelStr::contains(const char* str) const
  {
    return std::string::npos != toString().find(str);
  }

  size_t LabelStr::countElements(const char* delimiters) const
  {
    assertTrueMsg(delimiters != NULL && delimiters[0] != '\0',
                  "'NULL' and empty string are not valid delimiters");

    const std::string& str = toString();
    size_t result = 0;

    // Skip delimiters at beginning. Note the "not_of".
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos) {
      // Found a token
      ++result;
      // Skip next delimiter.
      lastPos = str.find_first_not_of(delimiters, pos);
      // Find next non-delimiter.
      pos = str.find_first_of(delimiters, lastPos);
    }
    return result;
  }

}
