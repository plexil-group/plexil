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

#ifndef PLEXIL_CACHED_VALUE_HH
#define PLEXIL_CACHED_VALUE_HH

#include "ArrayFwd.hh"
#include "ValueType.hh"

namespace PLEXIL
{
  class Value;

  /**
   * Pure virtual base class for storing state cache values
   */
  class CachedValue
  {
  public:
    CachedValue() : m_timestamp(0) {}
    CachedValue(CachedValue const &) : m_timestamp(0) {}
    virtual ~CachedValue() {}

    unsigned int getTimestamp() const
    {
      return m_timestamp;
    }

    // Delegated to derived classes.
    virtual ValueType const valueType() const = 0;
    virtual bool isKnown() const = 0;
    virtual CachedValue &operator=(CachedValue const &) = 0;
    virtual CachedValue *clone() const = 0;
    virtual bool operator==(CachedValue const &) const = 0;

    /**
     * @brief Retrieve the cached value.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown or invalid.
     * @note The expression value is not copied if the return value is false.
     * @note Derived classes should implement only the appropriate methods.
     */

    virtual bool getValue(bool &) const = 0;        // Boolean
    virtual bool getValue(double &) const = 0;      // Real
    virtual bool getValue(uint16_t &) const = 0;    // not implemented
    virtual bool getValue(int32_t &) const = 0;     // Integer
    virtual bool getValue(std::string &) const = 0; // String

    /**
     * @brief Retrieve a pointer to the (const) cached value.
     * @param ptr Reference to the pointer variable to receive the result.
     * @note The pointer is not copied if the return value is false.
     * @note Derived classes should implement only the appropriate method.
     * @note Default methods return an error in every case.
     */
    virtual bool getValuePointer(std::string const *&ptr) const = 0;
    virtual bool getValuePointer(Array const *&ptr) const = 0; // generic
    virtual bool getValuePointer(BooleanArray const *&ptr) const = 0; // specific
    virtual bool getValuePointer(IntegerArray const *&ptr) const = 0; //
    virtual bool getValuePointer(RealArray const *&ptr) const = 0;    //
    virtual bool getValuePointer(StringArray const *&ptr) const = 0;  //

    virtual Value toValue() const = 0;

    /**
     * @brief Set the state to unknown.
     * @param timestamp Sequence number.
     * @return True if changed, false otherwise.
     */
    virtual bool setUnknown(unsigned int timestamp) = 0;

    /**
     * @brief Update the cache entry with the given new value.
     * @param timestamp Sequence number.
     * @param val The new value.
     * @return True if changed, false otherwise.
     * @note The caller is responsible for deleting the object pointed to upon return.
     */
    virtual bool update(unsigned int timestamp, bool const &val) = 0;
    virtual bool update(unsigned int timestamp, int32_t const &val) = 0;
    virtual bool update(unsigned int timestamp, double const &val) = 0;
    virtual bool update(unsigned int timestamp, std::string const &val) = 0;
    virtual bool updatePtr(unsigned int timestamp, std::string const *valPtr) = 0;
    virtual bool updatePtr(unsigned int timestamp, BooleanArray const *valPtr) = 0;
    virtual bool updatePtr(unsigned int timestamp, IntegerArray const *valPtr) = 0;
    virtual bool updatePtr(unsigned int timestamp, RealArray const *valPtr) = 0;
    virtual bool updatePtr(unsigned int timestamp, StringArray const *valPtr) = 0;

    // For convenience of TestExternalInterface, others
    virtual bool update(unsigned int timestamp, Value const &val) = 0;

  protected:
    unsigned int m_timestamp;
  };

  // Factory function
  // See CachedValueImpl.cc
  extern CachedValue *CachedValueFactory(ValueType vtype);

  extern CachedValue *cloneCachedValue(CachedValue const * orig);

}

#endif // PLEXIL_CACHED_VALUE_HH
