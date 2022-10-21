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

#ifndef PLEXIL_FUNCTION_HH
#define PLEXIL_FUNCTION_HH

#include "ArrayFwd.hh"
#include "Expression.hh"
#include "Propagator.hh"
#include "Value.hh"
#include "ValueType.hh"

namespace PLEXIL
{
  // Forward reference
  class Operator;

  //! \class Function
  //! \brief Base class for expressions whose value depends on the
  //!        value(s) of zero or more subexpressions, and an operator on
  //!        those subexpressions.
  //!
  //! The Function class abstracts the storage of an expression's
  //! subexpressions from the computation performed on those
  //! subexpressions.  It bundles some number of expressions and an
  //! operator.
  //!
  //! Operator instances implement the desired computation via their
  //! operator() methods.
  //!
  //! \see Operator
  //! \ingroup Expressions
  class Function :
    public Expression,
    public Propagator
  {
  public:

    //! \brief Virtual destructor.
    virtual ~Function();

    //
    // Expression API
    //

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual const char *exprName() const override;

    //! \brief Return the type of the expression's value.
    //! \return The value type.
    virtual ValueType valueType() const override;

    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const override;

    //! \brief Get the value of this expression as a Value instance.
    //! \return The value.
    virtual Value toValue() const override;

    //! \brief Print the expression's value to a stream.
    //! \param s Reference to the stream.
    virtual void printValue(std::ostream &s) const override;

    //! \brief Retrieve the value of this Expression in its native form.
    //! \param result The appropriately typed place to put the result.
    //! \return True if result known, false if unknown.
    //! \note Derived classes may override the default methods for performance.
    virtual bool getValue(Boolean &result) const override;
    virtual bool getValue(Integer &result) const override;
    virtual bool getValue(Real &result) const override;
    virtual bool getValue(String &result) const override;

    virtual bool getValue(NodeState &result) const override;
    virtual bool getValue(NodeOutcome &result) const override;
    virtual bool getValue(FailureType &result) const override;
    virtual bool getValue(CommandHandleValue &result) const override;

    //! \brief Retrieve a pointer to the (const) value of this Expression.
    //! \param ptr Reference to the pointer variable to receive the result.
    //! \return True if known, false if unknown.
    //! \note Derived classes may override the default methods for performance.
    virtual bool getValuePointer(String const *&ptr) const override;

    // Maybe later?
    virtual bool getValuePointer(Array const *&ptr) const override;
    virtual bool getValuePointer(BooleanArray const *&ptr) const override;
    virtual bool getValuePointer(IntegerArray const *&ptr) const override;
    virtual bool getValuePointer(RealArray const *&ptr) const override;
    virtual bool getValuePointer(StringArray const *&ptr) const override;

    // Delegated to implementation classes

    // Argument accessors

    //! \brief Get the number of arguments to this Function.
    //! \return The argument count.
    virtual size_t size() const = 0;

    //! \brief Print the subexpressions of this expression to a stream.
    //! \param s Reference to the output stream.
    virtual void printSubexpressions(std::ostream &s) const override = 0;

    //! \brief Set the i'th argument of this Function.
    //! \param i The zero-based index of the desired argument.
    //! \param expr Pointer to an Expression to use for this argument.
    //! \param garbage true if the expression is to be deleted with the object,
    //!                false if not.
    virtual void setArgument(size_t i, Expression *expr, bool garbage) = 0;

    //! \brief Get the n'th argument of this Function.
    //! \param n The zero-based index of the desired argument.
    //! \return Const pointer to the argument's expression.
    virtual Expression const *operator[](size_t n) const = 0;

    //! \brief Query whether this expression is a source of change events.
    //! \return True if the value may change independently of any subexpressions, false otherwise.
    //! \note Delegated to the operator.
    virtual bool isPropagationSource() const override;

    //! \brief Apply the operator to the function's arguments,
    //!        and put the result in an Array.
    //! \param op Const pointer to an Operator.
    //! \param result Reference to the array where the result is stored.
    //! \return true if the value of the operation is known, false if not.
    //! \note Needed by Operator::calcNative for array types
    virtual bool apply(Operator const *op, Array &result) const;

  protected:

    //! \brief Protected constructor.  Only available to derived classes.
    //! \param op Const pointer to the function's operator.
    Function(Operator const *op);

    Operator const *m_op; //!< The operator for this Function.

  private:

    // Not implemented
    Function() = delete;
    Function(Function const &) = delete;
    Function(Function &&) = delete;
    Function& operator=(Function const &) = delete;
    Function& operator=(Function &&) = delete;
  };

  //
  // Factory functions
  //

  //! \brief Construct a Function with the given operator and number of arguments.
  //! \param op Const pointer to the Operator.
  //! \param nargs The number of arguments.
  //! \return Pointer to the newly constructed Function.
  //! \ingroup Expressions
  extern Function *makeFunction(Operator const *op,
                                size_t nargs);

  // Convenience wrappers for Node classes and unit test

  //! \brief Construct a Function with the given operator and one argument.
  //! \param op Const pointer to the Operator.
  //! \param expr Pointer to the argument expression.
  //! \param garbage true if the argument is to be deleted with the object,
  //!                false if not.
  //! \return Pointer to the newly constructed Function.
  //! \ingroup Expressions
  extern Function *makeFunction(Operator const *op,
                                Expression *expr,
                                bool garbage);

  //! \brief Construct a Function with the given operator and two arguments.
  //! \param op Const pointer to the Operator.
  //! \param expr1 Pointer to the first argument expression.
  //! \param expr2 Pointer to the second argument expression.
  //! \param garbage1 true if the first argument is to be deleted with the object,
  //!                 false if not.
  //! \param garbage2 true if the second argument is to be deleted with the object,
  //!                 false if not.
  //! \return Pointer to the newly constructed Function.
  //! \ingroup Expressions
  extern Function *makeFunction(Operator const *op, 
                                Expression *expr1,
                                Expression *expr2,
                                bool garbage1,
                                bool garbage2);

} // namespace PLEXIL

#endif // PLEXIL_FUNCTION_HH
