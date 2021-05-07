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

#ifndef PLEXIL_CONCRETE_EXPRESSION_FACTORY_HH
#define PLEXIL_CONCRETE_EXPRESSION_FACTORY_HH

#include "ExpressionFactory.hh"

#include "pugixml.hpp"

namespace PLEXIL
{

  //! Check the XML of the given expression for errors. If none found,
  //! return the value type of the expression.  Throw a ParserException
  //! if any errors are discovered.
  //! @tparam EXPR Notionally, a subclass of Expression which the
  //!              factory's allocate() method will return.
  //! @param nodeId The node ID of the PLEXIL node containing the expression.
  //! @param expr The pugixml representation of the expression.
  //! @param desiredType A value type suggested by the containing expression;
  //!                    used in Assignments.
  //! @return The result type the expression will return.
  //! @note C++ forbids partial specialization of function templates.

  template <class EXPR>
  ValueType factoryCheck(char const *nodeId,
                         pugi::xml_node const expr,
                         ValueType desiredType);

  //! Return a pointer to an Expression instance described by the XML.
  //! @tparam EXPR Notionally, a subclass of Expression which the function
  //!              will return; however, this is not checked or enforced.
  //! @param expr The pugixml representation of the expression.
  //! @param nodeId The node ID of the PLEXIL node containing the expression.
  //! @param wasCreated[out] This will be set to true if the returned object was
  //!                        constructed; false if an existing object was returned.
  //! @param desiredType A value type suggested by the containing expression;
  //!                    used in Assignments.
  //! @return Pointer to the Expression.
  //! @note C++ forbids partial specialization of function templates.

  template <class EXPR>
  Expression *factoryAllocate(pugi::xml_node const expr,
                              NodeConnector *node,
                              bool &wasCreated,
                              ValueType returnType);

  //! @class ConcreteExpressionFactory
  //! A skeleton factory class, which delegates its work to the two
  //! function templates above.
  //! @tparam EXPR Notionally, a subclass of Expression which the
  //!              factory's allocate() method will return;
  //!              this is not checked or enforced, however.
  //! @note Using a templated type (e.g. Constant<T>) as the type
  //! parameter will not save you from writing factoryCheck() and
  //! factoryAllocate() functions for each concrete instantiation
  //! of the templated type. This is because template functions,
  //! to which ConcreteExpressionFactory delegates its work,
  //! cannot be partially specialized.
  
  template <class EXPR>
  class ConcreteExpressionFactory : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    virtual ~ConcreteExpressionFactory() = default;

    virtual ValueType check(char const *nodeId,
                            pugi::xml_node const expr,
                            ValueType desiredType) const
    {
      return factoryCheck<EXPR>(nodeId, expr, desiredType);
    }

    virtual Expression *allocate(pugi::xml_node const expr,
                                 NodeConnector *node,
                                 bool &wasCreated,
                                 ValueType returnType) const
    {
      return factoryAllocate<EXPR>(expr, node, wasCreated, returnType);
    }

  private:
    // Default, copy, assign all prohibited
    ConcreteExpressionFactory() = delete;
    ConcreteExpressionFactory(ConcreteExpressionFactory const &) = delete;
    ConcreteExpressionFactory(ConcreteExpressionFactory &&) = delete;
    ConcreteExpressionFactory &operator=(ConcreteExpressionFactory const &) = delete;
    ConcreteExpressionFactory &operator=(ConcreteExpressionFactory &&) = delete;
  };

} // namespace PLEXIL

// Convenience macro
#define ENSURE_EXPRESSION_FACTORY(CLASS) template class PLEXIL::ConcreteExpressionFactory<CLASS>;

#endif // PLEXIL_CONCRETE_EXPRESSION_FACTORY_HH
