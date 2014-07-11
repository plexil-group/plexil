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

#include "ArithmeticFunctionFactory.hh"
#include "ArithmeticOperators.hh"
#include "ArrayOperators.hh"
#include "ArrayVariable.hh"
#include "BooleanOperators.hh"
#include "Comparisons.hh"
#include "ConcreteExpressionFactory.hh"
#include "Constant.hh"
#include "FunctionFactory.hh"
#include "NodeConstantExpressions.hh"
#include "StringOperators.hh"
#include "lifecycle-utils.h"

namespace PLEXIL
{
  //
  // Concrete instantiations of class templates
  //

  // Comparisons
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Equal);
  ENSURE_FUNCTION_FACTORY(Equal<bool>);
  ENSURE_FUNCTION_FACTORY(Equal<std::string>);
  ENSURE_FUNCTION_FACTORY(Equal<uint16_t>);

  ENSURE_ARITHMETIC_FUNCTION_FACTORY(NotEqual);
  ENSURE_FUNCTION_FACTORY(NotEqual<bool>);
  ENSURE_FUNCTION_FACTORY(NotEqual<std::string>);
  ENSURE_FUNCTION_FACTORY(NotEqual<uint16_t>);

  ENSURE_ARITHMETIC_FUNCTION_FACTORY(GreaterThan);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(GreaterEqual);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(LessThan);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(LessEqual);

  // Not currently in the schema
  // ENSURE_FUNCTION_FACTORY(GreaterThan<std::string>);
  // ENSURE_FUNCTION_FACTORY(GreaterEqual<std::string>);
  // ENSURE_FUNCTION_FACTORY(LessThan<std::string>);
  // ENSURE_FUNCTION_FACTORY(LessEqual<std::string>);

  // Boolean operators
  ENSURE_FUNCTION_FACTORY(BooleanNot);
  ENSURE_FUNCTION_FACTORY(BooleanOr);
  ENSURE_FUNCTION_FACTORY(BooleanAnd);
  ENSURE_FUNCTION_FACTORY(BooleanXor);

  // Arithmetic operators
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Addition);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Subtraction);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Multiplication);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Division);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Modulo);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Minimum);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Maximum);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(AbsoluteValue);

  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Ceiling);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Floor);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Round);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Truncate);

  ENSURE_FUNCTION_FACTORY(SquareRoot<double>);
  ENSURE_FUNCTION_FACTORY(RealToInteger);

  // Named constants
  ENSURE_NAMED_CONSTANT_FACTORY(NodeStateConstant);
  ENSURE_NAMED_CONSTANT_FACTORY(NodeOutcomeConstant);
  ENSURE_NAMED_CONSTANT_FACTORY(FailureTypeConstant);
  ENSURE_NAMED_CONSTANT_FACTORY(CommandHandleConstant);

  void registerBasicExpressionFactories()
  {
    static bool sl_inited = false;
    if (!sl_inited) {
      addFinalizer(&purgeExpressionFactories);

      // Constants
      REGISTER_EXPRESSION(BooleanConstant, BooleanValue);
      REGISTER_EXPRESSION(IntegerConstant, IntegerValue);
      REGISTER_EXPRESSION(RealConstant, RealValue);
      REGISTER_EXPRESSION(StringConstant, StringValue);
      REGISTER_EXPRESSION(BooleanArrayConstant, BooleanArrayValue);
      REGISTER_EXPRESSION(IntegerArrayConstant, IntegerArrayValue);
      REGISTER_EXPRESSION(RealArrayConstant, RealArrayValue);
      REGISTER_EXPRESSION(StringArrayConstant, StringArrayValue);

      // Named constants
      REGISTER_NAMED_CONSTANT_FACTORY(NodeStateConstant, NodeStateValue);
      REGISTER_NAMED_CONSTANT_FACTORY(NodeOutcomeConstant, NodeOutcomeValue);
      REGISTER_NAMED_CONSTANT_FACTORY(FailureTypeConstant, NodeFailureValue);
      REGISTER_NAMED_CONSTANT_FACTORY(CommandHandleConstant, NodeCommandHandleValue);

      // Variables
      REGISTER_EXPRESSION(BooleanVariable, BooleanVariable);
      REGISTER_EXPRESSION(IntegerVariable, IntegerVariable);
      REGISTER_EXPRESSION(RealVariable, RealVariable);
      REGISTER_EXPRESSION(StringVariable, StringVariable);
      REGISTER_EXPRESSION(BooleanArrayVariable, BooleanArrayVariable);
      REGISTER_EXPRESSION(IntegerArrayVariable, IntegerArrayVariable);
      REGISTER_EXPRESSION(RealArrayVariable, RealArrayVariable);
      REGISTER_EXPRESSION(StringArrayVariable, StringArrayVariable);
      new VariableReferenceFactory("ArrayVariable"); // for effect

      // Comparisons
      REGISTER_FUNCTION(IsKnown, IsKnown);

      REGISTER_ARITHMETIC_FUNCTION(Equal, EQNumeric);
      REGISTER_FUNCTION(Equal<bool>, EQBoolean);
      REGISTER_FUNCTION(Equal<std::string>, EQString);
      REGISTER_FUNCTION(Equal<uint16_t>, EQInternal);

      REGISTER_ARITHMETIC_FUNCTION(NotEqual, NENumeric);
      REGISTER_FUNCTION(NotEqual<bool>, NEBoolean);
      REGISTER_FUNCTION(NotEqual<std::string>, NEString);
      REGISTER_FUNCTION(NotEqual<uint16_t>, NEInternal);

      REGISTER_ARITHMETIC_FUNCTION(GreaterThan, GT);
      REGISTER_ARITHMETIC_FUNCTION(GreaterEqual, GE);
      REGISTER_ARITHMETIC_FUNCTION(LessThan, LT);
      REGISTER_ARITHMETIC_FUNCTION(LessEqual, LE);

      // Not currently in the schema
      // REGISTER_FUNCTION(GreaterThan<std::string>, GTString);
      // REGISTER_FUNCTION(GreaterEqual<std::string>, GEString);
      // REGISTER_FUNCTION(LessThan<std::string>, LTString);
      // REGISTER_FUNCTION(LessEqual<std::string>, LEString);

      // Arithmetic operations
      REGISTER_ARITHMETIC_FUNCTION(Addition, ADD);
      REGISTER_ARITHMETIC_FUNCTION(Subtraction, SUB);
      REGISTER_ARITHMETIC_FUNCTION(Multiplication, MUL);
      REGISTER_ARITHMETIC_FUNCTION(Division, DIV);
      REGISTER_ARITHMETIC_FUNCTION(Modulo, MOD);
      REGISTER_ARITHMETIC_FUNCTION(Maximum, MAX);
      REGISTER_ARITHMETIC_FUNCTION(Minimum, MIN);
      REGISTER_ARITHMETIC_FUNCTION(AbsoluteValue, ABS);

      REGISTER_FUNCTION(SquareRoot<double>, SQRT);
      REGISTER_FUNCTION(RealToInteger, REAL_TO_INT);

      // Not currently in the schema
      REGISTER_ARITHMETIC_FUNCTION(Ceiling, CEIL);
      REGISTER_ARITHMETIC_FUNCTION(Floor, FLOOR);
      REGISTER_ARITHMETIC_FUNCTION(Round, ROUND);
      REGISTER_ARITHMETIC_FUNCTION(Truncate, TRUNC);

      // Boolean operations
      REGISTER_FUNCTION(BooleanNot, NOT);
      REGISTER_FUNCTION(BooleanAnd, AND);
      REGISTER_FUNCTION(BooleanOr, OR);
      REGISTER_FUNCTION(BooleanXor, XOR);

      // String operations
      REGISTER_FUNCTION(StringConcat, Concat);
      REGISTER_FUNCTION(StringLength, STRLEN);

      // Array operations
      REGISTER_FUNCTION(ArrayLength, SIZE);
      REGISTER_FUNCTION(AllElementsKnown, ALL_KNOWN);
      REGISTER_FUNCTION(AnyElementsKnown, ANY_KNOWN);
      REGISTER_EXPRESSION(ArrayReference, ArrayElement);

      sl_inited = true;
    }
  }

} // namespace PLEXIL
