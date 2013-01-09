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

#include "StoredArray.hh"
#include "LabelStr.hh"
#include "Error.hh"
#include "Utils.hh" // for UNKNOWN()

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
   
  StoredArray::StoredArray(size_t size, const double& initValue) :
    StoredArray_item_t(StoredArray_value_t(size, initValue))
  {
  }

  /** 
   * @brief Construct a new array of a given size and initial values.
   * @param size The maximum number of elements that this array will contain.
   * @param initValues Initial values for the elements in the array.
   */
   
  StoredArray::StoredArray(size_t size, const std::vector<double>& initValues) 
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
         
  StoredArray::StoredArray(const std::vector<double>& initValues)
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
   * @brief Return the size of this array.
   * @return The size of this array.
   */
  size_t StoredArray::size() const
  {
    return getItem().size();
  }

  /**
   * @brief Operator for accessing elements in this array.
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
   * @brief Return const value of array elment.
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
   * @brief Generate a printed representation for this stored array.
   */
  std::string StoredArray::toString() const
  {
    std::ostringstream retval;

    retval << "Array: [";
    const StoredArray_value_t& theVector = getConstArray();
    for (size_t i = 0; i < theVector.size(); ++i) {
      const double value = theVector[i];
      if (i != 0)
        retval << ", ";
      if (value == UNKNOWN())
        retval << "<unknown>";
      else if (LabelStr::isString(value))
        retval << '\"' << LabelStr::toString(value) << '\"';
      else
        retval << value;
    }
    retval << ']';
    return retval.str();
  }

}
