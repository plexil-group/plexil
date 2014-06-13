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
#include "ArrayVariable.hh"
#include "Error.hh"
#include "ExpressionConstants.hh"
#include "NodeConnector.hh"
#include "ParserException.hh"
#include "PlexilExpr.hh"

#include <sstream>

namespace PLEXIL
{

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
    PlexilValue const *tmpl = (PlexilValue const *) expr;
    checkParserException(tmpl, "createExpression: Expression is not a PlexilValue");

    wasCreated = true;
    return this->create(tmpl, node);
  }

  // Since there are exactly 3 possible Boolean constants, return references to them.
  template <>
  ExpressionId ConcreteExpressionFactory<Constant<bool> >::allocate(const PlexilExprId& expr,
                                                                    const NodeConnectorId& node,
                                                                    bool &wasCreated) const
  {
    PlexilValue const *tmpl = (PlexilValue const *) expr;
    checkParserException(tmpl, "createExpression: Expression is not a PlexilValue");
    bool value;
    bool known = parseValue(tmpl->value(), value);
    // if we got here, there was no parsing exception
    if (!known)
      return UNKNOWN_BOOLEAN_EXP();
    else if (value)
      return TRUE_EXP();
    else
      return FALSE_EXP();
  }
  
  template <typename T>
  ExpressionId ConcreteExpressionFactory<Constant<T> >::create(PlexilValue const *tmpl,
                                                               const NodeConnectorId& node) const
  {
    T value;
    bool known = parseValue(tmpl->value(), value);
    if (known)
      return (new Constant<T>(value))->getId();
    else
      return (new Constant<T>())->getId();
  }
  
  // String is different

  template <>
  ExpressionId ConcreteExpressionFactory<Constant<std::string> >::create(PlexilValue const *tmpl,
                                                                         const NodeConnectorId& node) const
  {
    checkParserException(tmpl->type() == STRING_TYPE, "createExpression: Expression is not a PlexilValue");
    return (new Constant<std::string>(tmpl->value()))->getId();
  }

  //
  // Factories for array constants
  //

  template <typename T>
  ExpressionId ConcreteExpressionFactory<ArrayConstant<T> >::create(PlexilArrayValue const *val,
                                                                    const NodeConnectorId& node) const
  {
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
    checkParserException(val, "createExpression: Not an array value");

    wasCreated = true;
    return this->create(val, node);
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
      checkParserException(node.isId(),
                           "createExpression: Variable reference with null node"); // ??
      ExpressionId result = node->findVariable(varRef);
      checkParserException(result.isId(), "createExpression: Can't find variable named " << varRef->varName());
      checkParserException(result->valueType() == varRef->type(),
                           "createExpression: Variable " << varRef->varName()
                           << " is type " << valueTypeName(result->valueType())
                           << ", but reference is for type " << valueTypeName(varRef->type()));
      wasCreated = false;
      return result;
    }
    PlexilVar const *var = dynamic_cast<PlexilVar const *>((PlexilExpr const *) expr);
    if (var) {
      // Variable declaration - construct it
      wasCreated = true;
      return this->create(var, node);
    }
    checkParserException(ALWAYS_FAIL, "createExpression: Expression is neither a variable definition nor a variable reference");
    return ExpressionId::noId();
  }

  template <typename T>
  ExpressionId 
  ConcreteExpressionFactory<UserVariable<T> >::create(PlexilVar const *var,
                                                      const NodeConnectorId& node) const
  {
    bool garbage = false;
    ExpressionId initexp;
    PlexilExprId initval = var->value(); 
    if (initval.isId())
      initexp = createExpression(initval, node, garbage);
    return (new UserVariable<T>(node, var->varName(), initexp, garbage))->getId();
  }

  //
  // Array variables
  //

  template <typename T>
  ExpressionId ConcreteExpressionFactory<ArrayVariable<T> >::allocate(const PlexilExprId& expr,
                                                                      const NodeConnectorId& node,
                                                                      bool &wasCreated) const
  {
    PlexilVarRef const *varRef = dynamic_cast<PlexilVarRef const *>((PlexilExpr const *) expr);
    if (varRef) {
      // Variable reference - look it up
      checkParserException(node.isId(), "createExpression: Internal error: Can't find array variable reference with null node"); // ??
      ExpressionId result = node->findVariable(varRef);
      checkParserException(result.isId(), "createExpression: Can't find array variable named " << varRef->varName());
      checkParserException(result->valueType() == varRef->type(),
                           "createExpression: Variable " << varRef->varName()
                           << " is type " << valueTypeName(result->valueType())
                           << ", but reference is for type " << valueTypeName(varRef->type()));
      wasCreated = false;
      return result;
    }
    PlexilArrayVar const *var = dynamic_cast<PlexilArrayVar const *>((PlexilExpr const *) expr);
    if (var) {
      // Variable declaration - construct it
      wasCreated = true;
      return this->create(var, node);
    }
    checkParserException(ALWAYS_FAIL,
                         "createExpression: Expression is neither a variable reference nor an array variable declaration");
    return ExpressionId::noId();
  }

  template <typename T>
  ExpressionId 
  ConcreteExpressionFactory<ArrayVariable<T> >::create(PlexilArrayVar const *var,
                                                       const NodeConnectorId& node) const
  {
    bool garbage = false;
    ExpressionId sizeexp = (new IntegerConstant(var->maxSize()))->getId();
    ExpressionId initexp;
    PlexilExprId initval = var->value(); 
    if (initval.isId())
      initexp = createExpression(initval, node, garbage);
    return (new ArrayVariable<T>(node, var->varName(), sizeexp, initexp, true, garbage))->getId();
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

  ENSURE_EXPRESSION_FACTORY(BooleanVariable);
  ENSURE_EXPRESSION_FACTORY(IntegerVariable);
  ENSURE_EXPRESSION_FACTORY(RealVariable);
  ENSURE_EXPRESSION_FACTORY(StringVariable);
  ENSURE_EXPRESSION_FACTORY(BooleanArrayVariable);
  ENSURE_EXPRESSION_FACTORY(IntegerArrayVariable);
  ENSURE_EXPRESSION_FACTORY(RealArrayVariable);
  ENSURE_EXPRESSION_FACTORY(StringArrayVariable);

} // namespace PLEXIL

