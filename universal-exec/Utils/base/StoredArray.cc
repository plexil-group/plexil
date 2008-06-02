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
#include "Debug.hh"

namespace PLEXIL
{
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
   
  StoredArray::StoredArray(size_t size, double initValue) :
    StoredItem<double, std::vector<double> >(new std::vector<double>(size, initValue), 
                                             false),
    m_size(size)
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
   
  StoredArray::StoredArray(size_t size, std::vector<double>& initValues) :
    StoredItem<double, std::vector<double> >(new std::vector<double>(size, 0), 
                                             false),
    m_size(size)
  {
    for (unsigned i = 0; i < initValues.size(); ++i)
      (*this)[i] = initValues[i];
  }

  /** 
   * @brief Construct an array given a key from an existing array.
   *
   * @param key The key of the already existing array.
   */
   
  StoredArray::StoredArray(double key) :
    StoredItem<double, std::vector<double> >(key),
    m_size(getItem().size())
  {
  }
  /**
   * @brief Operator for accessing elements in this array.
   *
   * @return A reference to the specifed array element.
   */
   
  double& StoredArray::operator[] (size_t index)
  {
    checkError(index < m_size, "Array index value " << index << 
               " out of bounds of size " << m_size);
    return getItem()[index];
  }

  /**
   * @brief Return const value of array elment.
   *
   * @param index Index of array element.
   */
   
  const double& StoredArray::at(const size_t index) const
  {
    checkError(index < m_size, "Array index value " << index << 
               " out of bounds of size " << m_size);
    return getItem().at(index);
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
    std::vector<double>* theArray = &(getItem());
    StoredItem<double, std::vector<double> >::unregister();
    delete theArray;
  }

}
