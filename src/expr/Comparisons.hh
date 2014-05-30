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

#ifndef PLEXIL_COMPARISON_OPERATORS_HH
#define PLEXIL_COMPARISON_OPERATORS_HH

#include "Operator.hh"

namespace PLEXIL
{

  // TODO:
  // - Handle mixed type numeric operands

  class IsKnown : public Operator<bool>
  {
  public:
    ~IsKnown();

    bool operator()(bool &result, const ExpressionId &arg) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(IsKnown, bool)

  private:
    IsKnown();

    // Disallow copy, assignment
    IsKnown(const IsKnown &);
    IsKnown& operator=(const IsKnown &);
  };

  template <typename T>
  class Equal : public Operator<bool>
  {
  public:
    ~Equal();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(Equal<T>, bool)

  private:
    Equal();

    // Disallow copy, assignment
    Equal(const Equal<T> &);
    Equal &operator=(const Equal<T> &);
  };

  template <typename T>
  class NotEqual : public Operator<bool>
  {
  public:
    ~NotEqual();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(NotEqual<T>, bool)

  private:
    NotEqual();

    // Disallow copy, assignment
    NotEqual(const NotEqual<T> &);
    NotEqual &operator=(const NotEqual<T> &);
  };

  template <typename T>
  class GreaterThan : public Operator<bool>
  {
  public:
    ~GreaterThan();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(GreaterThan<T>, bool)

  private:
    GreaterThan();

    // Disallow copy, assignment
    GreaterThan(const GreaterThan<T> &);
    GreaterThan &operator=(const GreaterThan<T> &);
  };

  template <typename T>
  class GreaterEqual : public Operator<bool>
  {
  public:
    ~GreaterEqual();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(GreaterEqual<T>, bool)

  private:
    GreaterEqual();

    // Disallow copy, assignment
    GreaterEqual(const GreaterEqual<T> &);
    GreaterEqual &operator=(const GreaterEqual<T> &);
  };

  template <typename T>
  class LessThan : public Operator<bool>
  {
  public:
    ~LessThan();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(LessThan<T>, bool)

  private:
    LessThan();

    // Disallow copy, assignment
    LessThan(const LessThan<T> &);
    LessThan &operator=(const LessThan<T> &);
  };

  template <typename T>
  class LessEqual : public Operator<bool>
  {
  public:
    ~LessEqual();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(LessEqual<T>, bool)

  private:
    LessEqual();

    // Disallow assignment
    LessEqual(const LessEqual<T> &);
    LessEqual &operator=(const LessEqual<T> &);
  };

}

#endif // PLEXIL_COMPARISON_OPERATORS_HH
