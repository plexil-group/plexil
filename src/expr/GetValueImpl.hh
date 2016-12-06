/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_GET_VALUE_IMPL_HH
#define PLEXIL_GET_VALUE_IMPL_HH

#include "Expression.hh"

namespace PLEXIL
{
  //*
  // @class GetValueShim
  // @brief CRTP template adapter for strongly typed Expression classes

  template <class IMPL>
  class GetValueShim : public virtual Expression
  {
  protected:
    GetValueShim()
    {
    }
  
  private:
    // not implemented
    GetValueShim(GetValueShim const &);

  public:
    ~GetValueShim() {}

    // Local macro
#define DEFINE_GET_VALUE_METHOD_SHIM(_type_) \
  bool getValue(_type_ &result) const \
  { return static_cast<IMPL const *>(this)->getValueImpl(result); }

    DEFINE_GET_VALUE_METHOD_SHIM(Boolean)
    DEFINE_GET_VALUE_METHOD_SHIM(uint16_t)
    DEFINE_GET_VALUE_METHOD_SHIM(Integer)
    DEFINE_GET_VALUE_METHOD_SHIM(Real)
    DEFINE_GET_VALUE_METHOD_SHIM(String)

#undef DEFINE_GET_VALUE_METHOD_SHIM

    // Local macro
#define DEFINE_GET_VALUE_POINTER_METHOD_SHIM(_type_) \
  bool getValuePointer(_type_ const *&ptr) const \
  { return static_cast<IMPL const *>(this)->getValuePointerImpl(ptr); }

    DEFINE_GET_VALUE_POINTER_METHOD_SHIM(String)
    DEFINE_GET_VALUE_POINTER_METHOD_SHIM(Array)
    DEFINE_GET_VALUE_POINTER_METHOD_SHIM(BooleanArray)
    DEFINE_GET_VALUE_POINTER_METHOD_SHIM(IntegerArray)
    DEFINE_GET_VALUE_POINTER_METHOD_SHIM(RealArray)
    DEFINE_GET_VALUE_POINTER_METHOD_SHIM(StringArray)

#undef DEFINE_GET_VALUE_POINTER_METHOD_SHIM

  };

  //*
  // @class GetValueImpl
  // @brief CRTP template class
  template <typename T>
  class GetValueImpl : public GetValueShim<GetValueImpl<T> >
  {
  protected:
    GetValueImpl() {}

  private:
    // Not implemented
    GetValueImpl(GetValueImpl const &);

  public:
    ~GetValueImpl() {}

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    ValueType valueType() const;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     * @note May be overridden by derived classes.
     */
    bool isKnown() const;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    Value toValue() const;

    /**
     * @brief Retrieve the value of this object in its native type.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValueImpl(T &result) const = 0;

    // Conversion wrapper, error if particular conversion not supported
    template <typename U>
    bool getValueImpl(U &result) const;

    /**
     * @brief Retrieve the value of this object as a pointer to const.
     * @param ptr Reference to the pointer variable.
     * @return True if known, false if unknown.
     * @note These are errors for Boolean and numeric expressions.
     */

    // Error for scalar types
    bool getValuePointerImpl(T const *&) const;
    template <typename U>
    bool getValuePointerImpl(U const *&) const;

    void printValue(std::ostream &s) const;

  };

  // Specialization for string
  template <>
  class GetValueImpl<String> : public GetValueShim<GetValueImpl<String> >
  {
  protected:
    GetValueImpl() {}

  private:
    // Not implemented
    GetValueImpl(GetValueImpl const &);

  public:
    ~GetValueImpl() {}

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    ValueType valueType() const;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     * @note May be overridden by derived classes.
     */
    bool isKnown() const;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    Value toValue() const;

    /**
     * @brief Retrieve the value of this object in its native type.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValueImpl(String &result) const = 0;

    // Conversion wrapper, error if particular conversion not supported
    template <typename U>
    bool getValueImpl(U &result) const;

    /**
     * @brief Retrieve the value of this object as a pointer to const.
     * @param ptr Reference to the pointer variable.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointerImpl(String const *&) const = 0;

    // Type error
    template <typename U>
    bool getValuePointerImpl(U const *&) const;

    void printValue(std::ostream &s) const;
  };

  // Specialization for array types
  template <typename T>
  class GetValueImpl<ArrayImpl<T> > : public GetValueShim<GetValueImpl<ArrayImpl<T> > >
  {
  protected:
    GetValueImpl() {}

  private:
    // Not implemented
    GetValueImpl(GetValueImpl const &);

  public:
    ~GetValueImpl() {}

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    ValueType valueType() const;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     * @note May be overridden by derived classes.
     */
    bool isKnown() const;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    Value toValue() const;

    // Not implemented - no implicit copying allowed
    bool getValueImpl(ArrayImpl<T> &result) const;

    // Type mismatches
    template <typename U>
    bool getValueImpl(U &result) const;

    /**
     * @brief Retrieve the value of this object as a pointer to const.
     * @param ptr Reference to the pointer variable.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointerImpl(ArrayImpl<T> const *&) const = 0;
    bool getValuePointerImpl(Array const *& ptr) const;

    // Error for type mismatch
    template <typename U>
    bool getValuePointerImpl(U const *&) const;

    void printValue(std::ostream &s) const;

  };

}

#endif // PLEXIL_GET_VALUE_IMPL_HH
