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

#ifndef ITEM_TABLE_UNORDERED_MAP_HH
#define ITEM_TABLE_UNORDERED_MAP_HH

#include "plexil-config.h"
#include "Error.hh"

#if defined(HAVE_UNORDERED_MAP)
// C++11
#include <unordered_map>
#define ITEM_TABLE_UNORDERED_MAP std::unordered_map
#define ITEM_TABLE_HASH std::hash

#elif defined(HAVE_TR1_UNORDERED_MAP)
// C++0X TR1
#include <tr1/unordered_map>
#define ITEM_TABLE_UNORDERED_MAP std::tr1::unordered_map
#define ITEM_TABLE_HASH std::tr1::hash

#elif defined(HAVE_EXT_HASH_MAP) || defined(HAVE_BACKWARD_HASH_MAP)
// GNU SGI-based hash_map
#include "GNU_hash_map.hh"
#define ITEM_TABLE_UNORDERED_MAP __gnu_cxx::hash_map
#define ITEM_TABLE_HASH __gnu_cxx::hash

#else
# error "unordered_map or compatible equivalent not found in this environment."
#endif

#include "ItemStoreEntry.hh"

namespace PLEXIL
{

  /** 
   * @class ItemTable
   * @brief Implements an abstraction for a table, with storage for a reference count per entry.
   * @tparam key_t The key type.
   * @tparam item_t The item type.
   * @tparam key_equal_fn_t The equality-test type for keys.
   * @tparam key_hash_fn_t The hasher type for keys.
   * @see ItemStoreEntry, ItemStore, StoredArray, TwoWayTable
   */

  template <typename key_t,
            typename item_t,
            typename key_equal_fn_t = std::equal_to<key_t>,
            typename key_hash_fn_t = ITEM_TABLE_HASH<key_t>
            >
  class ItemTable
  {
  private:
    typedef ItemStoreEntry<item_t> entry_t;

    typedef typename ITEM_TABLE_UNORDERED_MAP<const key_t,
                                              entry_t*,
                                              key_hash_fn_t,
                                              key_equal_fn_t >
    table_t;

  public:
    //* Default constructor.
    ItemTable()
      : m_keyTable()
    {
    }

    //* Destructor.
    ~ItemTable()
    {
      // Clean up tables
      // N.B.: Done this way because GNU STL hash_map::erase() returns void, not an iterator.
      while (!m_keyTable.empty()) {
        typename table_t::iterator it = m_keyTable.begin();
        entry_t* entry = it->second;
        m_keyTable.erase(it);
        delete entry;
      }
    }

    bool empty() const
    {
      return m_keyTable.empty();
    }


    size_t size() const
    {
      return m_keyTable.size();
    }

    /**
     * @brief Find the entry with the requested key.
     * @param key The key.
     * @return Pointer to the requested ItemStoreEntry; NULL if not found.
     */
    entry_t* get(key_t key) const
    {
      typename table_t::const_iterator it = m_keyTable.find(key);
      if (it == m_keyTable.end())
        return NULL;
      else
        return it->second;
    }

    /**
     * @brief Insert the entry into the tables, based on its contents.
     * @param entry Pointer to the entry.
     */
    void insertEntry(key_t key, entry_t* entry)
    {
      assertTrue_1(entry != NULL);
      m_keyTable[key] = entry;
    }

    /**
     * @brief Remove the named entry from the table and delete it.
     * @param key Key of the entry to delete.
     * @note Does not signal an error if key not found.
     */
    virtual void removeEntry(key_t key)
    {
      typename table_t::iterator it = m_keyTable.find(key);
      if (it == m_keyTable.end())
        return;
      delete it->second;
      m_keyTable.erase(it);
    }

  private:

    // Deliberately not implemented
    ItemTable(const ItemTable&);
    ItemTable& operator=(const ItemTable&);

    // State
    table_t m_keyTable;
  };
            
}

#endif // ITEM_TABLE_UNORDERED_MAP_HH
