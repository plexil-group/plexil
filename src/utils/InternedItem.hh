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

#ifndef INTERNED_ITEM_HH
#define INTERNED_ITEM_HH

#include "Error.hh"

namespace PLEXIL
{
  /**
   * @class InternedItem
   * @brief Allows arbitrarily large objects to be shared by a (non-pointer type) key.
   * @tparam key_t The key type. Presumed to be a fundamental numeric type.
   * @tparam item_t The item type; presumed to be a container class.
   * @tparam item_store_t The item store class, which maintains the key-item associations.
   * The item_store_t class must implement the following member functions:
   *
   * bool isItem(key_t key)
   * Returns true if the key is associated with an item in the store.
   *
   * item_t* getItem(key_t key) const
   * Returns a pointer to the requested item, or NULL if not found.
   *
   * key_t storeItem(const item_t& item)
   * Stores the item and returns its newly created key.
   *
   * bool newReference(key_t key)
   * Logs the creation of another InternedItem instance from an existing key.
   * Returns true if the key is valid, false otherwise.
   * CALLER MUST CHECK RETURN VALUE!
   *
   * void deleteReference(key_t key)
   * Logs the deletion of a InternedItem instance.
   * If the key is now unreferenced, the item store is allowed to delete the item
   * and mark the key invalid.
   */

   template <typename key_t,
             typename item_t,
             typename two_way_store_t>
   class InternedItem
   {
   public:

     /**
      * @brief Zero argument constructor.
      * @note Should only be used indirectly, e.g., via std::list.
      */
     InternedItem()
       : m_key(itemStore().getEmptyKey())
     {
     }

     /**
      * @brief Copy the item to permanent storage and generate a key for it.
      * @param item The item to copy.
      * @return The key for retrieving the item from the store.
      */
     InternedItem(const item_t& item)
       : m_key(itemStore().storeItem(item))
     {
     }

     /**
      * @brief Constructor from existing key.
      *
      * Each InternedItem gets encoded as a key such that any 2
      * instances of a InternedItem constructed from the same item
      * will have the same key and that the key preserves
      * lexicographic ordering.
      *
      * @param key the key value for a previously created InternedItem instance.
      * @see m_key, getItem()
      */
     explicit InternedItem(key_t key)
     : m_key(key)
     {
       if (!itemStore().newReference(m_key)) {
         assertTrue(ALWAYS_FAIL, "InternedItem constructor: Invalid key");
       }
     }

     /**
      * @brief Copy constructor.
      *
      * Only needs to copy the key since the item value can be
      * recovered from the shared repository.
      *
      * @param other The source InternedItem.
      */
     InternedItem(const InternedItem& other)
     : m_key(other.getKey())
     {
       if (!itemStore().newReference(m_key)) {
         assertTrue(ALWAYS_FAIL, "InternedItem copy constructor: Invalid key");
       }
     }

     /**
      * @brief Virtual destructor.
      */
     virtual ~InternedItem()
     {
       itemStore().deleteReference(m_key);
     }

     /**
      * @brief Assignment operator.
      * @param other The source InternedItem.
      * @return This InternedItem.
      */
     InternedItem& operator=(const InternedItem& other)
     {
       key_t newKey = other.getKey();
       if (m_key != newKey) {
         assertTrueMsg(itemStore().newReference(newKey),
                       "InternedItem::operator=: Key " << newKey << " is not valid");
         key_t oldKey = m_key;
         itemStore().deleteReference(oldKey);
         m_key = newKey;
       }
       return *this;
     }

     /**
      * @brief Assignment operator from item type.
      * @param newItem Replacement for current item.
      * @return This InternedItem.
      */
     InternedItem& operator=(const item_t& newItem)
     {
       item_t& current = getItem();
       if (current != newItem) {
         key_t oldKey = m_key;
         m_key = itemStore().storeItem(newItem);
         itemStore().deleteReference(oldKey);
       }
       return *this;
     }

     /**
      * @brief Assignment operator from key type.
      * @param newKey Replacement for current key.
      * @return This InternedItem.
      */
     InternedItem& operator=(const key_t& newKey)
     {
       if (m_key != newKey) {
         assertTrueMsg(itemStore().newReference(newKey),
                       "InternedItem::operator=: key " << newKey << " is not valid");
         key_t oldKey = m_key;
         m_key = newKey;
         itemStore().deleteReference(oldKey);
       }
       return *this;
     }

     /**
      * @brief Equality operator.
      * @param other Another InternedItem.
      * @return true if equal, false otherwise.
      * @note Equal contents imply equal keys.
      */
     bool operator==(const InternedItem& other) const
     {
       return m_key == other.getKey();
     }

     /**
      * @brief Inequality operator.
      * @param other Another InternedItem.
      * @return false if equal, true otherwise.
      */
     bool operator!=(const InternedItem& other) const
     {
       return m_key != other.getKey();
     }

     /**
      * @brief Return the canonical empty item.
      * @return A static empty item.
      */
     static const item_t& empty()
     {
       static item_t sl_empty;
       return sl_empty;
     }

     /**
      * @brief Cast operator to get the key for this object.
      * @return The key to this item.
      */
     operator key_t() const
     {
       return m_key;
     }

     /**
      * @brief Obtain the encoded key value for the item.
      * @return The key for accessing the store of items.
      */
     inline const key_t& getKey() const
     {
       return m_key;
     }

     /**
      * @brief Test if the given key_t valued key maps to an item.
      * @param key The candidate key.
      * @return true if the key is valid, false otherwise.
      */
     inline static bool isKey(key_t key)
     {
       return itemStore().isKey(key);
     }

     /**
      * @brief Test if the given item_t value maps to an item.
      * @param key The candidate item.
      * @return true if the item is valid, false otherwise.
      */
     inline static bool isItem(const item_t& item)
     {
       return itemStore().isItem(item);
     }

     /**
      * @brief Returns the cached item.
      * @return Reference to the cached item.
      */
     item_t& getItem() const
     {
       item_t* result = itemStore().getItem(m_key);
       assertTrue(result != NULL,
                  "InternedItem::getItem: key not found");
       return *result;
     }

     /**
      * @brief Returns the cached item for a given key.
      * @param key The key
      * @return Reference to the cached item.
      */
     static item_t& getItem(key_t key)
     {
       item_t* result = itemStore().getItem(key);
       assertTrue(result != NULL,
                  "InternedItem::getItem: key not found");
       return *result;
     }

     /**
      * @brief Get the number of stored items of this type.
      * @return The size of the store.
      */
     static size_t getSize()
     {
       return itemStore().size();
     }

   private:

     /**
      * @brief Access to the item store.
      */
     static two_way_store_t& itemStore()
     {
       static two_way_store_t sl_itemStore;
       return sl_itemStore;
     }

     /**
      * @brief The key value used as a proxy for the original item.
      * @note The only instance data.
      */
     key_t m_key;

   };

}

#endif // INTERNED_ITEM_HH
