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

#include "StoredArray.hh"
#include "LabelStr.hh"
#include "Debug.hh"
#include "Error.hh"

namespace PLEXIL
{
  /**
   * @brief Default constructor.
   */
         
  StoredArray::StoredArray()
    : StoredItem<double, ArrayStorage>()
  {
  }
         
  /** 
   * @brief Copy constructor.
   *
   * @param other The existing StoredArray instance.
   */
  
  StoredArray::StoredArray(const StoredArray& other)
    : StoredItem<double, ArrayStorage>(other)
  {
  }

  /** 
   * @brief Construct a new array of a given size and with all
   * elements initialized to the provided value.
   *
   * @param size The number of elements that this array will
   * contain.
   *
   * @param initValue The value the elements in the array will be
   * initialized to.
   */
   
  StoredArray::StoredArray(size_t size, const double& initValue) :
    StoredItem<double, ArrayStorage>(new ArrayStorage(size, initValue),
				     false)
  {
  }

  /** 
   * @brief Construct a new array of a given size and initial values.
   *
   * @param size The maximum number of elements that this array
   * will contain.
   *
   * @param initValues The values the elements in the array will be
   * initialized to.
   */
   
  StoredArray::StoredArray(size_t size, const std::vector<double>& initValues) 
    : StoredItem<double, ArrayStorage>(new ArrayStorage(size, 0), false)
  {
    assertTrueMsg(initValues.size() <= size,
		  "StoredArray constructor: initial vector is larger than specified size");
    for (unsigned i = 0; i < initValues.size(); ++i)
      getArray()[i] = initValues[i];
  }

  /** 
   * @brief Construct a new array directly from a vector of initial values.
   *
   * @param initValues The values the elements in the array will be
   * initialized to.
   */
         
  StoredArray::StoredArray(const std::vector<double>& initValues)
    : StoredItem<double, ArrayStorage>(new ArrayStorage(initValues.size(), initValues), false)
  {
  }

  /** 
   * @brief Construct an array given a key from an existing array.
   *
   * @param key The key of the already existing array.
   */
   
  StoredArray::StoredArray(const double key)
    : StoredItem<double, ArrayStorage>(key)
  {
  }

  /** 
   * @brief Get the actual array.
   *
   * @return A reference to the array.
   */
  std::vector<double>& StoredArray::getArray()
  {
    return this->getItem().array;
  }

  /** 
   * @brief Get the actual array.
   *
   * @return A reference to the array.
   */
  const std::vector<double>& StoredArray::getConstArray() const
  {
    return this->getItem().array;
  }
         
  /**
   * @brief Return the size of this array.
   *
   * @return The size of this array.
   */
         
  size_t StoredArray::size() const
  {
    return this->getItem().size;
  }

  /**
   * @brief Operator for accessing elements in this array.
   *
   * @return A reference to the specifed array element.
   */
   
  double& StoredArray::operator[] (size_t index)
  {
    assertTrueMsg(index < size(), 
		  "Array index value " << index << 
		  " is equal to or larger than size " << size());
    return getArray()[index];
  }
         
  /**
   * @brief Return key which can be used to access this array at a
   * later time.
   *
   * @brief Access key of this array.
   */
         
  const double StoredArray::getKey() const
  {
    return StoredItem<double, ArrayStorage>::getKey();
  }

  /**
   * @brief Return true if key value is valid.
   *
   * @param key candidate key
   * @return true if valid key
   */

  bool StoredArray::isKey(double key)
  {
    return StoredItem<double, ArrayStorage>::isKey(key);
  }

  /**
   * @brief Return const value of array elment.
   *
   * @param index Index of array element.
   */
   
  const double& StoredArray::at(const size_t index) const
  {
    assertTrueMsg(index < size(), 
		  "Array index value " << index << 
		  " is equal to or larger than size " << size());
    return getConstArray().at(index);
  }


  /**
   * @brief Free memory for this stored array.
   *
   * Once freed the key value is set to an unassignedKey value.
   * If STORED_ITEM_REUSE_KEYS is defined at compile time the key
   * will be stored and potentially reissued in the future.  If
   * STORED_ITEM_REUSE_KEYS is NOT defined the key is retired,
   * any attempt to reuse the key will result in an error being
   * thrown.
   */

  void StoredArray::unregister()
  {
    StoredItem<double, ArrayStorage>::unregister();
  }

  /**
   * @brief Generate a printed representation for this stored array.
   */
  
  std::string StoredArray::toString() const
  {
    // *** KLUDGE ALERT ***
    // This is a local copy of the class constant Expression::UNKNOWN()
    static double MY_UNKNOWN = (std::numeric_limits<double>::has_infinity ?
                                std::numeric_limits<double>::infinity() :
                                std::numeric_limits<double>::max());
    std::stringstream retval;

    retval << "Array: [";
    const std::vector<double>& theVector = getConstArray();
    for (unsigned i = 0; i < theVector.size(); ++i)
      {
        const double value = theVector[i];
        if (i != 0)
          retval << ", ";
        if (value == MY_UNKNOWN)
          retval << "<unknown>";
        else if (LabelStr::isString(value))
          retval << '\"' << LabelStr(value).toString() << '\"';
        else
          retval << value;
      }
    retval << ']';
    return retval.str();
  }

}
