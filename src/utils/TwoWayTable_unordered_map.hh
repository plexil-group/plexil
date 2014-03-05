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

#ifndef TWO_WAY_TABLE_UNORDERED_MAP_HH
#define TWO_WAY_TABLE_UNORDERED_MAP_HH

#include "plexil-config.h"

#if defined(TWO_WAY_TABLE_DEBUG)
#include "Debug.hh"
#endif
#include "Error.hh"

// Include appropriate hash table type
#if defined(HAVE_UNORDERED_MAP)
// C++11
#include <unordered_map>
#define TWO_WAY_TABLE_UNORDERED_MAP std::unordered_map
#define TWO_WAY_TABLE_HASH std::hash

#elif defined(HAVE_TR1_UNORDERED_MAP)
// C++0X TR1
#include <tr1/unordered_map>
#define TWO_WAY_TABLE_UNORDERED_MAP std::tr1::unordered_map
#define TWO_WAY_TABLE_HASH std::tr1::hash

#elif defined(HAVE_EXT_HASH_MAP) || defined(HAVE_BACKWARD_HASH_MAP)
// GNU SGI-based hash_map
#include "GNU_hash_map.hh"
#define TWO_WAY_TABLE_UNORDERED_MAP __gnu_cxx::hash_map
#define TWO_WAY_TABLE_HASH __gnu_cxx::hash

#else
# error "std::unordered_map or compatible hash table not found."
#endif

#include "ItemStoreEntry.hh"

namespace PLEXIL
{

  /** 
   * @class TwoWayTable
   * @brief Implements an abstraction for a table with indices for both key and item,
   * with storage for a reference count per entry.
   * @tparam key_t The key type.
   * @tparam item_t The item type.
   * @tparam key_equal_fn_t The equality-test type for keys.
   * @tparam item_equal_fn_t The equality-test type for items.
   * @tparam key_hash_fn_t The hasher type for keys.
   * @tparam item_hash_fn_t The hasher type for items.
   * @see ItemStoreEntry, ItemTable, TwoWayStore, LabelStr
   */

  template <typename key_t,
            typename item_t,
            typename key_equal_fn_t = std::equal_to<key_t>,
            typename item_equal_fn_t = std::equal_to<item_t>,
            typename key_hash_fn_t = TWO_WAY_TABLE_HASH<key_t>,
            typename item_hash_fn_t = TWO_WAY_TABLE_HASH<item_t>
            >
  class TwoWayTable
  {
  private:
    typedef ItemStoreEntry<item_t> entry_t;

    typedef typename TWO_WAY_TABLE_UNORDERED_MAP<const key_t,
                                                 entry_t*,
                                                 key_hash_fn_t,
                                                 key_equal_fn_t>
    key_table_t;

    typedef typename TWO_WAY_TABLE_UNORDERED_MAP<const item_t,
                                                 key_t,
                                                 item_hash_fn_t,
                                                 item_equal_fn_t >
    item_table_t;

  public:
    //* Default constructor.
    TwoWayTable()
      : m_keyTable(), m_itemTable()
    {
    }

    //* Destructor.
    ~TwoWayTable()
    {
      // Clean up tables
      m_itemTable.clear();
      // N.B.: Done this way because GNU STL hash_map::erase() returns void, not an iterator.
      while (!m_keyTable.empty()) {
        typename key_table_t::iterator it = m_keyTable.begin();
        entry_t* entry = it->second;
        m_keyTable.erase(it);
        delete entry;
      }
    }

    bool empty() const
    {
      bool result = m_keyTable.empty();
      checkError(result == m_itemTable.empty(),
                 "TwoWayTable::empty: Results inconsistent, key table = " << result
                 << ", item table = " << m_itemTable.empty());
      return result;
    }


    size_t size() const
    {
      size_t result = m_keyTable.size();
      checkError(result == m_itemTable.size(),
                 "TwoWayTable::size: Size mismatch, key table = " << result
                 << ", item table = " << m_itemTable.size());
      return result;
    }

    /**
     * @brief Find the entry with the requested key.
     * @param key The key.
     * @return Pointer to the requested ItemStoreEntry; NULL if not found.
     */
    entry_t* getByKey(key_t key) const
    {
      typename key_table_t::const_iterator it = m_keyTable.find(key);
      if (it == m_keyTable.end())
        return NULL;
      else
        return it->second;
    }

    /**
     * @brief Find the key for the requested item.
     * @param item The item.
     * @param result Reference to the place to store the key.
     * @return true if the item was found, false otherwise.
     */
    bool getItemKey(const item_t& item, key_t& result) const
    {
      typename item_table_t::const_iterator it = m_itemTable.find(item);
      if (it == m_itemTable.end())
        return false;
      result = it->second;
      return true;
    }

    /**
     * @brief Find the entry with the requested item.
     * @param item The item.
     * @return Pointer to the requested ItemStoreEntry; NULL if not found.
     */
    entry_t* getByItem(const item_t& item) const
    {
      typename item_table_t::const_iterator it = m_itemTable.find(item);
      if (it == m_itemTable.end())
        return NULL;
      entry_t* result = getByKey(it->second);
      checkError(result != NULL,
                 "TwoWayTable::getByItem: item found but key not found");
      return result;
    }

    /**
     * @brief Insert the entry into the tables, based on its contents.
     * @param entry Pointer to the entry.
     */
    void insertEntry(const key_t key, entry_t* entry)
    {
      assertTrue_1(entry != NULL);
      m_keyTable[key] = entry;
      m_itemTable[entry->item] = key;
#if defined(TWO_WAY_TABLE_DEBUG)
      debugMsg("TwoWayTable:insertEntry",
               " inserted " << entry->item << " at key " << key);
#endif
    }

    /**
     * @brief Remove the named entry from the tables and delete it.
     * @param Key of the entry to delete.
     * @note Does not signal an error if key not found.
     */
    virtual void removeEntry(key_t key)
    {
      typename key_table_t::iterator it = m_keyTable.find(key);
      if (it == m_keyTable.end()) {
#if defined(TWO_WAY_TABLE_DEBUG)
        debugMsg("TwoWayTable:removeEntry",
                 " no item found at key " << key << ", ignoring");
#endif
        return;
      }
#if defined(TWO_WAY_TABLE_DEBUG)
      debugMsg("TwoWayTable:removeEntry",
               " removing " << it->second->item << " at key " << key);
#endif
      m_itemTable.erase(it->second->item);
      delete it->second;
      m_keyTable.erase(it);
    }

  private:

    // Deliberately not implemented
    TwoWayTable(const TwoWayTable&);
    TwoWayTable& operator=(const TwoWayTable&);

    // State
    key_table_t m_keyTable;
    item_table_t m_itemTable;
  };
            
}

#endif // TWO_WAY_TABLE_UNORDERED_MAP_HH
