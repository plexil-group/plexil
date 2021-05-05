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

#ifndef PLEXIL_COMPARISON_OPERATORS_HH
#define PLEXIL_COMPARISON_OPERATORS_HH

#include "OperatorImpl.hh"

namespace PLEXIL
{

  // TODO:
  // - Handle mixed type numeric operands

  class IsKnown final : public OperatorImpl<Boolean>
  {
  public:
    ~IsKnown() = default;

    bool checkArgCount(size_t count) const;

    bool operator()(bool &result, Expression const *arg) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(IsKnown, Boolean)

  private:
    IsKnown();

    // Disallow copy, assignment
    IsKnown(IsKnown const &) = delete;
    IsKnown(IsKnown &&) = delete;
    IsKnown& operator=(IsKnown const &) = delete;
    IsKnown& operator=(IsKnown &&) = delete;
  };

  class Equal final : public OperatorImpl<Boolean>
  {
  public:
    ~Equal() = default;

    bool checkArgCount(size_t count) const;

    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(Equal, Boolean)

  private:
    Equal();

    // Disallow copy, assignment
    Equal(Equal const &) = delete;
    Equal(Equal &&) = delete;
    Equal &operator=(Equal const &) = delete;
    Equal &operator=(Equal &&) = delete;
  };

  class NotEqual final : public OperatorImpl<Boolean>
  {
  public:
    ~NotEqual() = default;

    bool checkArgCount(size_t count) const;

    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(NotEqual, Boolean)

  private:
    NotEqual();

    // Disallow copy, assignment
    NotEqual(NotEqual const &) = delete;
    NotEqual(NotEqual &&) = delete;
    NotEqual &operator=(NotEqual const &) = delete;
    NotEqual &operator=(NotEqual &&) = delete;
  };

  template <typename T>
  class GreaterThan final : public OperatorImpl<Boolean>
  {
  public:
    ~GreaterThan() = default;

    bool checkArgCount(size_t count) const;

    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(GreaterThan<T>, Boolean)

  private:
    GreaterThan();

    // Disallow copy, assignment
    GreaterThan(GreaterThan<T> const &) = delete;
    GreaterThan(GreaterThan<T> &&) = delete;
    GreaterThan &operator=(GreaterThan<T> const &) = delete;
    GreaterThan &operator=(GreaterThan<T> &&) = delete;
  };

  template <typename T>
  class GreaterEqual final : public OperatorImpl<Boolean>
  {
  public:
    ~GreaterEqual() = default;

    bool checkArgCount(size_t count) const;

    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(GreaterEqual<T>, Boolean)

  private:
    GreaterEqual();

    // Disallow copy, assignment
    GreaterEqual(GreaterEqual<T> const &) = delete;
    GreaterEqual(GreaterEqual<T> &&) = delete;
    GreaterEqual &operator=(GreaterEqual<T> const &) = delete;
    GreaterEqual &operator=(GreaterEqual<T> &&) = delete;
  };

  template <typename T>
  class LessThan final : public OperatorImpl<Boolean>
  {
  public:
    ~LessThan() = default;

    bool checkArgCount(size_t count) const;

    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(LessThan<T>, Boolean)

  private:
    LessThan();

    // Disallow copy, assignment
    LessThan(LessThan<T> const &) = delete;
    LessThan(LessThan<T> &&) = delete;
    LessThan &operator=(LessThan<T> const &) = delete;
    LessThan &operator=(LessThan<T> &&) = delete;
  };

  template <typename T>
  class LessEqual final : public OperatorImpl<Boolean>
  {
  public:
    ~LessEqual() = default;

    bool checkArgCount(size_t count) const;

    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    bool operator()(bool &result, Expression const *argA, Expression const *argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(LessEqual<T>, Boolean)

  private:
    LessEqual();

    // Disallow assignment
    LessEqual(LessEqual<T> const &) = delete;
    LessEqual(LessEqual<T> &&) = delete;
    LessEqual &operator=(LessEqual<T> const &) = delete;
    LessEqual &operator=(LessEqual<T> &&) = delete;
  };

}

#endif // PLEXIL_COMPARISON_OPERATORS_HH
