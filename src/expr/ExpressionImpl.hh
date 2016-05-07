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

#ifndef PLEXIL_EXPRESSION_IMPL_HH
#define PLEXIL_EXPRESSION_IMPL_HH

#include "Expression.hh"

// Local macro

namespace PLEXIL
{

  /**
   * @class ExpressionShim
   * @brief Adapter (Design Patterns pp. 139-150) around templatized Expression derivatives,
   *        isolating the Expression API from the implementation.
   * @note This class should NEVER be explicitly mentioned outside this file.
   * @note If the template trickery confuses you, do a web search for "Curiously Recurring Template Pattern".
   */

  template <class IMPL>
  class ExpressionShim : public virtual Expression
  {
  public:

    /**
     * @brief Retrieve the value of this Expression in its native type.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     */

    // Local macro
#define DEFINE_GET_VALUE_METHOD(_type_) \
  bool getValue(_type_ &result) const \
  { return static_cast<IMPL const *>(this)->getValueImpl(result); }

    DEFINE_GET_VALUE_METHOD(Boolean)
    DEFINE_GET_VALUE_METHOD(Integer)
    DEFINE_GET_VALUE_METHOD(Real)
    DEFINE_GET_VALUE_METHOD(NodeState)
    DEFINE_GET_VALUE_METHOD(NodeOutcome)
    DEFINE_GET_VALUE_METHOD(FailureType)
    DEFINE_GET_VALUE_METHOD(CommandHandleValue)
    DEFINE_GET_VALUE_METHOD(String)

#undef DEFINE_GET_VALUE_METHOD
    
    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */


    // Local macro
#define DEFINE_GET_VALUE_POINTER_METHOD(_type_) \
  bool getValuePointer(_type_ const *&ptr) const \
  { return static_cast<IMPL const *>(this)->getValuePointerImpl(ptr); }

    DEFINE_GET_VALUE_POINTER_METHOD(String)
    DEFINE_GET_VALUE_POINTER_METHOD(Array)
    DEFINE_GET_VALUE_POINTER_METHOD(BooleanArray)
    DEFINE_GET_VALUE_POINTER_METHOD(IntegerArray)
    DEFINE_GET_VALUE_POINTER_METHOD(RealArray)
    DEFINE_GET_VALUE_POINTER_METHOD(StringArray)

#undef DEFINE_GET_VALUE_POINTER_METHOD

  };

  /**
   * @class ExpressionImpl
   * @brief A pure virtual interface mixin class, specializing the expression based on value type.
   * @note Templatized Expression implementations should derive from this template.
  */

  template <typename T>
  class ExpressionImpl : public ExpressionShim<ExpressionImpl<T> >
  {
  public:
    /**
     * @brief Query whether the expression's value is known.
     * @return True if known, false otherwise.
     * @note Default method uses getValueImpl().
     */
    virtual bool isKnown() const;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    const ValueType valueType() const;

	/**
	 * @brief Print the expression's value to the given stream.
	 * @param s The output stream.
	 */
    void printValue(std::ostream &) const; // may be overridden?

    /**
     * @brief Retrieve the value of this Expression in its native type.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValueImpl(T &result) const = 0;

    // Conversion wrapper, error if particular conversion not supported
    template <typename U>
    bool getValueImpl(U &result) const;

    /**
     * @brief Retrieve the value of this Expression as a pointer to const.
     * @param ptr Reference to the pointer variable.
     * @return True if known, false if unknown.
     * @note These are errors for Boolean and numeric expressions.
     */

    // Error for scalar types
    bool getValuePointerImpl(T const *&) const;
    template <typename U>
    bool getValuePointerImpl(U const *&) const;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    Value toValue() const;

  };

  // Specialization for string
  template <>
  class ExpressionImpl<String>
    : public ExpressionShim<ExpressionImpl<String> >
  {
  public:
    /**
     * @brief Query whether the expression's value is known.
     * @return True if known, false otherwise.
     * @note Default method uses getValuePointerImpl().
     */
    virtual bool isKnown() const;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    const ValueType valueType() const;

	/**
	 * @brief Print the expression's value to the given stream.
	 * @param s The output stream.
	 */
    void printValue(std::ostream &) const; // may be overridden?

    /**
     * @brief Retrieve the value of this Expression in its native type.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValueImpl(String &result) const = 0;

    // Conversion wrapper, error if particular conversion not supported
    template <typename U>
    bool getValueImpl(U &result) const;

    /**
     * @brief Retrieve the value of this Expression as a pointer to const.
     * @param ptr Reference to the pointer variable.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointerImpl(String const *& ptr) const = 0;

    // Error for wrong type call
    template <typename U>
    bool getValuePointerImpl(U const *&) const;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    Value toValue() const;

  };

  // Specialization for array types
  template <typename T>
  class ExpressionImpl<ArrayImpl<T> >
    : public ExpressionShim<ExpressionImpl<ArrayImpl<T> > >
  {
  public:
    /**
     * @brief Query whether the expression's value is known.
     * @return True if known, false otherwise.
     * @note Default method uses getValueImpl().
     */
    virtual bool isKnown() const;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    const ValueType valueType() const;

	/**
	 * @brief Print the expression's value to the given stream.
	 * @param s The output stream.
	 */
    void printValue(std::ostream &) const; // may be overridden?

    // Error for array types
    bool getValueImpl(ArrayImpl<T> &result) const;
    template <typename U>
    bool getValueImpl(U &result) const;

    /**
     * @brief Retrieve the value of this Expression as a pointer to const.
     * @param ptr Reference to the pointer variable.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointerImpl(ArrayImpl<T> const *& ptr) const = 0;
    bool getValuePointerImpl(Array const *& ptr) const;

    // Error for wrong type call
    template <typename U>
    bool getValuePointerImpl(U const *&) const;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    Value toValue() const;

  };
  
}

#endif // PLEXIL_EXPRESSION_IMPL_HH
