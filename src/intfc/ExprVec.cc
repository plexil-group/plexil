/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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
#include "Error.hh"
#include "Expression.hh"
#include "ParserException.hh"

namespace PLEXIL
{

  //
  // FixedExprVec
  //
  // General cases - optimized cases below
  //

  /**
   * @class FixedExprVec
   * @brief Concrete class template for small expression vectors.
   */

  template <unsigned N>
  class FixedExprVec : public ExprVec
  {
  public:
    FixedExprVec()
      : ExprVec()
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i] = nullptr;
      for (size_t i = 0; i < N; ++i)
        garbage[i] = false;
    }

    ~FixedExprVec()
    {
      for (size_t i = 0; i < N; ++i)
        if (exprs[i] && garbage[i])
          delete exprs[i];
    }

    virtual size_t size() const override
    {
      return N; 
    }

    virtual Expression const *operator[](size_t n) const override
    {
      check_error_1(n < N);
      return exprs[n]; 
    }

    virtual Expression *operator[](size_t n) override
    {
      check_error_1(n < N);
      return exprs[n]; 
    }

    virtual void setArgument(size_t i, Expression *exp, bool isGarbage) override
    {
      assertTrue_2(i < N, "setArgument(): too many args");
      exprs[i] = exp;
      garbage[i] = isGarbage;
    }

    virtual void activate() override
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i]->activate();
    }

    virtual void deactivate() override
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i]->deactivate();
    }

    virtual void addListener(ExpressionListener *l) override
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i]->addListener(l);
    }

    virtual void removeListener(ExpressionListener *l) override
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i]->removeListener(l);
    }

    virtual void doSubexprs(ListenableUnaryOperator const &opr) override
    {
      for (size_t i = 0; i < N; ++i)
        (opr)(exprs[i]);
    }

    virtual void print(std::ostream & str) const override
    {
      for (size_t i = 0; i < N; ++i) {
        str << ' ';
        exprs[i]->print(str);
      }
    }

  private:

    // Not implemented
    FixedExprVec(FixedExprVec const &) = delete;
    FixedExprVec(FixedExprVec &&) = delete;
    FixedExprVec &operator=(FixedExprVec const &) = delete;
    FixedExprVec &operator=(FixedExprVec &&) = delete;

    Expression *exprs[N];
    bool garbage[N];
  };

  //
  // GeneralExprVec
  //

  /**
   * @class GeneralExprVec
   * @brief Concrete variable-length variant of ExprVec which uses dynamically allocated arrays.
   */
  class GeneralExprVec : public ExprVec
  {
  public:
    GeneralExprVec(size_t n)
      : ExprVec(),
        m_size(n),
        exprs(new Expression*[n]()),
        garbage(new bool[n]())
    {
    }

    ~GeneralExprVec()
    {
      for (size_t i = 0; i < m_size; ++i)
        if (exprs[i] && garbage[i])
          delete exprs[i];
      delete[] garbage;
      delete[] exprs;
    }

    virtual size_t size() const override
    {
      return m_size; 
    }

    virtual Expression const *operator[](size_t n) const override
    {
      check_error_1(n < m_size);
      return exprs[n]; 
    }

    virtual Expression *operator[](size_t n) override
    {
      check_error_1(n < m_size);
      return exprs[n]; 
    }

    virtual void setArgument(size_t i, Expression *exp, bool isGarbage) override
    {
      assertTrue_2(i < m_size, "setArgument(): too many args");
      exprs[i] = exp;
      garbage[i] = isGarbage;
    }

    virtual void activate() override
    {
      for (size_t i = 0; i < m_size; ++i)
        exprs[i]->activate();
    }
      
    virtual void deactivate() override
    {
      for (size_t i = 0; i < m_size; ++i)
        exprs[i]->deactivate();
    }

    virtual void print(std::ostream & str) const override
    {
      for (size_t i = 0; i < m_size; ++i) {
        str << ' ';
        exprs[i]->print(str);
      }
    }

    virtual void addListener(ExpressionListener *l) override
    {
      for (size_t i = 0; i < m_size; ++i)
        exprs[i]->addListener(l);
    }

    virtual void removeListener(ExpressionListener *l) override
    {
      for (size_t i = 0; i < m_size; ++i)
        exprs[i]->addListener(l);
    }

    virtual void doSubexprs(ListenableUnaryOperator const &opr) override
    {
      for (size_t i = 0; i < m_size; ++i)
        (opr)(exprs[i]);
    }

  private:

    // Not implemented
    GeneralExprVec() = delete;
    GeneralExprVec(GeneralExprVec const &) = delete;
    GeneralExprVec(GeneralExprVec &&) = delete;
    GeneralExprVec &operator=(GeneralExprVec const &) = delete;
    GeneralExprVec &operator=(GeneralExprVec &&) = delete;

    size_t m_size;
    Expression **exprs;
    bool *garbage;
  };

  //
  // Factory function
  //

  ExprVec *makeExprVec(size_t n)
  {
    switch (n) {
    case 0:
      assertTrue_2(false, "makeExprVec: zero-length vector not implemented");
      return nullptr;
      
    case 1:
      return static_cast<ExprVec *>(new FixedExprVec<1>());

    case 2:
      return static_cast<ExprVec *>(new FixedExprVec<2>());

    case 3:
      return static_cast<ExprVec *>(new FixedExprVec<3>());

    case 4:
      return static_cast<ExprVec *>(new FixedExprVec<4>());

    default: // anything greater than 4
      return static_cast<ExprVec *>(new GeneralExprVec(n));
    }
  }

  template class FixedExprVec<1>;
  template class FixedExprVec<2>;
  template class FixedExprVec<3>;
  template class FixedExprVec<4>;

} // namespace PLEXIL
