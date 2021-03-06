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

#include "Function.hh"

#include "ArrayImpl.hh"
#include "Error.hh"
#include "Operator.hh"
#include "PlanError.hh"
#include "Value.hh"

namespace PLEXIL
{
  Function::Function(Operator const *oper)
    : Propagator(),
      m_op(oper)
  {
  }

  Function::~Function()
  {
  }

  const char *Function::exprName() const
  {
    return m_op->getName().c_str();
  }

  ValueType Function::valueType() const
  {
    return m_op->valueType();
  }

  bool Function::isKnown() const
  {
    return m_op->isKnown(*this);
  }

  void Function::printValue(std::ostream &str) const
  {
    m_op->printValue(str, *this);
  }

  Value Function::toValue() const
  {
    return m_op->toValue(*this);
  }

  bool Function::isPropagationSource() const
  {
    return m_op->isPropagationSource();
  }

  // Local macro for boilerplate
#define DEFINE_FUNC_DEFAULT_GET_VALUE_METHOD(_type) \
  bool Function::getValue(_type &result) const \
  { \
    return (*m_op)(result, *this); \
  }

  DEFINE_FUNC_DEFAULT_GET_VALUE_METHOD(Boolean)
  DEFINE_FUNC_DEFAULT_GET_VALUE_METHOD(Integer)
  DEFINE_FUNC_DEFAULT_GET_VALUE_METHOD(Real)
  DEFINE_FUNC_DEFAULT_GET_VALUE_METHOD(String)

  DEFINE_FUNC_DEFAULT_GET_VALUE_METHOD(NodeState)
  DEFINE_FUNC_DEFAULT_GET_VALUE_METHOD(NodeOutcome)
  DEFINE_FUNC_DEFAULT_GET_VALUE_METHOD(FailureType)
  DEFINE_FUNC_DEFAULT_GET_VALUE_METHOD(CommandHandleValue)

#undef DEFINE_FUNC_DEFAULT_GET_VALUE_METHOD

  // These types must use CachedFunction instead

#define DEFINE_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(_type) \
  bool Function::getValuePointer(_type const *&ptr) const \
  { \
    reportPlanError("getValuePointer not implemented for type " << #_type \
                    << " for " << m_op->getName());                     \
    return false;                                                       \
  }

  DEFINE_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(String)
  DEFINE_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(Array)
  DEFINE_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(BooleanArray)
  DEFINE_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(IntegerArray)
  DEFINE_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(RealArray)
  DEFINE_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(StringArray)
  
#undef DEFINE_FUNC_DEFAULT_GET_VALUE_PTR_METHOD
  
  // Default method
  bool Function::apply(Operator const *opr, Array &result) const
  {
    return (*opr)(result, *this);
  }

  //
  // NullaryFunction is a function which takes no arguments.
  //  E.g. random().
  //

  class NullaryFunction final : public Function
  {
  public:
    NullaryFunction(Operator const *oper)
      : Function(oper)
    {
    }

    virtual ~NullaryFunction()
    {
    }

    virtual size_t size() const override
    {
      return 0;
    }
      
    virtual Expression const *operator[](size_t /* n */) const override
    {
      errorMsg("operator[]: no arguments in NullaryFunction");
    }

    virtual void setArgument(size_t i, Expression * /* exp */, bool /* garbage */) override
    {
      errorMsg("setArgument(): no arguments to set in NullaryFunction");
    }

    virtual void printSubexpressions(std::ostream & /* s */) const override
    {
    }

    virtual void handleActivate() override
    {
    }

    virtual void handleDeactivate() override
    {
    }

    virtual void doSubexprs(ListenableUnaryOperator const & /* opr */) override
    {
    }

  private:

    // Not implemented
    NullaryFunction() = delete;
    NullaryFunction(NullaryFunction const &) = delete;
    NullaryFunction(NullaryFunction &&) = delete;
    NullaryFunction &operator=(NullaryFunction const &) = delete;
    NullaryFunction &operator=(NullaryFunction &&) = delete;
  };

  //
  // FixedSizeFunction
  //
  // General cases - optimized cases below
  //

  /**
   * @class FixedSizeFunction
   * @brief Concrete class template for small expression vectors.
   * Allows optimization for common cases (specifically one and two parameter function calls).
   */

  template <unsigned N>
  class FixedSizeFunction final : public Function
  {
  public:
    FixedSizeFunction(Operator const *oper)
      : Function(oper)
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i] = nullptr;
      for (size_t i = 0; i < N; ++i)
        garbage[i] = false;
    }

    ~FixedSizeFunction()
    {
      for (size_t i = 0; i < N; ++i) {
        if (exprs[i]) {
          if (garbage[i])
            delete exprs[i];
        }
      }
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

    virtual void setArgument(size_t i, Expression *exp, bool isGarbage) override
    {
      exprs[i] = exp;
      garbage[i] = isGarbage;
    }

    virtual void handleActivate() override
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i]->activate();
    }

    virtual void handleDeactivate() override
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i]->deactivate();
    }

    virtual void printSubexpressions(std::ostream & str) const override
    {
      for (size_t i = 0; i < N; ++i) {
        str << ' ';
        exprs[i]->print(str);
      }
    }

    // Have to define this so specialized template functions can be defined below
#define DEFINE_FIXED_ARG_GET_VALUE_METHOD(_type) \
  virtual bool getValue(_type &result) const override \
  { \
    return (*m_op)(result, *this); \
  }

    DEFINE_FIXED_ARG_GET_VALUE_METHOD(Boolean)
    DEFINE_FIXED_ARG_GET_VALUE_METHOD(Integer)
    DEFINE_FIXED_ARG_GET_VALUE_METHOD(Real)
    DEFINE_FIXED_ARG_GET_VALUE_METHOD(String)

    // Use base class method for now
    // DEFINE_FIXED_ARG_GET_VALUE_METHOD(NodeState)
    // DEFINE_FIXED_ARG_GET_VALUE_METHOD(NodeOutcome)
    // DEFINE_FIXED_ARG_GET_VALUE_METHOD(FailureType)
    // DEFINE_FIXED_ARG_GET_VALUE_METHOD(CommandHandleValue)

#undef DEFINE_FIXED_ARG_GET_VALUE_METHOD

    // Default method, overridden in specialized variants
    virtual bool apply(Operator const *oper, Array &result) const override
    {
      return (*oper)(result, this);
    }

    // Default method, overridden in specialized variants
    virtual void doSubexprs(ListenableUnaryOperator const &oper) override
    {
      for (size_t i = 0; i < N; ++i)
        (oper)(exprs[i]);
    }

  private:

    // Not implemented
    FixedSizeFunction() = delete;
    FixedSizeFunction(const FixedSizeFunction &) = delete;
    FixedSizeFunction(FixedSizeFunction &&) = delete;
    FixedSizeFunction &operator=(const FixedSizeFunction &) = delete;
    FixedSizeFunction &operator=(FixedSizeFunction &&) = delete;

    Expression *exprs[N];
    bool garbage[N];
  };

  // One-arg variants

  template <>
  FixedSizeFunction<1>::FixedSizeFunction(Operator const *oper)
    : Function(oper)
  {
    exprs[0] = nullptr;
    garbage[0] = false;
  }

  template <>
  FixedSizeFunction<1>::~FixedSizeFunction()
  {
    if (exprs[0]) {
      if (garbage[0])
        delete exprs[0];
    }
  }

  template <>
  void FixedSizeFunction<1>::handleActivate()
  {
    exprs[0]->activate();
  }

  template <>
  void FixedSizeFunction<1>::handleDeactivate()
  {
    exprs[0]->deactivate();
  }

  // Local macro for boilerplate
#define DEFINE_ONE_ARG_GET_VALUE_METHOD(_type) \
  template <> bool FixedSizeFunction<1>::getValue(_type &result) const \
  { \
    return (*m_op)(result, exprs[0]); \
  }

  DEFINE_ONE_ARG_GET_VALUE_METHOD(Boolean)
  DEFINE_ONE_ARG_GET_VALUE_METHOD(Integer)
  DEFINE_ONE_ARG_GET_VALUE_METHOD(Real)

  // Use base class method for now
  // DEFINE_ONE_ARG_GET_VALUE_METHOD(uint16_t)

#undef DEFINE_ONE_ARG_GET_VALUE_METHOD

  // Specialized method
  template <>
  bool FixedSizeFunction<1>::apply(Operator const *oper, Array &result) const
  {
    return (*oper)(result, exprs[0]);
  }

  // Specialized method
  template <>
  void FixedSizeFunction<1>::doSubexprs(ListenableUnaryOperator const &oper)
  {
    (oper)(exprs[0]);
  }

  //
  // Two-arg variants
  //

  template <>
  FixedSizeFunction<2>::FixedSizeFunction(Operator const *oper)
    : Function(oper)
  {
    exprs[0] = nullptr;
    exprs[1] = nullptr;
    garbage[0] = false;
    garbage[1] = false;
  }

  template <>
  FixedSizeFunction<2>::~FixedSizeFunction()
  {
    if (exprs[0]) {
      if (garbage[0])
        delete exprs[0];
    }
    if (exprs[1]) {
      if (garbage[1])
        delete exprs[1];
    }
  }

  template <>
  void FixedSizeFunction<2>::handleActivate()
  {
    exprs[0]->activate();
    exprs[1]->activate();
  }

  template <>
  void FixedSizeFunction<2>::handleDeactivate()
  {
    exprs[0]->deactivate();
    exprs[1]->deactivate();
  }

  // Local macro for boilerplate
#define DEFINE_TWO_ARG_GET_VALUE_METHOD(_type) \
  template <> bool FixedSizeFunction<2>::getValue(_type &result) const  \
  { \
    return (*m_op)(result, exprs[0], exprs[1]); \
  }

  DEFINE_TWO_ARG_GET_VALUE_METHOD(Boolean)
  DEFINE_TWO_ARG_GET_VALUE_METHOD(Integer)
  DEFINE_TWO_ARG_GET_VALUE_METHOD(Real)

  // Use base class method for now 
  // DEFINE_TWO_ARG_GET_VALUE_METHOD(uint16_t)

#undef DEFINE_TWO_ARG_GET_VALUE_METHOD

  // Specialized method
  template <>
  bool FixedSizeFunction<2>::apply(Operator const *oper, Array &result) const
  {
    return (*oper)(result, exprs[0], exprs[1]);
  }

  // Specialized method
  template <>
  void FixedSizeFunction<2>::doSubexprs(ListenableUnaryOperator const &oper)
  {
    (oper)(exprs[0]);
    (oper)(exprs[1]);
  }

  //
  // NaryFunction
  //

  /**
   * @class NaryFunction
   * @brief Concrete variable-length variant of Function which uses dynamically allocated arrays.
   */
  class NaryFunction final : public Function
  {
  public:
    NaryFunction(Operator const *oper, size_t n)
      : Function(oper),
        m_size(n),
        exprs(new Expression*[n]()),
        garbage(new bool[n]())
    {
    }

    ~NaryFunction()
    {
      for (size_t i = 0; i < m_size; ++i) {
        if (exprs[i]) {
          if (garbage[i])
            delete exprs[i];
        }
      }
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

    virtual void setArgument(size_t i, Expression *exp, bool isGarbage) override
    {
      assertTrue_2(i < m_size, "setArgument(): too many args");
      exprs[i] = exp;
      garbage[i] = isGarbage;
    }

    virtual void handleActivate() override
    {
      for (size_t i = 0; i < m_size; ++i)
        exprs[i]->activate();
    }
      
    virtual void handleDeactivate() override
    {
      for (size_t i = 0; i < m_size; ++i)
        exprs[i]->deactivate();
    }

    void printSubexpressions(std::ostream & str) const override
    {
      for (size_t i = 0; i < m_size; ++i) {
        str << ' ';
        exprs[i]->print(str);
      }
    }

    virtual void doSubexprs(ListenableUnaryOperator const &oper) override
    {
      for (size_t i = 0; i < this->size(); ++i)
        (oper)(exprs[i]);
    }

  private:
    // Not implemented
    NaryFunction() = delete;
    NaryFunction(NaryFunction const &) = delete;
    NaryFunction(NaryFunction &&) = delete;
    NaryFunction &operator=(NaryFunction const &) = delete;
    NaryFunction &operator=(NaryFunction &&) = delete;

    size_t m_size;
    Expression **exprs;
    bool *garbage;
  };

  //
  // Factory functions
  //
  
  Function *makeFunction(Operator const *oper,
                         size_t n)
  {
    assertTrue_2(oper, "makeFunction: null operator");

    switch (n) {
    case 0:
      return static_cast<Function *>(new NullaryFunction(oper));
    case 1:
      return static_cast<Function *>(new FixedSizeFunction<1>(oper));
    case 2:
      return static_cast<Function *>(new FixedSizeFunction<2>(oper));
    case 3:
      return static_cast<Function *>(new FixedSizeFunction<3>(oper));
    case 4:
      return static_cast<Function *>(new FixedSizeFunction<4>(oper));
    default: // anything greater than 4
      return static_cast<Function *>(new NaryFunction(oper, n));
    }
  }

  Function *makeFunction(Operator const *oper,
                         Expression *expr,
                         bool garbage)
  {
    assertTrue_2(oper && expr, "makeFunction: operator or argument is null");
    Function *result = new FixedSizeFunction<1>(oper);
    result->setArgument(0, expr, garbage);
    return result;
  }

  Function *makeFunction(Operator const *oper, 
                         Expression *expr1,
                         Expression *expr2,
                         bool garbage1,
                         bool garbage2)
  {
    assertTrue_2(oper && expr1 && expr2, "makeFunction: operator or argument is null");
    Function *result = new FixedSizeFunction<2>(oper);
    result->setArgument(0, expr1, garbage1);
    result->setArgument(1, expr2, garbage2);
    return result;
  }

} // namespace PLEXIL
