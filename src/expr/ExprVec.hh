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

#include "ArrayFwd.hh"
#include "Id.hh"

#include <string>
#include <vector>

namespace PLEXIL
{
  class Expression;
  DECLARE_ID(Expression);

  class ExpressionListener;
  DECLARE_ID(ExpressionListener);

  class Operator;

  /**
   * @class ExprVec
   * @brief Virtual base class for a family of expression vector classes,
   * whose representations vary by size.
   */

  class ExprVec
  {
  public:
    ExprVec() {}
    virtual ~ExprVec() {}
    virtual size_t size() const = 0;
    virtual ExpressionId const &operator[](size_t n) const = 0;

    // These are in critical path of exec inner loop, 
    // so should be optimized for each representation
    virtual void activate() = 0;
    virtual void deactivate() = 0;

    // Default methods, derived classes can use these
    virtual void addListener(ExpressionListenerId);
    virtual void removeListener(ExpressionListenerId);

    virtual bool apply(Operator const *op, bool &result) const;
    virtual bool apply(Operator const *op, int32_t &result) const;
    virtual bool apply(Operator const *op, double &result) const;
    virtual bool apply(Operator const *op, std::string &result) const;
    virtual bool apply(Operator const *op, Array &result) const;
    virtual bool apply(Operator const *op, BooleanArray &result) const;
    virtual bool apply(Operator const *op, IntegerArray &result) const;
    virtual bool apply(Operator const *op, RealArray &result) const;
    virtual bool apply(Operator const *op, StringArray &result) const;
  };

  /**
   * @class FixedExprVec
   * @brief Concrete class template for small expression vectors.
   * Allows optimization for common cases (specifically one and two parameter function calls).
   */

  template <unsigned N>
  class FixedExprVec : public ExprVec
  {
  public:
    FixedExprVec(std::vector<ExpressionId> const &exps,
                std::vector<bool> const &garb);
    ~FixedExprVec();

    size_t size() const { return N; }
    ExpressionId const &operator[](size_t n) const;

    void activate();
    void deactivate();

    // General case defers to base class for many of these operations
    void addListener(ExpressionListenerId id) 
    {
      ExprVec::addListener(id); 
    }

    void removeListener(ExpressionListenerId id) 
    {
      ExprVec::removeListener(id); 
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

    ExpressionId exprs[N];
    bool garbage[N];
  };

  /**
   * @class GeneralExprVec
   * @brief Concrete variable-length variant of ExprVec which uses std::vector instead of arrays.
   */
  class GeneralExprVec : public ExprVec
  {
  public:
    GeneralExprVec(std::vector<ExpressionId> const &exps,
                   std::vector<bool> const &garb);
    ~GeneralExprVec();

    size_t size() const { return exprs.size(); }
    ExpressionId const &operator[](size_t n) const;
    void activate();
    void deactivate();

    // Defer to base class for these
    // void addListener(ExpressionListenerId id);
    // void removeListener(ExpressionListenerId id);

    // bool apply(Operator const *op, bool &result) const;
    // bool apply(Operator const *op, int32_t &result) const;
    // bool apply(Operator const *op, double &result) const;
    // bool apply(Operator const *op, std::string &result) const;
    // bool apply(Operator const *op, Array &result) const;
    // bool apply(Operator const *op, BooleanArray &result) const;
    // bool apply(Operator const *op, IntegerArray &result) const;
    // bool apply(Operator const *op, RealArray &result) const;
    // bool apply(Operator const *op, StringArray &result) const;

  private:
    // Not implemented
    GeneralExprVec(const GeneralExprVec &);
    GeneralExprVec &operator=(const GeneralExprVec &);

    std::vector<ExpressionId> exprs;
    std::vector<bool> garbage;
  };

  // Factory function
  extern ExprVec *makeExprVec(std::vector<ExpressionId> const &exprs,
                              std::vector<bool> const &garbage);

} // namespace PLEXIL

#endif // PLEXIL_EXPR_VEC_HH
