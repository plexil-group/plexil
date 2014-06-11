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

#include "ConcreteExpressionFactory.hh"

#include "ArrayConstant.hh"
#include "Error.hh"
#include "NodeConnector.hh"
#include "PlexilExpr.hh"
#include "UserVariable.hh"

#include <sstream>

namespace PLEXIL
{
  //
  // General case
  //

  template <class FactoryType>
  ExpressionId ConcreteExpressionFactory<FactoryType>::allocate(const PlexilExprId& expr,
                                                                const NodeConnectorId& node,
                                                                bool &wasCreated) const
  {
    wasCreated = true;
    return this->create(expr, node);
  }

  // Meant to be overridden by specialized methods.
  template <class FactoryType>
  ExpressionId ConcreteExpressionFactory<FactoryType>::create(const PlexilExprId& expr,
                                                              const NodeConnectorId& node) const
  {
    assertTrue_2(ALWAYS_FAIL, "No expression factory create() method for this type");
    return ExpressionId::noId();
  }

  //
  // Factories for scalar constants
  //

  // TODO? - common registry of constants per type

  // N.B. For all but string types, the value string may not be empty.
  template <typename T>
  ExpressionId ConcreteExpressionFactory<Constant<T> >::allocate(const PlexilExprId& expr,
                                                                 const NodeConnectorId& node,
                                                                 bool &wasCreated) const
  {
    wasCreated = true;
    return this->create(expr, node);
  }
  
  template <typename T>
  ExpressionId ConcreteExpressionFactory<Constant<T> >::create(const PlexilExprId& expr,
                                                               const NodeConnectorId& node) const
  {
    PlexilValue const *tmpl = (PlexilValue const *) expr;
    assertTrue_2(tmpl, "createExpression: Expression is not a PlexilValue");
    T value;
    bool known = parseValue(tmpl->value(), value);
    if (known)
      return (new Constant<T>(value))->getId();
    else
      return (new Constant<T>())->getId();
  }
  
  // String is different

  template <>
  ExpressionId ConcreteExpressionFactory<Constant<std::string> >::create(const PlexilExprId& expr,
                                                                         const NodeConnectorId& node) const
  {
    PlexilValue const *tmpl = (PlexilValue const *) expr;
    assertTrue_2(tmpl, "ExpressionFactory<Constant>: Expression is not a PlexilValue");
    return (new Constant<std::string>(tmpl->value()))->getId();
  }

  //
  // Factories for array constants
  //

  template <typename T>
  ExpressionId ConcreteExpressionFactory<ArrayConstant<T> >::create(const PlexilExprId& expr,
                                                                    const NodeConnectorId& node) const
  {
    PlexilArrayValue const *val = dynamic_cast<PlexilArrayValue const *>((PlexilExpr const *) expr);
    // assertTrue_2(val, "createExpression: Not an array value"); // should have been checked in allocate()

    unsigned arraySize = val->maxSize();
    std::vector<std::string> const &eltVals = val->values();
    ArrayImpl<T> initVals(arraySize);
    for (size_t i = 0; i < eltVals.size(); ++i) {
      // Parse an element value and push it onto the vector
      T temp;
      if (parseValue<T>(eltVals[i], temp)) // will throw if format error
        initVals.setElement(i, temp);
      else
        initVals.setElementUnknown(i);
    }
    return (new ArrayConstant<T>(initVals))->getId();
  }

  template <typename T>
  ExpressionId ConcreteExpressionFactory<ArrayConstant<T> >::allocate(const PlexilExprId& expr,
                                                                      const NodeConnectorId& node,
                                                                      bool &wasCreated) const
  {
    PlexilArrayValue const *val = dynamic_cast<PlexilArrayValue const *>((PlexilExpr const *) expr);
    assertTrue_2(val, "createExpression: Not an array value");

    wasCreated = true;
    return this->create(expr, node);
  }

  //
  // Factories for scalar variables
  //

  template <typename T>
  ExpressionId 
  ConcreteExpressionFactory<UserVariable<T> >::allocate(const PlexilExprId& expr,
                                                        const NodeConnectorId& node,
                                                        bool &wasCreated) const
  {
    PlexilVarRef const *varRef = dynamic_cast<PlexilVarRef const *>((PlexilExpr const *) expr);
    if (varRef) {
      // Variable reference - look it up
      assertTrue_2(node.isId(), "createExpression: Can't find variable reference with null node");
      ExpressionId result = node->findVariable(varRef);
      assertTrueMsg(result.isId(), "createExpression: Can't find referenced variable" << varRef->varName());
      wasCreated = false;
      return result;
    }
    else {
      // Variable declaration - construct it
      wasCreated = true;
      return this->create(expr, node);
    }
  }

  template <typename T>
  ExpressionId 
  ConcreteExpressionFactory<UserVariable<T> >::create(const PlexilExprId& expr,
                                                      const NodeConnectorId& node) const
  {
    PlexilVar const *var = dynamic_cast<PlexilVar const * >((PlexilExpr const *)expr);
    assertTrue_2(var, "createExpression: Not a variable declaration");
    bool garbage = false;
    ExpressionId initexp;
    PlexilExprId initval = var->value(); 
    if (initval.isId())
      initexp = createExpression(initval, node, garbage);
    return (new UserVariable<T>(node, var->varName(), initexp, garbage))->getId();
  }

  // Explicit instantiations
  ENSURE_EXPRESSION_FACTORY(BooleanConstant);
  ENSURE_EXPRESSION_FACTORY(IntegerConstant);
  ENSURE_EXPRESSION_FACTORY(RealConstant);
  ENSURE_EXPRESSION_FACTORY(StringConstant);
  ENSURE_EXPRESSION_FACTORY(BooleanArrayConstant);
  ENSURE_EXPRESSION_FACTORY(IntegerArrayConstant);
  ENSURE_EXPRESSION_FACTORY(RealArrayConstant);
  ENSURE_EXPRESSION_FACTORY(StringArrayConstant);

  ENSURE_EXPRESSION_FACTORY(UserVariable<bool>);
  ENSURE_EXPRESSION_FACTORY(UserVariable<int32_t>);
  ENSURE_EXPRESSION_FACTORY(UserVariable<double>);
  ENSURE_EXPRESSION_FACTORY(UserVariable<std::string>);

} // namespace PLEXIL

