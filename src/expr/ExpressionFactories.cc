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
  ENSURE_FUNCTION_FACTORY(Addition<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(Addition<double>,  double);

  ENSURE_FUNCTION_FACTORY(Subtraction<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(Subtraction<double>,  double);

  ENSURE_FUNCTION_FACTORY(Multiplication<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(Multiplication<double>,  double);

  ENSURE_FUNCTION_FACTORY(Division<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(Division<double>,  double);

  ENSURE_FUNCTION_FACTORY(Modulo<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(Modulo<double>,  double);

  ENSURE_FUNCTION_FACTORY(Minimum<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(Minimum<double>,  double);

  ENSURE_FUNCTION_FACTORY(Maximum<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(Maximum<double>,  double);

  ENSURE_FUNCTION_FACTORY(AbsoluteValue<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(AbsoluteValue<double>,  double);

  ENSURE_FUNCTION_FACTORY(SquareRoot<double>,  double);

  ENSURE_FUNCTION_FACTORY(Ceiling<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(Ceiling<double>,  double);

  ENSURE_FUNCTION_FACTORY(Floor<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(Floor<double>,  double);

  ENSURE_FUNCTION_FACTORY(Round<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(Round<double>,  double);

  ENSURE_FUNCTION_FACTORY(Truncate<int32_t>, int32_t);
  ENSURE_FUNCTION_FACTORY(Truncate<double>,  double);

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
      REGISTER_EXPRESSION(BooleanVariable, Boolean);
      REGISTER_EXPRESSION(IntegerVariable, Integer);
      REGISTER_EXPRESSION(RealVariable, Real);
      REGISTER_EXPRESSION(StringVariable, String);
      REGISTER_EXPRESSION(BooleanArrayVariable, BooleanArray);
      REGISTER_EXPRESSION(IntegerArrayVariable, IntegerArray);
      REGISTER_EXPRESSION(RealArrayVariable, RealArray);
      REGISTER_EXPRESSION(StringArrayVariable, StringArray);

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
      REGISTER_FUNCTION(Addition<int32_t>, int32_t, ADDInteger);
      REGISTER_FUNCTION(Addition<double>,   double,  ADDReal);
      
      REGISTER_FUNCTION(Subtraction<int32_t>, int32_t, SUBInteger);
      REGISTER_FUNCTION(Subtraction<double>,  double,  SUBReal);
      
      REGISTER_FUNCTION(Multiplication<int32_t>, int32_t, MULInteger);
      REGISTER_FUNCTION(Multiplication<double>,  double,  MULReal);
      
      REGISTER_FUNCTION(Division<int32_t>, int32_t, DIVInteger);
      REGISTER_FUNCTION(Division<double>,  double,  DIVReal);
      
      REGISTER_FUNCTION(Modulo<int32_t>, int32_t, MODInteger);
      REGISTER_FUNCTION(Modulo<double>,  double,  MODReal);
      
      REGISTER_FUNCTION(Maximum<int32_t>, int32_t, MAXInteger);
      REGISTER_FUNCTION(Maximum<double>,  double,  MAXReal);
      
      REGISTER_FUNCTION(Minimum<int32_t>, int32_t, MINInteger);
      REGISTER_FUNCTION(Minimum<double>,  double,  MINReal);
      
      REGISTER_FUNCTION(AbsoluteValue<int32_t>, int32_t, ABSInteger);
      REGISTER_FUNCTION(AbsoluteValue<double>,  double,  ABSReal);

      REGISTER_FUNCTION(SquareRooot<double>, double, SQRT);
      
      REGISTER_FUNCTION(Ceiling<int32_t>, int32_t, CEILInteger);
      REGISTER_FUNCTION(Ceiling<double>,  double,  CEILReal);
      
      REGISTER_FUNCTION(Floor<int32_t>, int32_t, FLOORInteger);
      REGISTER_FUNCTION(Floor<double>,  double,  FLOORReal);
      
      REGISTER_FUNCTION(Round<int32_t>, int32_t, ROUNDInteger);
      REGISTER_FUNCTION(Round<double>,  double,  ROUNDReal);
      
      REGISTER_FUNCTION(Truncate<int32_t>, int32_t, TRUNCInteger);
      REGISTER_FUNCTION(Truncate<double>,  double,  TRUNCReal);

      REGISTER_FUNCTION(RealToInteger, int32_t, REAL_TO_INT);

      // Boolean operations
      REGISTER_FUNCTION(BooleanNot, bool, NOT);
      REGISTER_FUNCTION(BooleanAnd, bool, AND);
      REGISTER_FUNCTION(BooleanOr,  bool, OR);
      REGISTER_FUNCTION(BooleanXor, bool, XOR);

      // String operations
      REGISTER_FUNCTION(StringConcat, std::string, CONCAT);
      REGISTER_FUNCTION(StringLength, int32_t, STRLEN);

    }
  }

} // namespace PLEXIL
