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

/**
 * @file LabelStr.hh
 * @brief Declares the LabelStr class
 * @author Chuck Fry, from an earlier version by Robert Harris, based on code by Conor McGann
 * @date Decemeber 2012
 * @ingroup Utils
 */

#ifndef _H__LabelStr
#define _H__LabelStr

#include "ConstantMacros.hh"
#include "Error.hh"
#include "KeySource.hh"
#include "TwoWayStore.hh"
#include "TwoWayTable.hh"

#include <string>
#include <vector>

namespace PLEXIL
{
  
  // Forward reference
  class Value;

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

  typedef double LabelStr_key_t;

  typedef std::string LabelStr_value_t;

  typedef TwoWayTable<LabelStr_key_t, LabelStr_value_t>
  LabelStr_table_t;

  typedef PartitionedKeySource<LabelStr_key_t> LabelStr_keysource_t;

  typedef TwoWayStore<LabelStr_key_t, 
                      LabelStr_value_t,
                      LabelStr_keysource_t,
                      LabelStr_table_t>
  LabelStr_store_t;
                             
  class LabelStr
  {
         
  public:
    /**
     * Zero argument constructor.
     * @note Should only be used indirectly, e.g., via std::list.
     */
    LabelStr();

    /**
     * @brief Constructor
     * @param label The symbolic value as a string
     */
    LabelStr(const std::string& label);

    /**
     * @brief Constructor
     * @param str A null terminated string
     * @param permanent A value of true means the LabelStr value should be considered
     * a permanent constant (i.e. not reference counted).
     */
    LabelStr(const char* str, bool permanent = false);

    /**
     * @brief Constructor from Value instance
     * @param The Value.
     * @note CALLER MUST CHECK THAT VALUE IS A STRING!
     */
    LabelStr(const Value& value);

    /**
     * @brief Copy constructor.
     *
     * Only needs to copy the key since the string value can be
     * recovered from the shared repository.
     *
     * @param org The source LabelStr.
     */
    LabelStr(const LabelStr& org);

    /**
     * @brief Destructor.
     */
    ~LabelStr();

    /**
     * @brief Assignment operator
     * @param org LabelStr.
     * @return Reference to this LabelStr.
     */
    LabelStr& operator=(const LabelStr& org);

    /**
     * @brief Assignment operator from std::string
     * @param string The new value as a std::string.
     * @return Reference to this LabelStr.
     */
    LabelStr& operator=(const std::string& string);

    /**
     * @brief Assignment operator from char*
     * @param chars The new value as a const reference to const char*.
     * @return Reference to this LabelStr.
     */
    LabelStr& operator=(const char* chars);

    /**
     * @brief Assignment operator from key type
     * @param key The key from another existing LabelStr.
     * @return Reference to this LabelStr.
     */
    LabelStr& operator=(LabelStr_key_t key);

    /**
     * @brief Assignment operator from Value
     * @param key The value
     * @return Reference to this LabelStr.
     * @note CALLER MUST CHECK THAT VALUE IS A STRING!
     */
    LabelStr& operator=(const Value& value);

    /**
     * @brief Equality operator.
     * @param other LabelStr.
     * @return true if equal, false otherwise.
     */
    inline bool operator==(const LabelStr& other) const
    {
      return m_key == other.m_key;
    }

    /**
     * @brief Equality operator for std::string.
     * @param other A string for comparison.
     * @return true if equal, false otherwise.
     */
    bool operator==(const std::string& other) const;

    /**
     * @brief Equality operator for const char*.
     * @param other A char* constant for comparison.
     * @return true if equal, false otherwise.
     */
    bool operator==(const char* other) const;

    /**
     * @brief Equality operator for Value.
     * @param value The Value.
     * @return true if equal, false otherwise.
     */
    bool operator==(const Value& value) const;

    /**
     * @brief Inequality operator.
     * @param other LabelStr.
     * @return true if equal, false otherwise.
     */
    inline bool operator!=(const LabelStr& other) const
    {
      return !operator==(other);
    }

    /**
     * @brief Inequality operator for std::string.
     * @param other A string for comparison.
     * @return false if equal, true otherwise.
     */
    inline bool operator!=(const std::string& other) const
    {
      return !operator==(other);
    }

    /**
     * @brief Inequality operator for Value.
     * @param value The Value.
     * @return false if equal, true otherwise.
     */
    bool operator!=(const Value& value) const;

    /**
     * @brief Inequality operator for const char*.
     * @param other A char* constant for comparison.
     * @return false if equal, true otherwise.
     */
    inline bool operator!=(const char* other) const
    {
      return !operator==(other);
    }

    /**
     * @brief Return the represented string.
     * @return Const reference to the string.
     */
    const std::string& toString() const;

    /**
     * @brief Return the represent char*
     */
    inline const char* c_str() const
    {
      return toString().c_str();
    }

    /**
     * @brief Return the char* represented by the key
     * @param key The key.
     * @return The const char*.
     * @note Throws an exception if not found.
     */
    inline static const char* c_str(LabelStr_key_t key)
    {
      return toString(key).c_str();
    }

    /**
     * @brief Lexical ordering test - less than
     */
    bool operator<(const LabelStr& lbl) const;

    /**
     * @brief Lexical ordering test - greater than
     */
    bool operator>(const LabelStr& lbl) const;

    /**
     * @brief Tests if a given string is contained within this string.
     * @param lblStr The string to test for.
     * @return true if present, otherwise false.
     */
    bool contains(const LabelStr& lblStr) const;

    /**
     * @brief Tests if a given string is contained within this string.
     * @param str The string to test for.
     * @return true if present, otherwise false.
     */
    bool contains(const std::string& str) const;

    /**
     * @brief Tests if a given string is contained within this string.
     * @param str The const char* to test for.
     * @return true if present, otherwise false.
     */
    bool contains(const char* str) const;

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
    size_t countElements(const char* delimiter) const;

    /**
     * @brief Return the number of strings stored.
     */
    inline static size_t getSize()
    {
      return itemStore().size();
    }

    /**
     * @brief Test if the given double valued key represents a string.
     * @param key The double to be tested.
     */
    inline static bool isString(double key)
    {
      return itemStore().isKey(key);
    }

    /**
     * @brief Check whether a double value is in the LabelStr key range.
     * @param val The double to check.
     * @return True if in range, false otherwise.
     */
    inline static bool rangeCheck(double val)
    {
      return LabelStr_keysource_t::rangeCheck(val);
    }

    /**
     * @brief Tests if the given candidate is actually stored already as a LabelStr
     * @param candidate The string to be tested.
     */
    inline static bool isString(const std::string& candidate)
    {
      return itemStore().isItem(candidate);
    }

    /**
     * @brief Obtain the encoded key value for the string.
     * @return The key for accessing the store of strings.
     * @note Intended for use by Value class and hash functions below only.
     */
    inline double getKey() const
    {
      return m_key;
    }

  private:

    friend class Value;

    /**
     * @brief Get the string represented by this key.
     * @param key The key.
     * @return Const reference to the string.
     * @note Throws an exception if not found.
     * @note Only caller should be Value class.
     */
    static const std::string& toString(LabelStr_key_t key);

    /**
     * @brief Return the item store.
     * @note Only external caller should be Value class.
     */
    static LabelStr_store_t& itemStore();

    /**
     * @brief The key value used as a proxy for the original item.
     * @note The only instance data.
     */
    LabelStr_key_t m_key;

#if defined(LABEL_STR_DEBUG)
    // Pointer to stored string, for debugging use only
    const char* m_string;
#endif

  };

  DECLARE_GLOBAL_CONST(LabelStr, EMPTY_LABEL)

}

//
// Define a hash function for LabelStr
// Unfortunately implementation dependent
//

#if defined(HAVE_UNORDERED_MAP)

// C++11 unordered_map
#include <unordered_map>
namespace std
{
  template <>
  struct hash<PLEXIL::LabelStr> :
    public unary_function<PLEXIL::LabelStr, size_t>
  {
    size_t operator()(const PLEXIL::LabelStr& __val) const
    {
      return hash<PLEXIL::LabelStr_key_t>()(__val.getKey());
    }
  };
}

#elif defined(HAVE_TR1_UNORDERED_MAP)

// C++0x TR1 unordered_map
#include <tr1/unordered_map>

namespace std
{
  namespace tr1
  {
    template <>
    struct hash<PLEXIL::LabelStr> :
      public unary_function<PLEXIL::LabelStr, size_t>
    {
      size_t operator()(const PLEXIL::LabelStr& __val) const
      {
        return hash<PLEXIL::LabelStr_key_t>()(__val.getKey());
      }
    };
  }
}

#elif defined(HAVE_EXT_HASH_MAP) || defined(HAVE_BACKWARD_HASH_MAP)

// GNU libstdc++ hash_map
#include "GNU_hash_map.hh"

_GLIBCXX_BEGIN_NAMESPACE(__gnu_cxx)

template <>
struct hash<PLEXIL::LabelStr>
{
  size_t operator()(const PLEXIL::LabelStr& __val) const
  {
    return hash<PLEXIL::LabelStr_key_t>()(__val.getKey());
  }
};

_GLIBCXX_END_NAMESPACE

#elif defined(HAVE_HASH_MAP)
// Dinkumware or original SGI hash_map
#include "Dinkum_hash_map.hh"

namespace std
{
  template <>
  struct less<PLEXIL::LabelStr>
  {
    bool operator()(const PLEXIL::LabelStr& _Left, const PLEXIL::LabelStr& _Right) const
    {
      return (_Left.getKey() < _Right.getKey());
    }
  };

  template <>
  size_t hash_value(const PLEXIL::LabelStr& _Keyval);
}

#else
# error "Unable to find hash_map or equivalent class for this platform."
#endif

#endif //_H__LabelStr
