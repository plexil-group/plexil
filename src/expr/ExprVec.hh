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

#ifndef PLEXIL_EXPR_VEC_HH
#define PLEXIL_EXPR_VEC_HH

#include "Id.hh"

namespace PLEXIL
{
  class Expression;
  DECLARE_ID(Expression);

  class ExprVec
  {
  public:
    ExprVec() {}
    virtual ~ExprVec() {}
    virtual size_t size() const = 0;
    virtual ExpressionId const &operator[](size_t n) const = 0;

    void activate() 
    {
      for (size_t i = 0; i < this->size(); ++i)
        (*this)[i]->activate();
    }

    void deactivate()
    {
      for (size_t i = 0; i < this->size(); ++i)
        (*this)[i]->activate();
    }

    void addListener(ExpressionListenerId id)
    {
      for (size_t i = 0; i < this->size(); ++i)
        (*this)[i]->addListener(id);
    }

    void removeListener(ExpressionListenerId id)
    {
      for (size_t i = 0; i < this->size(); ++i)
        (*this)[i]->removeListener(id);
    }
      
  };

  template <unsigned N>
  class ExprVecImpl : public ExprVec
  {
  public:
    ExprVecImpl(std::vector<ExpressionId> const &exps,
                std::vector<bool> const &garb)
      : ExprVec()
    {
      check_error_1(exps.size() == N && garb.size() == N);
      for (size_t i = 0; i < N; ++i)
        exprs[i] = exps[i];
      for (size_t i = 0; i < N; ++i)
        garbage[i] = garb[i];
    }

    ~ExprVecImpl()
    {
      for (size_t i = 0; i < N; ++i)
        if (garbage[i])
          delete (Expression *) exprs[i];
    }

    size_t size() const { return N; }
    ExpressionId const &operator[](size_t n) const 
    {
      check_error_1(n < N);
      return exprs[n]; 
    }

  private:
    // Not implemented
    ExprVecImpl(const ExprVecImpl &);
    ExprVecImpl &operator=(const ExprVecImpl &);

    ExpressionId exprs[N];
    bool garbage[N];
  };

  // Factory function
  extern ExprVec const *makeExprVec(std::vector<ExpressionId> const &exprs,
                                    std::vector<bool> const &garbage);

} // namespace PLEXIL

#endif // PLEXIL_EXPR_VEC_HH
