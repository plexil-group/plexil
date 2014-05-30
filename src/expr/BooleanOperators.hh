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

#ifndef PLEXIL_BOOLEAN_OPERATORS_HH
#define PLEXIL_BOOLEAN_OPERATORS_HH

#include "Operator.hh"

namespace PLEXIL
{

  class BooleanNot : public Operator<bool>
  {
  public:
    BooleanNot();
    ~BooleanNot();

    bool operator()(bool &result, const ExpressionId &arg) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(BooleanNot, bool)

  private:
    BooleanNot(const BooleanNot &);
    BooleanNot &operator=(const BooleanNot &);
  };

  class BooleanOr : public Operator<bool>
  {
  public:
    BooleanOr();
    ~BooleanOr();

    bool operator()(bool &result, const ExpressionId &arg) const;
    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;
    bool operator()(bool &result, const std::vector<ExpressionId> &args) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(BooleanOr, bool)

  private:
    BooleanOr(const BooleanOr &);
    BooleanOr &operator=(const BooleanOr &);
  };

  class BooleanAnd : public Operator<bool>
  {
  public:
    BooleanAnd();
    ~BooleanAnd();

    bool operator()(bool &result, const ExpressionId &arg) const;
    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;
    bool operator()(bool &result, const std::vector<ExpressionId> &args) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(BooleanAnd, bool)

  private:
    BooleanAnd(const BooleanAnd &);
    BooleanAnd &operator=(const BooleanAnd &);
  };

  class BooleanXor : public Operator<bool>
  {
  public:
    BooleanXor();
    ~BooleanXor();

    bool operator()(bool &result, const ExpressionId &arg) const;
    bool operator()(bool &result, const ExpressionId &argA, const ExpressionId &argB) const;
    bool operator()(bool &result, const std::vector<ExpressionId> &args) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(BooleanXor, bool)

  private:
    BooleanXor(const BooleanXor &);
    BooleanXor &operator=(const BooleanXor &);
  };

} // namespace PLEXIL

#endif // PLEXIL_BOOLEAN_OPERATORS_HH
