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

#include "ArrayReference.hh"
#include "ArrayVariable.hh"
#include "Constant.hh"
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
  Expression *ConcreteExpressionFactory<Constant<T> >::allocate(PlexilExpr const *expr,
                                                                NodeConnector * /* node */,
                                                                bool &wasCreated) const
  {
    PlexilValue const *tmpl = dynamic_cast<PlexilValue const *>(expr);
    checkParserException(tmpl, "createExpression: Expression is not a PlexilValue");

    wasCreated = true;
    return this->create(tmpl);
  }

  // Since there are exactly 3 possible Boolean constants, return references to them.
  template <>
  Expression *ConcreteExpressionFactory<Constant<bool> >::allocate(PlexilExpr const *expr,
                                                                   NodeConnector * /* node */,
                                                                   bool &wasCreated) const
  {
    PlexilValue const *tmpl = dynamic_cast<PlexilValue const *>(expr);
    checkParserException(tmpl, "createExpression: Expression is not a PlexilValue");
    bool value;
    bool known = parseValue(tmpl->value(), value);
    // if we got here, there was no parsing exception
    wasCreated = false;
    if (!known)
      return UNKNOWN_BOOLEAN_EXP();
    else if (value)
      return TRUE_EXP();
    else
      return FALSE_EXP();
  }
  
  template <typename T>
  Expression *ConcreteExpressionFactory<Constant<T> >::create(PlexilValue const *tmpl) const
  {
    T value;
    bool known = parseValue(tmpl->value(), value);
    if (known)
      return new Constant<T>(value);
    else
      return new Constant<T>();
  }
  
  // String is different

  template <>
  Expression *ConcreteExpressionFactory<Constant<std::string> >::create(PlexilValue const *tmpl) const
  {
    checkParserException(tmpl->type() == STRING_TYPE, "createExpression: Expression is not a PlexilValue");
    return new Constant<std::string>(tmpl->value());
  }

  //
  // Factories for array constants
  //

  template <typename T>
  Expression *ConcreteExpressionFactory<Constant<ArrayImpl<T> > >::create(PlexilArrayValue const *val) const
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
    return new Constant<ArrayImpl<T> >(initVals);
  }

  template <typename T>
  Expression *ConcreteExpressionFactory<Constant<ArrayImpl<T> > >::allocate(PlexilExpr const *expr,
                                                                            NodeConnector * /* node */,
                                                                            bool &wasCreated) const
  {
    PlexilArrayValue const *val = dynamic_cast<PlexilArrayValue const *>(expr);
    checkParserException(val, "createExpression: Not an array value");

    wasCreated = true;
    return this->create(val);
  }

  //
  // Factories for scalar variables
  //

  template <typename T>
  Expression *
  ConcreteExpressionFactory<UserVariable<T> >::allocate(PlexilExpr const *expr,
                                                        NodeConnector *node,
                                                        bool &wasCreated) const
  {
    PlexilVarRef const *varRef = dynamic_cast<PlexilVarRef const *>(expr);
    if (varRef) {
      // Variable reference - look it up
      checkParserException(node,
                           "createExpression: Variable reference with null node"); // ??
      Expression *result = node->findVariable(varRef);
      checkParserException(result, "createExpression: Can't find variable named " << varRef->varName());
      checkParserException(result->valueType() == varRef->type(),
                           "createExpression: Variable " << varRef->varName()
                           << " is type " << valueTypeName(result->valueType())
                           << ", but reference is for type " << valueTypeName(varRef->type()));
      wasCreated = false;
      return result;
    }
    PlexilVar const *var = dynamic_cast<PlexilVar const *>(expr);
    if (var) {
      // Variable declaration - construct it
      wasCreated = true;
      return this->create(var, node);
    }
    checkParserException(false, "createExpression: Expression is neither a variable definition nor a variable reference");
    return NULL;
  }

  template <typename T>
  Expression *
  ConcreteExpressionFactory<UserVariable<T> >::create(PlexilVar const *var,
                                                      NodeConnector *node) const
  {
    bool garbage = false;
    Expression *initexp = NULL;
    PlexilExpr const *initval = var->value(); 
    if (initval)
      initexp = createExpression(initval, node, garbage);
    return new UserVariable<T>(node, var->varName(), initexp, garbage);
  }

  //
  // Array variables
  //

  template <typename T>
  Expression *ConcreteExpressionFactory<ArrayVariable<T> >::allocate(PlexilExpr const *expr,
                                                                     NodeConnector *node,
                                                                     bool &wasCreated) const
  {
    PlexilVarRef const *varRef = dynamic_cast<PlexilVarRef const *>(expr);
    if (varRef) {
      // Variable reference - look it up
      checkParserException(node, "createExpression: Internal error: Can't find array variable reference with null node"); // ??
      Expression *result = node->findVariable(varRef);
      checkParserException(result, "createExpression: Can't find array variable named " << varRef->varName());
      checkParserException(result->valueType() == varRef->type(),
                           "createExpression: Variable " << varRef->varName()
                           << " is type " << valueTypeName(result->valueType())
                           << ", but reference is for type " << valueTypeName(varRef->type()));
      wasCreated = false;
      return result;
    }
    PlexilArrayVar const *var = dynamic_cast<PlexilArrayVar const *>(expr);
    if (var) {
      // Variable declaration - construct it
      wasCreated = true;
      return this->create(var, node);
    }
    checkParserException(false,
                         "createExpression: Expression is neither a variable reference nor an array variable declaration");
    return NULL;
  }

  template <typename T>
  Expression *
  ConcreteExpressionFactory<ArrayVariable<T> >::create(PlexilArrayVar const *var,
                                                       NodeConnector *node) const
  {
    bool garbage = false;
    Expression *sizeexp = new IntegerConstant(var->maxSize());
    Expression *initexp = NULL;
    PlexilExpr const *initval = var->value(); 
    if (initval)
      initexp = createExpression(initval, node, garbage);
    return new ArrayVariable<T>(node, var->varName(), sizeexp, initexp, true, garbage);
  }

  Expression *
  ConcreteExpressionFactory<ArrayReference>::allocate(PlexilExpr const *expr,
                                                      NodeConnector *node,
                                                      bool & wasCreated) const
  {
    PlexilArrayElement const * ary = 
      dynamic_cast<PlexilArrayElement const *>(expr);
    checkParserException(ary != NULL, "createExpression: Expression is not a PlexilArrayElement");

    bool garbageArray;
    Expression *array = createExpression(ary->array(), node, garbageArray);
    checkParserException(array, "createExpression: Array expression not found for array reference");
    checkParserException(isArrayType(array->valueType()),
                         "createExpression: Array expression in array reference is not an array");
    
    bool garbageIndex;
    Expression *index = createExpression(ary->index(), node, garbageIndex);
    checkParserException(index, "createExpression: Index expression not found for array reference");

    wasCreated = true;
    return new ArrayReference(array, index, garbageArray, garbageIndex);
  }

  // Generic variable references
  Expression *
  VariableReferenceFactory::allocate(PlexilExpr const *expr,
                                     NodeConnector *node,
                                     bool & wasCreated) const
  {
    PlexilVarRef const *varRef = dynamic_cast<PlexilVarRef const *>(expr);
    checkParserException(varRef, "createExpression: Expression is not a variable reference");
    // Look it up
    wasCreated = false;
    checkParserException(node,
                         "createExpression: Variable reference with null node"); // ??
    Expression *result = node->findVariable(varRef);
    checkParserException(result, "createExpression: Can't find variable named " << varRef->varName());
    // FIXME: add more type checking later
    if (varRef->type() == ARRAY_TYPE) {
      checkParserException(isArrayType(result->valueType()),
                           "createExpression: Variable " << varRef->varName()
                           << " is type " << valueTypeName(result->valueType())
                           << ", but reference is for array type");
    }
    return result;
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

  // Redundant with above
  // ENSURE_EXPRESSION_FACTORY(ArrayReference);

} // namespace PLEXIL

