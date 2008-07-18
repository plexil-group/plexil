/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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
 * @author Robert Harris based on code by Conor McGann
 * @date 9 October 2007
 */

#include "StoredItem.hh"

namespace PLEXIL
{
   
   /**
    * @class StoredArray
    * @brief A StoredItem based implemention of an array.
    *
    * The StoredArray privides a minimal interface for accessing an array
    * for which a key can be obtained for later access to this array.
    */
   
   class StoredArray: private StoredItem<double, std::vector<double> >
   {
   public:
     /**
      * @brief Construct an empty array.
      */
         
     StoredArray()
     {
     }

     /** 
      * @brief Construct a new array of a given size and with all
      * elements initialized to the provided value.
      *
      * @param size The maximum number of elements that this array
      * will contain.
      *
      * @param initValue The value the elements in the array will be
      * initialized to.
      */
         
     StoredArray(size_t size, double initValue);

     /** 
      * @brief Construct a new array of a given size and initial values.
      *
      * @param size The maximum number of elements that this array
      * will contain.
      *
      * @param initValues The values the elements in the array will be
      * initialized to.
      */
         
     StoredArray(size_t size, const std::vector<double>& initValues);
         
     /** 
      * @brief Construct an array given a key from an existing array.
      *
      * @param key The key of the already existing array.
      */
         
     StoredArray(double key);

     /** 
      * @brief Get the actual array.
      *
      * @return A reference to the array.
      */
     inline std::vector<double>& getArray() const
     {
       return this->getItem();
     }

         
     /**
      * @brief Return the size of this array.
      *
      * @return The size of this array.
      */
         
     inline size_t size() const
     {
       return m_size;
     }
         
     /**
      * @brief Return key which can be used to access this array at a
      * later time.
      *
      * @brief Access key of this array.
      */
         
     inline const double getKey()
     {
       return StoredItem<double, std::vector<double> >::getKey();
     }

     /**
      * @brief Return true if key value is valid.
      *
      * @param key candidate key
      * @return true if valid key
      */

     inline static bool isItem(double key)
     {
       return StoredItem<double, std::vector<double> >::isItem(key);
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
      * @brief Free memory for for this stored array.
      *
      * Once freed the key value is set to an unassignedKey value.
      * If STORED_ITEM_REUSE_KEYS is defined at compile time the key
      * will be stored and potentially reissued in the future.  If
      * STORED_ITEM_REUSE_KEYS is NOT defined the key is retired,
      * any attempt to reuse the key will result in an error being
      * thrown.
      */
         
     void unregister();


     /**
      * @brief Generate a printed representation for this stored array.
      */
     std::string toString() const;

   private:
     size_t m_size;
   };
}
#endif
