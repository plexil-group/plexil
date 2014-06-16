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
#include "ParserException.hh"

namespace PLEXIL
{
  // A crude but effective factory for ExprVec instances

  template class ExprVecImpl<0>;
  template class ExprVecImpl<1>;
  template class ExprVecImpl<2>;
  template class ExprVecImpl<3>;
  template class ExprVecImpl<4>;
  template class ExprVecImpl<5>;
  template class ExprVecImpl<6>;
  template class ExprVecImpl<7>;
  template class ExprVecImpl<8>;
  template class ExprVecImpl<9>;

  ExprVec const *makeExprVec(std::vector<ExpressionId> const &exprs,
                             std::vector<bool> const &garbage)
  {
    checkParserException(exprs.size() == garbage.size(),
                         "makeExprVec: expression and garbage vectors of different lengths");
    switch (exprs.size()) {
    case 0:
      return static_cast<ExprVec const *>(new ExprVecImpl<0>(exprs, garbage));
    case 1:
      return static_cast<ExprVec const *>(new ExprVecImpl<1>(exprs, garbage));
    case 2:
      return static_cast<ExprVec const *>(new ExprVecImpl<2>(exprs, garbage));
    case 3:
      return static_cast<ExprVec const *>(new ExprVecImpl<3>(exprs, garbage));
    case 4:
      return static_cast<ExprVec const *>(new ExprVecImpl<4>(exprs, garbage));
    case 5:
      return static_cast<ExprVec const *>(new ExprVecImpl<5>(exprs, garbage));
    case 6:
      return static_cast<ExprVec const *>(new ExprVecImpl<6>(exprs, garbage));
    case 7:
      return static_cast<ExprVec const *>(new ExprVecImpl<7>(exprs, garbage));
    case 8:
      return static_cast<ExprVec const *>(new ExprVecImpl<8>(exprs, garbage));
    case 9:
      return static_cast<ExprVec const *>(new ExprVecImpl<9>(exprs, garbage));

    default:
      checkParserException(false,
                           "makeExprVec: functions of " << exprs.size()
                           << " arguments not yet implemented");
      return NULL;
    }
  }

} // namespace PLEXIL
