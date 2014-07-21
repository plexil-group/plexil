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

#ifndef PLEXIL_CACHED_VALUE_IMPL_HH
#define PLEXIL_CACHED_VALUE_IMPL_HH

#include "CachedValue.hh"

namespace PLEXIL
{

  // Placeholder object
  class VoidCachedValue : public CachedValue
  {
  public:
    VoidCachedValue();
    ~VoidCachedValue();


    ValueType const valueType() const;
    bool isKnown() const;
    CachedValue &operator=(CachedValue const &other);
    CachedValue *clone() const;
    bool operator==(CachedValue const &) const;

    /**
     * @brief Retrieve the cached value.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown or invalid.
     * @note The expression value is not copied if the return value is false.
     * @note Derived classes should implement only the appropriate methods.
     */

    bool getValue(bool &) const;        // Boolean
    bool getValue(double &) const;      // Real
    bool getValue(uint16_t &) const;    // not implemented
    bool getValue(int32_t &) const;     // Integer
    bool getValue(std::string &) const; // String

    /**
     * @brief Retrieve a pointer to the (const) cached value.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown or invalid.
     * @note The pointer is not copied if the return value is false.
     * @note Derived classes should implement only the appropriate method.
     * @note Default methods return an error in every case.
     */
    bool getValuePointer(std::string const *&ptr) const;
    bool getValuePointer(Array const *&ptr) const; // generic
    bool getValuePointer(BooleanArray const *&ptr) const; // specific
    bool getValuePointer(IntegerArray const *&ptr) const; //
    bool getValuePointer(RealArray const *&ptr) const;    //
    bool getValuePointer(StringArray const *&ptr) const;  //

    Value toValue() const;

    /**
     * @brief Set the state to unknown.

     */
    bool setUnknown(unsigned int timestamp);

    /**
     * @brief Update the cache entry with the given new value.
     * @param val The new value.
     * @return True if correct type for lookup, false otherwise.
     * @note Notifies all lookups of the new value.
     * @note The caller is responsible for deleting the object pointed to upon return.
     */
    bool update(unsigned int timestamp, bool const &val);
    bool update(unsigned int timestamp, int32_t const &val);
    bool update(unsigned int timestamp, double const &val);
    bool update(unsigned int timestamp, std::string const &val);
    bool updatePtr(unsigned int timestamp, std::string const *valPtr);
    bool updatePtr(unsigned int timestamp, BooleanArray const *valPtr);
    bool updatePtr(unsigned int timestamp, IntegerArray const *valPtr);
    bool updatePtr(unsigned int timestamp, RealArray const *valPtr);
    bool updatePtr(unsigned int timestamp, StringArray const *valPtr);

    // For convenience of TestExternalInterface, others
    bool update(unsigned int timestamp, Value const &val);
  };
  
  // Another invocation of CRTP.
  // This class should never be seen in public.
  template <class IMPL>
  class CachedValueShim : public CachedValue
  {
  public:
    CachedValueShim() : CachedValue() {}
    ~CachedValueShim() {}

    inline CachedValue *clone() const
    {
      return static_cast<IMPL const *>(this)->cloneImpl();
    }

    inline bool update(unsigned int timestamp, bool const &val)
    {
      return static_cast<IMPL *>(this)->updateImpl(timestamp, val);
    }

    inline bool update(unsigned int timestamp, int32_t const &val)
    {
      return static_cast<IMPL *>(this)->updateImpl(timestamp, val);
    }

    inline bool update(unsigned int timestamp, double const &val)
    {
      return static_cast<IMPL *>(this)->updateImpl(timestamp, val);
    }

    inline bool update(unsigned int timestamp, std::string const &val)
    {
      return static_cast<IMPL *>(this)->updateImpl(timestamp, val);
    }

    inline bool updatePtr(unsigned int timestamp, std::string const *valPtr)
    {
      return static_cast<IMPL *>(this)->updatePtrImpl(timestamp, valPtr);
    }

    inline bool updatePtr(unsigned int timestamp, BooleanArray const *valPtr)
    {
      return static_cast<IMPL *>(this)->updatePtrImpl(timestamp, valPtr);
    }

    inline bool updatePtr(unsigned int timestamp, IntegerArray const *valPtr)
    {
      return static_cast<IMPL *>(this)->updatePtrImpl(timestamp, valPtr);
    }

    inline bool updatePtr(unsigned int timestamp, RealArray const *valPtr)
    {
      return static_cast<IMPL *>(this)->updatePtrImpl(timestamp, valPtr);
    }

    inline bool updatePtr(unsigned int timestamp, StringArray const *valPtr)
    {
      return static_cast<IMPL *>(this)->updatePtrImpl(timestamp, valPtr);
    }

    inline bool update(unsigned int timestamp, Value const &val)
    {
      return static_cast<IMPL *>(this)->updateImpl(timestamp, val);
    }

    /**
     * @brief Retrieve the cached value in a particular type.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     */
    bool getValue(bool &result) const
    {
      return static_cast<const IMPL *>(this)->getValueImpl(result);
    }

    bool getValue(uint16_t &result) const
    {
      return static_cast<const IMPL *>(this)->getValueImpl(result);
    }

    bool getValue(int32_t &result) const
    {
      return static_cast<const IMPL *>(this)->getValueImpl(result);
    }

    bool getValue(double &result) const
    {
      return static_cast<const IMPL *>(this)->getValueImpl(result);
    }

    bool getValue(std::string &result) const
    {
      return static_cast<const IMPL *>(this)->getValueImpl(result);
    }

    /**
     * @brief Retrieve a pointer to the (const) cached value.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */
    bool getValuePointer(std::string const *&ptr) const
    {
      return static_cast<const IMPL *>(this)->getValuePointerImpl(ptr);
    }

    bool getValuePointer(Array const *&ptr) const
    {
      return static_cast<const IMPL *>(this)->getValuePointerImpl(ptr);
    }

    bool getValuePointer(BooleanArray const *&ptr) const
    {
      return static_cast<const IMPL *>(this)->getValuePointerImpl(ptr);
    }

    bool getValuePointer(IntegerArray const *&ptr) const
    {
      return static_cast<const IMPL *>(this)->getValuePointerImpl(ptr);
    }

    bool getValuePointer(RealArray const *&ptr) const
    {
      return static_cast<const IMPL *>(this)->getValuePointerImpl(ptr);
    }

    bool getValuePointer(StringArray const *&ptr) const
    {
      return static_cast<const IMPL *>(this)->getValuePointerImpl(ptr);
    }
    
  };

  // Scalar types
  template <typename T>
  class CachedValueImpl : public CachedValueShim<CachedValueImpl<T> >
  {
  public:
    CachedValueImpl();
    CachedValueImpl(CachedValueImpl<T> const &);

    ~CachedValueImpl();

    CachedValue &operator=(CachedValue const &);
    CachedValueImpl<T> &operator=(CachedValueImpl<T> const &);

    ValueType const valueType() const;
    bool isKnown() const;

    CachedValue *cloneImpl() const;

    bool operator==(CachedValue const &) const;

    bool getValueImpl(T &result) const;

    // Type conversion or invalid type
    template <typename U>
    bool getValueImpl(U &result) const;

    // Type conversion or invalid type
    template <typename U>
    bool getValuePointerImpl(U const *&ptr) const;

    Value toValue() const;

    // API to external interface
    bool setUnknown(unsigned int timestamp);

    virtual bool updateImpl(unsigned int timestamp, T const &val);

    // Type conversion or invalid type
    template <typename U>
    bool updateImpl(unsigned int timestamp, U const &val);

    bool updateImpl(unsigned int timestamp, Value const &val);

    // Type error
    template <typename U>
    bool updatePtrImpl(unsigned int timestamp, U const *valPtr);

  private:
    T m_value;
    bool m_known;
  };

  // String is special
  template <>
  class CachedValueImpl<std::string> : public CachedValueShim<CachedValueImpl<std::string> >
  {
  public:
    CachedValueImpl();
    CachedValueImpl(CachedValueImpl<std::string> const &);

    ~CachedValueImpl();

    CachedValue &operator=(CachedValue const &);
    CachedValueImpl<std::string> &operator=(CachedValueImpl<std::string> const &);

    ValueType const valueType() const;
    bool isKnown() const;

    CachedValue *cloneImpl() const;

    bool operator==(CachedValue const &) const;

    bool getValueImpl(std::string &result) const;

    // Type error
    template <typename U>
    bool getValueImpl(U &result) const;

    bool getValuePointerImpl(std::string const *&ptr) const;

    // Type error
    template <typename U>
    bool getValuePointerImpl(U const *&ptr) const;

    Value toValue() const;

    // API to external interface
    bool setUnknown(unsigned int timestamp);

    virtual bool updateImpl(unsigned int timestamp, std::string const &val);

    // Type error
    template <typename U>
    bool updateImpl(unsigned int timestamp, U const &val);

    bool updateImpl(unsigned int timestamp, Value const &val);

    bool updatePtrImpl(unsigned int timestamp, std::string const *valPtr);

    // Type error
    template <typename U>
    bool updatePtrImpl(unsigned int timestamp, U const *valPtr);

  private:
    std::string m_value;
    bool m_known;
  };

  // Specialized for arrays
  template <typename T>
  class CachedValueImpl<ArrayImpl<T> >
    : public CachedValueShim<CachedValueImpl<ArrayImpl<T> > >
  {
  public:
    CachedValueImpl();
    CachedValueImpl(CachedValueImpl<ArrayImpl<T> > const &);

    ~CachedValueImpl();

    CachedValue &operator=(CachedValue const &);
    CachedValueImpl<ArrayImpl<T> > &operator=(CachedValueImpl<ArrayImpl<T> > const &);

    ValueType const valueType() const;
    bool isKnown() const;

    CachedValue *cloneImpl() const;

    bool operator==(CachedValue const &) const;

    // Type conversion or invalid type
    template <typename U>
    bool getValueImpl(U &result) const;

    bool getValuePointerImpl(ArrayImpl<T> const *&ptr) const;

    // Type conversion or invalid type
    template <typename U>
    bool getValuePointerImpl(U const *&ptr) const;

    bool getValuePointerImpl(Array const *&ptr) const;

    Value toValue() const;

    // API to external interface
    bool setUnknown(unsigned int timestamp);

    template <typename U>
    bool updateImpl(unsigned int timestamp, U const &val);

    bool updateImpl(unsigned int timestamp, Value const &val);

    bool updatePtrImpl(unsigned int timestamp, ArrayImpl<T> const *valPtr);

    // Type conversion or invalid type
    template <typename U>
    bool updatePtrImpl(unsigned int timestamp, U const *valPtr);

  private:
    ArrayImpl<T> m_value;
    bool m_known;
  };

} // namespace PLEXIL

#endif // PLEXIL_CACHED_VALUE_IMPL_HH
