/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

#ifndef _H__LabelStr
#define _H__LabelStr

#include "CommonDefs.hh"
#include "Error.hh"
#include <string>
#include "StoredItem.hh"
#include "Debug.hh"

/**
 * @file LabelStr.hh
 * @brief Declares the LabelStr class
 * @author Robert Harris based on code by Conor McGann
 * @date November, 2007
 * @ingroup Utils
 */

namespace PLEXIL
{
  class LabelStr;
  DECLARE_GLOBAL_CONST(LabelStr, EMPTY_LABEL);
  DECLARE_GLOBAL_CONST(LabelStr, UNKNOWN_STR);

#ifdef PLATFORM_HAS_GNU_HASH_MAP
  // specialized hash function for pointers to string

  class StringHashFunction
  {
  public:
    size_t operator()(const std::string* str) const
    {
      return __gnu_cxx::hash<const char*>()(str->c_str());
    }
  };

  // specialized equal operator for pointers to strings

  struct StringEqualOoperator : public std::binary_function<
    const std::string*,
    const std::string*,
    bool>
  {
    bool operator()(const std::string* s1, const std::string* s2) const
    { 
      return *s1 == *s2;
    }
  };
   
  // 

  typedef StoredItem<
    double, 
    const std::string, 
    StringHashFunction, 
    StringEqualOoperator> StoredString;
#endif // PLATFORM_HAS_GNU_HASH_MAP

#ifdef PLATFORM_HAS_DINKUM_HASH_MAP
  // specialized hash_compare class for pointers to string
  class StringPointerCompare
    : public std::hash_compare<std::string const *>
  {
  public:
    // Hashing operator
    size_t operator()(std::string const * const &item) const
    {
      // Code taken from GNU's ext/hash_fun.h
      const char* s = item->c_str();
      unsigned long h = 0;
      for ( ; *s; ++s)
	h = 5 * h + *s;
      return size_t(h);
    }

    // less-than comparison
    bool operator()(std::string const * const &s1, std::string const * const &s2) const
    { 
      return *s1 < *s2;
    }
  };

  typedef StoredItem<
    double, 
    std::string const, 
    StoredItemKeyCompare<double>,
    StringPointerCompare > StoredString;
#endif // PLATFORM_HAS_DINKUM_HASH_MAP

/**
 * @class LabelStr
 * @brief Provides for a symbolic value to be handled in a domain.
 *
 * The reader should note that strings are stored in a static data
 * structure so that they can be shared. Access to the store is
 * provided by a key value. This reduces operations on LabelStr to
 * operations on double valued keys which is considerable more
 * efficient. This encoding is largely transparent to users.
 */

class LabelStr: protected StoredString
{
         
public:
  /**
   * Zero argument constructor.
   * @note Should only be used indirectly, e.g., via std::list.
   */
  LabelStr();

  /**
   * @brief Constructor
   * @param str The null terminated string in question
   */
  LabelStr(const char* str);

  /**
   * @brief Constructor
   * @param label The symbolic value as a string
   */
  LabelStr(const std::string& label);

  /**
   * @brief Constructor from encoded key
   *
   * Each LabelStr gets encoded as a key such that any 2
   * instances of a LabelStr constructed from the same string
   * will have the same key and that the key preserves
   * lexicographic ordering.
   *
   * @param key the key value for a previously created LabelStr
   * instance.  @see m_key, getString()
   */
  LabelStr(double key);

#ifdef PLEXIL_FAST

  LabelStr(const LabelStr& org) : StoredString(org.getKey())
  {
  }

  inline operator double () const
  {
    return getKey();
  }

#else

  /**
   * @brief Copy constructor.
   *
   * Only needs to copy the key since the string value can be
   * recovered from the shared repository.
   *
   * @param org The source LabelStr.
   */
  LabelStr(const LabelStr& org);

  operator double () const;

#endif

  /**
   * @brief Lexical ordering test - less than
   */
  bool operator <(const LabelStr& lbl) const;

  /**
   * @brief Lexical ordering test - greater than
   */
  bool operator >(const LabelStr& lbl) const;

  /**
   * @brief Test equivilency of two LabelStr, defined by having
   * the same key.
   */
  //bool LabelStr::operator==(const LabelStr& lbl) const;

  /**
   * @brief Return the represented string.
   */
  const std::string& toString() const;

  /**
   * @brief Return the represent char*
   */
  const char* c_str() const;

  /**
   * @brief Obtain the encoded key value for the string.
   * @return The key for accessing the store of strings.
   */
  inline double getKey() const
  {
    return StoredString::getKey();
  }

  /**
   * @brief Tests if a given string is contained within this string.
   * @param lblStr The string to test for
   * @return true if present, otherwise false.
   */
  bool contains(const LabelStr& lblStr) const;

  /**
   * @brief Return the number of elements in the string delimited by the given delimeter.
   *
   * Cases:
   * 1. 'A:B:C:DEF' will contain 4 elements
   * 2. 'A' will contain 1 element
   * 3. ....A:' is invalid.
   * 4. ':A... is invalid
   *
   * @param delimiter The delimeter to mark element boundaries
   * @return The number of elements found.
   * @see getElement
   */
  unsigned int countElements(const char* delimiter) const;

  /**
   * @brief Return the requested element in a delimited string
   *
   * Cases:
   * 1. 'A:B:C:DEF', 2  => 'C'
   * 2. 'A:B:C:DEF', < 0 => error
   * 2. 'A:B:C:DEF', > 3 => error
   *
   * @param index The position of the requested element
   * @param delimiter The delimeter to mark
   */
  LabelStr getElement(unsigned int index, const char* delimiter) const;

  /**
   * @brief Return the number of strings stored.
   */
  static unsigned int getSize();

  /**
   * @brief Test if the given double valued key represents a string.
   * @param key The double to be tested.
   */
  static bool isString(double key);

  /**
   * @brief Tests if the given candidate is actually stored already as a LabelStr
   * @param candidate The string to be tested.
   */
  static bool isString(const std::string& candidate);

private:

  friend class LabelStrLocalStatic;

  /**
   * @brief Obtain the string from the key.
   * @param key The double valued encoding of the string
   * @return a reference to the original string held in the string store.
   * @see s_stringFromKeys
   */
  static const std::string& getString(double key);


#ifndef PLEXIL_FAST
  const char* m_chars;
#endif

protected:
};
}
#endif
