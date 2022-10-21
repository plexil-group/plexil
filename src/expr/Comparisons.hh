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

#ifndef PLEXIL_COMPARISON_OPERATORS_HH
#define PLEXIL_COMPARISON_OPERATORS_HH

#include "OperatorImpl.hh"

namespace PLEXIL
{

  // TODO:
  // - Handle mixed type numeric operands

  //! \class IsKnown
  //! \brief Implements the IsKnown operator.
  //! \ingroup Expressions
  class IsKnown final : public OperatorImpl<Boolean>
  {
  public:

    //! \brief Virtual destructor.
    virtual ~IsKnown() = default;


    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Perform the operation on the expression and store the result.
    //! \param result Reference to the result variable.
    //! \param arg Pointer to the expression.
    //! \return true if the result is known, false if not.
    bool operator()(bool &result, Expression const *arg) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(IsKnown)

  private:

    //! \brief Private default constructor.  Only accessible to the singleton accessor.
    IsKnown();

    // Disallow copy, assignment
    IsKnown(IsKnown const &) = delete;
    IsKnown(IsKnown &&) = delete;
    IsKnown& operator=(IsKnown const &) = delete;
    IsKnown& operator=(IsKnown &&) = delete;
  };

  //! \class Equal
  //! \brief Implements the EQ operators.
  //! \ingroup Expressions
  class Equal final : public OperatorImpl<Boolean>
  {
  public:

    //! \brief Virtual destructor.
    virtual ~Equal() = default;


    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Check that the argument types are valid for this Operator.
    //! \param typeVec The vector of argument types.
    //! \return true if valid, false if not.
    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    //! \brief Perform the operation on the expressions and store the result.
    //! \param result Reference to the result variable.
    //! \param argA The first subexpression.
    //! \param argB The second subexpression.
    //! \return true if the result of the operation is known, false if not.
    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(Equal)

  private:

    //! \brief Private default constructor.  Only accessible to the singleton accessor.
    Equal();

    // Disallow copy, assignment
    Equal(Equal const &) = delete;
    Equal(Equal &&) = delete;
    Equal &operator=(Equal const &) = delete;
    Equal &operator=(Equal &&) = delete;
  };

  //! \class NotEqual
  //! \brief Implements the NE operators.
  //! \ingroup Expressions
  class NotEqual final : public OperatorImpl<Boolean>
  {
  public:

    //! \brief Virtual destructor.
    virtual ~NotEqual() = default;


    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Check that the argument types are valid for this Operator.
    //! \param typeVec The vector of argument types.
    //! \return true if valid, false if not.
    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    //! \brief Perform the operation on the expressions and store the result.
    //! \param result Reference to the result variable.
    //! \param argA The first subexpression.
    //! \param argB The second subexpression.
    //! \return true if the result of the operation is known, false if not.
    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(NotEqual)

  private:

    //! \brief Private default constructor.  Only accessible to the singleton accessor.
    NotEqual();

    // Disallow copy, assignment
    NotEqual(NotEqual const &) = delete;
    NotEqual(NotEqual &&) = delete;
    NotEqual &operator=(NotEqual const &) = delete;
    NotEqual &operator=(NotEqual &&) = delete;
  };

  //! \class GreaterThan
  //! \brief Implements the GT operators.
  //! \ingroup Expressions
  template <typename T>
  class GreaterThan final : public OperatorImpl<Boolean>
  {
  public:

    //! \brief Virtual destructor.
    virtual ~GreaterThan() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Check that the argument types are valid for this Operator.
    //! \param typeVec The vector of argument types.
    //! \return true if valid, false if not.
    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    //! \brief Perform the operation on the expressions and store the result.
    //! \param result Reference to the result variable.
    //! \param argA The first subexpression.
    //! \param argB The second subexpression.
    //! \return true if the result of the operation is known, false if not.
    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(GreaterThan<T>)

  private:

    //! \brief Private default constructor.  Only accessible to the singleton accessor.
    GreaterThan();

    // Disallow copy, assignment
    GreaterThan(GreaterThan<T> const &) = delete;
    GreaterThan(GreaterThan<T> &&) = delete;
    GreaterThan &operator=(GreaterThan<T> const &) = delete;
    GreaterThan &operator=(GreaterThan<T> &&) = delete;
  };

  //! \class GreaterEqual
  //! \brief Implements the GE operators.
  //! \ingroup Expressions
  template <typename T>
  class GreaterEqual final : public OperatorImpl<Boolean>
  {
  public:

    //! \brief Virtual destructor.
    virtual ~GreaterEqual() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Check that the argument types are valid for this Operator.
    //! \param typeVec The vector of argument types.
    //! \return true if valid, false if not.
    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    //! \brief Perform the operation on the expressions and store the result.
    //! \param result Reference to the result variable.
    //! \param argA The first subexpression.
    //! \param argB The second subexpression.
    //! \return true if the result of the operation is known, false if not.
    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(GreaterEqual<T>)

  private:

    //! \brief Private default constructor.  Only accessible to the singleton accessor.
    GreaterEqual();

    // Disallow copy, assignment
    GreaterEqual(GreaterEqual<T> const &) = delete;
    GreaterEqual(GreaterEqual<T> &&) = delete;
    GreaterEqual &operator=(GreaterEqual<T> const &) = delete;
    GreaterEqual &operator=(GreaterEqual<T> &&) = delete;
  };

  //! \class LessThan
  //! \brief Implements the LT operators.
  //! \ingroup Expressions
  template <typename T>
  class LessThan final : public OperatorImpl<Boolean>
  {
  public:

    //! \brief Virtual destructor.
    virtual ~LessThan() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Check that the argument types are valid for this Operator.
    //! \param typeVec The vector of argument types.
    //! \return true if valid, false if not.
    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    //! \brief Perform the operation on the expressions and store the result.
    //! \param result Reference to the result variable.
    //! \param argA The first subexpression.
    //! \param argB The second subexpression.
    //! \return true if the result of the operation is known, false if not.
    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(LessThan<T>)

  private:

    //! \brief Private default constructor.  Only accessible to the singleton accessor.
    LessThan();

    // Disallow copy, assignment
    LessThan(LessThan<T> const &) = delete;
    LessThan(LessThan<T> &&) = delete;
    LessThan &operator=(LessThan<T> const &) = delete;
    LessThan &operator=(LessThan<T> &&) = delete;
  };

  //! \class LessEqual
  //! \brief Implements the LE operators.
  //! \ingroup Expressions
  template <typename T>
  class LessEqual final : public OperatorImpl<Boolean>
  {
  public:

    //! \brief Virtual destructor.
    virtual ~LessEqual() = default;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    bool checkArgCount(size_t count) const;

    //! \brief Check that the argument types are valid for this Operator.
    //! \param typeVec The vector of argument types.
    //! \return true if valid, false if not.
    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    //! \brief Perform the operation on the expressions and store the result.
    //! \param result Reference to the result variable.
    //! \param argA The first subexpression.
    //! \param argB The second subexpression.
    //! \return true if the result of the operation is known, false if not.
    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    //! \brief Singleton accessor.
    DECLARE_OPERATOR_STATIC_INSTANCE(LessEqual<T>)

  private:

    //! \brief Private default constructor.  Only accessible to the singleton accessor.
    LessEqual();

    // Disallow assignment
    LessEqual(LessEqual<T> const &) = delete;
    LessEqual(LessEqual<T> &&) = delete;
    LessEqual &operator=(LessEqual<T> const &) = delete;
    LessEqual &operator=(LessEqual<T> &&) = delete;
  };

}

#endif // PLEXIL_COMPARISON_OPERATORS_HH
