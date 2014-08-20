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
#include "Function.hh"
#include "Constant.hh"
#include "TestSupport.hh"
#include "UserVariable.hh"

using namespace PLEXIL;

// Work both two-arg and three-arg versions
bool additionTest()
{
  //std::vector<bool> garbage1(1, false);
  std::vector<bool> garbage2(2, false);
  std::vector<bool> garbage3(3, false);

  // Integer
  IntegerConstant m1(-1);
  IntegerConstant won(1);
  IntegerConstant tree(3);
  IntegerVariable i;
  Addition<int32_t> addi;
  int32_t tempi;

  // Unary not implemented in schema

  // Binary

  // Set up arglists
  std::vector<Expression *> iexprs2_1, iexprs2_2, iexprs2_3;
  iexprs2_1.push_back(&m1);
  iexprs2_1.push_back(&tree);

  iexprs2_2.push_back(&i);
  iexprs2_2.push_back(&won);

  iexprs2_3.push_back(&tree);
  iexprs2_3.push_back(&i);

  Function iadd2_1(&addi, makeExprVec(iexprs2_1, garbage2));
  Function iadd2_2(&addi, makeExprVec(iexprs2_2, garbage2));
  Function iadd2_3(&addi, makeExprVec(iexprs2_3, garbage2));

  iadd2_1.activate();
  iadd2_2.activate();
  iadd2_3.activate();

  assertTrue_1(iadd2_1.getValue(tempi));
  assertTrue_1(tempi == 2);

  // Should be unknown because i not initialized yet
  assertTrue_1(!iadd2_2.getValue(tempi));
  assertTrue_1(!iadd2_3.getValue(tempi));

  i.setValue((int32_t) 5);
  // should be known now
  assertTrue_1(iadd2_2.getValue(tempi));
  assertTrue_1(tempi == 6);
  assertTrue_1(iadd2_3.getValue(tempi));
  assertTrue_1(tempi == 8);

  // N-ary

  // Set up arglists
  std::vector<Expression *> iexprs3_1, iexprs3_2, iexprs3_3;
  iexprs3_1.push_back(&m1);
  iexprs3_1.push_back(&won);
  iexprs3_1.push_back(&tree);

  iexprs3_2.push_back(&won);
  iexprs3_2.push_back(&tree);
  iexprs3_2.push_back(&i);

  iexprs3_3.push_back(&i);
  iexprs3_3.push_back(&m1);
  iexprs3_3.push_back(&won);

  Function iadd3_1(&addi, makeExprVec(iexprs3_1, garbage3));
  Function iadd3_2(&addi, makeExprVec(iexprs3_2, garbage3));
  Function iadd3_3(&addi, makeExprVec(iexprs3_3, garbage3));

  iadd3_1.activate();
  iadd3_2.activate();
  iadd3_3.activate();
  
  i.setUnknown();

  assertTrue_1(iadd3_1.getValue(tempi));
  assertTrue_1(tempi == 3);

  // Should be unknown because i unknown
  assertTrue_1(!iadd3_2.getValue(tempi));
  assertTrue_1(!iadd3_3.getValue(tempi));

  i.setValue((int32_t) 27);
  // should be known now
  assertTrue_1(iadd3_2.getValue(tempi));
  assertTrue_1(tempi == 31);
  assertTrue_1(iadd3_3.getValue(tempi));
  assertTrue_1(tempi == 27);

  // Integer operator conversion to real
  double tempr;

  assertTrue_1(iadd2_1.getValue(tempr));
  assertTrue_1(tempr == 2.0);
  assertTrue_1(iadd2_2.getValue(tempr));
  assertTrue_1(tempr == 28.0);
  assertTrue_1(iadd2_3.getValue(tempr));
  assertTrue_1(tempr == 30.0);

  assertTrue_1(iadd3_1.getValue(tempr));
  assertTrue_1(tempr == 3.0);
  assertTrue_1(iadd3_2.getValue(tempr));
  assertTrue_1(tempr == 31.0);
  assertTrue_1(iadd3_3.getValue(tempr));
  assertTrue_1(tempr == 27.0);

  // Real
  RealConstant too(2.5);
  RealConstant fore(4.5);
  RealVariable x;
  Addition<double> addr;

  // Unary not supported in schema

  // Binary

  // Set up arglists
  std::vector<Expression *> rexprs2_1, rexprs2_2, rexprs2_3;
  rexprs2_1.push_back(&too);
  rexprs2_1.push_back(&fore);

  rexprs2_2.push_back(&x);
  rexprs2_2.push_back(&too);

  rexprs2_3.push_back(&fore);
  rexprs2_3.push_back(&x);

  Function radd2_1(&addr, makeExprVec(rexprs2_1, garbage2));
  Function radd2_2(&addr, makeExprVec(rexprs2_2, garbage2));
  Function radd2_3(&addr, makeExprVec(rexprs2_3, garbage2));

  radd2_1.activate();
  radd2_2.activate();
  radd2_3.activate();

  assertTrue_1(radd2_1.getValue(tempr));
  assertTrue_1(tempr == 7.0);

  // Should be unknown because x not initialized yet
  assertTrue_1(!radd2_2.getValue(tempr));
  assertTrue_1(!radd2_3.getValue(tempr));

  x.setValue(-0.5);
  // should be known now
  assertTrue_1(radd2_2.getValue(tempr));
  assertTrue_1(tempr == 2.0);
  assertTrue_1(radd2_3.getValue(tempr));
  assertTrue_1(tempr == 4.0);

  // N-ary
  std::vector<Expression *> rexprs3_1, rexprs3_2, rexprs3_3;
  rexprs3_1.push_back(&too);
  rexprs3_1.push_back(&fore);
  rexprs3_1.push_back(&too);

  rexprs3_2.push_back(&x);
  rexprs3_2.push_back(&too);
  rexprs3_2.push_back(&fore);

  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&x);

  Function radd3_1(&addr, makeExprVec(rexprs3_1, garbage3));
  Function radd3_2(&addr, makeExprVec(rexprs3_2, garbage3));
  Function radd3_3(&addr, makeExprVec(rexprs3_3, garbage3));

  radd3_1.activate();
  radd3_2.activate();
  radd3_3.activate();
  
  x.setUnknown();

  assertTrue_1(radd3_1.getValue(tempr));
  assertTrue_1(tempr == 9.5);

  // Should be unknown because x unknown
  assertTrue_1(!radd3_2.getValue(tempr));
  assertTrue_1(!radd3_3.getValue(tempr));

  x.setValue(3.25);
  // should be known now
  assertTrue_1(radd3_2.getValue(tempr));
  assertTrue_1(tempr == 10.25);
  assertTrue_1(radd3_3.getValue(tempr));
  assertTrue_1(tempr == 8.25);


  // Mixed numeric

  // Binary

  // Set up arglists
  std::vector<Expression *> mexprs2_1, mexprs2_2, mexprs2_3;
  mexprs2_1.push_back(&too);
  mexprs2_1.push_back(&tree);

  mexprs2_2.push_back(&i);
  mexprs2_2.push_back(&too);

  mexprs2_3.push_back(&tree);
  mexprs2_3.push_back(&x);

  Function madd2_1(&addr, makeExprVec(mexprs2_1, garbage2));
  Function madd2_2(&addr, makeExprVec(mexprs2_2, garbage2));
  Function madd2_3(&addr, makeExprVec(mexprs2_3, garbage2));

  madd2_1.activate();
  madd2_2.activate();
  madd2_3.activate();
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(madd2_1.getValue(tempr));
  assertTrue_1(tempr == 5.5);

  // Should be unknown because i, x unknown
  assertTrue_1(!madd2_2.getValue(tempr));
  assertTrue_1(!madd2_3.getValue(tempr));

  i.setValue((int32_t) 42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(madd2_2.getValue(tempr));
  assertTrue_1(tempr == 44.5);
  assertTrue_1(madd2_3.getValue(tempr));
  assertTrue_1(tempr == 2.5);

  // N-ary
  rexprs3_1.clear();
  rexprs3_1.push_back(&tree);
  rexprs3_1.push_back(&fore);
  rexprs3_1.push_back(&m1);

  rexprs3_2.clear();
  rexprs3_2.push_back(&x);
  rexprs3_2.push_back(&won);
  rexprs3_2.push_back(&fore);

  rexprs3_3.clear();
  rexprs3_3.push_back(&tree);
  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&i);

  Function madd3_1(&addr, makeExprVec(rexprs3_1, garbage3));
  Function madd3_2(&addr, makeExprVec(rexprs3_2, garbage3));
  Function madd3_3(&addr, makeExprVec(rexprs3_3, garbage3));

  madd3_1.activate();
  madd3_2.activate();
  madd3_3.activate();
  
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(madd3_1.getValue(tempr));
  assertTrue_1(tempr == 6.5);

  // Should be unknown because i, x unknown
  assertTrue_1(!madd3_2.getValue(tempr));
  assertTrue_1(!madd3_3.getValue(tempr));

  i.setValue((int32_t) 42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(madd3_2.getValue(tempr));
  assertTrue_1(tempr == 5.0);
  assertTrue_1(madd3_3.getValue(tempr));
  assertTrue_1(tempr == 47.5);

  return true;
}

bool subtractionTest()
{
  // Integer
  IntegerConstant m1(-1);
  IntegerConstant won(1);
  IntegerConstant tree(3);
  IntegerVariable i;
  Subtraction<int32_t> subi;
  int32_t tempi;
  std::vector<bool> garbage1(1, false);
  std::vector<bool> garbage2(2, false);
  std::vector<bool> garbage3(3, false);

  // Unary
  std::vector<Expression *> iexprs1_1, iexprs1_2, iexprs1_3;
  iexprs1_1.push_back(&m1);
  iexprs1_2.push_back(&tree);
  iexprs1_3.push_back(&i);

  Function isub1_1(&subi, makeExprVec(iexprs1_1, garbage1));
  Function isub1_2(&subi, makeExprVec(iexprs1_2, garbage1));
  Function isub1_3(&subi, makeExprVec(iexprs1_3, garbage1));

  isub1_1.activate();
  isub1_2.activate();
  isub1_3.activate();

  assertTrue_1(isub1_1.getValue(tempi));
  assertTrue_1(tempi == 1);
  assertTrue_1(isub1_2.getValue(tempi));
  assertTrue_1(tempi == -3);

  // Should be unknown because i not initialized yet
  assertTrue_1(!isub1_3.getValue(tempi));

  i.setValue((int32_t) 7);
  // should be known now
  assertTrue_1(isub1_3.getValue(tempi));
  assertTrue_1(tempi == -7);

  // Integer operator to real result
  double tempr;

  assertTrue_1(isub1_1.getValue(tempr));
  assertTrue_1(tempr == 1.0);
  assertTrue_1(isub1_2.getValue(tempr));
  assertTrue_1(tempr == -3.0);
  assertTrue_1(isub1_3.getValue(tempr));
  assertTrue_1(tempr == -7.0);

  // Binary
  std::vector<Expression *> iexprs2_1, iexprs2_2, iexprs2_3;
  iexprs2_1.push_back(&m1);
  iexprs2_1.push_back(&tree);

  iexprs2_2.push_back(&i);
  iexprs2_2.push_back(&won);

  iexprs2_3.push_back(&tree);
  iexprs2_3.push_back(&i);

  Function isub2_1(&subi, makeExprVec(iexprs2_1, garbage2));
  Function isub2_2(&subi, makeExprVec(iexprs2_2, garbage2));
  Function isub2_3(&subi, makeExprVec(iexprs2_3, garbage2));

  isub2_1.activate();
  isub2_2.activate();
  isub2_3.activate();

  i.setUnknown();
  
  assertTrue_1(isub2_1.getValue(tempi));
  assertTrue_1(tempi == -4);

  // Should be unknown because i unknown
  assertTrue_1(!isub2_2.getValue(tempi));
  assertTrue_1(!isub2_3.getValue(tempi));

  i.setValue((int32_t) 5);
  // should be known now
  assertTrue_1(isub2_2.getValue(tempi));
  assertTrue_1(tempi == 4);
  assertTrue_1(isub2_3.getValue(tempi));
  assertTrue_1(tempi == -2);

  // N-ary (not supported in schema)

  // Set up arglists
  std::vector<Expression *> iexprs3_1, iexprs3_2, iexprs3_3;
  iexprs3_1.push_back(&m1);
  iexprs3_1.push_back(&won);
  iexprs3_1.push_back(&tree);

  iexprs3_2.push_back(&won);
  iexprs3_2.push_back(&tree);
  iexprs3_2.push_back(&i);

  iexprs3_3.push_back(&i);
  iexprs3_3.push_back(&m1);
  iexprs3_3.push_back(&won);

  Function isub3_1(&subi, makeExprVec(iexprs3_1, garbage3));
  Function isub3_2(&subi, makeExprVec(iexprs3_2, garbage3));
  Function isub3_3(&subi, makeExprVec(iexprs3_3, garbage3));

  isub3_1.activate();
  isub3_2.activate();
  isub3_3.activate();
  
  i.setUnknown();

  assertTrue_1(isub3_1.getValue(tempi));
  assertTrue_1(tempi == -5);

  // Should be unknown because i unknown
  assertTrue_1(!isub3_2.getValue(tempi));
  assertTrue_1(!isub3_3.getValue(tempi));

  i.setValue((int32_t) 27);
  // should be known now
  assertTrue_1(isub3_2.getValue(tempi));
  assertTrue_1(tempi == -29);
  assertTrue_1(isub3_3.getValue(tempi));
  assertTrue_1(tempi == 27);

  // Real
  RealConstant too(2.5);
  RealConstant fore(4.5);
  RealVariable x;
  Subtraction<double> subr;

  // Unary
  std::vector<Expression *> rexprs1_1, rexprs1_2, rexprs1_3;
  rexprs1_1.push_back(&too);
  rexprs1_2.push_back(&fore);
  rexprs1_3.push_back(&x);

  Function rsub1_1(&subr, makeExprVec(rexprs1_1, garbage1));
  Function rsub1_2(&subr, makeExprVec(rexprs1_2, garbage1));
  Function rsub1_3(&subr, makeExprVec(rexprs1_3, garbage1));

  rsub1_1.activate();
  rsub1_2.activate();
  rsub1_3.activate();

  assertTrue_1(rsub1_1.getValue(tempr));
  assertTrue_1(tempr == -2.5);
  assertTrue_1(rsub1_2.getValue(tempr));
  assertTrue_1(tempr == -4.5);

  // Should be unknown because x not initialized yet
  assertTrue_1(!rsub1_3.getValue(tempr));

  x.setValue(1.5);
  // should be known now
  assertTrue_1(rsub1_3.getValue(tempr));
  assertTrue_1(tempr == -1.5);

  // Binary
  std::vector<Expression *> rexprs2_1, rexprs2_2, rexprs2_3;
  rexprs2_1.push_back(&too);
  rexprs2_1.push_back(&fore);

  rexprs2_2.push_back(&x);
  rexprs2_2.push_back(&too);

  rexprs2_3.push_back(&fore);
  rexprs2_3.push_back(&x);

  Function rsub2_1(&subr, makeExprVec(rexprs2_1, garbage2));
  Function rsub2_2(&subr, makeExprVec(rexprs2_2, garbage2));
  Function rsub2_3(&subr, makeExprVec(rexprs2_3, garbage2));

  rsub2_1.activate();
  rsub2_2.activate();
  rsub2_3.activate();
  
  x.setUnknown();

  assertTrue_1(rsub2_1.getValue(tempr));
  assertTrue_1(tempr == -2.0);

  // Should be unknown because x unknown
  assertTrue_1(!rsub2_2.getValue(tempr));
  assertTrue_1(!rsub2_3.getValue(tempr));

  x.setValue(-0.5);
  // should be known now
  assertTrue_1(rsub2_2.getValue(tempr));
  assertTrue_1(tempr == -3.0);
  assertTrue_1(rsub2_3.getValue(tempr));
  assertTrue_1(tempr == 5.0);

  // N-ary (not supported in schema)
  std::vector<Expression *> rexprs3_1, rexprs3_2, rexprs3_3;
  rexprs3_1.push_back(&too);
  rexprs3_1.push_back(&fore);
  rexprs3_1.push_back(&too);

  rexprs3_2.push_back(&x);
  rexprs3_2.push_back(&too);
  rexprs3_2.push_back(&fore);

  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&x);

  Function rsub3_1(&subr, makeExprVec(rexprs3_1, garbage3));
  Function rsub3_2(&subr, makeExprVec(rexprs3_2, garbage3));
  Function rsub3_3(&subr, makeExprVec(rexprs3_3, garbage3));

  rsub3_1.activate();
  rsub3_2.activate();
  rsub3_3.activate();
  
  x.setUnknown();

  assertTrue_1(rsub3_1.getValue(tempr));
  assertTrue_1(tempr == -4.5);

  // Should be unknown because x unknown
  assertTrue_1(!rsub3_2.getValue(tempr));
  assertTrue_1(!rsub3_3.getValue(tempr));

  x.setValue(3.25);
  // should be known now
  assertTrue_1(rsub3_2.getValue(tempr));
  assertTrue_1(tempr == -3.75);
  assertTrue_1(rsub3_3.getValue(tempr));
  assertTrue_1(tempr == -3.25);

  // Mixed numeric

  // Unary
  std::vector<Expression *> mexprs1_1, mexprs1_2, mexprs1_3;
  mexprs1_1.push_back(&m1);
  mexprs1_2.push_back(&tree);
  mexprs1_3.push_back(&i);

  Function msub1_1(&subr, makeExprVec(mexprs1_1, garbage1));
  Function msub1_2(&subr, makeExprVec(mexprs1_2, garbage1));
  Function msub1_3(&subr, makeExprVec(mexprs1_3, garbage1));

  msub1_1.activate();
  msub1_2.activate();
  msub1_3.activate();

  i.setUnknown();

  assertTrue_1(msub1_1.getValue(tempr));
  assertTrue_1(tempr == 1);
  assertTrue_1(msub1_2.getValue(tempr));
  assertTrue_1(tempr == -3);

  // Should be unknown because i unknown
  assertTrue_1(!msub1_3.getValue(tempr));

  i.setValue((int32_t) 7);
  // should be known now
  assertTrue_1(msub1_3.getValue(tempr));
  assertTrue_1(tempr == -7);

  // Binary
  std::vector<Expression *> mexprs2_1, mexprs2_2, mexprs2_3;
  mexprs2_1.push_back(&too);
  mexprs2_1.push_back(&tree);

  mexprs2_2.push_back(&i);
  mexprs2_2.push_back(&too);

  mexprs2_3.push_back(&tree);
  mexprs2_3.push_back(&x);

  Function msub2_1(&subr, makeExprVec(mexprs2_1, garbage2));
  Function msub2_2(&subr, makeExprVec(mexprs2_2, garbage2));
  Function msub2_3(&subr, makeExprVec(mexprs2_3, garbage2));

  msub2_1.activate();
  msub2_2.activate();
  msub2_3.activate();
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(msub2_1.getValue(tempr));
  assertTrue_1(tempr == -0.5);

  // Should be unknown because i, x unknown
  assertTrue_1(!msub2_2.getValue(tempr));
  assertTrue_1(!msub2_3.getValue(tempr));

  i.setValue((int32_t) 42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(msub2_2.getValue(tempr));
  assertTrue_1(tempr == 39.5);
  assertTrue_1(msub2_3.getValue(tempr));
  assertTrue_1(tempr == 3.5);

  // N-ary (not supported in schema)
  rexprs3_1.clear();
  rexprs3_1.push_back(&tree);
  rexprs3_1.push_back(&fore);
  rexprs3_1.push_back(&m1);

  rexprs3_2.clear();
  rexprs3_2.push_back(&x);
  rexprs3_2.push_back(&won);
  rexprs3_2.push_back(&fore);

  rexprs3_3.clear();
  rexprs3_3.push_back(&tree);
  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&i);

  Function msub3_1(&subr, makeExprVec(rexprs3_1, garbage3));
  Function msub3_2(&subr, makeExprVec(rexprs3_2, garbage3));
  Function msub3_3(&subr, makeExprVec(rexprs3_3, garbage3));

  msub3_1.activate();
  msub3_2.activate();
  msub3_3.activate();
  
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(msub3_1.getValue(tempr));
  assertTrue_1(tempr == -0.5);

  // Should be unknown because i, x unknown
  assertTrue_1(!msub3_2.getValue(tempr));
  assertTrue_1(!msub3_3.getValue(tempr));

  i.setValue((int32_t) 42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(msub3_2.getValue(tempr));
  assertTrue_1(tempr == -6.0);
  assertTrue_1(msub3_3.getValue(tempr));
  assertTrue_1(tempr == -41.5);

  return true;
}

bool multiplicationTest()
{
  //std::vector<bool> garbage1(1, false);
  std::vector<bool> garbage2(2, false);
  std::vector<bool> garbage3(3, false);

  // Integer
  IntegerConstant m1(-1);
  IntegerConstant six(6);
  IntegerConstant tree(3);
  IntegerVariable i;
  Multiplication<int32_t> muli;
  int32_t tempi;

  // Unary not supported in schema

  // Binary
  std::vector<Expression *> iexprs2_1, iexprs2_2, iexprs2_3;
  iexprs2_1.push_back(&m1);
  iexprs2_1.push_back(&tree);

  iexprs2_2.push_back(&i);
  iexprs2_2.push_back(&six);

  iexprs2_3.push_back(&tree);
  iexprs2_3.push_back(&i);

  Function imul2_1(&muli, makeExprVec(iexprs2_1, garbage2));
  Function imul2_2(&muli, makeExprVec(iexprs2_2, garbage2));
  Function imul2_3(&muli, makeExprVec(iexprs2_3, garbage2));

  imul2_1.activate();
  imul2_2.activate();
  imul2_3.activate();

  assertTrue_1(imul2_1.getValue(tempi));
  assertTrue_1(tempi == -3);

  // Should be unknown because i not initialized yet
  assertTrue_1(!imul2_2.getValue(tempi));
  assertTrue_1(!imul2_3.getValue(tempi));

  i.setValue((int32_t) 5);
  // should be known now
  assertTrue_1(imul2_2.getValue(tempi));
  assertTrue_1(tempi == 30);
  assertTrue_1(imul2_3.getValue(tempi));
  assertTrue_1(tempi == 15);

  // N-ary

  // Set up arglists
  std::vector<Expression *> iexprs3_1, iexprs3_2, iexprs3_3;
  iexprs3_1.push_back(&m1);
  iexprs3_1.push_back(&six);
  iexprs3_1.push_back(&tree);

  iexprs3_2.push_back(&six);
  iexprs3_2.push_back(&tree);
  iexprs3_2.push_back(&i);

  iexprs3_3.push_back(&i);
  iexprs3_3.push_back(&m1);
  iexprs3_3.push_back(&six);

  Function imul3_1(&muli, makeExprVec(iexprs3_1, garbage3));
  Function imul3_2(&muli, makeExprVec(iexprs3_2, garbage3));
  Function imul3_3(&muli, makeExprVec(iexprs3_3, garbage3));

  imul3_1.activate();
  imul3_2.activate();
  imul3_3.activate();
  
  i.setUnknown();

  assertTrue_1(imul3_1.getValue(tempi));
  assertTrue_1(tempi == -18);

  // Should be unknown because i unknown
  assertTrue_1(!imul3_2.getValue(tempi));
  assertTrue_1(!imul3_3.getValue(tempi));

  i.setValue((int32_t) 2);
  // should be known now
  assertTrue_1(imul3_2.getValue(tempi));
  assertTrue_1(tempi == 36);
  assertTrue_1(imul3_3.getValue(tempi));
  assertTrue_1(tempi == -12);

  // Real
  RealConstant too(2.5);
  RealConstant fore(4);
  RealVariable x;
  Multiplication<double> mulr;
  double tempr;

  // Unary not supported in schema

  // Binary
  std::vector<Expression *> rexprs2_1, rexprs2_2, rexprs2_3;
  rexprs2_1.push_back(&too);
  rexprs2_1.push_back(&fore);

  rexprs2_2.push_back(&x);
  rexprs2_2.push_back(&too);

  rexprs2_3.push_back(&fore);
  rexprs2_3.push_back(&x);

  Function rmul2_1(&mulr, makeExprVec(rexprs2_1, garbage2));
  Function rmul2_2(&mulr, makeExprVec(rexprs2_2, garbage2));
  Function rmul2_3(&mulr, makeExprVec(rexprs2_3, garbage2));

  rmul2_1.activate();
  rmul2_2.activate();
  rmul2_3.activate();

  assertTrue_1(rmul2_1.getValue(tempr));
  assertTrue_1(tempr == 10);

  // Should be unknown because x not initialized yet
  assertTrue_1(!rmul2_2.getValue(tempr));
  assertTrue_1(!rmul2_3.getValue(tempr));

  x.setValue(-0.5);
  // should be known now
  assertTrue_1(rmul2_2.getValue(tempr));
  assertTrue_1(tempr == -1.25);
  assertTrue_1(rmul2_3.getValue(tempr));
  assertTrue_1(tempr == -2.0);

  // N-ary
  std::vector<Expression *> rexprs3_1, rexprs3_2, rexprs3_3;
  rexprs3_1.push_back(&too);
  rexprs3_1.push_back(&fore);
  rexprs3_1.push_back(&too);

  rexprs3_2.push_back(&x);
  rexprs3_2.push_back(&too);
  rexprs3_2.push_back(&fore);

  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&x);

  Function rmul3_1(&mulr, makeExprVec(rexprs3_1, garbage3));
  Function rmul3_2(&mulr, makeExprVec(rexprs3_2, garbage3));
  Function rmul3_3(&mulr, makeExprVec(rexprs3_3, garbage3));

  rmul3_1.activate();
  rmul3_2.activate();
  rmul3_3.activate();
  
  x.setUnknown();

  assertTrue_1(rmul3_1.getValue(tempr));
  assertTrue_1(tempr == 25);

  // Should be unknown because x unknown
  assertTrue_1(!rmul3_2.getValue(tempr));
  assertTrue_1(!rmul3_3.getValue(tempr));

  x.setValue((int32_t) 8);
  // should be known now
  assertTrue_1(rmul3_2.getValue(tempr));
  assertTrue_1(tempr == 80);
  assertTrue_1(rmul3_3.getValue(tempr));
  assertTrue_1(tempr == 50);

  // Mixed numeric

  // Unary not supported in schema

  // Binary
  std::vector<Expression *> mexprs2_1, mexprs2_2, mexprs2_3;
  mexprs2_1.push_back(&too);
  mexprs2_1.push_back(&tree);

  mexprs2_2.push_back(&i);
  mexprs2_2.push_back(&too);

  mexprs2_3.push_back(&tree);
  mexprs2_3.push_back(&x);

  Function mmul2_1(&mulr, makeExprVec(mexprs2_1, garbage2));
  Function mmul2_2(&mulr, makeExprVec(mexprs2_2, garbage2));
  Function mmul2_3(&mulr, makeExprVec(mexprs2_3, garbage2));

  mmul2_1.activate();
  mmul2_2.activate();
  mmul2_3.activate();
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(mmul2_1.getValue(tempr));
  assertTrue_1(tempr == 7.5);

  // Should be unknown because i, x unknown
  assertTrue_1(!mmul2_2.getValue(tempr));
  assertTrue_1(!mmul2_3.getValue(tempr));

  i.setValue((int32_t) 2);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(mmul2_2.getValue(tempr));
  assertTrue_1(tempr == 5);
  assertTrue_1(mmul2_3.getValue(tempr));
  assertTrue_1(tempr == -1.5);

  // N-ary
  rexprs3_1.clear();
  rexprs3_1.push_back(&tree);
  rexprs3_1.push_back(&fore);
  rexprs3_1.push_back(&m1);

  rexprs3_2.clear();
  rexprs3_2.push_back(&x);
  rexprs3_2.push_back(&six);
  rexprs3_2.push_back(&fore);

  rexprs3_3.clear();
  rexprs3_3.push_back(&tree);
  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&i);

  Function mmul3_1(&mulr, makeExprVec(rexprs3_1, garbage3));
  Function mmul3_2(&mulr, makeExprVec(rexprs3_2, garbage3));
  Function mmul3_3(&mulr, makeExprVec(rexprs3_3, garbage3));

  mmul3_1.activate();
  mmul3_2.activate();
  mmul3_3.activate();
  
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(mmul3_1.getValue(tempr));
  assertTrue_1(tempr == -12);

  // Should be unknown because i, x unknown
  assertTrue_1(!mmul3_2.getValue(tempr));
  assertTrue_1(!mmul3_3.getValue(tempr));

  i.setValue((int32_t) 2);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(mmul3_2.getValue(tempr));
  assertTrue_1(tempr == -12);
  assertTrue_1(mmul3_3.getValue(tempr));
  assertTrue_1(tempr == 15);

  return true;
}

bool divisionTest()
{
  // Integer
  IntegerConstant m1(-1);
  IntegerConstant five(5);
  IntegerConstant tree(3);
  IntegerVariable i;
  Division<int32_t> divi;
  int32_t tempi;
  std::vector<bool> garbage2(2, false);

  // Binary
  std::vector<Expression *> iexprs2_1, iexprs2_2, iexprs2_3;
  iexprs2_1.push_back(&five);
  iexprs2_1.push_back(&tree);

  iexprs2_2.push_back(&i);
  iexprs2_2.push_back(&five);

  iexprs2_3.push_back(&tree);
  iexprs2_3.push_back(&i);

  Function idiv2_1(&divi, makeExprVec(iexprs2_1, garbage2));
  Function idiv2_2(&divi, makeExprVec(iexprs2_2, garbage2));
  Function idiv2_3(&divi, makeExprVec(iexprs2_3, garbage2));

  idiv2_1.activate();
  idiv2_2.activate();
  idiv2_3.activate();

  assertTrue_1(idiv2_1.getValue(tempi));
  assertTrue_1(tempi == 1);

  // Should be unknown because i not initialized yet
  assertTrue_1(!idiv2_2.getValue(tempi));
  assertTrue_1(!idiv2_3.getValue(tempi));

  i.setValue((int32_t) 2);
  // should be known now
  assertTrue_1(idiv2_2.getValue(tempi));
  assertTrue_1(tempi == 0);
  assertTrue_1(idiv2_3.getValue(tempi));
  assertTrue_1(tempi == 1);

  // Real
  RealConstant too(2.5);
  RealConstant fore(4);
  RealVariable x;
  Division<double> divr;
  double tempr;

  // Binary
  std::vector<Expression *> rexprs2_1, rexprs2_2, rexprs2_3;
  rexprs2_1.push_back(&fore);
  rexprs2_1.push_back(&too);

  rexprs2_2.push_back(&x);
  rexprs2_2.push_back(&fore);

  rexprs2_3.push_back(&too);
  rexprs2_3.push_back(&x);

  Function rdiv2_1(&divr, makeExprVec(rexprs2_1, garbage2));
  Function rdiv2_2(&divr, makeExprVec(rexprs2_2, garbage2));
  Function rdiv2_3(&divr, makeExprVec(rexprs2_3, garbage2));

  rdiv2_1.activate();
  rdiv2_2.activate();
  rdiv2_3.activate();

  assertTrue_1(rdiv2_1.getValue(tempr));
  assertTrue_1(tempr == 1.6);

  // Should be unknown because x not initialized yet
  assertTrue_1(!rdiv2_2.getValue(tempr));
  assertTrue_1(!rdiv2_3.getValue(tempr));

  x.setValue(-1.25);
  // should be known now
  assertTrue_1(rdiv2_2.getValue(tempr));
  assertTrue_1(tempr == -0.3125);
  assertTrue_1(rdiv2_3.getValue(tempr));
  assertTrue_1(tempr == -2);

  // Mixed
  // Binary
  std::vector<Expression *> mexprs2_1, mexprs2_2, mexprs2_3;
  mexprs2_1.push_back(&too);
  mexprs2_1.push_back(&five);

  mexprs2_2.push_back(&i);
  mexprs2_2.push_back(&too);

  mexprs2_3.push_back(&tree);
  mexprs2_3.push_back(&x);

  Function mdiv2_1(&divr, makeExprVec(mexprs2_1, garbage2));
  Function mdiv2_2(&divr, makeExprVec(mexprs2_2, garbage2));
  Function mdiv2_3(&divr, makeExprVec(mexprs2_3, garbage2));

  mdiv2_1.activate();
  mdiv2_2.activate();
  mdiv2_3.activate();
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(mdiv2_1.getValue(tempr));
  assertTrue_1(tempr == 0.5);

  // Should be unknown because i, x unknown
  assertTrue_1(!mdiv2_2.getValue(tempr));
  assertTrue_1(!mdiv2_3.getValue(tempr));

  i.setValue((int32_t) 5);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(mdiv2_2.getValue(tempr));
  assertTrue_1(tempr == 2);
  assertTrue_1(mdiv2_3.getValue(tempr));
  assertTrue_1(tempr == -6);

  return true;
}

bool moduloTest()
{
  // Integer
  IntegerConstant too(2);
  IntegerConstant tree(3);
  IntegerConstant nein(9);
  IntegerVariable i;
  Modulo<int32_t> modi;
  int32_t tempi;
  std::vector<bool> garbage2(2, false);

  // Integer
  std::vector<Expression *> iexprs2_1, iexprs2_2, iexprs2_3;
  iexprs2_1.push_back(&tree);
  iexprs2_1.push_back(&too);

  iexprs2_2.push_back(&i);
  iexprs2_2.push_back(&tree);

  iexprs2_3.push_back(&nein);
  iexprs2_3.push_back(&i);

  Function imod2_1(&modi, makeExprVec(iexprs2_1, garbage2));
  Function imod2_2(&modi, makeExprVec(iexprs2_2, garbage2));
  Function imod2_3(&modi, makeExprVec(iexprs2_3, garbage2));

  imod2_1.activate();
  imod2_2.activate();
  imod2_3.activate();

  assertTrue_1(imod2_1.getValue(tempi));
  assertTrue_1(tempi == 1);

  // Not yet known
  assertTrue_1(!imod2_2.getValue(tempi));
  assertTrue_1(!imod2_3.getValue(tempi));

  i.setValue((int32_t) 5);
  assertTrue_1(imod2_2.getValue(tempi));
  assertTrue_1(tempi == 2);
  assertTrue_1(imod2_3.getValue(tempi));
  assertTrue_1(tempi == 4);

  // Real
  RealConstant tue(2);
  RealConstant three(3);
  RealConstant nin(9);
  RealVariable x;
  Modulo<double> modd;
  double tempd;

  std::vector<Expression *> dexprs2_1, dexprs2_2, dexprs2_3;
  dexprs2_1.push_back(&three);
  dexprs2_1.push_back(&tue);

  dexprs2_2.push_back(&x);
  dexprs2_2.push_back(&three);

  dexprs2_3.push_back(&nin);
  dexprs2_3.push_back(&x);
  
  Function dmod2_1(&modd, makeExprVec(dexprs2_1, garbage2));
  Function dmod2_2(&modd, makeExprVec(dexprs2_2, garbage2));
  Function dmod2_3(&modd, makeExprVec(dexprs2_3, garbage2)); 

  dmod2_1.activate();
  dmod2_2.activate();
  dmod2_3.activate();

  assertTrue_1(dmod2_1.getValue(tempd));
  assertTrue_1(tempd == 1);

  // Not yet known
  assertTrue_1(!dmod2_2.getValue(tempd));
  assertTrue_1(!dmod2_3.getValue(tempd));

  x.setValue((int32_t) 5);
  assertTrue_1(dmod2_2.getValue(tempd));
  assertTrue_1(tempd == 2);
  assertTrue_1(dmod2_3.getValue(tempd));
  assertTrue_1(tempd == 4);

  // TODO: Mixed

  return true;
}

bool minimumTest()
{
  // Integer
  IntegerConstant m1(-1);
  IntegerConstant won(1);
  IntegerConstant tree(3);
  IntegerVariable i;
  Minimum<int32_t> mini;
  int32_t tempi;
  //std::vector<bool> garbage1(1, false);
  std::vector<bool> garbage2(2, false);
  std::vector<bool> garbage3(3, false);

  // Unary not supported in schema

  // Binary
  std::vector<Expression *> iexprs2_1, iexprs2_2, iexprs2_3;
  iexprs2_1.push_back(&m1);
  iexprs2_1.push_back(&tree);

  iexprs2_2.push_back(&i);
  iexprs2_2.push_back(&won);

  iexprs2_3.push_back(&tree);
  iexprs2_3.push_back(&i);

  Function imin2_1(&mini, makeExprVec(iexprs2_1, garbage2));
  Function imin2_2(&mini, makeExprVec(iexprs2_2, garbage2));
  Function imin2_3(&mini, makeExprVec(iexprs2_3, garbage2));

  imin2_1.activate();
  imin2_2.activate();
  imin2_3.activate();

  assertTrue_1(imin2_1.getValue(tempi));
  assertTrue_1(tempi == -1);

  // Should be unknown because i not initialized yet
  assertTrue_1(!imin2_2.getValue(tempi));
  assertTrue_1(!imin2_3.getValue(tempi));

  i.setValue((int32_t) 5);
  // should be known now
  assertTrue_1(imin2_2.getValue(tempi));
  assertTrue_1(tempi == 1);
  assertTrue_1(imin2_3.getValue(tempi));
  assertTrue_1(tempi == 3);

  // N-ary (not supported in schema)

  // Set up arglists
  std::vector<Expression *> iexprs3_1, iexprs3_2, iexprs3_3;
  iexprs3_1.push_back(&m1);
  iexprs3_1.push_back(&won);
  iexprs3_1.push_back(&tree);

  iexprs3_2.push_back(&won);
  iexprs3_2.push_back(&tree);
  iexprs3_2.push_back(&i);

  iexprs3_3.push_back(&i);
  iexprs3_3.push_back(&m1);
  iexprs3_3.push_back(&won);

  Function imin3_1(&mini, makeExprVec(iexprs3_1, garbage3));
  Function imin3_2(&mini, makeExprVec(iexprs3_2, garbage3));
  Function imin3_3(&mini, makeExprVec(iexprs3_3, garbage3));

  imin3_1.activate();
  imin3_2.activate();
  imin3_3.activate();
  
  i.setUnknown();

  assertTrue_1(imin3_1.getValue(tempi));
  assertTrue_1(tempi == -1);

  // Should be unknown because i unknown
  assertTrue_1(!imin3_2.getValue(tempi));
  assertTrue_1(!imin3_3.getValue(tempi));

  i.setValue((int32_t) 27);
  // should be known now
  assertTrue_1(imin3_2.getValue(tempi));
  assertTrue_1(tempi == 1);
  assertTrue_1(imin3_3.getValue(tempi));
  assertTrue_1(tempi == -1);

  // Real
  RealConstant too(2.5);
  RealConstant fore(4.5);
  RealVariable x;
  Minimum<double> minr;
  double tempr;

  // Unary not supported in schema

  // Binary
  std::vector<Expression *> rexprs2_1, rexprs2_2, rexprs2_3;
  rexprs2_1.push_back(&too);
  rexprs2_1.push_back(&fore);

  rexprs2_2.push_back(&x);
  rexprs2_2.push_back(&too);

  rexprs2_3.push_back(&fore);
  rexprs2_3.push_back(&x);

  Function rmin2_1(&minr, makeExprVec(rexprs2_1, garbage2));
  Function rmin2_2(&minr, makeExprVec(rexprs2_2, garbage2));
  Function rmin2_3(&minr, makeExprVec(rexprs2_3, garbage2));

  rmin2_1.activate();
  rmin2_2.activate();
  rmin2_3.activate();

  assertTrue_1(rmin2_1.getValue(tempr));
  assertTrue_1(tempr == 2.5);

  // Should be unknown because x not initialized yet
  assertTrue_1(!rmin2_2.getValue(tempr));
  assertTrue_1(!rmin2_3.getValue(tempr));

  x.setValue(-0.5);
  // should be known now
  assertTrue_1(rmin2_2.getValue(tempr));
  assertTrue_1(tempr == -0.5);
  assertTrue_1(rmin2_3.getValue(tempr));
  assertTrue_1(tempr == -0.5);

  // N-ary (not supported in schema)
  std::vector<Expression *> rexprs3_1, rexprs3_2, rexprs3_3;
  rexprs3_1.push_back(&too);
  rexprs3_1.push_back(&fore);
  rexprs3_1.push_back(&too);

  rexprs3_2.push_back(&x);
  rexprs3_2.push_back(&too);
  rexprs3_2.push_back(&fore);

  rexprs3_3.push_back(&fore);
  rexprs3_3.push_back(&fore);
  rexprs3_3.push_back(&x);

  Function rmin3_1(&minr, makeExprVec(rexprs3_1, garbage3));
  Function rmin3_2(&minr, makeExprVec(rexprs3_2, garbage3));
  Function rmin3_3(&minr, makeExprVec(rexprs3_3, garbage3));

  rmin3_1.activate();
  rmin3_2.activate();
  rmin3_3.activate();
  
  x.setUnknown();

  assertTrue_1(rmin3_1.getValue(tempr));
  assertTrue_1(tempr == 2.5);

  // Should be unknown because x unknown
  assertTrue_1(!rmin3_2.getValue(tempr));
  assertTrue_1(!rmin3_3.getValue(tempr));

  x.setValue(3.25);
  // should be known now
  assertTrue_1(rmin3_2.getValue(tempr));
  assertTrue_1(tempr == 2.5);
  assertTrue_1(rmin3_3.getValue(tempr));
  assertTrue_1(tempr == 3.25);


  // Mixed numeric

  // Unary not supported in schema

  // Binary
  std::vector<Expression *> mexprs2_1, mexprs2_2, mexprs2_3;
  mexprs2_1.push_back(&too);
  mexprs2_1.push_back(&tree);

  mexprs2_2.push_back(&i);
  mexprs2_2.push_back(&too);

  mexprs2_3.push_back(&tree);
  mexprs2_3.push_back(&x);

  Function mmin2_1(&minr, makeExprVec(mexprs2_1, garbage2));
  Function mmin2_2(&minr, makeExprVec(mexprs2_2, garbage2));
  Function mmin2_3(&minr, makeExprVec(mexprs2_3, garbage2));

  mmin2_1.activate();
  mmin2_2.activate();
  mmin2_3.activate();
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(mmin2_1.getValue(tempr));
  assertTrue_1(tempr == 2.5);

  // Should be unknown because i, x unknown
  assertTrue_1(!mmin2_2.getValue(tempr));
  assertTrue_1(!mmin2_3.getValue(tempr));

  i.setValue((int32_t) 42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(mmin2_2.getValue(tempr));
  assertTrue_1(tempr == 2.5);
  assertTrue_1(mmin2_3.getValue(tempr));
  assertTrue_1(tempr == -0.5);

  // N-ary (not supported in schema)
  rexprs3_1.clear();
  rexprs3_1.push_back(&tree);
  rexprs3_1.push_back(&fore);
  rexprs3_1.push_back(&m1);

  rexprs3_2.clear();
  rexprs3_2.push_back(&x);
  rexprs3_2.push_back(&won);
  rexprs3_2.push_back(&fore);

  rexprs3_3.clear();
  rexprs3_3.push_back(&tree);
  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&i);

  Function mmin3_1(&minr, makeExprVec(rexprs3_1, garbage3));
  Function mmin3_2(&minr, makeExprVec(rexprs3_2, garbage3));
  Function mmin3_3(&minr, makeExprVec(rexprs3_3, garbage3));

  mmin3_1.activate();
  mmin3_2.activate();
  mmin3_3.activate();
  
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(mmin3_1.getValue(tempr));
  assertTrue_1(tempr == -1);

  // Should be unknown because i, x unknown
  assertTrue_1(!mmin3_2.getValue(tempr));
  assertTrue_1(!mmin3_3.getValue(tempr));

  i.setValue((int32_t) 42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(mmin3_2.getValue(tempr));
  assertTrue_1(tempr == -0.5);
  assertTrue_1(mmin3_3.getValue(tempr));
  assertTrue_1(tempr == 2.5);

  return true;
}

bool maximumTest()
{
  // Integer
  IntegerConstant m1(-1);
  IntegerConstant won(1);
  IntegerConstant tree(3);
  IntegerVariable i;
  Maximum<int32_t> maxi;
  int32_t tempi;
  //std::vector<bool> garbage1(1, false);
  std::vector<bool> garbage2(2, false);
  std::vector<bool> garbage3(3, false);

  // Unary not supported in schema

  // Binary
  std::vector<Expression *> iexprs2_1, iexprs2_2, iexprs2_3;
  iexprs2_1.push_back(&m1);
  iexprs2_1.push_back(&tree);

  iexprs2_2.push_back(&i);
  iexprs2_2.push_back(&won);

  iexprs2_3.push_back(&tree);
  iexprs2_3.push_back(&i);

  Function imax2_1(&maxi, makeExprVec(iexprs2_1, garbage2));
  Function imax2_2(&maxi, makeExprVec(iexprs2_2, garbage2));
  Function imax2_3(&maxi, makeExprVec(iexprs2_3, garbage2));

  imax2_1.activate();
  imax2_2.activate();
  imax2_3.activate();

  assertTrue_1(imax2_1.getValue(tempi));
  assertTrue_1(tempi == 3);

  // Should be unknown because i not initialized yet
  assertTrue_1(!imax2_2.getValue(tempi));
  assertTrue_1(!imax2_3.getValue(tempi));

  i.setValue((int32_t) 5);
  // should be known now
  assertTrue_1(imax2_2.getValue(tempi));
  assertTrue_1(tempi == 5);
  assertTrue_1(imax2_3.getValue(tempi));
  assertTrue_1(tempi == 5);

  // N-ary (not supported in schema)

  // Set up arglists
  std::vector<Expression *> iexprs3_1, iexprs3_2, iexprs3_3;
  iexprs3_1.push_back(&m1);
  iexprs3_1.push_back(&won);
  iexprs3_1.push_back(&tree);

  iexprs3_2.push_back(&won);
  iexprs3_2.push_back(&tree);
  iexprs3_2.push_back(&i);

  iexprs3_3.push_back(&i);
  iexprs3_3.push_back(&m1);
  iexprs3_3.push_back(&won);

  Function imax3_1(&maxi, makeExprVec(iexprs3_1, garbage3));
  Function imax3_2(&maxi, makeExprVec(iexprs3_2, garbage3));
  Function imax3_3(&maxi, makeExprVec(iexprs3_3, garbage3));

  imax3_1.activate();
  imax3_2.activate();
  imax3_3.activate();
  
  i.setUnknown();

  assertTrue_1(imax3_1.getValue(tempi));
  assertTrue_1(tempi == 3);

  // Should be unknown because i unknown
  assertTrue_1(!imax3_2.getValue(tempi));
  assertTrue_1(!imax3_3.getValue(tempi));

  i.setValue((int32_t) 27);
  // should be known now
  assertTrue_1(imax3_2.getValue(tempi));
  assertTrue_1(tempi == 27);
  assertTrue_1(imax3_3.getValue(tempi));
  assertTrue_1(tempi == 27);

  // Real
  RealConstant too(2.5);
  RealConstant fore(4.5);
  RealVariable x;
  Maximum<double> maxr;
  double tempr;

  // Unary not supported in schema

  // Binary
  std::vector<Expression *> rexprs2_1, rexprs2_2, rexprs2_3;
  rexprs2_1.push_back(&too);
  rexprs2_1.push_back(&fore);

  rexprs2_2.push_back(&x);
  rexprs2_2.push_back(&too);

  rexprs2_3.push_back(&fore);
  rexprs2_3.push_back(&x);

  Function rmax2_1(&maxr, makeExprVec(rexprs2_1, garbage2));
  Function rmax2_2(&maxr, makeExprVec(rexprs2_2, garbage2));
  Function rmax2_3(&maxr, makeExprVec(rexprs2_3, garbage2));

  rmax2_1.activate();
  rmax2_2.activate();
  rmax2_3.activate();

  assertTrue_1(rmax2_1.getValue(tempr));
  assertTrue_1(tempr == 4.5);

  // Should be unknown because x not initialized yet
  assertTrue_1(!rmax2_2.getValue(tempr));
  assertTrue_1(!rmax2_3.getValue(tempr));

  x.setValue(-0.5);
  // should be known now
  assertTrue_1(rmax2_2.getValue(tempr));
  assertTrue_1(tempr == 2.5);
  assertTrue_1(rmax2_3.getValue(tempr));
  assertTrue_1(tempr == 4.5);

  // N-ary (not supported in schema)
  std::vector<Expression *> rexprs3_1, rexprs3_2, rexprs3_3;
  rexprs3_1.push_back(&too);
  rexprs3_1.push_back(&fore);
  rexprs3_1.push_back(&too);

  rexprs3_2.push_back(&x);
  rexprs3_2.push_back(&too);
  rexprs3_2.push_back(&fore);

  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&x);

  Function rmax3_1(&maxr, makeExprVec(rexprs3_1, garbage3));
  Function rmax3_2(&maxr, makeExprVec(rexprs3_2, garbage3));
  Function rmax3_3(&maxr, makeExprVec(rexprs3_3, garbage3));

  rmax3_1.activate();
  rmax3_2.activate();
  rmax3_3.activate();
  
  x.setUnknown();

  assertTrue_1(rmax3_1.getValue(tempr));
  assertTrue_1(tempr == 4.5);

  // Should be unknown because x unknown
  assertTrue_1(!rmax3_2.getValue(tempr));
  assertTrue_1(!rmax3_3.getValue(tempr));

  x.setValue(3.25);
  // should be known now
  assertTrue_1(rmax3_2.getValue(tempr));
  assertTrue_1(tempr == 4.5);
  assertTrue_1(rmax3_3.getValue(tempr));
  assertTrue_1(tempr == 3.25);

  // Mixed numeric

  // Unary not supported in schema

  // Binary
  std::vector<Expression *> mexprs2_1, mexprs2_2, mexprs2_3;
  mexprs2_1.push_back(&too);
  mexprs2_1.push_back(&tree);

  mexprs2_2.push_back(&i);
  mexprs2_2.push_back(&too);

  mexprs2_3.push_back(&tree);
  mexprs2_3.push_back(&x);

  Function mmax2_1(&maxr, makeExprVec(mexprs2_1, garbage2));
  Function mmax2_2(&maxr, makeExprVec(mexprs2_2, garbage2));
  Function mmax2_3(&maxr, makeExprVec(mexprs2_3, garbage2));

  mmax2_1.activate();
  mmax2_2.activate();
  mmax2_3.activate();
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(mmax2_1.getValue(tempr));
  assertTrue_1(tempr == 3);

  // Should be unknown because i, x unknown
  assertTrue_1(!mmax2_2.getValue(tempr));
  assertTrue_1(!mmax2_3.getValue(tempr));

  i.setValue((int32_t) 42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(mmax2_2.getValue(tempr));
  assertTrue_1(tempr == 42);
  assertTrue_1(mmax2_3.getValue(tempr));
  assertTrue_1(tempr == 3);

  // N-ary (not supported in schema)
  rexprs3_1.clear();
  rexprs3_1.push_back(&tree);
  rexprs3_1.push_back(&fore);
  rexprs3_1.push_back(&m1);

  rexprs3_2.clear();
  rexprs3_2.push_back(&x);
  rexprs3_2.push_back(&won);
  rexprs3_2.push_back(&fore);

  rexprs3_3.clear();
  rexprs3_3.push_back(&tree);
  rexprs3_3.push_back(&too);
  rexprs3_3.push_back(&i);

  Function mmax3_1(&maxr, makeExprVec(rexprs3_1, garbage3));
  Function mmax3_2(&maxr, makeExprVec(rexprs3_2, garbage3));
  Function mmax3_3(&maxr, makeExprVec(rexprs3_3, garbage3));

  mmax3_1.activate();
  mmax3_2.activate();
  mmax3_3.activate();
  
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(mmax3_1.getValue(tempr));
  assertTrue_1(tempr == 4.5);

  // Should be unknown because i, x unknown
  assertTrue_1(!mmax3_2.getValue(tempr));
  assertTrue_1(!mmax3_3.getValue(tempr));

  i.setValue((int32_t) 42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(mmax3_2.getValue(tempr));
  assertTrue_1(tempr == 4.5);
  assertTrue_1(mmax3_3.getValue(tempr));
  assertTrue_1(tempr == 42);

  return true;
}

bool absTest()
{
  // Integer
  IntegerConstant m1(-1);
  IntegerConstant won(1);
  IntegerVariable i;
  AbsoluteValue<int32_t> absi;
  int32_t tempi;
  std::vector<bool> garbage1(1, false);

  std::vector<Expression *> iexpr1, iexpr2, iexpr3;
  iexpr1.push_back(&m1);
  iexpr2.push_back(&won);
  iexpr3.push_back(&i);

  Function iabs1(&absi, makeExprVec(iexpr1, garbage1));
  Function iabs2(&absi, makeExprVec(iexpr2, garbage1));
  Function iabs3(&absi, makeExprVec(iexpr3, garbage1)); 

  iabs1.activate();
  iabs2.activate();
  iabs3.activate();

  assertTrue_1(iabs1.getValue(tempi));
  assertTrue_1(tempi == 1);
  assertTrue_1(iabs2.getValue(tempi));
  assertTrue_1(tempi == 1);
  
  assertTrue_1(!iabs3.getValue(tempi));

  i.setValue((int32_t) -22);
  assertTrue_1(iabs3.getValue(tempi));
  assertTrue_1(tempi == 22);

  // Real
  RealConstant mtoo(-2);
  RealConstant tree(3.5);
  RealVariable x;
  AbsoluteValue<double> absr;
  double tempr;

  std::vector<Expression *> rexpr1, rexpr2, rexpr3;
  rexpr1.push_back(&mtoo);
  rexpr2.push_back(&tree);
  rexpr3.push_back(&x);

  Function rabs1(&absr, makeExprVec(rexpr1, garbage1));
  Function rabs2(&absr, makeExprVec(rexpr2, garbage1));
  Function rabs3(&absr, makeExprVec(rexpr3, garbage1));

  rabs1.activate();
  rabs2.activate();
  rabs3.activate();

  assertTrue_1(rabs1.getValue(tempr));
  assertTrue_1(tempr == 2);
  assertTrue_1(rabs2.getValue(tempr));
  assertTrue_1(tempr == 3.5);
  
  assertTrue_1(!rabs3.getValue(tempr));

  x.setValue(-18.5);
  assertTrue_1(rabs3.getValue(tempr));
  assertTrue_1(tempr == 18.5);

  // Mixed
  std::vector<Expression *> mexpr1, mexpr2, mexpr3;
  mexpr1.push_back(&m1);
  mexpr2.push_back(&won);
  mexpr3.push_back(&i);
  
  Function mabs1(&absr, makeExprVec(mexpr1, garbage1));
  Function mabs2(&absr, makeExprVec(mexpr2, garbage1));
  Function mabs3(&absr, makeExprVec(mexpr3, garbage1));

  mabs1.activate();
  mabs2.activate();
  mabs3.activate();

  i.setUnknown();

  assertTrue_1(mabs1.getValue(tempr));
  assertTrue_1(tempr == 1);
  assertTrue_1(mabs2.getValue(tempr));
  assertTrue_1(tempr == 1);
  
  assertTrue_1(!mabs3.getValue(tempr));

  i.setValue((int32_t) 14);
  assertTrue_1(mabs3.getValue(tempr));
  assertTrue_1(tempr == 14);

  return true;
}

bool sqrtTest()
{

  // Real
  RealConstant too(2.25);
  RealConstant nein(9);
  RealVariable x;
  SquareRoot<double> sqrt;
  double tempr;
  std::vector<bool> garbage1(1, false);

  std::vector<Expression *> rexpr1, rexpr2, rexpr3;
  rexpr1.push_back(&too);
  rexpr2.push_back(&nein);
  rexpr3.push_back(&x);

  Function rsqrt1(&sqrt, makeExprVec(rexpr1, garbage1));
  Function rsqrt2(&sqrt, makeExprVec(rexpr2, garbage1));
  Function rsqrt3(&sqrt, makeExprVec(rexpr3, garbage1));

  rsqrt1.activate();
  rsqrt2.activate();
  rsqrt3.activate();

  assertTrue_1(rsqrt1.getValue(tempr));
  assertTrue_1(tempr == 1.5);
  assertTrue_1(rsqrt2.getValue(tempr));
  assertTrue_1(tempr == 3);
  
  assertTrue_1(!rsqrt3.getValue(tempr));

  x.setValue(0.25);
  assertTrue_1(rsqrt3.getValue(tempr));
  assertTrue_1(tempr == 0.5);

  // Mixed
  IntegerConstant fore(4);
  IntegerConstant sixteen(16);
  IntegerVariable i;

  std::vector<Expression *> mexpr1, mexpr2, mexpr3;
  mexpr1.push_back(&fore);
  mexpr2.push_back(&sixteen);
  mexpr3.push_back(&i);

  Function msqrt1(&sqrt, makeExprVec(mexpr1, garbage1));
  Function msqrt2(&sqrt, makeExprVec(mexpr2, garbage1));
  Function msqrt3(&sqrt, makeExprVec(mexpr3, garbage1));

  msqrt1.activate();
  msqrt2.activate();
  msqrt3.activate();

  assertTrue_1(msqrt1.getValue(tempr));
  assertTrue_1(tempr == 2);
  assertTrue_1(msqrt2.getValue(tempr));
  assertTrue_1(tempr == 4);
  
  assertTrue_1(!msqrt3.getValue(tempr));

  i.setValue((int32_t) 49);
  assertTrue_1(msqrt3.getValue(tempr));
  assertTrue_1(tempr == 7);

  return true;
}

static bool testCeiling()
{
  RealConstant three(3);
  RealConstant pi(3.14);
  RealConstant e(2.718);
  RealConstant minusthree(-3);
  RealConstant minuspi(-3.14);
  RealConstant minuse(-2.718);
  RealConstant toobig(3000000000.5);
  RealConstant toonegative(-3000000000.5);

  Ceiling<double> rCeiling;
  Ceiling<int32_t> iCeiling;

  RealVariable x;
  std::vector<Expression *> xexpr(1, &x);
  std::vector<bool> garbage1(1, false);

  Function realCeiling(&rCeiling, makeExprVec(xexpr, garbage1));
  Function intCeiling(&iCeiling, makeExprVec(xexpr, garbage1)); 
  double rtemp;
  int32_t itemp;

  x.activate();
  realCeiling.activate();
  intCeiling.activate();

  assertTrue_1(!x.isKnown());
  assertTrue_1(!realCeiling.isKnown());
  assertTrue_1(!intCeiling.isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realCeiling.getValue(rtemp));
  assertTrue_1(!intCeiling.getValue(itemp));

  x.setValue(&three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling.isKnown());
  assertTrue_1(intCeiling.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realCeiling.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(intCeiling.getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(&pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling.isKnown());
  assertTrue_1(intCeiling.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realCeiling.getValue(rtemp));
  assertTrue_1(rtemp == 4);
  assertTrue_1(intCeiling.getValue(itemp));
  assertTrue_1(itemp == 4);

  x.setValue(&e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling.isKnown());
  assertTrue_1(intCeiling.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realCeiling.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(intCeiling.getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(&minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling.isKnown());
  assertTrue_1(intCeiling.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realCeiling.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(intCeiling.getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(&minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling.isKnown());
  assertTrue_1(intCeiling.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realCeiling.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(intCeiling.getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(&minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling.isKnown());
  assertTrue_1(intCeiling.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realCeiling.getValue(rtemp));
  assertTrue_1(rtemp == -2);
  assertTrue_1(intCeiling.getValue(itemp));
  assertTrue_1(itemp == -2);

  x.setValue(&toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling.isKnown());
  assertTrue_1(!intCeiling.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realCeiling.getValue(rtemp));
  assertTrue_1(rtemp == 3000000001.0);
  assertTrue_1(!intCeiling.getValue(itemp));

  x.setValue(&toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling.isKnown());
  assertTrue_1(!intCeiling.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realCeiling.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.0);
  assertTrue_1(!intCeiling.getValue(itemp));
  
  return true;
}

static bool testFloor()
{
  RealConstant three(3);
  RealConstant pi(3.14);
  RealConstant e(2.718);
  RealConstant minusthree(-3);
  RealConstant minuspi(-3.14);
  RealConstant minuse(-2.718);
  RealConstant toobig(3000000000.5);
  RealConstant toonegative(-3000000000.5);

  Floor<double> rFloor;
  Floor<int32_t> iFloor;

  RealVariable x;
  std::vector<Expression *> xexpr(1, &x);
  std::vector<bool> garbage1(1, false);

  Function realFloor(&rFloor, makeExprVec(xexpr, garbage1));
  Function intFloor(&iFloor, makeExprVec(xexpr, garbage1));
  double rtemp;
  int32_t itemp;

  x.activate();
  realFloor.activate();
  intFloor.activate();

  assertTrue_1(!x.isKnown());
  assertTrue_1(!realFloor.isKnown());
  assertTrue_1(!intFloor.isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realFloor.getValue(rtemp));
  assertTrue_1(!intFloor.getValue(itemp));

  x.setValue(&three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor.isKnown());
  assertTrue_1(intFloor.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realFloor.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(intFloor.getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(&pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor.isKnown());
  assertTrue_1(intFloor.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realFloor.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(intFloor.getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(&e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor.isKnown());
  assertTrue_1(intFloor.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realFloor.getValue(rtemp));
  assertTrue_1(rtemp == 2);
  assertTrue_1(intFloor.getValue(itemp));
  assertTrue_1(itemp == 2);

  x.setValue(&minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor.isKnown());
  assertTrue_1(intFloor.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realFloor.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(intFloor.getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(&minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor.isKnown());
  assertTrue_1(intFloor.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realFloor.getValue(rtemp));
  assertTrue_1(rtemp == -4);
  assertTrue_1(intFloor.getValue(itemp));
  assertTrue_1(itemp == -4);

  x.setValue(&minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor.isKnown());
  assertTrue_1(intFloor.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realFloor.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(intFloor.getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(&toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor.isKnown());
  assertTrue_1(!intFloor.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realFloor.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.0);
  assertTrue_1(!intFloor.getValue(itemp));

  x.setValue(&toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor.isKnown());
  assertTrue_1(!intFloor.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realFloor.getValue(rtemp));
  assertTrue_1(rtemp == -3000000001.0);
  assertTrue_1(!intFloor.getValue(itemp));

  return true;
}

  // Believe it or not, VxWorks 6.8 for PowerPC doesn't have round() or trunc()
#if !defined(__VXWORKS__)

static bool testRound()
{
  RealConstant three(3);
  RealConstant pi(3.14);
  RealConstant e(2.718);
  RealConstant minusthree(-3);
  RealConstant minuspi(-3.14);
  RealConstant minuse(-2.718);
  RealConstant toobig(3000000000.5);
  RealConstant toonegative(-3000000000.5);

  Round<double> rRound;
  Round<int32_t> iRound;

  RealVariable x;
  std::vector<Expression *> xexpr(1, &x);
  std::vector<bool> garbage1(1, false);

  Function realRound(&rRound, makeExprVec(xexpr, garbage1));
  Function intRound(&iRound, makeExprVec(xexpr, garbage1));
  double rtemp;
  int32_t itemp;

  x.activate();
  realRound.activate();
  intRound.activate();

  assertTrue_1(!x.isKnown());
  assertTrue_1(!realRound.isKnown());
  assertTrue_1(!intRound.isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realRound.getValue(rtemp));
  assertTrue_1(!intRound.getValue(itemp));

  x.setValue(&three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound.isKnown());
  assertTrue_1(intRound.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realRound.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(intRound.getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(&pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound.isKnown());
  assertTrue_1(intRound.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realRound.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(intRound.getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(&e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound.isKnown());
  assertTrue_1(intRound.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realRound.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(intRound.getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(&minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound.isKnown());
  assertTrue_1(intRound.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realRound.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(intRound.getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(&minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound.isKnown());
  assertTrue_1(intRound.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realRound.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(intRound.getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(&minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound.isKnown());
  assertTrue_1(intRound.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realRound.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(intRound.getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(&toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound.isKnown());
  assertTrue_1(!intRound.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realRound.getValue(rtemp));
  assertTrue_1(rtemp == 3000000001.0);
  assertTrue_1(!intRound.getValue(itemp));

  x.setValue(&toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound.isKnown());
  assertTrue_1(!intRound.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realRound.getValue(rtemp));
  assertTrue_1(rtemp == -3000000001.0);
  assertTrue_1(!intRound.getValue(itemp));

  return true;
}

static bool testTruncate()
{
  RealConstant three(3);
  RealConstant pi(3.14);
  RealConstant e(2.718);
  RealConstant minusthree(-3);
  RealConstant minuspi(-3.14);
  RealConstant minuse(-2.718);
  RealConstant toobig(3000000000.5);
  RealConstant toonegative(-3000000000.5);

  Truncate<double> rTruncate;
  Truncate<int32_t> iTruncate;

  RealVariable x;
  std::vector<Expression *> xexpr(1, &x);
  std::vector<bool> garbage1(1, false);

  Function realTruncate(&rTruncate, makeExprVec(xexpr, garbage1));
  Function intTruncate(&iTruncate, makeExprVec(xexpr, garbage1));
  double rtemp;
  int32_t itemp;

  x.activate();
  realTruncate.activate();
  intTruncate.activate();

  assertTrue_1(!x.isKnown());
  assertTrue_1(!realTruncate.isKnown());
  assertTrue_1(!intTruncate.isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realTruncate.getValue(rtemp));
  assertTrue_1(!intTruncate.getValue(itemp));

  x.setValue(&three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate.isKnown());
  assertTrue_1(intTruncate.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realTruncate.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(intTruncate.getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(&pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate.isKnown());
  assertTrue_1(intTruncate.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realTruncate.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(intTruncate.getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(&e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate.isKnown());
  assertTrue_1(intTruncate.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realTruncate.getValue(rtemp));
  assertTrue_1(rtemp == 2);
  assertTrue_1(intTruncate.getValue(itemp));
  assertTrue_1(itemp == 2);

  x.setValue(&minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate.isKnown());
  assertTrue_1(intTruncate.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realTruncate.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(intTruncate.getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(&minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate.isKnown());
  assertTrue_1(intTruncate.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realTruncate.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(intTruncate.getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(&minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate.isKnown());
  assertTrue_1(intTruncate.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realTruncate.getValue(rtemp));
  assertTrue_1(rtemp == -2);
  assertTrue_1(intTruncate.getValue(itemp));
  assertTrue_1(itemp == -2);

  x.setValue(&toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate.isKnown());
  assertTrue_1(!intTruncate.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realTruncate.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.0);
  assertTrue_1(!intTruncate.getValue(itemp));

  x.setValue(&toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate.isKnown());
  assertTrue_1(!intTruncate.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realTruncate.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.0);
  assertTrue_1(!intTruncate.getValue(itemp));

  return true;
}

#endif // !defined(__VXWORKS__)

static bool testRealToInteger()
{
  RealConstant zero((double) 0);
  RealConstant three(3);
  RealConstant pi(3.14);
  RealConstant minusthree(-3);
  RealConstant minuspi(-3.14);
  RealConstant toobig(3000000000.0);
  RealConstant toonegative(-3000000000.0);

  RealToInteger rtiOp;
  RealVariable x;
  std::vector<Expression *> xexpr(1, &x);
  std::vector<bool> garbage1(1, false);

  Function rti(&rtiOp, makeExprVec(xexpr, garbage1));
  double rtemp;
  int32_t itemp;

  x.activate();
  rti.activate();

  // Uninitialized
  assertTrue_1(!x.isKnown());
  assertTrue_1(!rti.isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!rti.getValue(itemp));

  x.setValue(&zero);
  assertTrue_1(x.isKnown());
  assertTrue_1(rti.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 0);
  assertTrue_1(rti.getValue(itemp));
  assertTrue_1(itemp == 0);

  x.setValue(&three);
  assertTrue_1(x.isKnown());
  assertTrue_1(rti.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(rti.getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(&pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(!rti.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(!rti.getValue(itemp));

  x.setValue(&minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(rti.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(rti.getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(&minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(!rti.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(!rti.getValue(itemp));

  x.setValue(&toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(!rti.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.0);
  assertTrue_1(!rti.getValue(itemp));

  x.setValue(&toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(!rti.isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.0);
  assertTrue_1(!rti.getValue(itemp));

  return true;
}

bool arithmeticTest()
{
  runTest(additionTest);
  runTest(subtractionTest);
  runTest(multiplicationTest);
  runTest(divisionTest);
  runTest(moduloTest);
  runTest(minimumTest);
  runTest(maximumTest);
  runTest(absTest);
  runTest(sqrtTest);
  runTest(testCeiling);
  runTest(testFloor);
  // Believe it or not, VxWorks 6.8 for PowerPC doesn't have round() or trunc()
#if !defined(__VXWORKS__)
  runTest(testRound);
  runTest(testTruncate);
#endif // !defined(__VXWORKS__)
  runTest(testRealToInteger);
  return true;
}
