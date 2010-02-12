/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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
 * @file KeySource.hh
 * @brief Declares the KeySource class, a prerequisite of StoredItem
 * @author Robert Harris based on code by Conor McGann
 * @date 4 October 2007
 * @ingroup Utils
 */

#ifndef KEY_SOURCE_HH
#define KEY_SOURCE_HH

#include <limits>
#include <stack>
#include "Error.hh"
#include "ThreadMutex.hh"

namespace PLEXIL
{

  /**
   * @class KeySource
   * @brief Provides an abstract key source for StoredItem.
   *
   * Floating point keys use the the denormalized range of values for
   * keys.  Integer types use the full range of values for that type.
   */

  template <class key_t>
  class KeySource
  {
    // numeric limits of key type

    typedef typename std::numeric_limits<key_t>       keyLimits_t;

#ifdef STORED_ITEM_REUSE_KEYS
    // key pool type

    typedef typename std::stack<key_t>                keyPool_t;
#endif

  public:

    /**
     * @brief Returns the next available key.
     */

    inline static const key_t next()
    {
      // make key generation thread safe

      ThreadMutexGuard guard(getMutex());

      key_t sl_key;

#ifdef STORED_ITEM_REUSE_KEYS

      // if the key pool is not empty, use a key from there

      if (!keyPool().empty())
	{
	  sl_key = keyPool().top();
	  keyPool().pop();
	}
            
      // otherwise increment the counter
            
      else
#endif
	{
	  assertTrue(availableKeys() > 0, "Key space exhausted.");
	  sl_key = counter();
	  counter() += increment();
	}

      // get next key value 

      return sl_key;
    }

    /**
     * @brief Returns the total number of keys which may be
     * generated.
     */

    static const size_t totalKeys()
    {
      return ((size_t)((keyMax() - keyMin()) / increment())) - 1;
    }

    /**
     * @brief Returns the remaing number of keys which are
     * available.
     */

    static const size_t availableKeys()
    {
      return (totalKeys() 
	      - (size_t)((counter() - keyMin()) / increment()))
#ifdef STORED_ITEM_REUSE_KEYS
	+ keyPool().size()
#endif
	;
    }

    /**
     * @brief Free key for possible reassignment.
     *
     * If STORED_ITEM_REUSE_KEYS is defined at compile time the key
     * will be stored and potentially reissued in the future.  If
     * STORED_ITEM_REUSE_KEYS is NOT defined the key is retired.
     *
     * @param key Key which will be unregistered.
     */

    inline static void unregister(key_t& key)
    {
#ifdef STORED_ITEM_REUSE_KEYS
      // make key generation thread safe

      ThreadMutexGuard guard(getMutex());
      keyPool().push(key);
#endif           
    }

    /**
     * @brief Return the unassigned key value.
     *
     * @return The unassigned key value.
     */
         
    inline static const key_t& unassigned()
    {
      static const key_t sl_unassigned =
	(keyLimits_t::has_denorm == std::denorm_present
	 ? keyLimits_t::denorm_min()
	 : keyLimits_t::min());
      return sl_unassigned;
    }

    /**
     * @brief Return the key increment value.
     *
     * @return The smallest allowable increment for a given key_t.
     */

    inline static const key_t& increment()
    {
      static const key_t sl_increment = 
	(keyLimits_t::has_denorm == std::denorm_present
	 ? keyLimits_t::denorm_min()
	 : 1);
      return sl_increment;
    }

    /**
     * @brief Return the minimum key value.
     *
     * @return The smallest value of a key.
     */

    inline static const key_t& keyMin()
    {
      static const key_t sl_min = unassigned() + increment();
      return sl_min;
    }

    /**
     * @brief Return the maximum key value.
     *
     * @return The largest value of a key.
     */

    inline static const key_t& keyMax()
    {
      static const key_t sl_max = 
	(keyLimits_t::has_denorm == std::denorm_present
	 ? keyLimits_t::min()
	 : keyLimits_t::max());
      return sl_max;
    }


    /**
     * @brief Return the key infinity value.
     *
     * @return The largest value of a key.
     */

    inline static const key_t& infinity()
    {
      static const key_t sl_infinity = 
	(keyLimits_t::has_infinity
	 ? keyLimits_t::infinity()
	 : keyLimits_t::max());
      return sl_infinity;
    }

  protected:

    /**
     * @brief Returns the value of the next key to be released.
     */

    inline static key_t & counter()
    {
      static key_t sl_nextKey = keyMin();
      return sl_nextKey;
    }

    /**
     * @brief The mutex for thread safing.
     */
         
    inline static ThreadMutex& getMutex()
    {
      static ThreadMutex sl_mutex;
      return sl_mutex;
    }

#ifdef STORED_ITEM_REUSE_KEYS
    /**
     * @brief Stores unused keys for reuse.
     */

    inline static keyPool_t& keyPool()
    {
      static keyPool_t sl_keyPool;
      return sl_keyPool;
    }
#endif         
  };

}

#endif // KEY_SOURCE_HH
