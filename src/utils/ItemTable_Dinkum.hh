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

#ifndef ITEM_TABLE_DINKUM_HH
#define ITEM_TABLE_DINKUM_HH

#include "plexil-config.h"

#include "Dinkum_hash_map.hh"
#include "Error.hh"
#include "ItemStoreEntry.hh"

namespace PLEXIL
{

  /** 
   * @class ItemTable
   * @brief Implements an abstraction for a table, with storage for a reference count per entry.
   * @tparam key_t The key type.
   * @tparam item_t The item type.
   * @tparam key_comp_t The equality-test type for keys.
   * @see ItemStoreEntry, ItemStore, StoredArray, TwoWayTable
   */

  template <typename key_t,
            typename item_t,
            typename key_comp_t = PLEXIL::hash_compare<key_t>
            >
  class ItemTable
  {
  private:
    typedef ItemStoreEntry<item_t> entry_t;

    typedef typename std::hash_map<const key_t,
                                   entry_t*,
                                   key_comp_t>
    table_t;

  public:
    //* Default constructor.
    ItemTable()
      : m_keyTable()
    {
    }

    //* Destructor.
    virtual ~ItemTable()
    {
      // Clean up tables
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
      assertTrue(entry != NULL);
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

#endif // ITEM_TABLE_DINKUM_HH
