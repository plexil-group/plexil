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

#include "CachedFunction.hh"
#include "Function.hh"
#include "Operator.hh"
#include "PlanError.hh"

namespace PLEXIL
{

  /**
   * @class CachedFunction
   * @brief Variant of Function with a result cache to implement getValuePointer().
   * @note Required by functions returning anything requiring storage.
   */

  class CachedFunction : public Function
  {
  public:
    virtual ~CachedFunction()
    {
      m_op->deleteCache(m_valueCache);
      m_valueCache = NULL;
    }

    // Local macros for boilerplate
#define DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_METHOD(_type) \
    virtual bool getValue(_type &result) const                          \
    {                                                                   \
      reportPlanError("getValuePointer not implemented for type " << #_type \
                      << " for " << m_op->getName());                   \
      return false; \
    }

    DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_METHOD(Boolean)
    DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_METHOD(Integer)
    DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_METHOD(Real)
    DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_METHOD(uint16_t)

#undef DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_METHOD

    // Strings are legal with CachedFunction
    virtual bool getValue(String &result) const
    {
      return (*m_op)(result, *this);
    }

#define DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(_type) \
    virtual bool getValuePointer(_type const *&ptr) const               \
    {                                                                   \
      bool result = (*m_op)(*static_cast<_type *>(m_valueCache), *this); \
      if (result)                                                       \
        ptr = static_cast<_type const *>(m_valueCache); /* trust me */  \
      return result;                                                    \
    }

    DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(String)
    DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(Array)
    DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(BooleanArray)
    DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(IntegerArray)
    DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(RealArray)
    DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_PTR_METHOD(StringArray)
  
#undef DEFINE_CACHED_FUNC_DEFAULT_GET_VALUE_PTR_METHOD

  protected:

    // Only available to derived classes
    CachedFunction(Operator const *oper)
      : Function(oper),
        m_valueCache(oper->allocateCache())
    {
    }

    // For implementing getValuePointer().
    // Must be a pointer to preserve const-ness.
    // Cache is allocated and deleted by the operator, which knows its size.

    void *m_valueCache;
  };


  //
  // Implementations
  //

  //
  // NullaryCachedFunction is a function which takes no arguments.
  //  E.g. random().
  //

  class NullaryCachedFunction : public CachedFunction
  {
  public:
    NullaryCachedFunction(Operator const *oper)
      : CachedFunction(oper)
    {
    }

    virtual ~NullaryCachedFunction()
    {
    }

    virtual size_t size() const
    {
      return 0;
    }
      
    virtual Expression const *operator[](size_t /* n */) const
    {
      errorMsg("operator[]: no arguments in NullaryCachedFunction");
    }

    virtual void setArgument(size_t i, Expression * /* exp */, bool /* garbage */)
    {
      errorMsg("setArgument(): no arguments to set in NullaryCachedFunction");
    }

    virtual bool allSameTypeOrUnknown(ValueType /* vt */) const
    {
      return true;
    }

    virtual void printSubexpressions(std::ostream & /* s */) const
    {
    }

    virtual void handleActivate()
    {
    }

    virtual void handleDeactivate()
    {
    }

    virtual void doSubexprs(ListenableUnaryOperator const & /* opr */)
    {
    }

  private:

    // Not implemented
    NullaryCachedFunction();
    NullaryCachedFunction(NullaryCachedFunction const &);
    NullaryCachedFunction &operator=(NullaryCachedFunction const &);
  };

  //
  // FixedSizeCachedFunction
  //
  // General cases - optimized cases below
  //

  /**
   * @class FixedSizeCachedFunction
   * @brief Concrete class template for small expression vectors.
   * Allows optimization for common cases (specifically one and two parameter function calls).
   */

  template <unsigned N>
  class FixedSizeCachedFunction : public CachedFunction
  {
  public:
    FixedSizeCachedFunction(Operator const *oper)
      : CachedFunction(oper)
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i] = NULL;
      for (size_t i = 0; i < N; ++i)
        garbage[i] = false;
    }

    ~FixedSizeCachedFunction()
    {
      for (size_t i = 0; i < N; ++i) {
        if (exprs[i]) {
          if (garbage[i])
            delete exprs[i];
        }
      }
    }

    // Not worth optimizing this, it's only used once per function at load time.
    virtual bool allSameTypeOrUnknown(ValueType vtyp) const
    {
      for (size_t i = 0; i < N; ++i) {
        ValueType vti = exprs[i]->valueType();
        if (vti != vtyp && vti != UNKNOWN_TYPE)
          return false;
      }
      return true;
    }

    virtual size_t size() const
    {
      return N;
    }

    virtual Expression const *operator[](size_t n) const
    {
      check_error_1(n < N);
      return exprs[n]; 
    }

    void setArgument(size_t i, Expression *exp, bool isGarbage)
    {
      exprs[i] = exp;
      garbage[i] = isGarbage;
    }

    virtual void handleActivate()
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i]->activate();
    }

    virtual void handleDeactivate()
    {
      for (size_t i = 0; i < N; ++i)
        exprs[i]->deactivate();
    }

    virtual void printSubexpressions(std::ostream & str) const
    {
      for (size_t i = 0; i < N; ++i) {
        str << ' ';
        exprs[i]->print(str);
      }
    }

    // Have to define these so specialized template functions can be defined below

    virtual bool getValue(String &result) const
    {
      return (*m_op)(result, *this);
    }

#define DEFINE_FIXED_ARG_CACHED_GET_VALUE_PTR_METHOD(_type) \
    virtual bool getValuePointer(_type const *&ptr) const   \
    {                                                                   \
      bool result = (*m_op)(*static_cast<_type *>(m_valueCache), this); \
      if (result)                                                       \
        ptr = static_cast<_type const *>(m_valueCache); /* trust me */  \
      return result;                                                    \
    }

    DEFINE_FIXED_ARG_CACHED_GET_VALUE_PTR_METHOD(String)
    DEFINE_FIXED_ARG_CACHED_GET_VALUE_PTR_METHOD(Array)
    DEFINE_FIXED_ARG_CACHED_GET_VALUE_PTR_METHOD(BooleanArray)
    DEFINE_FIXED_ARG_CACHED_GET_VALUE_PTR_METHOD(IntegerArray)
    DEFINE_FIXED_ARG_CACHED_GET_VALUE_PTR_METHOD(RealArray)
    DEFINE_FIXED_ARG_CACHED_GET_VALUE_PTR_METHOD(StringArray)

#undef DEFINE_FIXED_ARG_CACHED_GET_VALUE_PTR_METHOD

    // Default method, overridden in specialized variants
    virtual bool apply(Operator const *oper, Array &result) const
    {
      return (*oper)(result, this);
    }

    // Default method, overridden in specialized variants
    virtual void doSubexprs(ListenableUnaryOperator const &oper)
    {
      for (size_t i = 0; i < N; ++i)
        (oper)(exprs[i]);
    }

  private:

    // Not implemented
    FixedSizeCachedFunction();
    FixedSizeCachedFunction(const FixedSizeCachedFunction &);
    FixedSizeCachedFunction &operator=(const FixedSizeCachedFunction &);

    Expression *exprs[N];
    bool garbage[N];
  };

  // One-arg variants

  template <>
  FixedSizeCachedFunction<1>::FixedSizeCachedFunction(Operator const *oper)
    : CachedFunction(oper)
  {
    exprs[0] = NULL;
    garbage[0] = false;
  }

  template <>
  FixedSizeCachedFunction<1>::~FixedSizeCachedFunction()
  {
    if (exprs[0]) {
      if (garbage[0])
        delete exprs[0];
    }
  }

  template <>
  void FixedSizeCachedFunction<1>::handleActivate()
  {
    exprs[0]->activate();
  }

  template <>
  void FixedSizeCachedFunction<1>::handleDeactivate()
  {
    exprs[0]->deactivate();
  }

  template <>
  bool FixedSizeCachedFunction<1>::getValue(String &result) const
  {
    return (*m_op)(result, exprs[0]);
  }

#define DEFINE_ONE_ARG_CACHED_GET_VALUE_PTR_METHOD(_type) \
  template <> bool FixedSizeCachedFunction<1>::getValuePointer(_type const *&ptr) const \
  {                                                                     \
    bool result = (*m_op)(*static_cast<_type *>(m_valueCache), exprs[0]); \
    if (result)                                                         \
      ptr = static_cast<_type const *>(m_valueCache); /* trust me */    \
    return result; \
  }

  DEFINE_ONE_ARG_CACHED_GET_VALUE_PTR_METHOD(String)
  DEFINE_ONE_ARG_CACHED_GET_VALUE_PTR_METHOD(Array)
  DEFINE_ONE_ARG_CACHED_GET_VALUE_PTR_METHOD(BooleanArray)
  DEFINE_ONE_ARG_CACHED_GET_VALUE_PTR_METHOD(IntegerArray)
  DEFINE_ONE_ARG_CACHED_GET_VALUE_PTR_METHOD(RealArray)
  DEFINE_ONE_ARG_CACHED_GET_VALUE_PTR_METHOD(StringArray)
  
#undef DEFINE_ONE_ARG_CACHED_GET_VALUE_PTR_METHOD

  // Specialized method
  template <>
  bool FixedSizeCachedFunction<1>::apply(Operator const *oper, Array &result) const
  {
    return (*oper)(result, exprs[0]);
  }

  // Specialized method
  template <>
  void FixedSizeCachedFunction<1>::doSubexprs(ListenableUnaryOperator const &oper)
  {
    (oper)(exprs[0]);
  }

  //
  // Two-arg variants
  //

  template <>
  FixedSizeCachedFunction<2>::FixedSizeCachedFunction(Operator const *oper)
    : CachedFunction(oper)
  {
    exprs[0] = NULL;
    exprs[1] = NULL;
    garbage[0] = false;
    garbage[1] = false;
  }

  template <>
  FixedSizeCachedFunction<2>::~FixedSizeCachedFunction()
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
  void FixedSizeCachedFunction<2>::handleActivate() 
  {
    exprs[0]->activate();
    exprs[1]->activate();
  }

  template <>
  void FixedSizeCachedFunction<2>::handleDeactivate()
  {
    exprs[0]->deactivate();
    exprs[1]->deactivate();
  }

  template <>
  bool FixedSizeCachedFunction<2>::getValue(String &result) const
  {
    return (*m_op)(result, exprs[0], exprs[1]);
  }

#define DEFINE_TWO_ARG_CACHED_GET_VALUE_PTR_METHOD(_type) \
  template <> bool FixedSizeCachedFunction<2>::getValuePointer(_type const *&ptr) const \
  { \
    bool result = (*m_op)(*static_cast<_type *>(m_valueCache), exprs[0], exprs[1]); \
    if (result) \
      ptr = static_cast<_type const *>(m_valueCache); /* trust me */ \
    return result; \
  }

  DEFINE_TWO_ARG_CACHED_GET_VALUE_PTR_METHOD(String)
  DEFINE_TWO_ARG_CACHED_GET_VALUE_PTR_METHOD(Array)
  DEFINE_TWO_ARG_CACHED_GET_VALUE_PTR_METHOD(BooleanArray)
  DEFINE_TWO_ARG_CACHED_GET_VALUE_PTR_METHOD(IntegerArray)
  DEFINE_TWO_ARG_CACHED_GET_VALUE_PTR_METHOD(RealArray)
  DEFINE_TWO_ARG_CACHED_GET_VALUE_PTR_METHOD(StringArray)
  
#undef DEFINE_TWO_ARG_CACHED_GET_VALUE_PTR_METHOD

  // Specialized method
  template <>
  bool FixedSizeCachedFunction<2>::apply(Operator const *oper, Array &result) const
  {
    return (*oper)(result, exprs[0], exprs[1]);
  }

  // Specialized method
  template <>
  void FixedSizeCachedFunction<2>::doSubexprs(ListenableUnaryOperator const &oper)
  {
    (oper)(exprs[0]);
    (oper)(exprs[1]);
  }

  //
  // NaryCachedFunction
  //

  /**
   * @class NaryCachedFunction
   * @brief Concrete variable-length variant of CachedFunction which uses dynamically allocated arrays.
   */
  class NaryCachedFunction : public CachedFunction
  {
  public:
    NaryCachedFunction(Operator const *oper, size_t n)
      : CachedFunction(oper),
        m_size(n),
        exprs(new Expression*[n]()),
        garbage(new bool[n]())
    {
    }

    ~NaryCachedFunction()
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

    virtual size_t size() const
    {
      return m_size;
    }

    virtual Expression const *operator[](size_t n) const
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

    virtual bool allSameTypeOrUnknown(ValueType vtyp) const
    {
      for (size_t i = 0; i < m_size; ++i) {
        ValueType vti = exprs[i]->valueType();
        if (vti != vtyp && vti != UNKNOWN_TYPE)
          return false;
      }
      return true;
    }

    void handleActivate()
    {
      for (size_t i = 0; i < m_size; ++i)
        exprs[i]->activate();
    }
      
    void handleDeactivate()
    {
      for (size_t i = 0; i < m_size; ++i)
        exprs[i]->deactivate();
    }

    void printSubexpressions(std::ostream & str) const
    {
      for (size_t i = 0; i < m_size; ++i) {
        str << ' ';
        exprs[i]->print(str);
      }
    }

    virtual void doSubexprs(ListenableUnaryOperator const &oper)
    {
      for (size_t i = 0; i < this->size(); ++i)
        (oper)(exprs[i]);
    }

  private:
    // Not implemented
    NaryCachedFunction();
    NaryCachedFunction(NaryCachedFunction const &);
    NaryCachedFunction &operator=(NaryCachedFunction const &);

    size_t m_size;
    Expression **exprs;
    bool *garbage;
  };


  //
  // Public API
  //
  
  Function *makeCachedFunction(Operator const *oper,
                               size_t n)
  {
    assertTrue_2(oper, "makeCachedFunction: null operator");

    switch (n) {
    case 0:
      return static_cast<CachedFunction *>(new NullaryCachedFunction(oper));
    case 1:
      return static_cast<CachedFunction *>(new FixedSizeCachedFunction<1>(oper));
    case 2:
      return static_cast<CachedFunction *>(new FixedSizeCachedFunction<2>(oper));
    case 3:
      return static_cast<CachedFunction *>(new FixedSizeCachedFunction<3>(oper));
    case 4:
      return static_cast<CachedFunction *>(new FixedSizeCachedFunction<4>(oper));
    default: // anything greater than 4
      return static_cast<CachedFunction *>(new NaryCachedFunction(oper, n));
    }
  }

  Function *makeCachedFunction(Operator const *oper,
                               Expression *expr,
                               bool garbage)
  {
    assertTrue_2(oper && expr, "makeCachedFunction: operator or argument is null");
    CachedFunction *result = new FixedSizeCachedFunction<1>(oper);
    result->setArgument(0, expr, garbage);
    return result;
  }

  Function *makeCachedFunction(Operator const *oper, 
                               Expression *expr1,
                               Expression *expr2,
                               bool garbage1,
                               bool garbage2)
  {
    assertTrue_2(oper && expr1 && expr2, "makeCachedFunction: operator or argument is null");
    CachedFunction *result = new FixedSizeCachedFunction<2>(oper);
    result->setArgument(0, expr1, garbage1);
    result->setArgument(1, expr2, garbage2);
    return result;
  }
  
}
