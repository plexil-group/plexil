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

#ifndef TWO_WAY_STORE_HH
#define TWO_WAY_STORE_HH

#include "plexil-config.h"

#if defined(TWO_WAY_STORE_DEBUG)
#include "Debug.hh"
#endif

#include "Error.hh"
#include "ItemStoreEntry.hh"
#ifdef PLEXIL_WITH_THREADS
#include "ThreadMutex.hh"
#endif

namespace PLEXIL
{

  /**
   * @class TwoWayStore
   * @brief A templatized class defining the interface to item storage
   *        used by StoredItem.
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
   * ItemStoreEntry<item_t>* getByKey(key_t key) const;
   * Finds the entry with the requested key. Returns NULL if not found.
   *
   * bool getItemKey(const item_t& item, key_t& result) const;
   * Finds the key corresponding to the requested item and stores it in result.
   * Returns true if found, false otherwise.
   *
   * ItemStoreEntry<item_t>* getByItem(const item_t& item) const;
   * Finds the entry with the requested item. Returns NULL if not found.
   *
   * void insertEntry(key, ItemStoreEntry<item_t>* entry);
   * Inserts the entry into the tables, based on its contents.
   *
   * void removeEntry(key_t key);
   * Removes the entry corresponding to the key (if any) from the tables, and deletes the entry.
   *
   * Additionally, the table_t destructor must delete all remaining ItemStoreEntry
   * instances.
   */

  template <typename key_t,
            typename item_t,
            typename key_source_t,
            typename table_t>
  class TwoWayStore
  {
  private:
    typedef ItemStoreEntry<item_t> entry_t;

  public:

    /** 
     * @brief Default constructor.
     * @note The empty item is not reference counted for efficiency's sake.
     */
    TwoWayStore()
      : m_emptyKey(key_source_t::unassigned()),
#ifdef PLEXIL_WITH_THREADS
        m_mutex(new ThreadMutex()),
#endif
        m_table()
    {
      m_emptyKey = storeItem(item_t(), true);
    }

    /** 
     * @brief Destructor. Cleans up any remaining ItemStoreEntry instances.
     */
    ~TwoWayStore()
    {
#ifdef PLEXIL_WITH_THREADS
      delete m_mutex;
      m_mutex = NULL;
#endif
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
     * @brief Determine whether the key is in the store.
     * @param key The key value.
     * @return true if the key is in the store, false otherwise.
     */
    bool isKey(key_t key) const
    {
      if (!key_source_t::rangeCheck(key))
        return false;
#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(*m_mutex);
#endif
      return NULL != m_table.getByKey(key);
    }
    
    /**
     * @brief Determine whether the key is in the store.
     * @param key The key value.
     * @return true if the key is in the store, false otherwise.
     */
    bool isItem(const item_t& item) const
    {
#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(*m_mutex);
#endif
      return NULL != m_table.getByItem(item);
    }


    /**
     * @brief Get the value stored at this key.
     * @param key The key value.
     * @return Pointer to the value; NULL if not found.
     */
    item_t* getItem(key_t key) const
    {
      // Quick cheap sanity check
#ifdef PLEXIL_FAST
      if (!key_source_t::rangeCheck())
        return NULL;
#else
      checkError(key_source_t::rangeCheck(key),
                 "TwoWayStore::getItem: key not in valid range");
#endif

#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(*m_mutex);
#endif
      entry_t* entry = m_table.getByKey(key);
      if (entry == NULL)
        return NULL;
      else
        return &(entry->item);
    }

    /**
     * @brief Store the item, and return the associated key.
     * @param item The item to copy and store.
     * @param permanent Set to true if the value should be considered permanent
     * (i.e. not reference counted).
     * @return The key value for the stored item.
     * @note If an equivalent item is already stored, its key is returned, and 
     *       the item is not copied.
     */
    key_t storeItem(const item_t& item, bool permanent = false)
    {
#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(*m_mutex);
#endif
      key_t key;
      if (m_table.getItemKey(item, key)) {
        // Bypass refcounting for permanent keys
        if (key_source_t::isSpecial(key)) {
#if defined(TWO_WAY_STORE_DEBUG)
          debugMsg("TwoWayStore:storeItem", " for existing permanent item " << item);
#endif
          return key; 
        }
        entry_t* entry = m_table.getByKey(key);
        assertTrueMsg(entry != NULL,
                      "TwoWayStore::storeItem: Consistency failure: item has key, but key has no entry");
        ++(entry->refcount);
        assertTrue(entry->refcount != 0,
                   "TwoWayStore::storeItem: Internal error: item's refcount wrapped around to zero!");
#if defined(TWO_WAY_STORE_DEBUG)
        debugMsg("TwoWayStore:storeItem",
                 " for existing item " << item << ", new refcount is " << entry->refcount);
#endif
        return key;
      }
#if defined(TWO_WAY_STORE_DEBUG)
      debugMsg("TwoWayStore:storeItem", " new item " << item << ", refcount = 1");
#endif
      entry_t* entry = new entry_t;
      entry->refcount = 1;
      entry->item = item;
      key = m_keySource.next(permanent);
      m_table.insertEntry(key, entry);
      return key;
    }


    /**
     * @brief Record the existence of another StoredItem with an existing key.
     * @param key The key value for the StoredItem.
     * @return True if the key is valid, false if not.
     * @note Caller MUST check return value!
     * @note Special items are not reference counted for efficiency's sake.
     */
    bool newReference(key_t key)
    {
      // Don't reference count "permanent" items
      if (key_source_t::isSpecial(key))
        return true;

#ifdef PLEXIL_FAST
      if (!key_source_t::rangeCheck(key))
        return false;
#else
      checkError(key_source_t::rangeCheck(key),
                 "TwoWayStore::newReference: key not in valid range");
#endif

#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(*m_mutex);
#endif
      entry_t* entry = m_table.getByKey(key);
      if (entry == NULL)
        return false;

      ++(entry->refcount);
      assertTrue(entry->refcount != 0,
                 "TwoWayStore::newReference: Internal error: item's refcount wrapped around to zero!");
#if defined(TWO_WAY_STORE_DEBUG)
      debugMsg("TwoWayStore:newReference",
               " for item " << entry->item << ", new refcount = " << entry->refcount);
#endif
      return true;
    }

    /**
     * @brief Record the deletion of a StoredItem with the given key.
     * If no references remain, the key-item pair will be deleted from the store.
     * @param key The key value for the deleted StoredItem.
     * @note Special items are not reference counted for efficiency's sake.
     */
    void deleteReference(key_t key)
    {
      // Don't reference count "permanent" items
      if (key_source_t::isSpecial(key))
        return;

#ifdef PLEXIL_FAST
      if (!key_source_t::rangeCheck(key))
        return; // fail fast on invalid keys
#else
      checkError(key_source_t::rangeCheck(key),
                 "TwoWayStore::deleteReference: key not in valid range");
#endif

#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(*m_mutex);
#endif
      entry_t* entry = m_table.getByKey(key);
      assertTrue(entry != NULL,
                 "TwoWayStore::deleteReference: Internal error: key not found");
      assertTrue(entry->refcount != 0,
                 "TwoWayStore::deleteReference: Internal error: item's refcount is already zero");

      // Delete entry when all references have been deleted
#if defined(TWO_WAY_STORE_DEBUG)
      debugMsg("TwoWayStore:deleteReference",
               " for item " << entry->item << ", new refcount = " << entry->refcount - 1);
#endif
      if (0 == --(entry->refcount)) {
#if defined(TWO_WAY_STORE_DEBUG)
        debugMsg("TwoWayStore:deleteReference", " deleting item " << entry->item);
#endif
        m_table.removeEntry(key);
      }
    }

    size_t size() const
    {
      return m_table.size();
    }

  private:

    // Deliberately not implemented
    TwoWayStore(const TwoWayStore&);
    TwoWayStore& operator=(const TwoWayStore&);

    key_t m_emptyKey;
#ifdef PLEXIL_WITH_THREADS
    ThreadMutex* m_mutex; //!< Mutex to serialize access to the item store. A pointer so reads of the store can be const.
#endif
    key_source_t m_keySource; //!< Implements key generation.
    table_t m_table; //!< Implements item storage.

  };

}

#endif // TWO_WAY_STORE_HH
