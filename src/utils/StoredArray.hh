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

#ifndef _H_StoredArray
#define _H_StoredArray

/**
 * @file StoredArray.hh
 * @brief Declares the StoredArray class
 * @author Chuck Fry, rewrite of earlier version by Robert Harris based on code by Conor McGann
 * @date 21 Dec 2012
 */

#include "ItemStore.hh"
#include "ItemTable.hh"
#include "KeySource.hh"
#include "StoredItem.hh"
#include <vector>

namespace PLEXIL
{
  typedef double StoredArray_key_t;

  typedef std::vector<double> StoredArray_value_t;

  typedef ItemTable<StoredArray_key_t, StoredArray_value_t>
  StoredArray_table_t;

  typedef ItemStore<StoredArray_key_t,
                    StoredArray_value_t,
                    KeySource<double, NegativeDenormKeyTraits<double> >,
                    StoredArray_table_t>
  StoredArray_store_t;

  typedef StoredItem<StoredArray_key_t,
                     StoredArray_value_t,
                     StoredArray_store_t> 
  StoredArray_item_t;

  /**
   * @class StoredArray
   * @brief A StoredItem based implemention of a one-dimensional Plexil array.
   *
   * The StoredArray privides a minimal interface for accessing an array
   * for which a key can be obtained for later access to this array.
   */

  class StoredArray :
    protected StoredArray_item_t
  {
  public:

    /**
     * @brief Default constructor. Constructs an empty array.
     */
    StoredArray();

    /** 
     * @brief Copy constructor.
     * @param other The existing array.
     */
    StoredArray(const StoredArray& other);

    /** 
     * @brief Construct a new array of a given size and with all
     * elements initialized to the provided value.
     * @param size The maximum number of elements that this array
     * will contain.
     * @param initValue The value the elements in the array will be
     * initialized to.
     */
    StoredArray(size_t size, const double& initValue);

    /** 
     * @brief Construct a new array of a given size and initial values.
     * @param size The maximum number of elements that this array
     * will contain.
     * @param initValues The values the elements in the array will be
     * initialized to.
     */
    StoredArray(size_t size, const StoredArray_value_t& initValues);

    /** 
     * @brief Construct a new array directly from a vector of initial values.
     * @param initValues The values the elements in the array will be
     * initialized to.
     * @note The size is set from initValues.size().
     */
    StoredArray(const StoredArray_value_t& initValues);
         
    /** 
     * @brief Construct an array given a key from an existing array.
     * @param key The key of the already existing array.
     */
    StoredArray(StoredArray_key_t key);

    /**
     * @brief Destructor.
     */
    virtual ~StoredArray()
    {
    }

    /*
     * @brief Assignment operator from StoredArray.
     * @param other The other StoredArray.
     * @return Reference to this.
     */
    inline StoredArray& operator=(const StoredArray& other)
    {
      StoredArray_item_t::operator=(other);
      return *this;
    }

    /*
     * @brief Assignment operator from key to another StoredArray.
     * @param key The key.
     * @return Reference to this.
     * @note Can assert if key is invalid.
     */
    inline StoredArray& operator=(const StoredArray_key_t& key)
    {
      StoredArray_item_t::operator=(key);
      return *this;
    }

    /*
     * @brief Equality operator.
     * @param other The other StoredArray.
     * @return True if equal, false otherwise.
     */
    inline bool operator==(const StoredArray& other) const
    {
      return StoredArray_item_t::operator==(other);
    }

    /*
     * @brief Equality operator from value type.
     * @param value The value to compare.
     * @return True if equal, false otherwise.
     */
    inline bool operator==(const StoredArray_value_t& value) const
    {
      return getItem() == value;
    }

    /*
     * @brief Equality operator from key type.
     * @param key The key to compare.
     * @return True if equal, false otherwise.
     */
    inline bool operator==(const StoredArray_key_t& key) const
    {
      return getKey() == key;
    }

    /*
     * @brief Inequality operator.
     * @param other The other StoredArray.
     * @return False if equal, true otherwise.
     */
    inline bool operator!=(const StoredArray& other) const
    {
      return !operator==(other);
    }

    /*
     * @brief Inequality operator from value type.
     * @param value The value to compare.
     * @return False if equal, true otherwise.
     */
    inline bool operator!=(const StoredArray_value_t& value) const
    {
      return !operator==(value);
    }

    /*
     * @brief Inequality operator from key type.
     * @param key The key to compare.
     * @return False if equal, true otherwise.
     */
    inline bool operator!=(const StoredArray_key_t& key) const
    {
      return !operator==(key);
    }

    /** 
     * @brief Get the actual array.
     * @return A reference to the array.
     */
    inline StoredArray_value_t& getArray()
    {
      return getItem();
    }

    /** 
     * @brief Get the array given an existing key, without allocating a new StoredItem.
     * @param key The existing key.
     * @return A reference to the array.
     */
    inline static StoredArray_value_t& getArray(StoredArray_key_t key)
    {
      return getItem(key);
    }

    /** 
     * @brief Get the actual array.
     *
     * @return A reference to the array.
     */
    inline const StoredArray_value_t& getConstArray() const
    {
      return getItem();
    }
         
    /**
     * @brief Return the size of this array.
     *
     * @return The size of this array.
     */
    size_t size() const;
         
    /**
     * @brief Return key which can be used to access this array at a
     * later time.
     *
     * @brief Access key of this array.
     */
    inline StoredArray_key_t getKey() const
    {
      return StoredArray_item_t::getKey();
    }

    /**
     * @brief Return true if key value is valid.
     *
     * @param key candidate key
     * @return true if valid key
     */
    inline static bool isKey(StoredArray_key_t key)
    {
      return StoredArray_item_t::isKey(key);
    }
         
    /**
     * @brief Return const value of array elment.
     *
     * @param index Index of array element.
     */
    const double& at(const size_t index) const;
         
    /**
     * @brief Operator for accessing elements in this array.
     *
     * @return A reference to the specifed array element.
     */
    double& operator[] (const size_t index);

    /**
     * @brief Generate a printed representation for this stored array.
     */
    std::string toString() const;

    /**
     * @brief Get the number of stored instances.
     * @return The number of stored instances.
     */
    inline static size_t getSize()
    {
      return StoredArray_item_t::getSize();
    }

  };
}
#endif
