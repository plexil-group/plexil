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

#ifndef ITEM_STORE_HH
#define ITEM_STORE_HH

#include "Error.hh"
#include "ItemStoreEntry.hh"
#ifdef PLEXIL_WITH_THREADS
#include "ThreadMutex.hh"
#endif

namespace PLEXIL
{

  /**
   * @class ItemStore
   * @brief A templatized class defining the interface to item storage
   *        used by StoredArray.
   * @tparam key_t The key type; presumed to be a numeric type.
   * @tparam item_t The item type; presumed to be a container class.
   * @tparam key_source_t The key source generator class.
   * The key_source_t class must be instantiable and must implement
   * the following member functions:
   *
   * key_t next();
   * Returns the next available key value and marks it as used.
   *
   * static bool rangeCheck(const key_t& key);
   * Returns true if the key is in the valid range, false otherwise.
   *
   * The key_source_t class's destructor must delete any storage used to 
   * record the keys in use and/or available.
   *
   * @tparam table_t The class implementing the actual storage.
   * The table_t class must be instantiable and must implement the
   * following member functions:
   *
   * ItemStoreEntry<item_t>* get(key_t key) const;
   * Finds the entry with the requested key. Returns NULL if not found.
   *
   * void insertEntry(key_t key, ItemStoreEntry<item_t>* entry);
   * Inserts the entry into the tables, based on its contents.
   *
   * void removeEntry(key_t key);
   * Removes the named entry from the tables, and deletes the entry.
   *
   * Additionally, the table_t destructor must delete all remaining Entry 
   * instances.
   *
   * @see ItemStoreEntry, ItemTable, TwoWayStore, TwoWayTable, StoredArray
   */

  template <typename key_t,
            typename item_t,
            typename key_source_t,
            typename table_t>
  class ItemStore
  {
  private:
    typedef ItemStoreEntry<item_t> entry_t;

  public:

    /** 
     * @brief Default constructor.
     * @note The empty item is not reference counted for efficiency's sake.
     */
    ItemStore()
      : m_emptyKey(key_source_t::unassigned()),
        m_emptyItem(),
#ifdef PLEXIL_WITH_THREADS
        m_mutex(),
#endif
        m_table()
    {
      m_emptyKey = storeItem(m_emptyItem);
    }

    /** 
     * @brief Destructor. Cleans up any remaining ItemStoreEntry instances.
     */
    ~ItemStore()
    {
    }

    /**
     * @brief Return the key for the canonical empty item.
     * @return The key.
     * @note Used by client objects' default constructors.
     */
    inline const key_t & getEmptyKey() const
    {
      return m_emptyKey;
    }

    /**
     * @brief Return the canonical empty item.
     * @return Const reference to the item.
     * @note Used by client objects' assignment operators.
     */
    inline const item_t & getEmptyItem() const
    {
      return m_emptyItem;
    }
    
    /**
     * @brief Determine whether the key is in the store.
     * @param key The key value.
     * @return true if the key is in the store, false otherwise.
     */
    bool isKey(key_t key)
    {
      if (!key_source_t::rangeCheck(key))
        return false;
#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(m_mutex);
#endif
      return NULL != m_table.get(key);
    }

    /**
     * @brief Get the value stored at this key.
     * @param key The key value.
     * @return Pointer to the value; NULL if not found.
     * @note Can't be const because of the mutex, sigh.
     */
    item_t* getItem(key_t key)
    {
#ifdef PLEXIL_FAST
      if (!key_source_t::rangeCheck(key))
        return NULL;
#else
      checkError(key_source_t::rangeCheck(key),
                 "ItemStore::getItem: key not in valid range");
#endif
#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(m_mutex);
#endif
      entry_t* entry = m_table.get(key);
      if (entry == NULL)
        return NULL;
      else
        return &(entry->item);
    }

    /**
     * @brief Store a copy of the item, and return the associated key.
     * @param item The item to copy and store.
     * @return The key value for the stored item.
     * @note Called during default constructor, to store the empty item.
     */
    key_t storeItem(const item_t& item)
    {
      entry_t* entry = new entry_t;
      entry->item = item;
      entry->refcount = 1;
#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(m_mutex);
#endif
      key_t key = m_keySource.next();
      m_table.insertEntry(key, entry);
      return key;
    }


    /**
     * @brief Record the existence of another StoredItem with an existing key.
     * @param key The key value for the StoredItem.
     * @return True if the key is valid, false if not.
     * @note Caller MUST check return value!
     * @note The empty item is not reference counted for efficiency's sake.
     */
    bool newReference(key_t key)
    {
#ifdef PLEXIL_FAST
      if (!key_source_t::rangeCheck(key))
        return false;
#else
      checkError(key_source_t::rangeCheck(key),
                 "ItemStore::newReference: key not in valid range");
#endif
      if (key == m_emptyKey)
        return true;

#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(m_mutex);
#endif
      entry_t* entry = m_table.get(key);
      if (entry == NULL)
        return false;

      ++(entry->refcount);
      return true;
    }

    /**
     * @brief Record the deletion of a StoredItem with the given key.
     * If no references remain, the key-item pair will be deleted from the store.
     * @param key The key value for the deleted StoredItem.
     * @note The empty item is not reference counted for efficiency's sake.
     */
    void deleteReference(key_t key)
    {
#ifdef PLEXIL_FAST
      if (!key_source_t::rangeCheck(key))
        return;
#else
      checkError(key_source_t::rangeCheck(key),
                 "ItemStore::deleteReference: key not in valid range");
#endif
      if (key == m_emptyKey)
        return;

#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(m_mutex);
#endif
      entry_t* entry = m_table.get(key);
      assertTrue(entry != NULL,
                 "ItemStore::deleteReference: Internal error: key not found");
      assertTrue(entry->refcount != 0,
                 "ItemStore::deleteReference: Internal error: item's refcount is already zero");

      // Delete entry when all references have been deleted
      if (0 == --(entry->refcount)) {
        m_table.removeEntry(key);
      }
    }

    size_t size() const
    {
      return m_table.size();
    }

  private:

    // Deliberately not implemented
    ItemStore(const ItemStore&);
    ItemStore& operator=(const ItemStore&);

    key_t m_emptyKey;
    item_t m_emptyItem;
#ifdef PLEXIL_WITH_THREADS
    ThreadMutex m_mutex; //!< Mutex to serialize access to the item store.
#endif
    key_source_t m_keySource; //!< Implements key generation.
    table_t m_table; //!< Implements item storage.


  };

}

#endif // ITEM_STORE_HH
