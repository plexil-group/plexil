/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_FUNCTION_FACTORY_HH
#define PLEXIL_FUNCTION_FACTORY_HH

#include "ExpressionFactory.hh"

namespace PLEXIL
{
  // Forward reference
  class Function;
  class Operator;

  // Base class
  class FunctionFactory : public ExpressionFactory
  {
  public:
    FunctionFactory(Operator const *op, std::string const &name);
    virtual ~FunctionFactory();

    virtual ValueType check(char const *nodeId, pugi::xml_node expr) const;

    virtual Expression *allocate(pugi::xml_node const expr,
                                 NodeConnector *node,
                                 bool & wasCreated,
                                 ValueType returnType) const;

  protected:
    virtual Function *constructFunction(Operator const *op, size_t n) const;

  private:
    // Unimplemented
    FunctionFactory();
    FunctionFactory(FunctionFactory const &);
    FunctionFactory &operator=(FunctionFactory const &);

    Operator const *m_op;
  };

  // Derived class for functions requiring a cache
  class CachedFunctionFactory : public FunctionFactory
  {
  public:
    CachedFunctionFactory(Operator const *op, std::string const &name);
    virtual ~CachedFunctionFactory();

  protected:
    virtual Function *constructFunction(Operator const *op, size_t n) const;

  private:
    // Unimplemented
    CachedFunctionFactory();
    CachedFunctionFactory(CachedFunctionFactory const &);
    CachedFunctionFactory &operator=(CachedFunctionFactory const &);
  };

} // namespace PLEXIL

// Convenience macros
#define REGISTER_FUNCTION(CLASS,NAME) new PLEXIL::FunctionFactory(CLASS::instance(), #NAME)
#define REGISTER_CACHED_FUNCTION(CLASS,NAME) new PLEXIL::CachedFunctionFactory(CLASS::instance(), #NAME)

#endif // PLEXIL_FUNCTION_FACTORY_HH
