// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_GET_VALUE_IMPL_HH
#define PLEXIL_GET_VALUE_IMPL_HH

#include "Expression.hh"

namespace PLEXIL
{

  //! \class GetValueImpl
  //! \brief Stateless mixin class providing common behavior for Expression classes
  //!        specialized for a specific value type.
  //! \ingroup Expressions
  template <typename T>
  class GetValueImpl : virtual public Expression
  {
  public:

    //! \brief Return the value type of the expression.
    //! \return The value type.
    //! \note May be overridden by derived classes.
    virtual ValueType valueType() const;

    //! \brief Determine whether the value is known or unknown.
    //! \return True if known, false otherwise.
    //! \note May be overridden by derived classes.
    virtual bool isKnown() const;

    //! \brief Get the value of this expression as a Value instance.
    //! \return The Value instance.
    virtual Value toValue() const;

    //! \brief Retrieve the value of this object in its native type.
    //! \param result The appropriately typed place to put the result.
    //! \return True if known, false if unknown.
    virtual bool getValue(T &result) const = 0;

    //! \brief Print the value of this expression to a stream.
    //! \param s Reference to the stream.
    virtual void printValue(std::ostream &s) const;

  };

  //! \brief Specialization of GetValueImpl for Integer typed expressions.
  //! \ingroup Expressions
  template <>
  class GetValueImpl<Integer> : virtual public Expression
  {
  public:

    //! \brief Return the value type of the expression.
    //! \return The value type.
    //! \note May be overridden by derived classes.
    virtual ValueType valueType() const;

    //! \brief Determine whether the value is known or unknown.
    //! \return True if known, false otherwise.
    //! \note May be overridden by derived classes.
    virtual bool isKnown() const;

    //! \brief Get the value of this expression as a Value instance.
    //! \return The Value instance.
    virtual Value toValue() const;

    //! \brief Retrieve the value of this object in its native type.
    //! \param result Reference to the appropriately typed place to put the result.
    //! \return True if known, false if unknown.
    virtual bool getValue(Integer &result) const = 0;

    //! \brief Retrieve the value of this object as a Real.
    //! \param result Reference to Real variable.
    //! \return True if known, false if unknown.
    //! \note Conversion method.
    virtual bool getValue(Real &result) const;

    //! \brief Print the value of this expression to a stream.
    //! \param s The stream.
    virtual void printValue(std::ostream &s) const;
  };

  //! \brief Specialization of GetValueImpl for string valued expressions.
  //! \ingroup Expressions
  template <>
  class GetValueImpl<String> : virtual public Expression
  {
  public:

    //! \brief Return the value type of the expression.
    //! \return The value type.
    //! \note May be overridden by derived classes.
    virtual ValueType valueType() const;

    //! \brief Determine whether the value is known or unknown.
    //! \return True if known, false otherwise.
    //! \note May be overridden by derived classes.
    virtual bool isKnown() const;

    //! \brief Get the value of this expression as a Value instance.
    //! \return The Value instance.
    virtual Value toValue() const;

    //! \brief Retrieve the value of this object in its native type.
    //! \param result Reference to a String variable.
    //! \return True if known, false if unknown.
    virtual bool getValue(String &result) const = 0;

    //! \brief Retrieve the value of this object as a pointer to const.
    //! \param ptr Reference to the pointer variable.
    //! \return True if known, false if unknown.
    virtual bool getValuePointer(String const * &ptr) const = 0;

    //! \brief Print the value of this expression to a stream.
    //! \param s The stream.
    virtual void printValue(std::ostream &s) const;
  };

  //! \brief Specialization of GetValueImpl for array valued expressions.
  //! \ingroup Expressions
  template <typename T>
  class GetValueImpl<ArrayImpl<T> > : virtual public Expression
  {
  public:

    //! \brief Return the value type of the expression.
    //! \return The value type.
    //! \note May be overridden by derived classes.
    virtual ValueType valueType() const;

    //! \brief Determine whether the value is known or unknown.
    //! \return True if known, false otherwise.
    //! \note May be overridden by derived classes.
    virtual bool isKnown() const;

    //! \brief Get the value of this expression as a Value instance.
    //! \return The Value instance.
    virtual Value toValue() const;

    //! \brief Retrieve the value of this object as a pointer to const.
    //! \param ptr Reference to the pointer variable of the specific ArrayImpl specialization.
    //! \return True if known, false if unknown.
    virtual bool getValuePointer(ArrayImpl<T> const * &ptr) const = 0;

    //! \brief Retrieve the value of this object as a pointer to const.
    //! \param ptr Reference to the pointer variable of the Array virtual base class.
    //! \return True if known, false if unknown.
    virtual bool getValuePointer(Array const * & ptr) const;

    //! \brief Print the value of this expression to a stream.
    //! \param s The stream.
    virtual void printValue(std::ostream &s) const;
  };

}

#endif // PLEXIL_GET_VALUE_IMPL_HH
