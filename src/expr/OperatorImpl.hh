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

#ifndef PLEXIL_OPERATOR_IMPL_HH
#define PLEXIL_OPERATOR_IMPL_HH

#include "Operator.hh"

#include "Error.hh"
#include "ExprVec.hh"
#include "Value.hh"

namespace PLEXIL
{

  template <class IMPL>
  class OperatorShim : public Operator
  {
  protected:
    OperatorShim(std::string const &name) : Operator(name) {}

  public:
    virtual ~OperatorShim() {}

    inline bool operator()(bool &result, Expression const *arg) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg);
    }

    inline bool operator()(bool &result, Expression const *arg0, Expression const *arg1) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg0, arg1);
    }

    inline bool operator()(bool &result, ExprVec const &args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    inline bool operator()(int32_t &result, Expression const *arg) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg);
    }

    inline bool operator()(int32_t &result, Expression const *arg0, Expression const *arg1) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg0, arg1);
    }

    inline bool operator()(int32_t &result, ExprVec const &args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    inline bool operator()(double &result, Expression const *arg) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg);
    }

    inline bool operator()(double &result, Expression const *arg0, Expression const *arg1) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg0, arg1);
    }

    inline bool operator()(double &result, ExprVec const &args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    inline bool operator()(std::string &result, Expression const *arg) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg);
    }

    inline bool operator()(std::string &result, Expression const *arg0, Expression const *arg1) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg0, arg1);
    }

    inline bool operator()(std::string &result, ExprVec const &args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    inline bool operator()(Array &result, Expression const *arg) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg);
    }

    inline bool operator()(Array &result, Expression const *arg0, Expression const *arg1) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg0, arg1);
    }

    inline bool operator()(Array &result, ExprVec const &args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    inline bool operator()(BooleanArray &result, Expression const *arg) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg);
    }

    inline bool operator()(BooleanArray &result, Expression const *arg0, Expression const *arg1) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg0, arg1);
    }

    inline bool operator()(BooleanArray &result, ExprVec const &args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    inline bool operator()(IntegerArray &result, Expression const *arg) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg);
    }

    inline bool operator()(IntegerArray &result, Expression const *arg0, Expression const *arg1) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg0, arg1);
    }

    inline bool operator()(IntegerArray &result, ExprVec const &args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    inline bool operator()(RealArray &result, Expression const *arg) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg);
    }

    inline bool operator()(RealArray &result, Expression const *arg0, Expression const *arg1) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg0, arg1);
    }

    inline bool operator()(RealArray &result, ExprVec const &args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    inline bool operator()(StringArray &result, Expression const *arg) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg);
    }

    inline bool operator()(StringArray &result, Expression const *arg0, Expression const *arg1) const
    {
      return static_cast<IMPL const *>(this)->calc(result, arg0, arg1);
    }

    inline bool operator()(StringArray &result, ExprVec const &args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

  };

  template <typename R>
  class OperatorImpl : public OperatorShim<OperatorImpl<R> >
  {
  public:
    virtual ~OperatorImpl() {}

    // Default methods, based on R
    ValueType valueType() const;
    void *allocateCache() const { return static_cast<void *>(new R); }
    void deleteCache(void *ptr) const { delete static_cast<R *>(ptr); }

    bool calcNative(void *cache, ExprVec const &exprs) const;
    void printValue(std::ostream &s, void *cache, ExprVec const &exprs) const;
    Value toValue(void *cache, ExprVec const &exprs) const;

    // Delegated to derived classes
    // Default methods issue "wrong argument count" error
    virtual bool calc(R &result, Expression const *arg) const;
    virtual bool calc(R &result, Expression const *arg0, Expression const *arg1) const;
    virtual bool calc(R &result, ExprVec const &args) const;

    // Conversion or type error
    // *** OS X 10.9.x requires these to be here, instead of the .cc file ***
    template <typename U>
    bool calc(U & /* result */, Expression const */* arg */) const
    {
      assertTrueMsg(ALWAYS_FAIL, "Type error for " << this->getName());
      return false;
    }

    template <typename U>
    bool calc(U & /* result */, Expression const */* arg0 */, Expression const */* arg1 */) const
    {
      assertTrueMsg(ALWAYS_FAIL, "Type error for " << this->getName());
      return false;
    }

    template <typename U>
    bool calc(U & /* result */, ExprVec const & /* args */) const
    {
      assertTrueMsg(ALWAYS_FAIL, "Type error for " << this->getName());
      return false;
    }

  protected:
    // Base class shouldn't be instantiated by itself
    OperatorImpl(std::string const &name)
      : OperatorShim<OperatorImpl<R> >(name)
    {
    }

  private:
    // Unimplemented
    OperatorImpl();
    OperatorImpl(OperatorImpl const &);
    OperatorImpl &operator=(OperatorImpl const &);
  };

  // Specialized conversions for Integer operator to Real
  // *** Must be declared here for OS X 10.9 ***
  template <>
  template <>
  bool OperatorImpl<int32_t>::calc(double &result, Expression const *arg) const;

  template <>
  template <>
  bool OperatorImpl<int32_t>::calc(double &result, Expression const *arg0, Expression const *arg1) const;

  template <>
  template <>
  bool OperatorImpl<int32_t>::calc(double &result, ExprVec const &args) const;

  template <typename R>
  class OperatorImpl<ArrayImpl<R> >
    : public OperatorShim<OperatorImpl<ArrayImpl<R> > >
  {
  public:
    virtual ~OperatorImpl() {}

    // Default methods, based on R
    ValueType valueType() const;
    void *allocateCache() const { return static_cast<void *>(new ArrayImpl<R>); }
    void deleteCache(void *ptr) const { delete static_cast<ArrayImpl<R> *>(ptr); }

    bool calcNative(void *cache, ExprVec const &exprs) const;
    void printValue(std::ostream &s, void *cache, ExprVec const &exprs) const;
    Value toValue(void *cache, ExprVec const &exprs) const;

    // Delegated to derived classes
    virtual bool calc(ArrayImpl<R> &result, Expression const *arg) const = 0;
    virtual bool calc(ArrayImpl<R> &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool calc(ArrayImpl<R> &result, ExprVec const &args) const = 0;

    // Downcast to Array base
    virtual bool calc(Array &result, Expression const *arg) const = 0;
    virtual bool calc(Array &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool calc(Array &result, ExprVec const &args) const = 0;

    // Conversion or type error
    // *** OS X 10.9.x requires these to be here, instead of the .cc file ***
    template <typename U>
    bool calc(U & /* result */, Expression const */* arg */) const
    {
      assertTrueMsg(ALWAYS_FAIL, "Type error for " << this->getName());
      return false;
    }

    template <typename U>
    bool calc(U & /* result */, Expression const */* arg0 */, Expression const */* arg1 */) const
    {
      assertTrueMsg(ALWAYS_FAIL, "Type error for " << this->getName());
      return false;
    }

    template <typename U>
    bool calc(U & /* result */, ExprVec const & /* args */) const
    {
      assertTrueMsg(ALWAYS_FAIL, "Type error for " << this->getName());
      return false;
    }

  protected:
    // Base class shouldn't be instantiated by itself
    OperatorImpl(std::string const &name)
      : OperatorShim<OperatorImpl<R> >(name)
    {
    }

  private:
    // Unimplemented
    OperatorImpl();
    OperatorImpl(OperatorImpl const &);
    OperatorImpl &operator=(OperatorImpl const &);
  };

} // namespace PLEXIL

/**
 * @brief Helper macro, intended to implement "boilerplate" singleton accessors
 *        for classes derived from OperatorImpl<R>.
 */
#define DECLARE_OPERATOR_STATIC_INSTANCE(CLASS, RETURNS) \
  static PLEXIL::Operator const *instance() \
  { \
    static CLASS const sl_instance; \
    return static_cast<PLEXIL::Operator const *>(&sl_instance); \
  }

#endif // PLEXIL_OPERATOR_IMPL_HH
