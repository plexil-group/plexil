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

#ifndef PLEXIL_ARITHMETIC_OPERATORS_HH
#define PLEXIL_ARITHMETIC_OPERATORS_HH

#include "OperatorImpl.hh"

namespace PLEXIL
{

  //! \class Addition
  //! \brief Implements the ADD operators.
  //! \ingroup Expressions
  template <typename NUM>
  class Addition final : public OperatorImpl<NUM>
  {
  public:

    //! \brief Virtual destructor.
    virtual ~Addition() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Perform the operation on the expression, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg Const pointer to the argument Expression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg) const;

    //! \brief Perform the operation on the expressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg0 Const pointer to the first subexpression.
    //! \param arg1 Const pointer to the second subexpression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg0, Expression const *arg1) const;

    //! \brief Perform the operation on the Function's subexpressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param args Const reference to the Function.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Function const &args) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(Addition<NUM>);

  private:

    //! \brief Private default constructor.  Accessible to singleton accessor only.
    Addition();

    // Not implemented.
    Addition(Addition const &) = delete;
    Addition(Addition &&) = delete;
    Addition &operator=(Addition const &) = delete;
    Addition &operator=(Addition &&) = delete;
  };

  template <typename NUM>
  class Subtraction final : public OperatorImpl<NUM>
  {
  public:

    //! \brief Virtual destructor.
    virtual ~Subtraction() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Perform the operation on the expression, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg Const pointer to the argument Expression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg) const;

    //! \brief Perform the operation on the expressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg0 Const pointer to the first subexpression.
    //! \param arg1 Const pointer to the second subexpression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg0, Expression const *arg1) const;

    //! \brief Perform the operation on the Function's subexpressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param args Const reference to the Function.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Function const &args) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(Subtraction<NUM>);

  private:

    //! \brief Private default constructor.  Accessible to singleton accessor only.
    Subtraction();

    // Not implemented.
    Subtraction(Subtraction const &) = delete;
    Subtraction(Subtraction &&) = delete;
    Subtraction &operator=(Subtraction const &) = delete;
    Subtraction &operator=(Subtraction &&) = delete;
  };

  template <typename NUM>
  class Multiplication final : public OperatorImpl<NUM>
  {
  public:
    
    //! \brief Virtual destructor.
    virtual ~Multiplication() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Perform the operation on the expression, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg Const pointer to the argument Expression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg) const;

    //! \brief Perform the operation on the expressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg0 Const pointer to the first subexpression.
    //! \param arg1 Const pointer to the second subexpression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg0, Expression const *arg1) const;

    //! \brief Perform the operation on the Function's subexpressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param args Const reference to the Function.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Function const &args) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(Multiplication<NUM>);

  private:

    //! \brief Private default constructor.  Accessible to singleton accessor only.
    Multiplication();

    // Not implemented.
    Multiplication(Multiplication const &) = delete;
    Multiplication(Multiplication &&) = delete;
    Multiplication &operator=(Multiplication const &) = delete;
    Multiplication &operator=(Multiplication &&) = delete;
  };

  template <typename NUM>
  class Division final : public OperatorImpl<NUM>
  {
  public:
    
    //! \brief Virtual destructor.
    virtual ~Division() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Perform the operation on the expressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg0 Const pointer to the first subexpression.
    //! \param arg1 Const pointer to the second subexpression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg0, Expression const *arg1) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(Division<NUM>);

  private:

    //! \brief Private default constructor.  Accessible to singleton accessor only.
    Division();

    // Not implemented.
    Division(Division const &) = delete;
    Division(Division &&) = delete;
    Division &operator=(Division const &) = delete;
    Division &operator=(Division &&) = delete;
  };

  template <typename NUM>
  class Modulo final : public OperatorImpl<NUM>
  {
  public:
    
    //! \brief Virtual destructor.
    virtual ~Modulo() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Perform the operation on the expressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg0 Const pointer to the first subexpression.
    //! \param arg1 Const pointer to the second subexpression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg0, Expression const *arg1) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(Modulo<NUM>);

  private:

    //! \brief Private default constructor.  Accessible to singleton accessor only.
    Modulo();

    // Not implemented.
    Modulo(Modulo const &) = delete;
    Modulo(Modulo &&) = delete;
    Modulo &operator=(Modulo const &) = delete;
    Modulo &operator=(Modulo &&) = delete;
  };

  template <typename NUM>
  class Minimum final : public OperatorImpl<NUM>
  {
  public:
    
    //! \brief Virtual destructor.
    virtual ~Minimum() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Perform the operation on the expression, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg Const pointer to the argument Expression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg) const;

    //! \brief Perform the operation on the expressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg0 Const pointer to the first subexpression.
    //! \param arg1 Const pointer to the second subexpression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg0, Expression const *arg1) const;

    //! \brief Perform the operation on the Function's subexpressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param args Const reference to the Function.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Function const &args) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(Minimum<NUM>);

  private:

    //! \brief Private default constructor.  Accessible to singleton accessor only.
    Minimum();

    // Not implemented.
    Minimum(Minimum const &) = delete;
    Minimum(Minimum &&) = delete;
    Minimum &operator=(Minimum const &) = delete;
    Minimum &operator=(Minimum &&) = delete;
  };

  template <typename NUM>
  class Maximum final : public OperatorImpl<NUM>
  {
  public:
    
    //! \brief Virtual destructor.
    virtual ~Maximum() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Perform the operation on the expression, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg Const pointer to the argument Expression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg) const;

    //! \brief Perform the operation on the expressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg0 Const pointer to the first subexpression.
    //! \param arg1 Const pointer to the second subexpression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg0, Expression const *arg1) const;

    //! \brief Perform the operation on the Function's subexpressions, and store the result.
    //! \param result Reference to the result variable.
    //! \param args Const reference to the Function.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Function const &args) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(Maximum<NUM>);

  private:

    //! \brief Private default constructor.  Accessible to singleton accessor only.
    Maximum();

    // Not implemented.
    Maximum(Maximum const &) = delete;
    Maximum(Maximum &&) = delete;
    Maximum &operator=(Maximum const &) = delete;
    Maximum &operator=(Maximum &&) = delete;
  };

  //
  // Strictly unary operations
  //

  template <typename NUM>
  class AbsoluteValue final : public OperatorImpl<NUM>
  {
  public:
    
    //! \brief Virtual destructor.
    virtual ~AbsoluteValue() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Perform the operation on the expression, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg Const pointer to the argument Expression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(AbsoluteValue<NUM>);

  private:

    //! \brief Private default constructor.  Accessible to singleton accessor only.
    AbsoluteValue();

    // Not implemented.
    AbsoluteValue(AbsoluteValue const &) = delete;
    AbsoluteValue(AbsoluteValue &&) = delete;
    AbsoluteValue &operator=(AbsoluteValue const &) = delete;
    AbsoluteValue &operator=(AbsoluteValue &&) = delete;
  };
  

  template <typename NUM>
  class SquareRoot final : public OperatorImpl<NUM>
  {
  public:
    
    //! \brief Virtual destructor.
    virtual ~SquareRoot() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Check that the argument types are valid for this Operator.
    //! \param typeVec The vector of argument types.
    //! \return true if valid, false if not.
    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    //! \brief Perform the operation on the expression, and store the result.
    //! \param result Reference to the result variable.
    //! \param arg Const pointer to the argument Expression.
    //! \return true if the result is known, false if not.
    bool calc(NUM &result, Expression const *arg) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(SquareRoot<NUM>);

  private:

    //! \brief Private default constructor.  Accessible to singleton accessor only.
    SquareRoot();

    // Not implemented.
    SquareRoot(SquareRoot const &) = delete;
    SquareRoot(SquareRoot &&) = delete;
    SquareRoot &operator=(SquareRoot const &) = delete;
    SquareRoot &operator=(SquareRoot &&) = delete;
  };

}

#endif // PLEXIL_ARITHMETIC_OPERATORS_HH
