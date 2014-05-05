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
  // Integer
  IntegerConstant m1(-1);
  IntegerConstant won(1);
  IntegerConstant tree(3);
  IntegerVariable i;
  Addition<int32_t> addi;
  int32_t tempi;

  // Binary
  BinaryFunction<int32_t> iadd2_1(&addi, m1.getId(), tree.getId());
  BinaryFunction<int32_t> iadd2_2(&addi, i.getId(), won.getId());
  BinaryFunction<int32_t> iadd2_3(&addi, tree.getId(), i.getId());

  iadd2_1.activate();
  iadd2_2.activate();
  iadd2_3.activate();

  assertTrue_1(iadd2_1.getValue(tempi));
  assertTrue_1(tempi == 2);

  // Should be unknown because i not initialized yet
  assertTrue_1(!iadd2_2.getValue(tempi));
  assertTrue_1(!iadd2_3.getValue(tempi));

  i.setValue(5);
  // should be known now
  assertTrue_1(iadd2_2.getValue(tempi));
  assertTrue_1(tempi == 6);
  assertTrue_1(iadd2_3.getValue(tempi));
  assertTrue_1(tempi == 8);

  // N-ary

  // Set up arglists
  std::vector<bool> garbage(3, false);
  std::vector<ExpressionId> iexprs1, iexprs2, iexprs3;
  iexprs1.push_back(m1.getId());
  iexprs1.push_back(won.getId());
  iexprs1.push_back(tree.getId());

  iexprs2.push_back(won.getId());
  iexprs2.push_back(tree.getId());
  iexprs2.push_back(i.getId());

  iexprs3.push_back(i.getId());
  iexprs3.push_back(m1.getId());
  iexprs3.push_back(won.getId());

  NaryFunction<int32_t> iadd3_1(&addi, iexprs1, garbage);
  NaryFunction<int32_t> iadd3_2(&addi, iexprs2, garbage);
  NaryFunction<int32_t> iadd3_3(&addi, iexprs3, garbage);

  iadd3_1.activate();
  iadd3_2.activate();
  iadd3_3.activate();
  
  i.setUnknown();

  assertTrue_1(iadd3_1.getValue(tempi));
  assertTrue_1(tempi == 3);

  // Should be unknown because i not initialized yet
  assertTrue_1(!iadd3_2.getValue(tempi));
  assertTrue_1(!iadd3_3.getValue(tempi));

  i.setValue(27);
  // should be known now
  assertTrue_1(iadd3_2.getValue(tempi));
  assertTrue_1(tempi == 31);
  assertTrue_1(iadd3_3.getValue(tempi));
  assertTrue_1(tempi == 27);

  // Real
  RealConstant too(2.5);
  RealConstant fore(4.5);
  RealVariable x;
  Addition<double> addr;
  double tempr;

  // Binary
  BinaryFunction<double> radd2_1(&addr, too.getId(), fore.getId());
  BinaryFunction<double> radd2_2(&addr, x.getId(), too.getId());
  BinaryFunction<double> radd2_3(&addr, fore.getId(), x.getId());

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
  std::vector<ExpressionId> rexprs1, rexprs2, rexprs3;
  rexprs1.push_back(too.getId());
  rexprs1.push_back(fore.getId());
  rexprs1.push_back(too.getId());

  rexprs2.push_back(x.getId());
  rexprs2.push_back(too.getId());
  rexprs2.push_back(fore.getId());

  rexprs3.push_back(too.getId());
  rexprs3.push_back(too.getId());
  rexprs3.push_back(x.getId());

  NaryFunction<double> radd3_1(&addr, rexprs1, garbage);
  NaryFunction<double> radd3_2(&addr, rexprs2, garbage);
  NaryFunction<double> radd3_3(&addr, rexprs3, garbage);

  radd3_1.activate();
  radd3_2.activate();
  radd3_3.activate();
  
  x.setUnknown();

  assertTrue_1(radd3_1.getValue(tempr));
  assertTrue_1(tempr == 9.5);

  // Should be unknown because x not initialized yet
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
  BinaryFunction<double> madd2_1(&addr, too.getId(), tree.getId());
  BinaryFunction<double> madd2_2(&addr, i.getId(), too.getId());
  BinaryFunction<double> madd2_3(&addr, tree.getId(), x.getId());

  madd2_1.activate();
  madd2_2.activate();
  madd2_3.activate();
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(madd2_1.getValue(tempr));
  assertTrue_1(tempr == 5.5);

  // Should be unknown because i, x not initialized yet
  assertTrue_1(!madd2_2.getValue(tempr));
  assertTrue_1(!madd2_3.getValue(tempr));

  i.setValue(42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(madd2_2.getValue(tempr));
  assertTrue_1(tempr == 44.5);
  assertTrue_1(madd2_3.getValue(tempr));
  assertTrue_1(tempr == 2.5);

  // N-ary
  rexprs1.clear();
  rexprs1.push_back(tree.getId());
  rexprs1.push_back(fore.getId());
  rexprs1.push_back(m1.getId());

  rexprs2.clear();
  rexprs2.push_back(x.getId());
  rexprs2.push_back(won.getId());
  rexprs2.push_back(fore.getId());

  rexprs3.clear();
  rexprs3.push_back(tree.getId());
  rexprs3.push_back(too.getId());
  rexprs3.push_back(i.getId());

  NaryFunction<double> madd3_1(&addr, rexprs1, garbage);
  NaryFunction<double> madd3_2(&addr, rexprs2, garbage);
  NaryFunction<double> madd3_3(&addr, rexprs3, garbage);

  madd3_1.activate();
  madd3_2.activate();
  madd3_3.activate();
  
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(madd3_1.getValue(tempr));
  assertTrue_1(tempr == 6.5);

  // Should be unknown because i, x not initialized yet
  assertTrue_1(!madd3_2.getValue(tempr));
  assertTrue_1(!madd3_3.getValue(tempr));

  i.setValue(42);
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

  // Unary
  UnaryFunction<int32_t> isub1_1(&subi, m1.getId());
  UnaryFunction<int32_t> isub1_2(&subi, tree.getId());
  UnaryFunction<int32_t> isub1_3(&subi, i.getId());

  isub1_1.activate();
  isub1_2.activate();
  isub1_3.activate();

  assertTrue(isub1_1.getValue(tempi));
  assertTrue(tempi == 1);
  assertTrue(isub1_2.getValue(tempi));
  assertTrue(tempi == -3);

  // Should be unknown because i not initialized yet
  assertTrue_1(!isub1_3.getValue(tempi));

  i.setValue(7);
  // should be known now
  assertTrue_1(isub1_3.getValue(tempi));
  assertTrue_1(tempi == -7);

  // Binary
  BinaryFunction<int32_t> isub2_1(&subi, m1.getId(), tree.getId());
  BinaryFunction<int32_t> isub2_2(&subi, i.getId(), won.getId());
  BinaryFunction<int32_t> isub2_3(&subi, tree.getId(), i.getId());

  isub2_1.activate();
  isub2_2.activate();
  isub2_3.activate();

  i.setUnknown();
  
  assertTrue_1(isub2_1.getValue(tempi));
  assertTrue_1(tempi == -4);

  // Should be unknown because i unknown
  assertTrue_1(!isub2_2.getValue(tempi));
  assertTrue_1(!isub2_3.getValue(tempi));

  i.setValue(5);
  // should be known now
  assertTrue_1(isub2_2.getValue(tempi));
  assertTrue_1(tempi == 4);
  assertTrue_1(isub2_3.getValue(tempi));
  assertTrue_1(tempi == -2);

  // N-ary

  // Set up arglists
  std::vector<bool> garbage(3, false);
  std::vector<ExpressionId> iexprs1, iexprs2, iexprs3;
  iexprs1.push_back(m1.getId());
  iexprs1.push_back(won.getId());
  iexprs1.push_back(tree.getId());

  iexprs2.push_back(won.getId());
  iexprs2.push_back(tree.getId());
  iexprs2.push_back(i.getId());

  iexprs3.push_back(i.getId());
  iexprs3.push_back(m1.getId());
  iexprs3.push_back(won.getId());

  NaryFunction<int32_t> isub3_1(&subi, iexprs1, garbage);
  NaryFunction<int32_t> isub3_2(&subi, iexprs2, garbage);
  NaryFunction<int32_t> isub3_3(&subi, iexprs3, garbage);

  isub3_1.activate();
  isub3_2.activate();
  isub3_3.activate();
  
  i.setUnknown();

  assertTrue_1(isub3_1.getValue(tempi));
  assertTrue_1(tempi == -5);

  // Should be unknown because i not initialized yet
  assertTrue_1(!isub3_2.getValue(tempi));
  assertTrue_1(!isub3_3.getValue(tempi));

  i.setValue(27);
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
  double tempr;

  // Unary
  UnaryFunction<double> rsub1_1(&subr, too.getId());
  UnaryFunction<double> rsub1_2(&subr, fore.getId());
  UnaryFunction<double> rsub1_3(&subr, x.getId());

  rsub1_1.activate();
  rsub1_2.activate();
  rsub1_3.activate();

  assertTrue(rsub1_1.getValue(tempr));
  assertTrue(tempr == -2.5);
  assertTrue(rsub1_2.getValue(tempr));
  assertTrue(tempr == -4.5);

  // Should be unknown because x not initialized yet
  assertTrue_1(!rsub1_3.getValue(tempr));

  x.setValue(1.5);
  // should be known now
  assertTrue_1(rsub1_3.getValue(tempr));
  assertTrue_1(tempr == -1.5);

  // Binary
  BinaryFunction<double> rsub2_1(&subr, too.getId(), fore.getId());
  BinaryFunction<double> rsub2_2(&subr, x.getId(), too.getId());
  BinaryFunction<double> rsub2_3(&subr, fore.getId(), x.getId());

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

  // N-ary
  std::vector<ExpressionId> rexprs1, rexprs2, rexprs3;
  rexprs1.push_back(too.getId());
  rexprs1.push_back(fore.getId());
  rexprs1.push_back(too.getId());

  rexprs2.push_back(x.getId());
  rexprs2.push_back(too.getId());
  rexprs2.push_back(fore.getId());

  rexprs3.push_back(too.getId());
  rexprs3.push_back(too.getId());
  rexprs3.push_back(x.getId());

  NaryFunction<double> rsub3_1(&subr, rexprs1, garbage);
  NaryFunction<double> rsub3_2(&subr, rexprs2, garbage);
  NaryFunction<double> rsub3_3(&subr, rexprs3, garbage);

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
  UnaryFunction<double> msub1_1(&subr, m1.getId());
  UnaryFunction<double> msub1_2(&subr, tree.getId());
  UnaryFunction<double> msub1_3(&subr, i.getId());

  msub1_1.activate();
  msub1_2.activate();
  msub1_3.activate();

  i.setUnknown();

  assertTrue(msub1_1.getValue(tempr));
  assertTrue(tempr == 1);
  assertTrue(msub1_2.getValue(tempr));
  assertTrue(tempr == -3);

  // Should be unknown because i not initialized yet
  assertTrue_1(!msub1_3.getValue(tempr));

  i.setValue(7);
  // should be known now
  assertTrue_1(msub1_3.getValue(tempr));
  assertTrue_1(tempr == -7);

  // Binary
  BinaryFunction<double> msub2_1(&subr, too.getId(), tree.getId());
  BinaryFunction<double> msub2_2(&subr, i.getId(), too.getId());
  BinaryFunction<double> msub2_3(&subr, tree.getId(), x.getId());

  msub2_1.activate();
  msub2_2.activate();
  msub2_3.activate();
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(msub2_1.getValue(tempr));
  assertTrue_1(tempr == -0.5);

  // Should be unknown because i, x not initialized yet
  assertTrue_1(!msub2_2.getValue(tempr));
  assertTrue_1(!msub2_3.getValue(tempr));

  i.setValue(42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(msub2_2.getValue(tempr));
  assertTrue_1(tempr == 39.5);
  assertTrue_1(msub2_3.getValue(tempr));
  assertTrue_1(tempr == 3.5);

  // N-ary
  rexprs1.clear();
  rexprs1.push_back(tree.getId());
  rexprs1.push_back(fore.getId());
  rexprs1.push_back(m1.getId());

  rexprs2.clear();
  rexprs2.push_back(x.getId());
  rexprs2.push_back(won.getId());
  rexprs2.push_back(fore.getId());

  rexprs3.clear();
  rexprs3.push_back(tree.getId());
  rexprs3.push_back(too.getId());
  rexprs3.push_back(i.getId());

  NaryFunction<double> msub3_1(&subr, rexprs1, garbage);
  NaryFunction<double> msub3_2(&subr, rexprs2, garbage);
  NaryFunction<double> msub3_3(&subr, rexprs3, garbage);

  msub3_1.activate();
  msub3_2.activate();
  msub3_3.activate();
  
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(msub3_1.getValue(tempr));
  assertTrue_1(tempr == -0.5);

  // Should be unknown because i, x not initialized yet
  assertTrue_1(!msub3_2.getValue(tempr));
  assertTrue_1(!msub3_3.getValue(tempr));

  i.setValue(42);
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
  return true;
}

bool divisionTest()
{
  return true;
}

bool moduloTest()
{
  return true;
}

bool minimumTest()
{
  return true;
}

bool maximumTest()
{
  return true;
}

bool absTest()
{
  return true;
}

bool sqrtTest()
{
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
  return true;
}
