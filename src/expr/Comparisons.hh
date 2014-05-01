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
    IsKnown();
    IsKnown(const IsKnown &);

    ~IsKnown();

    bool operator()(bool &result, const ExpressionId &arg) const;

  private:
    // Disallow assignment
    IsKnown& operator=(const IsKnown &);
  };

  template <typename T>
  class Equal : public Operator<bool>
  {
  public:
    Equal();
    Equal(const Equal<T> &);

    ~Equal();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

  private:
    // Disallow assignment
    Equal &operator=(const Equal<T> &);
  };

  template <typename T>
  class NotEqual : public Operator<bool>
  {
  public:
    NotEqual();
    NotEqual(const NotEqual<T> &);

    ~NotEqual();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

  private:
    // Disallow assignment
    NotEqual &operator=(const NotEqual<T> &);
  };

  template <typename T>
  class GreaterThan : public Operator<bool>
  {
  public:
    GreaterThan();
    GreaterThan(const GreaterThan<T> &);

    ~GreaterThan();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

  private:
    // Disallow assignment
    GreaterThan &operator=(const GreaterThan<T> &);
  };

  template <typename T>
  class GreaterEqual : public Operator<bool>
  {
  public:
    GreaterEqual();
    GreaterEqual(const GreaterEqual<T> &);

    ~GreaterEqual();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

  private:
    // Disallow assignment
    GreaterEqual &operator=(const GreaterEqual<T> &);
  };

  template <typename T>
  class LessThan : public Operator<bool>
  {
  public:
    LessThan();
    LessThan(const LessThan<T> &);

    ~LessThan();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

  private:
    // Disallow assignment
    LessThan &operator=(const LessThan<T> &);
  };

  template <typename T>
  class LessEqual : public Operator<bool>
  {
  public:
    LessEqual();
    LessEqual(const LessEqual<T> &);

    ~LessEqual();

    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;

  private:
    // Disallow assignment
    LessEqual &operator=(const LessEqual<T> &);
  };

}

#endif // PLEXIL_COMPARISON_OPERATORS_HH
