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

#ifndef STORED_ITEM__DINKUM_HH
#define STORED_ITEM__DINKUM_HH

/**
 * @file StoredItem_Dinkum.hh
 * @brief Implements the StoredItem class for platforms with Dinkum STL extensions.
 * @author Chuck Fry, based on code by Conor McGann and Robert Harris
 * @date 12 February 2010
 * @ingroup Utils
 */

#include "KeySource.hh"
#include <hash_map>
#include <cstdint>

#ifndef STORED_ITEM_NO_MUTEX
#include "ThreadMutex.hh"
#endif

namespace PLEXIL
{

  /** 
   * @class StoredItemKeyCompare
   *
   * @brief Provides a specialized hash_compare class for key_t in StoredItem.
   */

  template <class key_t>
  class StoredItemKeyCompare
    : public std::hash_compare<key_t>
  {
  public:
    // Hashing operator
    size_t operator()(const key_t& key) const
    {
      return (size_t) (key / KeySource<key_t>::increment());
    }

    // Comparison operator (less-than)
    bool operator()(const key_t& keyval1, const key_t& keyval2) const
    {
      return keyval1 < keyval2;
    }

  };

  /** 
   * @class StoredItemPointerCompare
   *
   * @brief Provides a specialized hash_compare class for item_t in StoredItem.
   */

  template <class item_ptr_t>
  class StoredItemPointerCompare
    : public std::hash_compare<item_ptr_t>
  {
  public:
    // Hashing operator
    size_t operator()(item_ptr_t const &item) const
    {
      return (size_t)item;
    }

    // Comparison operator (less-than)
    bool operator()(item_ptr_t const &keyval1, item_ptr_t const &keyval2) const
    {
      return ((uintptr_t) keyval1) < ((uintptr_t) keyval2);
    }
  };


  /**
   * @class StoredItem
   * @brief Provides for a symbolic value to be handled in a domain.
   *
   * The reader should note that items are stored in a static data
   * structure so that they can be shared. Access to the store is
   * provided by a key value. This reduces operations on StoredItem to
   * operations on key_t valued keys which is considerably more
   * efficient. This encoding is largely transparent to users.
   */

  template <class key_t, 
	    class item_t, 
	    class keyComp_t = StoredItemKeyCompare<key_t>,
	    class itemPtrComp_t = StoredItemPointerCompare<item_t*>
	    >
  class StoredItem
  {
  protected:   
    // define types for item and key store
         
    typedef typename std::hash_map<const key_t, item_t*, keyComp_t> itemStore_t;
    typedef typename itemStore_t::const_iterator               itemConstIterator_t;
    typedef typename itemStore_t::iterator                     itemIterator_t;
    typedef typename std::pair<const key_t, item_t*>           itemPair_t;

    typedef typename std::hash_map<item_t*, const key_t, itemPtrComp_t> keyStore_t;
    typedef typename keyStore_t::const_iterator                    keyConstIterator_t;
    typedef typename keyStore_t::iterator                          keyIterator_t;
    typedef typename std::pair<item_t*, const key_t>         keyPair_t;

  public:

    /**
     * @brief Zero argument constructor.
     * @note Should only be used indirectly, e.g., via std::list.
     */

    StoredItem()
      : m_key(ensureKey(&empty(), false))
    {
    }

    /**
     * @brief This constructor copies the passed item into a
     * permanent location while the StoredItem is created.  The
     * passed item should be discarded.
     *
     * @param item The value of the item from which the created
     * item will be copied.
     */

    StoredItem(item_t& item)
      : m_key(ensureKey(&item, true))
    {
    }

    /**
     * @brief This constructor which optionally copies the passed
     * item into a permanent location while the StoredItem is
     * created.  The passed item should be discarded if copyItem is
     * true.
     *
     * @param item A pointer to value of the item, which will be
     * stored, or optionally copied before it's stored.
     *
     * @param copyItem If true, indicates that the item is not
     * currently stored in a permanent location and should be
     * copied into such a location.
     */

    StoredItem(item_t* item, bool copyItem)
      : m_key(ensureKey(item, copyItem))
    {
    }

    /**
     * @brief Constructor from encoded key.
     *
     * Each StoredItem gets encoded as a key such that any 2
     * instances of a StoredItem constructed from the same item
     * will have the same key and that the key preserves
     * lexicographic ordering.
     *
     * @param key the key value for a previously created StoredItem instance.
     * @see m_key, getItem()
     */
    StoredItem(key_t key)
      : m_key(key)
    {
      assertTrueMsg(isKey(m_key), "Invalid key " << key << " provided.");
    }

    /**
     * @brief Copy constructor.
     *
     * Only needs to copy the key since the item value can be
     * recovered from the shared repository.
     *
     * @param other The source StoredItem.
     */
    StoredItem(const StoredItem& other)
      : m_key(other.m_key)
    {
    }

    /**
     * @brief Return the canonical empty item.
     *
     * @return A static empty item.
     */

    static item_t& empty()
    {
      static item_t* sl_empty = NULL;
      if (sl_empty == NULL)
	sl_empty = new item_t();
      return *sl_empty;
    }

    /**
     * @brief Cast operator to get the key for this object.
     *
     * @return The key to this item.
     */

    operator key_t () const
    {
      return(m_key);
    }

    /**
     * @brief Lexical ordering test - less than
     */
    bool operator <(const StoredItem& item) const
    {
      return getItem() < item.getItem();
    }

    /**
     * @brief Lexical ordering test - greater than
     */

    bool operator >(const StoredItem& item) const
    {
      return getItem() > item.getItem();
    }

    /**
     * @brief Obtain the encoded key value for the item.
     * @return The key for accessing the store of items.
     */
    inline key_t getKey() const
    {
      return(m_key);
    }

    /**
     * @brief Return the number of items stored.
     * 
     * @return the number of items in this store.
     */
    static unsigned int getSize()
    {
#ifndef STORED_ITEM_NO_MUTEX
      ThreadMutexGuard guard(mutex());
#endif
      checkError(itemStore().size() == keyStore().size(), 
		 "itemStore size " << itemStore().size() << 
		 " does not match key store size " << keyStore().size());
      return(itemStore().size());
    }

    /**
     * @brief Test if the given key_t valued key maps to an item.
     */
    static bool isKey(key_t key)
    {
#ifndef STORED_ITEM_NO_MUTEX
      ThreadMutexGuard guard(mutex());
#endif
      return isKeyInternal(key);
    }
         
    /**
     * @brief Test if the given key_t valued key maps to an item.
     */
    static bool isItem(item_t* item)
    {
#ifndef STORED_ITEM_NO_MUTEX
      ThreadMutexGuard guard(mutex());
#endif
      return(keyStore().find(item) != keyStore().end());
    }
         
    /**
     * @brief Returns the cached item.
     */

    item_t& getItem() const
    {
      return getItem(m_key);
    }

    /**
     * @brief Free memory for for this stored item.
     *
     * Once freed the key value is set to an unassignedKey value.
     * If STORED_ITEM_REUSE_KEYS is defined at compile time the key
     * will be stored and potentially reissued in the future.  If
     * STORED_ITEM_REUSE_KEYS is NOT defined the key is retired,
     * any attempt to reuse the key will result in an error being
     * thrown.
     */
         
    void unregister()
    {
      unregister(m_key);
    }

    /**
     * @brief Obtain the key for the given item conducting an
     * insertion into sytem of it does not exits.
     *
     * @param item a pointer to the item to be added the store.
     * This item should be permanently allocated as it's address is
     * used to compute a hash.
     * @param copyItem Whether or not to create a copy of the item.
     *
     * @return The key value created.
     */

    static const key_t ensureKey(item_t* item, bool copyItem)
    {
#ifndef STORED_ITEM_NO_MUTEX
      ThreadMutexGuard guard(mutex());
#endif

      // if item is already in the system, return its key
      keyConstIterator_t ki = keyStore().find(item);
      if (ki != keyStore().end())
	return ki->second;
            
      // allocate a key for this item
      // can throw an exception if key values are exhausted
      const key_t key = KeySource<key_t>::next();

      // insert the item
      handleInsertion(key, copyItem ? new item_t(*item) : item);
      return(key);
    }

    /**
     * @brief Free memory for item associated with provided key.
     *
     * Once freed the key value is set to an unassignedKey value.
     * If STORED_ITEM_REUSE_KEYS is defined at compile time the key
     * will be stored and potentially reissued in the future.  If
     * STORED_ITEM_REUSE_KEYS is NOT defined the key is retired,
     * any attempt to reuse the key will result in an error being
     * thrown.
     *
     * @param key Key which will be unregistered.
     */

    static void unregister(key_t& key)
    {
#ifndef STORED_ITEM_NO_MUTEX
      ThreadMutexGuard guard(mutex());
#endif
      assertTrueMsg(isKeyInternal(key), "Invalid key " << key << " provided.");
      delete handleRemoval(key);
      KeySource<key_t>::unregister(key);
      key = KeySource<key_t>::unassigned();
    }

    /**
     * @brief Obtain the item from the key.
     * @param key The key_t valued encoding of the item
     * @return a reference to the original item held in the item store.
     * @see s_itemStore
     */
    static item_t& getItem(key_t key)
    {
#ifndef STORED_ITEM_NO_MUTEX
      ThreadMutexGuard guard(mutex());
#endif
      const itemIterator_t& it = itemStore().find(key);
      check_error(it != itemStore().end());
      return *it->second;
    }

    /**
     * @brief Insert the given item with the given key.
     * @param item (Pointer to) the item.
     * @param key The desired key for the item.
     * @note Used by LabelStr initialization for UNKNOWN_STR
     */

    static void insertItemAtKey(item_t* item, key_t key)
    {
#ifndef STORED_ITEM_NO_MUTEX
      ThreadMutexGuard guard(mutex());
#endif
      handleInsertion(key, item);
    }

  private:

    /**
     * @brief The key value used as a proxy for the original item.
     * @note The only instance data.
     * @see handleInsertion.
     */
    key_t m_key;

    /**
     * @brief Check whether the given key is valid.
     * @param key The putative key.
     * @return True if valid, false otherwise.
     * @note In a multi-threaded environment, must be called with mutex locked.
     */
    inline static bool isKeyInternal(key_t key)
    {
      return itemStore().find(key) != itemStore().end();
    }

    /**
     * @brief Constructs an entry in itemStore to allow lookup
     * of items from a key.
     *
     * @param key The key_t value encoding for the given item
     * @param item the item for which the key has been encoded.
     * @note In a multi-threaded environment, must be called with mutex locked.
     */
    static void handleInsertion(const key_t key, item_t* item)
    {
      itemStore().insert(itemPair_t(key, item));
      keyStore().insert(keyPair_t(item, key));
    }

    /**
     * @brief Removes the entry in itemStore associated with the
     * given key.
     *
     * @param key The key_t value encoding for the given item.
     * @return The item associated with the key.
     * @note In a multi-threaded environment, must be called with mutex locked.
     */

    static item_t* handleRemoval(key_t key)
    {
      itemIterator_t it = itemStore().find(key);
      check_error(it != itemStore().end());
      item_t* item = *it->second;
      itemStore().erase(it);
      keyStore().erase(item);
      return item;
    }

    /**
     * @brief Map keys to keys for key retrieval - i.e. getKey().
     */
    static keyStore_t& keyStore()
    {
      static keyStore_t* sl_keyStore = NULL;
      if (sl_keyStore == NULL)
	sl_keyStore = new keyStore_t();
      return *sl_keyStore;
    }

    /**
     * @brief Map keys to items for item retrieval - i.e. getItem(). 
     */
    static itemStore_t& itemStore()
    {
      static itemStore_t* sl_itemStore = NULL;
      if (sl_itemStore == NULL)
	{
	  sl_itemStore = new itemStore_t();
	}
      return *sl_itemStore;
    }


#ifndef STORED_ITEM_NO_MUTEX
    /**
     * @brief Get the mutex for this store.
     */
    static ThreadMutex& mutex()
    {
      static ThreadMutex sl_mutex;
      return sl_mutex;
    }
#endif

  };
}
#endif // STORED_ITEM__DINKUM_HH
