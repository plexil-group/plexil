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

#ifndef TWO_WAY_TABLE_DINKUM_HH
#define TWO_WAY_TABLE_DINKUM_HH

#include "plexil-config.h"

#include "Dinkum_hash_map.hh"
#include "Error.hh"
#include "ItemStoreEntry.hh"

namespace PLEXIL
{

  /** 
   * @class TwoWayTable
   * @brief Implements an abstraction for a table with indices for both key and item,
   * with storage for a reference count per entry.
   * @tparam key_t The key type.
   * @tparam item_t The item type.
   * @tparam key_comp_t The comparison type for keys.
   * @tparam item_comp_t The comparison type for items.
   * @see ItemStoreEntry, ItemTable, TwoWayStore, LabelStr
   */

  template <typename key_t,
            typename item_t,
            typename key_comp_t = PLEXIL::hash_compare<key_t>,
            typename item_comp_t = PLEXIL::hash_compare<item_t>
            >
  class TwoWayTable
  {
  private:
    typedef ItemStoreEntry<item_t> entry_t;

    typedef typename std::hash_map<const key_t,
                                   entry_t*,
                                   key_comp_t>
    key_table_t;

    typedef typename std::hash_map<const item_t,
                                   key_t,
                                   item_comp_t>
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
      typename key_table_t::iterator it = m_keyTable.begin();
      while (it != m_keyTable.end()) {
        entry_t* entry = it->second;
        it = m_keyTable.erase(it);
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
      assertTrue(entry != NULL);
      m_keyTable[key] = entry;
      m_itemTable[entry->item] = key;
    }

    /**
     * @brief Remove the named entry from the tables and delete it.
     * @param Key of the entry to delete.
     * @note Does not signal an error if key not found.
     */
    virtual void removeEntry(key_t key)
    {
      typename key_table_t::iterator it = m_keyTable.find(key);
      if (it == m_keyTable.end())
        return;
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

#endif // TWO_WAY_TABLE_DINKUM_HH
