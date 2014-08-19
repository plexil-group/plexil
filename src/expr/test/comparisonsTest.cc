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


#include "Comparisons.hh"  // includes Operator.hh
#include "Constant.hh"     // includes Expression.hh
#include "Function.hh"     // includes NotifierImpl.hh
#include "TestSupport.hh"  // includes Error.hh
#include "UserVariable.hh"

using namespace PLEXIL;

static bool testIsKnown()
{
  bool tempb;

  BooleanVariable varb;
  IntegerVariable vari;
  RealVariable varr;
  StringVariable vars;

  std::vector<bool> garbage1(1, false);
  std::vector<Expression *> bexp(1, &varb);
  std::vector<Expression *> iexp(1, &vari);
  std::vector<Expression *> rexp(1, &varr);
  std::vector<Expression *> sexp(1, &vars);
  
  Function iskfnb(IsKnown::instance(), makeExprVec(bexp, garbage1));
  Function iskfni(IsKnown::instance(), makeExprVec(iexp, garbage1));
  Function iskfnr(IsKnown::instance(), makeExprVec(rexp, garbage1));
  Function iskfns(IsKnown::instance(), makeExprVec(sexp, garbage1));

  iskfnb.activate();
  iskfni.activate();
  iskfnr.activate();
  iskfns.activate();

  // All should be known but false because variables not inited
  assertTrue_1(iskfnb.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(iskfni.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(iskfnr.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(iskfns.getValue(tempb));
  assertTrue_1(!tempb);

  // Set variables and try again
  varb.setValue(true);
  vari.setValue((int32_t) 93);
  varr.setValue(27.1);
  vars.setValue(std::string("Oop"));

  // Now all should be known and true
  assertTrue_1(iskfnb.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(iskfni.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(iskfnr.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(iskfns.getValue(tempb));
  assertTrue_1(tempb);

  return true;
}

static bool testBoolean()
{
  bool tempb;

  // Boolean
  BooleanConstant troo(true);
  BooleanConstant falls(false);
  BooleanVariable varb;

  std::vector<bool> garbage2(2, false);
  std::vector<Expression *> v1, v2, v3, v4, v5, v6;
  v1.push_back(&troo);
  v1.push_back(&troo);

  v2.push_back(&falls);
  v2.push_back(&falls);

  v3.push_back(&troo);
  v3.push_back(&falls);

  v4.push_back(&falls);
  v4.push_back(&troo);

  v5.push_back(&troo);
  v5.push_back(&varb);

  v6.push_back(&varb);
  v6.push_back(&falls);

  Function boolEq1(Equal<bool>::instance(), makeExprVec(v1, garbage2));
  Function boolEq2(Equal<bool>::instance(), makeExprVec(v2, garbage2));
  Function boolEq3(Equal<bool>::instance(), makeExprVec(v3, garbage2));
  Function boolEq4(Equal<bool>::instance(), makeExprVec(v4, garbage2));
  Function boolEq5(Equal<bool>::instance(), makeExprVec(v5, garbage2));
  Function boolEq6(Equal<bool>::instance(), makeExprVec(v6, garbage2));

  Function boolNeq1(NotEqual<bool>::instance(), makeExprVec(v1, garbage2));
  Function boolNeq2(NotEqual<bool>::instance(), makeExprVec(v2, garbage2));
  Function boolNeq3(NotEqual<bool>::instance(), makeExprVec(v3, garbage2));
  Function boolNeq4(NotEqual<bool>::instance(), makeExprVec(v4, garbage2));
  Function boolNeq5(NotEqual<bool>::instance(), makeExprVec(v5, garbage2));
  Function boolNeq6(NotEqual<bool>::instance(), makeExprVec(v6, garbage2));

  boolEq1.activate();
  boolEq2.activate();
  boolEq3.activate();
  boolEq4.activate();
  boolEq5.activate();
  boolEq6.activate();

  boolNeq1.activate();
  boolNeq2.activate();
  boolNeq3.activate();
  boolNeq4.activate();
  boolNeq5.activate();
  boolNeq6.activate();

  assertTrue_1(boolEq1.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(boolEq2.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(boolEq3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(boolEq4.getValue(tempb));
  assertTrue_1(!tempb);

  // These should be unknown because varb is uninitialized
  assertTrue_1(!boolEq5.getValue(tempb));
  assertTrue_1(!boolEq6.getValue(tempb));

  assertTrue_1(boolNeq1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(boolNeq2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(boolNeq3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(boolNeq4.getValue(tempb));
  assertTrue_1(tempb);

  // These should be unknown because varb is uninitialized
  assertTrue_1(!boolNeq5.getValue(tempb));
  assertTrue_1(!boolNeq6.getValue(tempb));

  // Set varb and try again
  varb.setValue(false);
  assertTrue_1(boolEq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(boolEq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(boolNeq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(boolNeq6.getValue(tempb));
  assertTrue_1(!tempb);

  varb.setValue(true);
  assertTrue_1(boolEq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(boolEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(boolNeq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(boolNeq6.getValue(tempb));
  assertTrue_1(tempb);

  return true;
}

static bool testString()
{
  bool tempb;

  // String
  StringConstant yo("Yo!");
  StringConstant mama("Mama");
  StringVariable vars;

  std::vector<bool> garbage2(2, false);
  std::vector<Expression *> v1, v2, v3, v4, v5, v6;

  v1.push_back(&yo);
  v1.push_back(&yo);

  v2.push_back(&mama);
  v2.push_back(&mama);

  v3.push_back(&yo);
  v3.push_back(&mama);

  v4.push_back(&mama);
  v4.push_back(&yo);

  v5.push_back(&yo);
  v5.push_back(&vars);

  v6.push_back(&vars);
  v6.push_back(&mama);

  Function strEq1(Equal<std::string>::instance(), makeExprVec(v1, garbage2));
  Function strEq2(Equal<std::string>::instance(), makeExprVec(v2, garbage2));
  Function strEq3(Equal<std::string>::instance(), makeExprVec(v3, garbage2));
  Function strEq4(Equal<std::string>::instance(), makeExprVec(v4, garbage2));
  Function strEq5(Equal<std::string>::instance(), makeExprVec(v5, garbage2));
  Function strEq6(Equal<std::string>::instance(), makeExprVec(v6, garbage2));

  Function strNeq1(NotEqual<std::string>::instance(), makeExprVec(v1, garbage2));
  Function strNeq2(NotEqual<std::string>::instance(), makeExprVec(v2, garbage2));
  Function strNeq3(NotEqual<std::string>::instance(), makeExprVec(v3, garbage2));
  Function strNeq4(NotEqual<std::string>::instance(), makeExprVec(v4, garbage2));
  Function strNeq5(NotEqual<std::string>::instance(), makeExprVec(v5, garbage2));
  Function strNeq6(NotEqual<std::string>::instance(), makeExprVec(v6, garbage2));

  Function strGt1(GreaterThan<std::string>::instance(), makeExprVec(v1, garbage2));
  Function strGt2(GreaterThan<std::string>::instance(), makeExprVec(v2, garbage2));
  Function strGt3(GreaterThan<std::string>::instance(), makeExprVec(v3, garbage2));
  Function strGt4(GreaterThan<std::string>::instance(), makeExprVec(v4, garbage2));
  Function strGt5(GreaterThan<std::string>::instance(), makeExprVec(v5, garbage2));
  Function strGt6(GreaterThan<std::string>::instance(), makeExprVec(v6, garbage2));

  Function strGe1(GreaterEqual<std::string>::instance(), makeExprVec(v1, garbage2));
  Function strGe2(GreaterEqual<std::string>::instance(), makeExprVec(v2, garbage2));
  Function strGe3(GreaterEqual<std::string>::instance(), makeExprVec(v3, garbage2));
  Function strGe4(GreaterEqual<std::string>::instance(), makeExprVec(v4, garbage2));
  Function strGe5(GreaterEqual<std::string>::instance(), makeExprVec(v5, garbage2));
  Function strGe6(GreaterEqual<std::string>::instance(), makeExprVec(v6, garbage2));

  Function strLt1(LessThan<std::string>::instance(), makeExprVec(v1, garbage2));
  Function strLt2(LessThan<std::string>::instance(), makeExprVec(v2, garbage2));
  Function strLt3(LessThan<std::string>::instance(), makeExprVec(v3, garbage2));
  Function strLt4(LessThan<std::string>::instance(), makeExprVec(v4, garbage2));
  Function strLt5(LessThan<std::string>::instance(), makeExprVec(v5, garbage2));
  Function strLt6(LessThan<std::string>::instance(), makeExprVec(v6, garbage2));

  Function strLe1(LessEqual<std::string>::instance(), makeExprVec(v1, garbage2));
  Function strLe2(LessEqual<std::string>::instance(), makeExprVec(v2, garbage2));
  Function strLe3(LessEqual<std::string>::instance(), makeExprVec(v3, garbage2));
  Function strLe4(LessEqual<std::string>::instance(), makeExprVec(v4, garbage2));
  Function strLe5(LessEqual<std::string>::instance(), makeExprVec(v5, garbage2));
  Function strLe6(LessEqual<std::string>::instance(), makeExprVec(v6, garbage2));

  strEq1.activate();
  strEq2.activate();
  strEq3.activate();
  strEq4.activate();
  strEq5.activate();
  strEq6.activate();
  strNeq1.activate();
  strNeq2.activate();
  strNeq3.activate();
  strNeq4.activate();
  strNeq5.activate();
  strNeq6.activate();
  strGt1.activate();
  strGt2.activate();
  strGt3.activate();
  strGt4.activate();
  strGt5.activate();
  strGt6.activate();
  strGe1.activate();
  strGe2.activate();
  strGe3.activate();
  strGe4.activate();
  strGe5.activate();
  strGe6.activate();
  strLt1.activate();
  strLt2.activate();
  strLt3.activate();
  strLt4.activate();
  strLt5.activate();
  strLt6.activate();
  strLe1.activate();
  strLe2.activate();
  strLe3.activate();
  strLe4.activate();
  strLe5.activate();
  strLe6.activate();

  // "Yo!" ? "Yo!"
  assertTrue_1(strEq1.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strNeq1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strGt1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strGe1.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strLt1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strLe1.getValue(tempb));
  assertTrue_1(tempb);

  // "Mama" ? "Mama"
  assertTrue_1(strEq2.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strNeq2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strGt2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strGe2.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strLt2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strLe2.getValue(tempb));
  assertTrue_1(tempb);

  // "Yo!" ? "Mama"
  assertTrue_1(strEq3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strGt3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strGe3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strLt3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strLe3.getValue(tempb));
  assertTrue_1(!tempb);

  // "Mama" ? "Yo!"
  assertTrue_1(strEq4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq4.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strGt4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strGe4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strLt4.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strLe4.getValue(tempb));
  assertTrue_1(tempb);

  // These should be unknown because vars is uninitialized
  assertTrue_1(!strEq5.getValue(tempb));
  assertTrue_1(!strNeq5.getValue(tempb));
  assertTrue_1(!strGt5.getValue(tempb));
  assertTrue_1(!strGe5.getValue(tempb));
  assertTrue_1(!strLt5.getValue(tempb));
  assertTrue_1(!strLe5.getValue(tempb));

  assertTrue_1(!strEq6.getValue(tempb));
  assertTrue_1(!strNeq6.getValue(tempb));
  assertTrue_1(!strGt6.getValue(tempb));
  assertTrue_1(!strGe6.getValue(tempb));
  assertTrue_1(!strLt6.getValue(tempb));
  assertTrue_1(!strLe6.getValue(tempb));

  // Set vars and try again
  vars.setValue(std::string("Mama"));

  // "Yo!" ? "Mama"
  assertTrue_1(strEq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strGt5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strGe5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strLt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strLe5.getValue(tempb));
  assertTrue_1(!tempb);

  // "Mama" ? "Mama"
  assertTrue_1(strEq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strNeq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strGt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strGe6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strLt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strLe6.getValue(tempb));
  assertTrue_1(tempb);

  vars.setValue(std::string("Yo!"));
  // "Yo!" ? "Yo!"
  assertTrue_1(strEq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strNeq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strGt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strGe5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strLt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strLe5.getValue(tempb));
  assertTrue_1(tempb);

  // "Yo!" ? "Mama"
  assertTrue_1(strEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strGt6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strGe6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strLt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strLe6.getValue(tempb));
  assertTrue_1(!tempb);

  vars.setValue(std::string("Yo Mama!"));
  // "Yo!" ? "Yo mama!"
  assertTrue_1(strEq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strGt5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strGe5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strLt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strLe5.getValue(tempb));
  assertTrue_1(!tempb);

  // "Yo mama!" ? "Mama"
  assertTrue_1(strEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strGt6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strGe6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strLt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strLe6.getValue(tempb));
  assertTrue_1(!tempb);

  return true;
}

static bool testInteger()
{
  bool tempb;

  // Integer
  IntegerConstant won(1);
  IntegerConstant too(2);
  IntegerVariable vari;

  std::vector<bool> garbage2(2, false);
  std::vector<Expression *> v1, v2, v3, v4, v5, v6;

  v1.push_back(&won);
  v1.push_back(&won);

  v2.push_back(&too);
  v2.push_back(&too);

  v3.push_back(&won);
  v3.push_back(&too);

  v4.push_back(&too);
  v4.push_back(&won);

  v5.push_back(&won);
  v5.push_back(&vari);

  v6.push_back(&vari);
  v6.push_back(&too);

  Function intEq1(Equal<int32_t>::instance(), makeExprVec(v1, garbage2));
  Function intEq2(Equal<int32_t>::instance(), makeExprVec(v2, garbage2));
  Function intEq3(Equal<int32_t>::instance(), makeExprVec(v3, garbage2));
  Function intEq4(Equal<int32_t>::instance(), makeExprVec(v4, garbage2));
  Function intEq5(Equal<int32_t>::instance(), makeExprVec(v5, garbage2));
  Function intEq6(Equal<int32_t>::instance(), makeExprVec(v6, garbage2));

  Function intNeq1(NotEqual<int32_t>::instance(), makeExprVec(v1, garbage2));
  Function intNeq2(NotEqual<int32_t>::instance(), makeExprVec(v2, garbage2));
  Function intNeq3(NotEqual<int32_t>::instance(), makeExprVec(v3, garbage2));
  Function intNeq4(NotEqual<int32_t>::instance(), makeExprVec(v4, garbage2));
  Function intNeq5(NotEqual<int32_t>::instance(), makeExprVec(v5, garbage2));
  Function intNeq6(NotEqual<int32_t>::instance(), makeExprVec(v6, garbage2));

  Function intGt1(GreaterThan<int32_t>::instance(), makeExprVec(v1, garbage2));
  Function intGt2(GreaterThan<int32_t>::instance(), makeExprVec(v2, garbage2));
  Function intGt3(GreaterThan<int32_t>::instance(), makeExprVec(v3, garbage2));
  Function intGt4(GreaterThan<int32_t>::instance(), makeExprVec(v4, garbage2));
  Function intGt5(GreaterThan<int32_t>::instance(), makeExprVec(v5, garbage2));
  Function intGt6(GreaterThan<int32_t>::instance(), makeExprVec(v6, garbage2));

  Function intGe1(GreaterEqual<int32_t>::instance(), makeExprVec(v1, garbage2));
  Function intGe2(GreaterEqual<int32_t>::instance(), makeExprVec(v2, garbage2));
  Function intGe3(GreaterEqual<int32_t>::instance(), makeExprVec(v3, garbage2));
  Function intGe4(GreaterEqual<int32_t>::instance(), makeExprVec(v4, garbage2));
  Function intGe5(GreaterEqual<int32_t>::instance(), makeExprVec(v5, garbage2));
  Function intGe6(GreaterEqual<int32_t>::instance(), makeExprVec(v6, garbage2));

  Function intLt1(LessThan<int32_t>::instance(), makeExprVec(v1, garbage2));
  Function intLt2(LessThan<int32_t>::instance(), makeExprVec(v2, garbage2));
  Function intLt3(LessThan<int32_t>::instance(), makeExprVec(v3, garbage2));
  Function intLt4(LessThan<int32_t>::instance(), makeExprVec(v4, garbage2));
  Function intLt5(LessThan<int32_t>::instance(), makeExprVec(v5, garbage2));
  Function intLt6(LessThan<int32_t>::instance(), makeExprVec(v6, garbage2));

  Function intLe1(LessEqual<int32_t>::instance(), makeExprVec(v1, garbage2));
  Function intLe2(LessEqual<int32_t>::instance(), makeExprVec(v2, garbage2));
  Function intLe3(LessEqual<int32_t>::instance(), makeExprVec(v3, garbage2));
  Function intLe4(LessEqual<int32_t>::instance(), makeExprVec(v4, garbage2));
  Function intLe5(LessEqual<int32_t>::instance(), makeExprVec(v5, garbage2));
  Function intLe6(LessEqual<int32_t>::instance(), makeExprVec(v6, garbage2));

  intEq1.activate();
  intEq2.activate();
  intEq3.activate();
  intEq4.activate();
  intEq5.activate();
  intEq6.activate();
  intNeq1.activate();
  intNeq2.activate();
  intNeq3.activate();
  intNeq4.activate();
  intNeq5.activate();
  intNeq6.activate();
  intGt1.activate();
  intGt2.activate();
  intGt3.activate();
  intGt4.activate();
  intGt5.activate();
  intGt6.activate();
  intGe1.activate();
  intGe2.activate();
  intGe3.activate();
  intGe4.activate();
  intGe5.activate();
  intGe6.activate();
  intLt1.activate();
  intLt2.activate();
  intLt3.activate();
  intLt4.activate();
  intLt5.activate();
  intLt6.activate();
  intLe1.activate();
  intLe2.activate();
  intLe3.activate();
  intLe4.activate();
  intLe5.activate();
  intLe6.activate();

  // 1 ? 1
  assertTrue_1(intEq1.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intNeq1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGt1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGe1.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intLt1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intLe1.getValue(tempb));
  assertTrue_1(tempb);

  // 2 ? 2
  assertTrue_1(intEq2.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intNeq2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGt2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGe2.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intLt2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intLe2.getValue(tempb));
  assertTrue_1(tempb);

  // 1 ? 2
  assertTrue_1(intEq3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intNeq3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intGt3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGe3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intLt3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intLe3.getValue(tempb));
  assertTrue_1(tempb);

  // 2 ? 1
  assertTrue_1(intEq4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intNeq4.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intGt4.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intGe4.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intLt4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intLe4.getValue(tempb));
  assertTrue_1(!tempb);

  // These should be unknown because vari is uninitialized
  // 1 ? vari
  assertTrue_1(!intEq5.getValue(tempb));
  assertTrue_1(!intNeq5.getValue(tempb));
  assertTrue_1(!intGt5.getValue(tempb));
  assertTrue_1(!intGe5.getValue(tempb));
  assertTrue_1(!intLt5.getValue(tempb));
  assertTrue_1(!intLe5.getValue(tempb));

  // vari ? 2
  assertTrue_1(!intEq6.getValue(tempb));
  assertTrue_1(!intNeq6.getValue(tempb));
  assertTrue_1(!intGt6.getValue(tempb));
  assertTrue_1(!intGe6.getValue(tempb));
  assertTrue_1(!intLt6.getValue(tempb));
  assertTrue_1(!intLe6.getValue(tempb));

  // Set vari and try again
  vari.setValue((int32_t) 2);
    
  // 1 ? vari
  assertTrue_1(intEq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intNeq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intGt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGe5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intLt5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intLe5.getValue(tempb));
  assertTrue_1(tempb);

  // vari ? 2
  assertTrue_1(intEq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intNeq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGe6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intLt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intLe6.getValue(tempb));
  assertTrue_1(tempb);

  vari.setValue((int32_t) 1);

  // 1 ? vari
  assertTrue_1(intEq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intNeq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGe5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intLt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intLe5.getValue(tempb));
  assertTrue_1(tempb);

  // vari ? 2
  assertTrue_1(intEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intNeq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intGt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGe6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intLt6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intLe6.getValue(tempb));
  assertTrue_1(tempb);

  vari.setValue((int32_t) -1);

  // 1 ? vari
  assertTrue_1(intEq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intNeq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intGt5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intGe5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intLt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intLe5.getValue(tempb));
  assertTrue_1(!tempb);

  // vari ? 2
  assertTrue_1(intEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intNeq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intGt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intGe6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(intLt6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(intLe6.getValue(tempb));
  assertTrue_1(tempb);

  return true;
}

static bool testReal()
{
  bool tempb;

  // Real
  RealConstant wontoo(1);
  RealConstant tootoo(2);
  RealVariable varr;

  std::vector<bool> garbage2(2, false);
  std::vector<Expression *> v1, v2, v3, v4, v5, v6;

  v1.push_back(&wontoo);
  v1.push_back(&wontoo);

  v2.push_back(&tootoo);
  v2.push_back(&tootoo);

  v3.push_back(&wontoo);
  v3.push_back(&tootoo);

  v4.push_back(&tootoo);
  v4.push_back(&wontoo);

  v5.push_back(&wontoo);
  v5.push_back(&varr);

  v6.push_back(&varr);
  v6.push_back(&tootoo);

  Function dblEq1(Equal<double>::instance(), makeExprVec(v1, garbage2));
  Function dblEq2(Equal<double>::instance(), makeExprVec(v2, garbage2));
  Function dblEq3(Equal<double>::instance(), makeExprVec(v3, garbage2));
  Function dblEq4(Equal<double>::instance(), makeExprVec(v4, garbage2));
  Function dblEq5(Equal<double>::instance(), makeExprVec(v5, garbage2));
  Function dblEq6(Equal<double>::instance(), makeExprVec(v6, garbage2));

  Function dblNeq1(NotEqual<double>::instance(), makeExprVec(v1, garbage2));
  Function dblNeq2(NotEqual<double>::instance(), makeExprVec(v2, garbage2));
  Function dblNeq3(NotEqual<double>::instance(), makeExprVec(v3, garbage2));
  Function dblNeq4(NotEqual<double>::instance(), makeExprVec(v4, garbage2));
  Function dblNeq5(NotEqual<double>::instance(), makeExprVec(v5, garbage2));
  Function dblNeq6(NotEqual<double>::instance(), makeExprVec(v6, garbage2));

  Function dblGt1(GreaterThan<double>::instance(), makeExprVec(v1, garbage2));
  Function dblGt2(GreaterThan<double>::instance(), makeExprVec(v2, garbage2));
  Function dblGt3(GreaterThan<double>::instance(), makeExprVec(v3, garbage2));
  Function dblGt4(GreaterThan<double>::instance(), makeExprVec(v4, garbage2));
  Function dblGt5(GreaterThan<double>::instance(), makeExprVec(v5, garbage2));
  Function dblGt6(GreaterThan<double>::instance(), makeExprVec(v6, garbage2));

  Function dblGe1(GreaterEqual<double>::instance(), makeExprVec(v1, garbage2));
  Function dblGe2(GreaterEqual<double>::instance(), makeExprVec(v2, garbage2));
  Function dblGe3(GreaterEqual<double>::instance(), makeExprVec(v3, garbage2));
  Function dblGe4(GreaterEqual<double>::instance(), makeExprVec(v4, garbage2));
  Function dblGe5(GreaterEqual<double>::instance(), makeExprVec(v5, garbage2));
  Function dblGe6(GreaterEqual<double>::instance(), makeExprVec(v6, garbage2));

  Function dblLt1(LessThan<double>::instance(), makeExprVec(v1, garbage2));
  Function dblLt2(LessThan<double>::instance(), makeExprVec(v2, garbage2));
  Function dblLt3(LessThan<double>::instance(), makeExprVec(v3, garbage2));
  Function dblLt4(LessThan<double>::instance(), makeExprVec(v4, garbage2));
  Function dblLt5(LessThan<double>::instance(), makeExprVec(v5, garbage2));
  Function dblLt6(LessThan<double>::instance(), makeExprVec(v6, garbage2));

  Function dblLe1(LessEqual<double>::instance(), makeExprVec(v1, garbage2));
  Function dblLe2(LessEqual<double>::instance(), makeExprVec(v2, garbage2));
  Function dblLe3(LessEqual<double>::instance(), makeExprVec(v3, garbage2));
  Function dblLe4(LessEqual<double>::instance(), makeExprVec(v4, garbage2));
  Function dblLe5(LessEqual<double>::instance(), makeExprVec(v5, garbage2));
  Function dblLe6(LessEqual<double>::instance(), makeExprVec(v6, garbage2));

  dblEq1.activate();
  dblEq2.activate();
  dblEq3.activate();
  dblEq4.activate();
  dblEq5.activate();
  dblEq6.activate();
  dblNeq1.activate();
  dblNeq2.activate();
  dblNeq3.activate();
  dblNeq4.activate();
  dblNeq5.activate();
  dblNeq6.activate();
  dblGt1.activate();
  dblGt2.activate();
  dblGt3.activate();
  dblGt4.activate();
  dblGt5.activate();
  dblGt6.activate();
  dblGe1.activate();
  dblGe2.activate();
  dblGe3.activate();
  dblGe4.activate();
  dblGe5.activate();
  dblGe6.activate();
  dblLt1.activate();
  dblLt2.activate();
  dblLt3.activate();
  dblLt4.activate();
  dblLt5.activate();
  dblLt6.activate();
  dblLe1.activate();
  dblLe2.activate();
  dblLe3.activate();
  dblLe4.activate();
  dblLe5.activate();
  dblLe6.activate();

  // 1 ? 1
  assertTrue_1(dblEq1.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblNeq1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGt1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe1.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLt1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLe1.getValue(tempb));
  assertTrue_1(tempb);

  // 2 ? 2
  assertTrue_1(dblEq2.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblNeq2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGt2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe2.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLt2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLe2.getValue(tempb));
  assertTrue_1(tempb);

  // 1 ? 2
  assertTrue_1(dblEq3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblGt3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLt3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLe3.getValue(tempb));
  assertTrue_1(tempb);

  // 2 ? 1
  assertTrue_1(dblEq4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq4.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblGt4.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblGe4.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLt4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLe4.getValue(tempb));
  assertTrue_1(!tempb);

  // These should be unknown because varr is uninitialized
  assertTrue_1(!dblEq5.getValue(tempb));
  assertTrue_1(!dblNeq5.getValue(tempb));
  assertTrue_1(!dblGt5.getValue(tempb));
  assertTrue_1(!dblGe5.getValue(tempb));
  assertTrue_1(!dblLt5.getValue(tempb));
  assertTrue_1(!dblLe5.getValue(tempb));

  assertTrue_1(!dblEq6.getValue(tempb));
  assertTrue_1(!dblNeq6.getValue(tempb));
  assertTrue_1(!dblGt6.getValue(tempb));
  assertTrue_1(!dblGe6.getValue(tempb));
  assertTrue_1(!dblLt6.getValue(tempb));
  assertTrue_1(!dblLe6.getValue(tempb));

  // Set varr and try again
  varr.setValue((int32_t) 2);

  // 1 ? 2
  assertTrue_1(dblEq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblGt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLt5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLe5.getValue(tempb));
  assertTrue_1(tempb);

  // 2 ? 2
  assertTrue_1(dblEq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblNeq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLe6.getValue(tempb));
  assertTrue_1(tempb);

  varr.setValue((int32_t) 1);
  // 1 ? 1
  assertTrue_1(dblEq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblNeq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLe5.getValue(tempb));
  assertTrue_1(tempb);

  // 1 ? 2
  assertTrue_1(dblEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblGt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLt6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLe6.getValue(tempb));
  assertTrue_1(tempb);

  varr.setValue((int32_t) -1);
  // 1 ? -1
  assertTrue_1(dblEq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblGt5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblGe5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLe5.getValue(tempb));
  assertTrue_1(!tempb);

  // -1 ? 2
  assertTrue_1(dblEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblGt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLt6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLe6.getValue(tempb));
  assertTrue_1(tempb);

  return true;
}

static bool testMixedNumerics()
{
  bool tempb;

  RealConstant won(1);
  RealConstant too(2);
  IntegerConstant wontoo(1);
  IntegerConstant tootoo(2);

  std::vector<bool> garbage2(2, false);
  std::vector<Expression *> v1, v2, v3, v4, v5, v6;

  v1.push_back(&won);
  v1.push_back(&wontoo);

  v2.push_back(&tootoo);
  v2.push_back(&too);

  v3.push_back(&wontoo);
  v3.push_back(&too);

  v4.push_back(&won);
  v4.push_back(&tootoo);

  v5.push_back(&wontoo);
  v5.push_back(&wontoo);

  v6.push_back(&wontoo);
  v6.push_back(&tootoo);

  Function dblEq1(Equal<double>::instance(), makeExprVec(v1, garbage2));
  Function dblEq2(Equal<double>::instance(), makeExprVec(v2, garbage2));
  Function dblEq3(Equal<double>::instance(), makeExprVec(v3, garbage2));
  Function dblEq4(Equal<double>::instance(), makeExprVec(v4, garbage2));
  Function dblEq5(Equal<double>::instance(), makeExprVec(v5, garbage2));
  Function dblEq6(Equal<double>::instance(), makeExprVec(v6, garbage2));

  Function dblNeq1(NotEqual<double>::instance(), makeExprVec(v1, garbage2));
  Function dblNeq2(NotEqual<double>::instance(), makeExprVec(v2, garbage2));
  Function dblNeq3(NotEqual<double>::instance(), makeExprVec(v3, garbage2));
  Function dblNeq4(NotEqual<double>::instance(), makeExprVec(v4, garbage2));
  Function dblNeq5(NotEqual<double>::instance(), makeExprVec(v5, garbage2));
  Function dblNeq6(NotEqual<double>::instance(), makeExprVec(v6, garbage2));

  Function dblGt1(GreaterThan<double>::instance(), makeExprVec(v1, garbage2));
  Function dblGt2(GreaterThan<double>::instance(), makeExprVec(v2, garbage2));
  Function dblGt3(GreaterThan<double>::instance(), makeExprVec(v3, garbage2));
  Function dblGt4(GreaterThan<double>::instance(), makeExprVec(v4, garbage2));
  Function dblGt5(GreaterThan<double>::instance(), makeExprVec(v5, garbage2));
  Function dblGt6(GreaterThan<double>::instance(), makeExprVec(v6, garbage2));

  Function dblGe1(GreaterEqual<double>::instance(), makeExprVec(v1, garbage2));
  Function dblGe2(GreaterEqual<double>::instance(), makeExprVec(v2, garbage2));
  Function dblGe3(GreaterEqual<double>::instance(), makeExprVec(v3, garbage2));
  Function dblGe4(GreaterEqual<double>::instance(), makeExprVec(v4, garbage2));
  Function dblGe5(GreaterEqual<double>::instance(), makeExprVec(v5, garbage2));
  Function dblGe6(GreaterEqual<double>::instance(), makeExprVec(v6, garbage2));

  Function dblLt1(LessThan<double>::instance(), makeExprVec(v1, garbage2));
  Function dblLt2(LessThan<double>::instance(), makeExprVec(v2, garbage2));
  Function dblLt3(LessThan<double>::instance(), makeExprVec(v3, garbage2));
  Function dblLt4(LessThan<double>::instance(), makeExprVec(v4, garbage2));
  Function dblLt5(LessThan<double>::instance(), makeExprVec(v5, garbage2));
  Function dblLt6(LessThan<double>::instance(), makeExprVec(v6, garbage2));

  Function dblLe1(LessEqual<double>::instance(), makeExprVec(v1, garbage2));
  Function dblLe2(LessEqual<double>::instance(), makeExprVec(v2, garbage2));
  Function dblLe3(LessEqual<double>::instance(), makeExprVec(v3, garbage2));
  Function dblLe4(LessEqual<double>::instance(), makeExprVec(v4, garbage2));
  Function dblLe5(LessEqual<double>::instance(), makeExprVec(v5, garbage2));
  Function dblLe6(LessEqual<double>::instance(), makeExprVec(v6, garbage2));

  dblEq1.activate();
  dblEq2.activate();
  dblEq3.activate();
  dblEq4.activate();
  dblEq5.activate();
  dblEq6.activate();
  dblNeq1.activate();
  dblNeq2.activate();
  dblNeq3.activate();
  dblNeq4.activate();
  dblNeq5.activate();
  dblNeq6.activate();
  dblGt1.activate();
  dblGt2.activate();
  dblGt3.activate();
  dblGt4.activate();
  dblGt5.activate();
  dblGt6.activate();
  dblGe1.activate();
  dblGe2.activate();
  dblGe3.activate();
  dblGe4.activate();
  dblGe5.activate();
  dblGe6.activate();
  dblLt1.activate();
  dblLt2.activate();
  dblLt3.activate();
  dblLt4.activate();
  dblLt5.activate();
  dblLt6.activate();
  dblLe1.activate();
  dblLe2.activate();
  dblLe3.activate();
  dblLe4.activate();
  dblLe5.activate();
  dblLe6.activate();

  // Real 1 ? Integer 1
  assertTrue_1(dblEq1.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblNeq1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGt1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe1.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLt1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLe1.getValue(tempb));
  assertTrue_1(tempb);

  // Int 2 ? Real 2
  assertTrue_1(dblEq2.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblNeq2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGt2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe2.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLt2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLe2.getValue(tempb));
  assertTrue_1(tempb);

  // Int 1 ? Real 2
  assertTrue_1(dblEq3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblGt3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLt3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLe3.getValue(tempb));
  assertTrue_1(tempb);

  // Real 1 ? Int 2
  assertTrue_1(dblEq4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq4.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblGt4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLt4.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLe4.getValue(tempb));
  assertTrue_1(tempb);

  // Int 1 ? Int 1
  assertTrue_1(dblEq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblNeq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLt5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLe5.getValue(tempb));
  assertTrue_1(tempb);

  // Int 1 ? Int 2
  assertTrue_1(dblEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblGt6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblGe6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblLt6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblLe6.getValue(tempb));
  assertTrue_1(tempb);
    
  return true;
}

bool comparisonsTest()
{
  runTest(testIsKnown);
  runTest(testBoolean);
  runTest(testString);
  runTest(testInteger);
  runTest(testReal);
  runTest(testMixedNumerics);
  return true;
}
