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
#include "ArrayConstant.hh"
#include "ArrayOperators.hh"
#include "ArrayVariable.hh"
#include "BooleanOperators.hh"
#include "Comparisons.hh"
#include "ConcreteExpressionFactory.hh"
#include "FunctionFactory.hh"
#include "StringOperators.hh"
#include "lifecycle-utils.h"

namespace PLEXIL
{
  //
  // Concrete instantiations of class templates
  //

  // Comparisons
  ENSURE_FUNCTION_FACTORY(Equal<bool>,        bool);
  ENSURE_FUNCTION_FACTORY(Equal<int32_t>,     bool);
  ENSURE_FUNCTION_FACTORY(Equal<double>,      bool);
  ENSURE_FUNCTION_FACTORY(Equal<std::string>, bool);

  ENSURE_FUNCTION_FACTORY(NotEqual<bool>,        bool);
  ENSURE_FUNCTION_FACTORY(NotEqual<int32_t>,     bool);
  ENSURE_FUNCTION_FACTORY(NotEqual<double>,      bool);
  ENSURE_FUNCTION_FACTORY(NotEqual<std::string>, bool);

  ENSURE_FUNCTION_FACTORY(GreaterThan<int32_t>,     bool);
  ENSURE_FUNCTION_FACTORY(GreaterThan<double>,      bool);
  ENSURE_FUNCTION_FACTORY(GreaterThan<std::string>, bool);

  ENSURE_FUNCTION_FACTORY(GreaterEqual<int32_t>,     bool);
  ENSURE_FUNCTION_FACTORY(GreaterEqual<double>,      bool);
  ENSURE_FUNCTION_FACTORY(GreaterEqual<std::string>, bool);

  ENSURE_FUNCTION_FACTORY(LessThan<int32_t>,     bool);
  ENSURE_FUNCTION_FACTORY(LessThan<double>,      bool);
  ENSURE_FUNCTION_FACTORY(LessThan<std::string>, bool);

  ENSURE_FUNCTION_FACTORY(LessEqual<int32_t>,     bool);
  ENSURE_FUNCTION_FACTORY(LessEqual<double>,      bool);
  ENSURE_FUNCTION_FACTORY(LessEqual<std::string>, bool);

  // Boolean operators
  ENSURE_FUNCTION_FACTORY(BooleanNot, bool);
  ENSURE_FUNCTION_FACTORY(BooleanOr,  bool);
  ENSURE_FUNCTION_FACTORY(BooleanAnd, bool);
  ENSURE_FUNCTION_FACTORY(BooleanXor, bool);

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

  ENSURE_FUNCTION_FACTORY(SquareRoot<double>,  double);
  ENSURE_FUNCTION_FACTORY(RealToInteger, int32_t);

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

      // Variables
      REGISTER_EXPRESSION(BooleanVariable, BooleanVariable);
      REGISTER_EXPRESSION(IntegerVariable, IntegerVariable);
      REGISTER_EXPRESSION(RealVariable, RealVariable);
      REGISTER_EXPRESSION(StringVariable, StringVariable);
      REGISTER_EXPRESSION(BooleanArrayVariable, BooleanArrayVariable);
      REGISTER_EXPRESSION(IntegerArrayVariable, IntegerArrayVariable);
      REGISTER_EXPRESSION(RealArrayVariable, RealArrayVariable);
      REGISTER_EXPRESSION(StringArrayVariable, StringArrayVariable);

      // Comparisons
      REGISTER_FUNCTION(IsKnown, bool, IsKnown);

      REGISTER_FUNCTION(Equal<bool>,        bool, EQBoolean);
      REGISTER_FUNCTION(Equal<int32_t>,     bool, EQInteger);
      REGISTER_FUNCTION(Equal<double>,      bool, EQReal);
      REGISTER_FUNCTION(Equal<std::string>, bool, EQString);

      REGISTER_FUNCTION(NotEqual<bool>,        bool, NEBoolean);
      REGISTER_FUNCTION(NotEqual<int32_t>,     bool, NEInteger);
      REGISTER_FUNCTION(NotEqual<double>,      bool, NEReal);
      REGISTER_FUNCTION(NotEqual<std::string>, bool, NEString);

      REGISTER_FUNCTION(GreaterThan<int32_t>,     bool, GTInteger);
      REGISTER_FUNCTION(GreaterThan<double>,      bool, GTReal);
      REGISTER_FUNCTION(GreaterThan<std::string>, bool, GTString);

      REGISTER_FUNCTION(GreaterEqual<int32_t>,     bool, GEInteger);
      REGISTER_FUNCTION(GreaterEqual<double>,      bool, GEReal);
      REGISTER_FUNCTION(GreaterEqual<std::string>, bool, GEString);

      REGISTER_FUNCTION(LessThan<int32_t>,     bool, LTInteger);
      REGISTER_FUNCTION(LessThan<double>,      bool, LTReal);
      REGISTER_FUNCTION(LessThan<std::string>, bool, LTString);

      REGISTER_FUNCTION(LessEqual<int32_t>,     bool, LEInteger);
      REGISTER_FUNCTION(LessEqual<double>,      bool, LEReal);
      REGISTER_FUNCTION(LessEqual<std::string>, bool, LEString);

      // Arithmetic operations
      REGISTER_ARITHMETIC_FUNCTION(Addition, ADD);
      REGISTER_ARITHMETIC_FUNCTION(Subtraction, SUB);
      REGISTER_ARITHMETIC_FUNCTION(Multiplication, MUL);
      REGISTER_ARITHMETIC_FUNCTION(Division, DIV);
      REGISTER_ARITHMETIC_FUNCTION(Modulo, MOD);
      REGISTER_ARITHMETIC_FUNCTION(Maximum, MAX);
      REGISTER_ARITHMETIC_FUNCTION(Minimum, MIN);
      REGISTER_ARITHMETIC_FUNCTION(AbsoluteValue, ABS);

      REGISTER_ARITHMETIC_FUNCTION(Ceiling, CEIL);
      REGISTER_ARITHMETIC_FUNCTION(Floor, FLOOR);
      REGISTER_ARITHMETIC_FUNCTION(Round, ROUND);
      REGISTER_ARITHMETIC_FUNCTION(Truncate, TRUNC);

      REGISTER_FUNCTION(SquareRoot<double>, double, SQRT);
      REGISTER_FUNCTION(RealToInteger, int32_t, REAL_TO_INT);

      // Boolean operations
      REGISTER_FUNCTION(BooleanNot, bool, NOT);
      REGISTER_FUNCTION(BooleanAnd, bool, AND);
      REGISTER_FUNCTION(BooleanOr,  bool, OR);
      REGISTER_FUNCTION(BooleanXor, bool, XOR);

      // String operations
      REGISTER_FUNCTION(StringConcat, std::string, CONCAT);
      REGISTER_FUNCTION(StringLength, int32_t, STRLEN);

      sl_inited = true;
    }
  }

} // namespace PLEXIL
