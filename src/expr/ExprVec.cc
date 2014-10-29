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
#include "Error.hh"
#include "Expression.hh"
#include "Operator.hh"
#include "ParserException.hh"

namespace PLEXIL
{
  //
  // ExprVec base class methods
  //
  ExprVec::~ExprVec()
  {
  }

  void ExprVec::addListener(ExpressionListener * ptr)
  {
    for (size_t i = 0; i < this->size(); ++i)
      (*this)[i]->addListener(ptr);
  }

  void ExprVec::removeListener(ExpressionListener * ptr)
  {
    for (size_t i = 0; i < this->size(); ++i)
      (*this)[i]->removeListener(ptr);
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

  /**
   * @class NullExprVec
   * @brief Concrete class template for empty expression vectors.
   */

  class NullExprVec : public ExprVec
  {
  public:
    NullExprVec()
    {
    }

    ~NullExprVec()
    {
    }

    size_t size() const
    {
      return 0; 
    }

    Expression const *operator[](size_t /* n */) const
    {
      return NULL;
    }

    Expression *operator[](size_t /* n */)
    {
      return NULL;
    }

    void setArgument(size_t /* n */, Expression * /* exp */, bool /* garbage */)
    {
      assertTrue_2(ALWAYS_FAIL, "setArgument(): no arguments to set in NullExprVec");
    }

    void activate()
    {
    }

    void deactivate()
    {
    }

    void addListener(ExpressionListener * /* ptr */) 
    {
    }

    void removeListener(ExpressionListener * /* id */) 
    {
    }

    void print(std::ostream & /* s */) const
    {
    }

  private:
    // Not implemented
    NullExprVec(const NullExprVec &);
    NullExprVec &operator=(const NullExprVec &);
  };

  //
  // FixedExprVec
  //
  // General cases - optimized cases below
  //

  /**
   * @class FixedExprVec
   * @brief Concrete class template for small expression vectors.
   * Allows optimization for common cases (specifically one and two parameter function calls).
   */

  template <unsigned N>
  class FixedExprVec : public ExprVec
  {
  public:
    FixedExprVec()
      : ExprVec()
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i] = NULL;
      for (size_t i = 0; i < N; ++i)
        garbage[i] = false;
    }

    void setArgument(size_t i, Expression *exp, bool isGarbage)
    {
      assertTrue_2(i < N, "setArgument(): too many args");
      exprs[i] = exp;
      garbage[i] = isGarbage;
    }

    ~FixedExprVec()
    {
      for (size_t i = 0; i < N; ++i)
        if (garbage[i])
          delete exprs[i];
    }

    size_t size() const 
    {
      return N; 
    }

    Expression const *operator[](size_t n) const
    {
      check_error_1(n < N);
      return exprs[n]; 
    }

    Expression *operator[](size_t n)
    {
      check_error_1(n < N);
      return exprs[n]; 
    }

    void activate()
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i]->activate();
    }

    void deactivate()
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i]->deactivate();
    }

    // General case defers to base class for many of these operations
    void addListener(ExpressionListener * ptr) 
    {
      ExprVec::addListener(ptr);
    }

    void removeListener(ExpressionListener * ptr) 
    {
      ExprVec::removeListener(ptr);
    }

    void print(std::ostream & s) const
    {
      for (size_t i = 0; i < N; ++i) {
        s << ' ';
        exprs[i]->print(s);
      }
    }

    bool apply(Operator const *op, bool &result) const
    {
      return ExprVec::apply(op, result); 
    }

    bool apply(Operator const *op, int32_t &result) const
    {
      return ExprVec::apply(op, result); 
    }

    bool apply(Operator const *op, double &result) const
    {
      return ExprVec::apply(op, result); 
    }

    bool apply(Operator const *op, std::string &result) const
    {
      return ExprVec::apply(op, result); 
    }

    bool apply(Operator const *op, Array &result) const
    {
      return ExprVec::apply(op, result); 
    }

    bool apply(Operator const *op, BooleanArray &result) const
    {
      return ExprVec::apply(op, result); 
    }

    bool apply(Operator const *op, IntegerArray &result) const
    {
      return ExprVec::apply(op, result); 
    }

    bool apply(Operator const *op, RealArray &result) const
    {
      return ExprVec::apply(op, result); 
    }

    bool apply(Operator const *op, StringArray &result) const
    {
      return ExprVec::apply(op, result); 
    }

  private:
    // Not implemented
    FixedExprVec(const FixedExprVec &);
    FixedExprVec &operator=(const FixedExprVec &);

    Expression *exprs[N];
    bool garbage[N];
  };

  // One-arg variants

  template <>
  FixedExprVec<1>::FixedExprVec()
    : ExprVec()
  {
    exprs[0] = NULL;
    garbage[0] = false;
  }

  template <>
  FixedExprVec<1>::~FixedExprVec()
  {
    if (garbage[0])
      delete exprs[0];
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
  void FixedExprVec<1>::addListener(ExpressionListener * ptr)
  {
    exprs[0]->addListener(ptr);
  }

  template <>
  void FixedExprVec<1>::removeListener(ExpressionListener * ptr)
  {
    exprs[0]->removeListener(ptr);
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
 
  template <>
  void FixedExprVec<1>::print(std::ostream & s) const
  {
      s << ' ';
      exprs[0]->print(s);
  }


  //
  // Two-arg variants
  //

  template <>
  FixedExprVec<2>::FixedExprVec()
    : ExprVec()
  {
    exprs[0] = NULL;
    exprs[1] = NULL;
    garbage[0] = false;
    garbage[1] = false;
  }

  template <>
  FixedExprVec<2>::~FixedExprVec()
  {
    if (garbage[0])
      delete exprs[0];
    if (garbage[1])
      delete exprs[1];
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
  void FixedExprVec<2>::addListener(ExpressionListener * ptr)
  {
    exprs[0]->addListener(ptr);
    exprs[1]->addListener(ptr);
  }

  template <>
  void FixedExprVec<2>::removeListener(ExpressionListener * ptr)
  {
    exprs[0]->removeListener(ptr);
    exprs[1]->removeListener(ptr);
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
 
  template <>
  void FixedExprVec<2>::print(std::ostream & s) const
  {
      s << ' ';
      exprs[0]->print(s);
      s << ' ';
      exprs[1]->print(s);
  }

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
        if (garbage[i])
          delete exprs[i];
      delete[] garbage;
      delete[] exprs;
    }

    size_t size() const
    {
      return m_size; 
    }

    Expression const *operator[](size_t n) const
    {
      check_error_1(n < m_size);
      return exprs[n]; 
    }

    Expression *operator[](size_t n)
    {
      check_error_1(n < m_size);
      return exprs[n]; 
    }

    void setArgument(size_t i, Expression *exp, bool isGarbage)
    {
      assertTrue_2(i < m_size, "setArgument(): too many args");
      exprs[i] = exp;
      garbage[i] = isGarbage;
    }

    void activate()
    {
      for (size_t i = 0; i < m_size; ++i)
        exprs[i]->activate();
    }
      
    void deactivate()
    {
      for (size_t i = 0; i < m_size; ++i)
        exprs[i]->deactivate();
    }

    void print(std::ostream & s) const
    {
      for (size_t i = 0; i < m_size; ++i) {
        s << ' ';
        exprs[i]->print(s);
      }
    }

  private:
    // Not implemented
    GeneralExprVec();
    GeneralExprVec(const GeneralExprVec &);
    GeneralExprVec &operator=(const GeneralExprVec &);

    size_t m_size;
    Expression **exprs;
    bool *garbage;
  };

  //
  // Factory function
  //

  ExprVec *makeExprVec(std::vector<Expression *> const &exprs,
                       std::vector<bool> const &garbage)
  {
    size_t n = exprs.size();
    checkParserException(n == garbage.size(),
                         "makeExprVec: expression and garbage vectors of different lengths");
    ExprVec *result = makeExprVec(n);
    for (size_t i = 0; i < n; ++i)
      result->setArgument(i, exprs[i], garbage[i]);
    return result;
  }
  
  ExprVec *makeExprVec(size_t n)
  {
    switch (n) {
    case 0:
      return static_cast<ExprVec *>(new NullExprVec());
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
