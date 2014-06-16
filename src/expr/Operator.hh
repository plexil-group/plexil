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

#ifndef PLEXIL_OPERATOR_HH
#define PLEXIL_OPERATOR_HH

#include "ArrayFwd.hh"
#include "Id.hh"
#include "Value.hh"
#include "ValueType.hh"

#include <string>

namespace PLEXIL
{
  // Forward references
  class Expression;
  DECLARE_ID(Expression);

  class ExprVec;

  // TODO:
  // - Support printing

  // Type-independent components of Operator
  class Operator
  {
  public:
    virtual ~Operator()
    {
    }

    std::string const &getName() const
    {
      return m_name;
    }

    // Delegated to each individual operator.
    virtual bool checkArgCount(size_t count) const = 0;

    // Delegated to OperatorImpl by default
    virtual ValueType valueType() const = 0;
    virtual void *allocateCache() const = 0;
    virtual void deleteCache(void *Ptr) const = 0;
    virtual bool calcNative(void *cache, ExprVec const *exprs) const = 0;
    virtual void printValue(std::ostream &s, void *cache, ExprVec const *exprs) const = 0;
    virtual Value toValue(void *cache, ExprVec const *exprs) const = 0;

  protected:
    Operator(std::string const &name)
      : m_name(name)
    {
    }

    std::string const m_name;

  private:
    // unimplemented
    Operator();
    Operator(Operator const &);
    Operator &operator=(Operator const &);
  };

  template <class IMPL>
  class OperatorShim : public Operator
  {
  public:
    OperatorShim(std::string const &name) : Operator(name) {}
    virtual ~OperatorShim() {}

    bool operator()(bool &result, ExprVec const *args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    bool operator()(uint16_t &result, ExprVec const *args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    bool operator()(int32_t &result, ExprVec const *args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    bool operator()(double &result, ExprVec const *args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    bool operator()(std::string &result, ExprVec const *args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    bool operator()(BooleanArray &result, ExprVec const *args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    bool operator()(IntegerArray &result, ExprVec const *args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    bool operator()(RealArray &result, ExprVec const *args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

    bool operator()(StringArray &result, ExprVec const *args) const
    {
      return static_cast<IMPL const *>(this)->calc(result, args);
    }

  protected:

    // Default methods
    virtual bool calc(bool &result, ExprVec const *args) const
    {
      assertTrueMsg(ALWAYS_FAIL, this->getName() << ": Return type error");
      return false;
    }

    virtual bool calc(uint16_t &result, ExprVec const *args) const
    {
      assertTrueMsg(ALWAYS_FAIL, this->getName() << ": Return type error");
      return false;
    }

    virtual bool calc(int32_t &result, ExprVec const *args) const
    {
      assertTrueMsg(ALWAYS_FAIL, this->getName() << ": Return type error");
      return false;
    }

    virtual bool calc(double &result, ExprVec const *args) const
    {
      assertTrueMsg(ALWAYS_FAIL, this->getName() << ": Return type error");
      return false;
    }

    virtual bool calc(std::string &result, ExprVec const *args) const
    {
      assertTrueMsg(ALWAYS_FAIL, this->getName() << ": Return type error");
      return false;
    }

    virtual bool calc(BooleanArray &result, ExprVec const *args) const
    {
      assertTrueMsg(ALWAYS_FAIL, this->getName() << ": Return type error");
      return false;
    }

    virtual bool calc(IntegerArray &result, ExprVec const *args) const
    {
      assertTrueMsg(ALWAYS_FAIL, this->getName() << ": Return type error");
      return false;
    }

    virtual bool calc(RealArray &result, ExprVec const *args) const
    {
      assertTrueMsg(ALWAYS_FAIL, this->getName() << ": Return type error");
      return false;
    }

    virtual bool calc(StringArray &result, ExprVec const *args) const
    {
      assertTrueMsg(ALWAYS_FAIL, this->getName() << ": Return type error");
      return false;
    }
  };

  template <typename R>
  class OperatorImpl : public OperatorShim<OperatorImpl<R> >
  {
  public:
    virtual ~OperatorImpl() {}

    virtual bool calc(R &result, ExprVec const *) const = 0;

    // Default methods, based on R
    ValueType valueType() const;
    void *allocateCache() const { return static_cast<void *>(new R); }
    void deleteCache(void *ptr) const { delete static_cast<R *>(ptr); }
    bool calcNative(void *cache, ExprVec const *exprs) const
    {
      return calc(*(static_cast<R *>(cache)), exprs);
    }

    void printValue(std::ostream &s, void *cache, ExprVec const *exprs) const
    {
      R *nativeCache = static_cast<R *>(cache);
      if (calc(*nativeCache, exprs))
        PLEXIL::printValue(*nativeCache, s);
      else
        s << "UNKNOWN";
    }

    Value toValue(void *cache, ExprVec const *exprs) const
    {
      R *nativeCache = static_cast<R *>(cache);
      bool known = calc(*nativeCache, exprs);
      if (known)
        return Value(*nativeCache);
      else
        return Value();
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

  template <typename R>
  class OperatorImpl<ArrayImpl<R> >
    : public OperatorShim<OperatorImpl<ArrayImpl<R> > >
  {
  public:
    virtual ~OperatorImpl() {}

    virtual bool calc(R &result, ExprVec const *) const = 0;
    virtual bool calc(Array &result, ExprVec const *) const = 0;

    // Default methods, based on R
    virtual ValueType valueType() const;
    virtual void *allocateCache() const { return new R; }
    virtual void deleteCache(void *ptr) const { delete static_cast<ArrayImpl<R> *>(ptr); }

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

#endif // PLEXIL_OPERATOR_HH
