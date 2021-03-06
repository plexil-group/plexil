/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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
  // @class GetValueImpl
  template <typename T>
  class GetValueImpl : virtual public Expression
  {
  protected:
    GetValueImpl() = default;

  private:
    // Not implemented
    GetValueImpl(GetValueImpl const &) = delete;
    GetValueImpl(GetValueImpl &&) = delete;

  public:
    virtual ~GetValueImpl() = default;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    virtual ValueType valueType() const override;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     * @note May be overridden by derived classes.
     */
    virtual bool isKnown() const override;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    virtual Value toValue() const override;

    /**
     * @brief Retrieve the value of this object in its native type.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValue(T &result) const override = 0;

    virtual void printValue(std::ostream &s) const override;

  };

  // Specialization for Integer
  template <>
  class GetValueImpl<Integer> : virtual public Expression
  {
  protected:
    GetValueImpl() = default;

  private:
    // Not implemented
    GetValueImpl(GetValueImpl const &) = delete;
    GetValueImpl(GetValueImpl &&) = delete;

  public:
    virtual ~GetValueImpl() = default;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    virtual ValueType valueType() const override;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     * @note May be overridden by derived classes.
     */
    virtual bool isKnown() const override;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    virtual Value toValue() const override;

    /**
     * @brief Retrieve the value of this object in its native type.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValue(Integer &result) const override = 0;

    /**
     * @brief Retrieve the value of this object as a Real.
     * @param Reference to Real variable.
     * @return True if known, false if unknown.
     * @note Conversion method.
     */
    virtual bool getValue(Real &result) const override;

    virtual void printValue(std::ostream &s) const override;
  };

  // Specialization for string
  template <>
  class GetValueImpl<String> : virtual public Expression
  {
  protected:
    GetValueImpl() = default;

  private:
    // Not implemented
    GetValueImpl(GetValueImpl const &) = delete;
    GetValueImpl(GetValueImpl &&) = delete;

  public:
    virtual ~GetValueImpl() = default;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    virtual ValueType valueType() const override;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     * @note May be overridden by derived classes.
     */
    virtual bool isKnown() const override;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    virtual Value toValue() const override;

    /**
     * @brief Retrieve the value of this object in its native type.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValue(String &result) const override = 0;

    /**
     * @brief Retrieve the value of this object as a pointer to const.
     * @param ptr Reference to the pointer variable.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointer(String const *&) const override = 0;

    virtual void printValue(std::ostream &s) const override;
  };

  // Specialization for array types
  template <typename T>
  class GetValueImpl<ArrayImpl<T> > : virtual public Expression
  {
  protected:
    GetValueImpl() = default;

  private:
    // Not implemented
    GetValueImpl(GetValueImpl const &) = delete;
    GetValueImpl(GetValueImpl &&) = delete;

  public:
    virtual ~GetValueImpl() = default;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    virtual ValueType valueType() const override;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     * @note May be overridden by derived classes.
     */
    virtual bool isKnown() const override;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    virtual Value toValue() const override;

    /**
     * @brief Retrieve the value of this object as a pointer to const.
     * @param ptr Reference to the pointer variable.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointer(ArrayImpl<T> const *&) const override = 0;
    virtual bool getValuePointer(Array const *& ptr) const override;

    virtual void printValue(std::ostream &s) const override;
  };

}

#endif // PLEXIL_GET_VALUE_IMPL_HH
