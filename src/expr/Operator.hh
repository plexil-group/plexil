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
#include "ValueType.hh"

namespace PLEXIL
{
  // Forward references
  class Expression;
  class ExprVec;
  class Value;

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
    // Default method returns false.
    virtual bool checkArgCount(size_t count) const { return false; }

    // Delegated to OperatorImpl by default
    virtual ValueType valueType() const = 0;
    virtual void *allocateCache() const = 0;
    virtual void deleteCache(void *Ptr) const = 0;

    virtual bool operator()(bool &result, Expression const *arg) const = 0;
    virtual bool operator()(bool &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool operator()(bool &result, ExprVec const &args) const = 0;

    virtual bool operator()(int32_t &result, Expression const *arg) const = 0;
    virtual bool operator()(int32_t &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool operator()(int32_t &result, ExprVec const &args) const = 0;

    virtual bool operator()(double &result, Expression const *arg) const = 0;
    virtual bool operator()(double &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool operator()(double &result, ExprVec const &args) const = 0;

    virtual bool operator()(std::string &result, Expression const *arg) const = 0;
    virtual bool operator()(std::string &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool operator()(std::string &result, ExprVec const &args) const = 0;

    virtual bool operator()(Array &result, Expression const *arg) const = 0;
    virtual bool operator()(Array &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool operator()(Array &result, ExprVec const &args) const = 0;

    virtual bool operator()(BooleanArray &result, Expression const *arg) const = 0;
    virtual bool operator()(BooleanArray &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool operator()(BooleanArray &result, ExprVec const &args) const = 0;

    virtual bool operator()(IntegerArray &result, Expression const *arg) const = 0;
    virtual bool operator()(IntegerArray &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool operator()(IntegerArray &result, ExprVec const &args) const = 0;

    virtual bool operator()(RealArray &result, Expression const *arg) const = 0;
    virtual bool operator()(RealArray &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool operator()(RealArray &result, ExprVec const &args) const = 0;

    virtual bool operator()(StringArray &result, Expression const *arg) const = 0;
    virtual bool operator()(StringArray &result, Expression const *arg0, Expression const *arg1) const = 0;
    virtual bool operator()(StringArray &result, ExprVec const &args) const = 0;

    virtual bool calcNative(void *cache, ExprVec const &exprs) const = 0;
    virtual void printValue(std::ostream &s, void *cache, ExprVec const &exprs) const = 0;
    virtual Value toValue(void *cache, ExprVec const &exprs) const = 0;

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

} // namespace PLEXIL

#endif // PLEXIL_OPERATOR_HH
