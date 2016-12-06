/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

#include "ValueType.hh"

namespace PLEXIL
{
  // Forward references
  class Expression;
  class Function;
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

    // Delegated to each individual operator.
    // Default method returns true.
    virtual bool checkArgTypes(Function const *ev) const { return true; }

    // Delegated to OperatorImpl by default
    virtual ValueType valueType() const = 0;
    virtual void *allocateCache() const = 0;
    virtual void deleteCache(void *Ptr) const = 0;

    // Local macro to generate a truckload of boilerplate
#define DECLARE_OPERATOR_METHODS(_rtype_) \
    virtual bool operator()(_rtype_ &result, Expression const *arg) const = 0; \
    virtual bool operator()(_rtype_ &result, Expression const *arg0, Expression const *arg1) const = 0; \
    virtual bool operator()(_rtype_ &result, Function const &args) const = 0; \

    DECLARE_OPERATOR_METHODS(Boolean)
    DECLARE_OPERATOR_METHODS(Integer)
    DECLARE_OPERATOR_METHODS(Real)
    DECLARE_OPERATOR_METHODS(String)

    DECLARE_OPERATOR_METHODS(uint16_t)

    DECLARE_OPERATOR_METHODS(Array)
    DECLARE_OPERATOR_METHODS(BooleanArray)
    DECLARE_OPERATOR_METHODS(IntegerArray)
    DECLARE_OPERATOR_METHODS(RealArray)
    DECLARE_OPERATOR_METHODS(StringArray)

#undef DECLARE_OPERATOR_METHODS

    virtual bool calcNative(void *cache, Function const &exprs) const = 0;
    virtual void printValue(std::ostream &s, void *cache, Function const &exprs) const = 0;
    virtual Value toValue(void *cache, Function const &exprs) const = 0;

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
