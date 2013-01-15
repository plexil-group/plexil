/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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

#include "Error.hh"
#include "LabelStr.hh"

namespace PLEXIL
{
  /**
   * @brief Default constructor.
   */
         
  StoredArray::StoredArray()
    : StoredArray_item_t()
  {
  }
         
  /** 
   * @brief Copy constructor.
   *
   * @param other The existing StoredArray instance.
   */
  
  StoredArray::StoredArray(const StoredArray& other)
    : StoredArray_item_t(other)
  {
  }

  /** 
   * @brief Construct a new array of a given size and with all
   * elements initialized to the provided value.
   * @param size The number of elements that this array will contain.
   * @param initValue Initial value for the elements in the array.
   */
   
  StoredArray::StoredArray(size_t size, const Value& initValue) :
    StoredArray_item_t(StoredArray_value_t(size, initValue))
  {
  }

  /** 
   * @brief Construct a new array of a given size and initial values.
   * @param size The maximum number of elements that this array will contain.
   * @param initValues Initial values for the elements in the array.
   */
   
  StoredArray::StoredArray(size_t size, const StoredArray_value_t& initValues) 
    : StoredArray_item_t(StoredArray_value_t(size, UNKNOWN()))
  {
    assertTrueMsg(initValues.size() <= size,
                  "StoredArray constructor: initial vector is larger than specified size");
    StoredArray_value_t& array = getArray();
    for (size_t i = 0; i < initValues.size(); ++i)
      array[i] = initValues[i];
  }

  /** 
   * @brief Construct a new array directly from a vector of initial values.
   * @param initValues The initial vector.
   */
         
  StoredArray::StoredArray(const StoredArray_value_t& initValues)
    : StoredArray_item_t(initValues)
  {
  }

  /** 
   * @brief Construct an array given a key from an existing array.
   * @param key The key of the already existing array.
   */
   
  StoredArray::StoredArray(const double key)
    : StoredArray_item_t(key)
  {
  }

  /** 
   * @brief Constructor from a Value instance.
   * @param value The value.
   * @note CALLER MUST ENSURE THAT THE VALUE IS AN ARRAY!
   */
  StoredArray::StoredArray(const Value& value)
    : StoredArray_item_t(value.getRawValue())
  {
  }

  /*
   * @brief Assignment operator from StoredArray.
   * @param other The other StoredArray.
   * @return Reference to this.
   */
  StoredArray& StoredArray::operator=(const StoredArray& other)
  {
    StoredArray_item_t::operator=(other);
    return *this;
  }

  /*
   * @brief Assignment operator from key to another StoredArray.
   * @param key The key.
   * @return Reference to this.
   * @note Can assert if key is invalid.
   */
  StoredArray& StoredArray::operator=(const StoredArray_key_t& key)
  {
    StoredArray_item_t::operator=(key);
    return *this;
  }

  /*
   * @brief Assignment operator from Value.
   * @param value The value.
   * @return Reference to this.
   * @note CALLER MUST ENSURE THAT THE VALUE IS AN ARRAY!
   */
  StoredArray& StoredArray::operator=(const Value& value)
  {
    StoredArray_item_t::operator=(value.getRawValue());
    return *this;
  }

  /**
   * @brief Return the size of this array.
   * @return The size of this array.
   */
  size_t StoredArray::size() const
  {
    return getItem().size();
  }

  /**
   * @brief Return const value of array elment.
   * @param index Index of array element.
   */
  const Value& StoredArray::at(const size_t index) const
  {
    assertTrueMsg(index < size(), 
                  "Array index value " << index << 
                  " is equal to or larger than size " << size());
    return getConstArray().at(index);
  }

  /**
   * @brief Operator for accessing elements in this array.
   * @return A reference to the specifed array element.
   */
  Value& StoredArray::operator[] (size_t index)
  {
    assertTrueMsg(index < size(), 
                  "Array index value " << index << 
                  " is equal to or larger than size " << size());
    return getArray()[index];
  }

  /**
   * @brief Generate a printed representation for this stored array.
   */
  std::string StoredArray::toString() const
  {
    std::ostringstream retval;

    retval << "Array: [";
    const StoredArray_value_t& theVector = getConstArray();
    for (size_t i = 0; i < theVector.size(); ++i) {
      const Value& value = theVector[i];
      if (i != 0)
        retval << ", ";
      if (value.isUnknown())
        retval << "<unknown>";
      else if (value.isString())
        retval << '\"' << value.getStringValue() << '\"';
      else
        retval << value.getDoubleValue();
    }
    retval << ']';
    return retval.str();
  }

}
