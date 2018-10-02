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

#ifndef PLEXIL_NODE_FUNCTION_FACTORY_HH
#define PLEXIL_NODE_FUNCTION_FACTORY_HH

#include "ExpressionFactory.hh"

namespace PLEXIL
{
  class NodeOperator;

  // Base class
  class NodeFunctionFactory : public ExpressionFactory
  {
  public:
    NodeFunctionFactory(std::string const &name);
    virtual ~NodeFunctionFactory();

    ValueType check(char const *nodeId, pugi::xml_node expr) const
      throw (ParserException);

    Expression *allocate(pugi::xml_node const expr,
                         NodeConnector *node,
                         bool & wasCreated,
                         ValueType returnType) const
      throw (ParserException);

  protected:

    // Delegated to derived class
    virtual NodeOperator const *getOperator() const = 0;

  private:
    // Unimplemented
    NodeFunctionFactory();
    NodeFunctionFactory(NodeFunctionFactory const &);
    NodeFunctionFactory &operator=(NodeFunctionFactory const &);
  };

  template <class OP>
  class NodeFunctionFactoryImpl : public NodeFunctionFactory
  {
  public:
    NodeFunctionFactoryImpl(std::string const &name)
      : NodeFunctionFactory(name)
    {
    }

    ~NodeFunctionFactoryImpl()
    {
    }

  protected:
    NodeOperator const *getOperator() const
    {
      return OP::instance();
    }

  private:
    // Unimplemented
    NodeFunctionFactoryImpl();
    NodeFunctionFactoryImpl(NodeFunctionFactoryImpl const &);
    NodeFunctionFactoryImpl &operator=(NodeFunctionFactoryImpl const &);
  };

} // namespace PLEXIL

// Convenience macros
#define REGISTER_NODE_FUNCTION(CLASS,NAME) {new PLEXIL::NodeFunctionFactoryImpl<CLASS>(#NAME);}

#endif // PLEXIL_NODE_FUNCTION_FACTORY_HH
