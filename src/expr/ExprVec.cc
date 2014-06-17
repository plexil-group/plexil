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

#include "ExprVec.hh"
#include "Expression.hh"
#include "Operator.hh"
#include "ParserException.hh"

namespace PLEXIL
{
  //
  // ExprVec base class methods
  //

  void ExprVec::addListener(ExpressionListenerId id)
  {
    for (size_t i = 0; i < this->size(); ++i)
      (*this)[i]->addListener(id);
  }

  void ExprVec::removeListener(ExpressionListenerId id)
  {
    for (size_t i = 0; i < this->size(); ++i)
      (*this)[i]->removeListener(id);
  }

  bool ExprVec::apply(Operator const *op, bool &result) const
  {
    return (*op)(result, *this);
  }

  bool ExprVec::apply(Operator const *op, int32_t &result) const
  {
    return (*op)(result, *this);
  }

  bool ExprVec::apply(Operator const *op, double &result) const
  {
    return (*op)(result, *this);
  }

  bool ExprVec::apply(Operator const *op, std::string &result) const
  {
    return (*op)(result, *this);
  }

  bool ExprVec::apply(Operator const *op, Array &result) const
  {
    return (*op)(result, *this);
  }

  bool ExprVec::apply(Operator const *op, BooleanArray &result) const
  {
    return (*op)(result, *this);
  }

  bool ExprVec::apply(Operator const *op, IntegerArray &result) const
  {
    return (*op)(result, *this);
  }

  bool ExprVec::apply(Operator const *op, RealArray &result) const
  {
    return (*op)(result, *this);
  }

  bool ExprVec::apply(Operator const *op, StringArray &result) const
  {
    return (*op)(result, *this);
  }

  //
  // FixedExprVec
  //
  // General cases - optimized cases below
  //

  template <unsigned N>
  FixedExprVec<N>::FixedExprVec(std::vector<ExpressionId> const &exps,
                              std::vector<bool> const &garb)
    : ExprVec()
  {
    check_error_1(exps.size() == N && garb.size() == N);
    for (size_t i = 0; i < N; ++i)
      exprs[i] = exps[i];
    for (size_t i = 0; i < N; ++i)
      garbage[i] = garb[i];
  }

  template <unsigned N>
  FixedExprVec<N>::~FixedExprVec()
  {
    for (size_t i = 0; i < N; ++i)
      if (garbage[i])
        delete (Expression *) exprs[i];
  }

  template <unsigned N>
  ExpressionId const &FixedExprVec<N>::operator[](size_t n) const 
  {
    check_error_1(n < N);
    return exprs[n]; 
  }

  template <unsigned N>
  void FixedExprVec<N>::activate() 
  {
    for (size_t i = 0; i < N; ++i)
      exprs[i]->activate();
  }

  template <unsigned N>
  void FixedExprVec<N>::deactivate()
  {
    for (size_t i = 0; i < N; ++i)
     exprs[i]->deactivate();
  }

  // One-arg variants

  template <>
  FixedExprVec<1>::FixedExprVec(std::vector<ExpressionId> const &exps,
                              std::vector<bool> const &garb)
    : ExprVec()
  {
    check_error_1(exps.size() == 1 && garb.size() == 1);
    exprs[0] = exps[0];
    garbage[0] = garb[0];
  }

  template <>
  FixedExprVec<1>::~FixedExprVec()
  {
    if (garbage[0])
      delete (Expression *) exprs[0];
  }

  template <>
  void FixedExprVec<1>::activate() 
  {
    exprs[0]->activate();
  }

  template <>
  void FixedExprVec<1>::deactivate()
  {
    exprs[0]->deactivate();
  }

  template <>
  void FixedExprVec<1>::addListener(ExpressionListenerId id)
  {
    exprs[0]->addListener(id);
  }

  template <>
  void FixedExprVec<1>::removeListener(ExpressionListenerId id)
  {
    exprs[0]->removeListener(id);
  }

  template <>
  bool FixedExprVec<1>::apply(Operator const *op, bool &result) const
  {
    return (*op)(result, exprs[0]);
  }

  template <>
  bool FixedExprVec<1>::apply(Operator const *op, int32_t &result) const
  {
    return (*op)(result, exprs[0]);
  }

  template <>
  bool FixedExprVec<1>::apply(Operator const *op, double &result) const
  {
    return (*op)(result, exprs[0]);
  }

  template <>
  bool FixedExprVec<1>::apply(Operator const *op, std::string &result) const
  {
    return (*op)(result, exprs[0]);
  }

  template <>
  bool FixedExprVec<1>::apply(Operator const *op, Array &result) const
  {
    return (*op)(result, exprs[0]);
  }

  template <>
  bool FixedExprVec<1>::apply(Operator const *op, BooleanArray &result) const
  {
    return (*op)(result, exprs[0]);
  }

  template <>
  bool FixedExprVec<1>::apply(Operator const *op, IntegerArray &result) const
  {
    return (*op)(result, exprs[0]);
  }

  template <>
  bool FixedExprVec<1>::apply(Operator const *op, RealArray &result) const
  {
    return (*op)(result, exprs[0]);
  }

  template <>
  bool FixedExprVec<1>::apply(Operator const *op, StringArray &result) const
  {
    return (*op)(result, exprs[0]);
  }

  //
  // Two-arg variants
  //

  template <>
  FixedExprVec<2>::FixedExprVec(std::vector<ExpressionId> const &exps,
                              std::vector<bool> const &garb)
    : ExprVec()
  {
    check_error_1(exps.size() == 2 && garb.size() == 2);
    exprs[0] = exps[0];
    exprs[1] = exps[1];
    garbage[0] = garb[0];
    garbage[1] = garb[1];
  }

  template <>
  FixedExprVec<2>::~FixedExprVec()
  {
    if (garbage[0])
      delete (Expression *) exprs[0];
    if (garbage[1])
      delete (Expression *) exprs[1];
  }

  template <>
  void FixedExprVec<2>::activate() 
  {
    exprs[0]->activate();
    exprs[1]->activate();
  }

  template <>
  void FixedExprVec<2>::deactivate()
  {
    exprs[0]->deactivate();
    exprs[1]->deactivate();
  }

  template <>
  void FixedExprVec<2>::addListener(ExpressionListenerId id)
  {
    exprs[0]->addListener(id);
    exprs[1]->addListener(id);
  }

  template <>
  void FixedExprVec<2>::removeListener(ExpressionListenerId id)
  {
    exprs[0]->removeListener(id);
    exprs[1]->removeListener(id);
  }

  template <>
  bool FixedExprVec<2>::apply(Operator const *op, bool &result) const
  {
    return (*op)(result, exprs[0], exprs[1]);
  }

  template <>
  bool FixedExprVec<2>::apply(Operator const *op, int32_t &result) const
  {
    return (*op)(result, exprs[0], exprs[1]);
  }

  template <>
  bool FixedExprVec<2>::apply(Operator const *op, double &result) const
  {
    return (*op)(result, exprs[0], exprs[1]);
  }

  template <>
  bool FixedExprVec<2>::apply(Operator const *op, std::string &result) const
  {
    return (*op)(result, exprs[0], exprs[1]);
  }

  template <>
  bool FixedExprVec<2>::apply(Operator const *op, Array &result) const
  {
    return (*op)(result, exprs[0], exprs[1]);
  }

  template <>
  bool FixedExprVec<2>::apply(Operator const *op, BooleanArray &result) const
  {
    return (*op)(result, exprs[0], exprs[1]);
  }

  template <>
  bool FixedExprVec<2>::apply(Operator const *op, IntegerArray &result) const
  {
    return (*op)(result, exprs[0], exprs[1]);
  }

  template <>
  bool FixedExprVec<2>::apply(Operator const *op, RealArray &result) const
  {
    return (*op)(result, exprs[0], exprs[1]);
  }

  template <>
  bool FixedExprVec<2>::apply(Operator const *op, StringArray &result) const
  {
    return (*op)(result, exprs[0], exprs[1]);
  }

  //
  // GeneralExprVec methods
  //

  GeneralExprVec::GeneralExprVec(std::vector<ExpressionId> const &exps,
                                 std::vector<bool> const &garb)
    : ExprVec(),
      exprs(exps),
      garbage(garb)
  {
    check_error_1(exps.size() == garb.size());
  }

  GeneralExprVec::~GeneralExprVec()
  {
    size_t n = exprs.size();
    for (size_t i = 0; i < n; ++i)
      if (garbage[i])
        delete (Expression *) exprs[i];
  }

  ExpressionId const &GeneralExprVec::operator[](size_t n) const 
  {
    check_error_1(n < exprs.size());
    return exprs[n]; 
  }

  void GeneralExprVec::activate() 
  {
    size_t n = exprs.size();
    for (size_t i = 0; i < n; ++i)
      exprs[i]->activate();
  }

  void GeneralExprVec::deactivate()
  {
    size_t n = exprs.size();
    for (size_t i = 0; i < n; ++i)
     exprs[i]->deactivate();
  }

  //
  // Factory function
  //

  // A crude but effective factory for ExprVec instances
  ExprVec *makeExprVec(std::vector<ExpressionId> const &exprs,
                       std::vector<bool> const &garbage)
  {
    checkParserException(exprs.size() == garbage.size(),
                         "makeExprVec: expression and garbage vectors of different lengths");
    switch (exprs.size()) {
    case 0:
      return static_cast<ExprVec *>(new FixedExprVec<0>(exprs, garbage));
    case 1:
      return static_cast<ExprVec *>(new FixedExprVec<1>(exprs, garbage));
    case 2:
      return static_cast<ExprVec *>(new FixedExprVec<2>(exprs, garbage));
    case 3:
      return static_cast<ExprVec *>(new FixedExprVec<3>(exprs, garbage));
    case 4:
      return static_cast<ExprVec *>(new FixedExprVec<4>(exprs, garbage));
    case 5:
      return static_cast<ExprVec *>(new FixedExprVec<5>(exprs, garbage));
    case 6:
      return static_cast<ExprVec *>(new FixedExprVec<6>(exprs, garbage));
    case 7:
      return static_cast<ExprVec *>(new FixedExprVec<7>(exprs, garbage));
    case 8:
      return static_cast<ExprVec *>(new FixedExprVec<8>(exprs, garbage));
    default: // anything greater than 8
      return static_cast<ExprVec *>(new GeneralExprVec(exprs, garbage));
    }
  }

  template class FixedExprVec<0>;
  template class FixedExprVec<1>;
  template class FixedExprVec<2>;
  template class FixedExprVec<3>;
  template class FixedExprVec<4>;
  template class FixedExprVec<5>;
  template class FixedExprVec<6>;
  template class FixedExprVec<7>;
  template class FixedExprVec<8>;

} // namespace PLEXIL
