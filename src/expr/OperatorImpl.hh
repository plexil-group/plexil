// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_OPERATOR_IMPL_HH
#define PLEXIL_OPERATOR_IMPL_HH

#include "Operator.hh"

#include "Value.hh"

namespace PLEXIL
{

  //! \brief Virtual templated class for operators whose return type is known and fixed.
  //!
  //! OperatorImpl defines operator() methods which delegate to the
  //! corresponding calc member functions, whose methods are
  //! implemented by derived classes.  This allows the derived classes
  //! to only implement the appropriate calc methods based on argument
  //! count; the default calc methods on OperatorImpl throw a "wrong
  //! argument count" PlanError.
  //!
  //! For arithmetic operators, it also allows the operator() method
  //! to perform implicit type promotions.  The plan reader selects an
  //! arithmetic operator by the types of the arguments, not the
  //! result type.  This is the reason OperatorImpl<Integer> has
  //! additional operator() methods with Real result variables.
  //!
  //! If more numeric types are added in the future, implementors
  //! should add more specializations of OperatorImpl to support the
  //! additional implicit type promotions, or should implement an
  //! ArithmeticOperator base class template which supports them.
  //!
  //! \see ArithmeticFunctionFactory
  //! \ingroup Expressions
  template <typename R>
  class OperatorImpl : public Operator
  {
  public:

    //! \brief Virtual destructor.
    virtual ~OperatorImpl() = default;

    //! \brief Operate on the given Expression, and store the result in a variable.
    //! \param result Reference to the variable.
    //! \param arg Const pointer to the expression.
    //! \return true if the result is known, false if not.
    //! \note Delegates to the corresponding calc() method.
    virtual bool operator()(R &result, Expression const *arg) const;

    //! \brief Operate on the given Expressions, and store the result in a variable.
    //! \param result Reference to the variable.
    //! \param arg0 Const pointer to the first expression.
    //! \param arg1 Const pointer to the second expression.
    //! \return true if the result is known, false if not.
    //! \note Delegates to the corresponding calc() method.
    virtual bool operator()(R &result, Expression const *arg0, Expression const *arg1) const;

    //! \brief Operate on the given Function, and store the result in a variable.
    //! \param result Reference to the variable.
    //! \param fn Const reference to the function.
    //! \return true if the result is known, false if not.
    //! \note Delegates to the corresponding calc() method.
    virtual bool operator()(R &result, Function const &args) const;

    // Default methods, based on R

    //! \brief Return the value type of this Operation's result.
    //! \return The ValueType.
    ValueType valueType() const;

    //! \brief Allocate a cache for the result of this Operation.
    //! \return Pointer to the cache.  May be NULL.
    void *allocateCache() const;

    //! \brief Delete a cache for the result of this Operation.
    //! \param ptr Pointer to the cachel
    void deleteCache(void *ptr) const;

    //! \brief Is the result of this Operation on this Function known?
    //! \param exprs Const reference to Function containing subexpressions.
    //! \return true if known, false if unknown.
    bool isKnown(Function const &exprs) const;

    //! \brief Print the result of this Operation on this Function to an output stream.
    //! \param s The stream.
    //! \param exprs Const reference to Function containing subexpressions.
    void printValue(std::ostream &s, Function const &exprs) const;

    //! \brief Return the result of this Operation on this Function as a Value instance.
    //! \param exprs Const reference to Function containing subexpressions.
    //! \return The Value.
    Value toValue(Function const &exprs) const;

    // Delegated to derived classes

    //! \brief Actually perform the operation on the expression and store the result.
    //! \param result Reference to the variable.
    //! \param arg Const pointer to the expression.
    //! \return true if the result is known, false if not.
    //! \note Default method.  Throws "wrong argument count" error.
    virtual bool calc(R &result, Expression const *arg) const;

    //! \brief Actually perform the operation on the expressions and store the result.
    //! \param result Reference to the variable.
    //! \param arg0 Const pointer to the first expression.
    //! \param arg1 Const pointer to the second expression.
    //! \return true if the result is known, false if not.
    //! \note Default method.  Throws "wrong argument count" error.
    virtual bool calc(R &result, Expression const *arg0, Expression const *arg1) const;

    //! \brief Actually perform the operation on the function and store the result.
    //! \param result Reference to the variable.
    //! \param fn Const reference to the function.
    //! \return true if the result is known, false if not.
    //! \note Default method.  Throws "wrong argument count" error.
    virtual bool calc(R &result, Function const &args) const;

  protected:

    //! \brief Protected constructor.  Only accessible to derived classes.
    OperatorImpl(std::string const &name);

  private:
    // Unimplemented
    OperatorImpl() = delete;
    OperatorImpl(OperatorImpl const &) = delete;
    OperatorImpl(OperatorImpl &&) = delete;
    OperatorImpl &operator=(OperatorImpl const &) = delete;
    OperatorImpl &operator=(OperatorImpl &&) = delete;
  };

  //! \brief Specialization of OperatorImpl for Integer typed
  //!        operations.  Implements implicit promotion from Integer
  //!        operands to Real result types.
  //! \ingroup Expressions
  //! \see ArithmeticFunctionFactory
  template <>
  class OperatorImpl<Integer> : public Operator
  {
  public:
    virtual ~OperatorImpl() = default;

    virtual bool operator()(Integer &result, Expression const *arg) const;
    virtual bool operator()(Integer &result, Expression const *arg0, Expression const *arg1) const;
    virtual bool operator()(Integer &result, Function const &args) const;

    //
    // Conversion methods
    //

    //! \brief Operate on the given Expression, and store the result in a Real variable.
    //! \param result Reference to the Real variable.
    //! \param arg Const pointer to the expression.
    //! \return true if the result is known, false if not.
    //! \note Delegates to the Integer-typed calc method and promotes its result to Real.
    virtual bool operator()(Real &result, Expression const *arg) const;

    //! \brief Operate on the given Expressions, and store the result in a Real variable.
    //! \param result Reference to the Real variable.
    //! \param arg0 Const pointer to the first expression.
    //! \param arg1 Const pointer to the second expression.
    //! \return true if the result is known, false if not.
    //! \note Delegates to the Integer-typed calc method and promotes its result to Real.
    virtual bool operator()(Real &result, Expression const *arg0, Expression const *arg1) const;

    //! \brief Operate on the given Function, and store the result in a Real variable.
    //! \param result Reference to the Real variable.
    //! \param fn Const reference to the function.
    //! \return true if the result is known, false if not.
    //! \note Delegates to the Integer-typed calc method and promotes its result to Real.
    virtual bool operator()(Real &result, Function const &args) const;

    ValueType valueType() const;
    void *allocateCache() const;
    void deleteCache(void *ptr) const;

    bool isKnown(Function const &exprs) const;
    void printValue(std::ostream &s, Function const &exprs) const;
    Value toValue(Function const &exprs) const;

    virtual bool calc(Integer &result, Expression const *arg) const;
    virtual bool calc(Integer &result, Expression const *arg0, Expression const *arg1) const;
    virtual bool calc(Integer &result, Function const &args) const;

  protected:
    OperatorImpl(std::string const &name);

  private:
    // Unimplemented
    OperatorImpl() = delete;
    OperatorImpl(OperatorImpl const &) = delete;
    OperatorImpl(OperatorImpl &&) = delete;
    OperatorImpl &operator=(OperatorImpl const &) = delete;
    OperatorImpl &operator=(OperatorImpl &&) = delete;
  };

  //! \brief Specialization of the OperatorImpl template for operations returning array values.
  //! \ingroup Expressions
  template <typename R>
  class OperatorImpl<ArrayImpl<R> > : public Operator
  {
  public:
    virtual ~OperatorImpl() = default;

    virtual bool operator()(ArrayImpl<R> &result, Expression const *arg) const;
    virtual bool operator()(ArrayImpl<R> &result, Expression const *arg0, Expression const *arg1) const;
    virtual bool operator()(ArrayImpl<R> &result, Function const &args) const;

    // Default methods, based on R
    ValueType valueType() const;
    void *allocateCache() const;
    void deleteCache(void *ptr) const;

    bool isKnown(Function const &exprs) const;
    void printValue(std::ostream &s, Function const &exprs) const;
    Value toValue(Function const &exprs) const;

    // Delegated to derived classes
    virtual bool calc(ArrayImpl<R> &result, Expression const *arg) const = 0;
    virtual bool calc(ArrayImpl<R> &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool calc(ArrayImpl<R> &result, Function const &args) const = 0;

  protected:
    // Base class shouldn't be instantiated by itself
    OperatorImpl(std::string const &name);

  private:
    // Unimplemented
    OperatorImpl() = delete;
    OperatorImpl(OperatorImpl const &) = delete;
    OperatorImpl(OperatorImpl &&) = delete;
    OperatorImpl &operator=(OperatorImpl const &) = delete;
    OperatorImpl &operator=(OperatorImpl &&) = delete;
  };

} // namespace PLEXIL

//! \brief Helper macro, intended to implement "boilerplate" singleton accessors
//!        for classes derived from OperatorImpl<R>.  Meant to be called from
//!        the body of the class declaration.
//! \param CLASS Name of the class.
#define DECLARE_OPERATOR_STATIC_INSTANCE(CLASS) \
  static PLEXIL::Operator const *instance() \
  { \
    static CLASS const sl_instance; \
    return static_cast<PLEXIL::Operator const *>(&sl_instance); \
  }

#endif // PLEXIL_OPERATOR_IMPL_HH
