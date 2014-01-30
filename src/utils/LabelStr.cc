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

#include "LabelStr.hh"
#include "Value.hh"
#include <cstring> // for strcmp()

#if defined(HAVE_HASH_MAP)
namespace std
{
  template <>
  size_t hash_value(const PLEXIL::LabelStr& _Keyval)
  {
    return ((size_t)hash_value(_Keyval.getKey()));
  }
}
#endif

namespace PLEXIL
{

  // define the empty label
  DEFINE_GLOBAL_CONST(LabelStr, EMPTY_LABEL, "")

  /**
   * Zero argument constructor.
   * @note Should only be used indirectly, e.g., via std::list.
   */
  LabelStr::LabelStr()
    : m_key(itemStore().getEmptyKey())
#if defined(LABEL_STR_DEBUG)
    , m_string(getItem().c_str())
#endif
  {
  }

  /**
   * @brief Constructor
   * @param label The symbolic value as a string
   */
  LabelStr::LabelStr(const std::string& label)
    : m_key(itemStore().storeItem(label))
#if defined(LABEL_STR_DEBUG)
    , m_string(getItem().c_str())
#endif
  {
  }

  /**
   * @brief Constructor
   * @param str A null terminated string
   * @param permanent A value of true means the LabelStr value should be considered
   * a permanent constant (i.e. not reference counted).
   */
  LabelStr::LabelStr(const char* label, bool permanent)
    : m_key(itemStore().storeItem(std::string(label), permanent))
#if defined(LABEL_STR_DEBUG)
    , m_string(getItem().c_str())
#endif
  {
  }

  /**
   * @brief Constructor from Value instance
   * @param The Value.
   * @note Value's strings share the LabelStr representation.
   */
  LabelStr::LabelStr(const Value& value)
    : m_key(value.getRawValue())
  {
    if (!itemStore().newReference(m_key)) {
      assertTrue(ALWAYS_FAIL, "LabelStr constructor from Value: Invalid key");
    }
#if defined(LABEL_STR_DEBUG)
    m_string = getItem().c_str();
#endif
  }

  /**
   * @brief Copy constructor.
   * @param org The source LabelStr.
   */
  LabelStr::LabelStr(const LabelStr& org)
    : m_key(org.m_key)
#if defined(LABEL_STR_DEBUG)
    , m_string(org.m_string)
#endif
  {
    if (!itemStore().newReference(m_key)) {
      assertTrue(ALWAYS_FAIL, "LabelStr copy constructor: Invalid key");
    }
  }

  /**
   * @brief Destructor.
   */
  LabelStr::~LabelStr()
  {
    itemStore().deleteReference(m_key);
  }

  /**
   * @brief Assignment operator
   * @param org LabelStr.
   * @return Reference to this LabelStr.
   */
  LabelStr& LabelStr::operator=(const LabelStr& org)
  {
    if (m_key != org.m_key) {
      assertTrueMsg(itemStore().newReference(org.m_key),
                    "InternedItem::operator=: Invalid key");
      LabelStr_key_t oldKey = m_key;
      itemStore().deleteReference(oldKey);
      m_key = org.m_key;
#if defined(LABEL_STR_DEBUG)
      m_string = org.m_string;
#endif
    }
    return *this;
  }

  /**
   * @brief Assignment operator
   * @param string The new value as a std::string.
   * @return Reference to this LabelStr.
   */
  // TODO: optimize by only hitting the table twice, instead of 3x?
  LabelStr& LabelStr::operator=(const std::string& string)
  {
    const std::string& current = toString();
    if (current != string) {
      LabelStr_key_t oldKey = m_key;
      m_key = itemStore().storeItem(string);
      itemStore().deleteReference(oldKey);
#if defined(LABEL_STR_DEBUG)
      m_string = getItem().c_str();
#endif
    }
    return *this;
  }

  /**
   * @brief Assignment operator from char*
   * @param chars The new value as a const reference to const char*.
   * @return Reference to this LabelStr.
   */
  LabelStr& LabelStr::operator=(const char* chars)
  {
    return operator=(std::string(chars));
  }

  /**
   * @brief Assignment operator from key type
   * @param key The key from another existing LabelStr.
   * @return Reference to this LabelStr.
   */
  LabelStr& LabelStr::operator=(LabelStr_key_t newKey)
  {
    if (m_key != newKey) {
      assertTrueMsg(itemStore().newReference(newKey),
                    "InternedItem::operator=: key " << newKey << " is not valid");
      LabelStr_key_t oldKey = m_key;
      m_key = newKey;
      itemStore().deleteReference(oldKey);
#if defined(LABEL_STR_DEBUG)
      m_string = getItem().c_str();
#endif
    }
    return *this;
  }

  /**
   * @brief Assignment operator from Value
   * @param key The value
   * @return Reference to this LabelStr.
   */
  LabelStr& LabelStr::operator=(const Value& value)
  {
    return operator=(value.getRawValue());
  }

  /**
   * @brief Equality operator for std::string.
   * @param other A string for comparison.
   * @return true if equal, false otherwise.
   */
  bool LabelStr::operator==(const std::string& other) const
  {
    return toString() == other;
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

  /**
   * @brief Equality operator for Value.
   * @param value The Value. 
   * @return true if equal, false otherwise.
   */
  bool LabelStr::operator==(const Value& value) const
  {
    return getKey() == value.getRawValue();
  }

  /**
   * @brief Inequality operator for Value.
   * @param value The Value. 
   * @return false if equal, true otherwise.
   */
  bool LabelStr::operator!=(const Value& value) const
  {
    return !operator==(value);
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
   * @brief Return the represented string.
   * @return Const reference to the string.
   */
  const std::string& LabelStr::toString() const
  {
    LabelStr_value_t* result = itemStore().getItem(m_key);
    assertTrue(result != NULL,
               "LabelStr::toString: key not found");
    return *result;
  }

  /**
   * @brief Get the string represented by this key.
   * @param key The key.
   * @return Const reference to the string.
   * @note Throws an exception if not found.
   */
  const std::string& LabelStr::toString(LabelStr_key_t key)
  {
    LabelStr_value_t* result = itemStore().getItem(key);
    assertTrue(result != NULL,
               "LabelStr::toString: key not found");
    return *result;
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

  /**
   * @brief Return the item store.
   * @note Only external user should be Value class.
   */
  LabelStr_store_t& LabelStr::itemStore()
  {
    static LabelStr_store_t sl_itemStore;
    return sl_itemStore;
  }

}
