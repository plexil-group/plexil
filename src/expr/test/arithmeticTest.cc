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
  // Integer

  // Real

  // Mixed

  return true;
}

bool divisionTest()
{
  // Integer

  // Real

  // Mixed

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

  // Binary
  BinaryFunction<int32_t> imod2_1(&modi, tree.getId(), too.getId());
  BinaryFunction<int32_t> imod2_2(&modi, i.getId(), tree.getId());
  BinaryFunction<int32_t> imod2_3(&modi, nein.getId(), i.getId());

  imod2_1.activate();
  imod2_2.activate();
  imod2_3.activate();

  assertTrue_1(imod2_1.getValue(tempi));
  assertTrue_1(tempi == 1);

  // Not yet known
  assertTrue_1(!imod2_2.getValue(tempi));
  assertTrue_1(!imod2_3.getValue(tempi));

  i.setValue(5);
  assertTrue_1(imod2_2.getValue(tempi));
  assertTrue_1(tempi == 2);
  assertTrue_1(imod2_3.getValue(tempi));
  assertTrue_1(tempi == 4);

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

  // Binary
  BinaryFunction<int32_t> imin2_1(&mini, m1.getId(), tree.getId());
  BinaryFunction<int32_t> imin2_2(&mini, i.getId(), won.getId());
  BinaryFunction<int32_t> imin2_3(&mini, tree.getId(), i.getId());

  imin2_1.activate();
  imin2_2.activate();
  imin2_3.activate();

  assertTrue_1(imin2_1.getValue(tempi));
  assertTrue_1(tempi == -1);

  // Should be unknown because i not initialized yet
  assertTrue_1(!imin2_2.getValue(tempi));
  assertTrue_1(!imin2_3.getValue(tempi));

  i.setValue(5);
  // should be known now
  assertTrue_1(imin2_2.getValue(tempi));
  assertTrue_1(tempi == 1);
  assertTrue_1(imin2_3.getValue(tempi));
  assertTrue_1(tempi == 3);

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

  NaryFunction<int32_t> imin3_1(&mini, iexprs1, garbage);
  NaryFunction<int32_t> imin3_2(&mini, iexprs2, garbage);
  NaryFunction<int32_t> imin3_3(&mini, iexprs3, garbage);

  imin3_1.activate();
  imin3_2.activate();
  imin3_3.activate();
  
  i.setUnknown();

  assertTrue_1(imin3_1.getValue(tempi));
  assertTrue_1(tempi == -1);

  // Should be unknown because i not initialized yet
  assertTrue_1(!imin3_2.getValue(tempi));
  assertTrue_1(!imin3_3.getValue(tempi));

  i.setValue(27);
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

  // Binary
  BinaryFunction<double> rmin2_1(&minr, too.getId(), fore.getId());
  BinaryFunction<double> rmin2_2(&minr, x.getId(), too.getId());
  BinaryFunction<double> rmin2_3(&minr, fore.getId(), x.getId());

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

  // N-ary
  std::vector<ExpressionId> rexprs1, rexprs2, rexprs3;
  rexprs1.push_back(too.getId());
  rexprs1.push_back(fore.getId());
  rexprs1.push_back(too.getId());

  rexprs2.push_back(x.getId());
  rexprs2.push_back(too.getId());
  rexprs2.push_back(fore.getId());

  rexprs3.push_back(fore.getId());
  rexprs3.push_back(fore.getId());
  rexprs3.push_back(x.getId());

  NaryFunction<double> rmin3_1(&minr, rexprs1, garbage);
  NaryFunction<double> rmin3_2(&minr, rexprs2, garbage);
  NaryFunction<double> rmin3_3(&minr, rexprs3, garbage);

  rmin3_1.activate();
  rmin3_2.activate();
  rmin3_3.activate();
  
  x.setUnknown();

  assertTrue_1(rmin3_1.getValue(tempr));
  assertTrue_1(tempr == 2.5);

  // Should be unknown because x not initialized yet
  assertTrue_1(!rmin3_2.getValue(tempr));
  assertTrue_1(!rmin3_3.getValue(tempr));

  x.setValue(3.25);
  // should be known now
  assertTrue_1(rmin3_2.getValue(tempr));
  assertTrue_1(tempr == 2.5);
  assertTrue_1(rmin3_3.getValue(tempr));
  assertTrue_1(tempr == 3.25);


  // Mixed numeric
  // Binary
  BinaryFunction<double> mmin2_1(&minr, too.getId(), tree.getId());
  BinaryFunction<double> mmin2_2(&minr, i.getId(), too.getId());
  BinaryFunction<double> mmin2_3(&minr, tree.getId(), x.getId());

  mmin2_1.activate();
  mmin2_2.activate();
  mmin2_3.activate();
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(mmin2_1.getValue(tempr));
  assertTrue_1(tempr == 2.5);

  // Should be unknown because i, x not initialized yet
  assertTrue_1(!mmin2_2.getValue(tempr));
  assertTrue_1(!mmin2_3.getValue(tempr));

  i.setValue(42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(mmin2_2.getValue(tempr));
  assertTrue_1(tempr == 2.5);
  assertTrue_1(mmin2_3.getValue(tempr));
  assertTrue_1(tempr == -0.5);

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

  NaryFunction<double> mmin3_1(&minr, rexprs1, garbage);
  NaryFunction<double> mmin3_2(&minr, rexprs2, garbage);
  NaryFunction<double> mmin3_3(&minr, rexprs3, garbage);

  mmin3_1.activate();
  mmin3_2.activate();
  mmin3_3.activate();
  
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(mmin3_1.getValue(tempr));
  assertTrue_1(tempr == -1);

  // Should be unknown because i, x not initialized yet
  assertTrue_1(!mmin3_2.getValue(tempr));
  assertTrue_1(!mmin3_3.getValue(tempr));

  i.setValue(42);
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

  // Binary
  BinaryFunction<int32_t> imax2_1(&maxi, m1.getId(), tree.getId());
  BinaryFunction<int32_t> imax2_2(&maxi, i.getId(), won.getId());
  BinaryFunction<int32_t> imax2_3(&maxi, tree.getId(), i.getId());

  imax2_1.activate();
  imax2_2.activate();
  imax2_3.activate();

  assertTrue_1(imax2_1.getValue(tempi));
  assertTrue_1(tempi == 3);

  // Should be unknown because i not initialized yet
  assertTrue_1(!imax2_2.getValue(tempi));
  assertTrue_1(!imax2_3.getValue(tempi));

  i.setValue(5);
  // should be known now
  assertTrue_1(imax2_2.getValue(tempi));
  assertTrue_1(tempi == 5);
  assertTrue_1(imax2_3.getValue(tempi));
  assertTrue_1(tempi == 5);

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

  NaryFunction<int32_t> imax3_1(&maxi, iexprs1, garbage);
  NaryFunction<int32_t> imax3_2(&maxi, iexprs2, garbage);
  NaryFunction<int32_t> imax3_3(&maxi, iexprs3, garbage);

  imax3_1.activate();
  imax3_2.activate();
  imax3_3.activate();
  
  i.setUnknown();

  assertTrue_1(imax3_1.getValue(tempi));
  assertTrue_1(tempi == 3);

  // Should be unknown because i not initialized yet
  assertTrue_1(!imax3_2.getValue(tempi));
  assertTrue_1(!imax3_3.getValue(tempi));

  i.setValue(27);
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

  // Binary
  BinaryFunction<double> rmax2_1(&maxr, too.getId(), fore.getId());
  BinaryFunction<double> rmax2_2(&maxr, x.getId(), too.getId());
  BinaryFunction<double> rmax2_3(&maxr, fore.getId(), x.getId());

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

  NaryFunction<double> rmax3_1(&maxr, rexprs1, garbage);
  NaryFunction<double> rmax3_2(&maxr, rexprs2, garbage);
  NaryFunction<double> rmax3_3(&maxr, rexprs3, garbage);

  rmax3_1.activate();
  rmax3_2.activate();
  rmax3_3.activate();
  
  x.setUnknown();

  assertTrue_1(rmax3_1.getValue(tempr));
  assertTrue_1(tempr == 4.5);

  // Should be unknown because x not initialized yet
  assertTrue_1(!rmax3_2.getValue(tempr));
  assertTrue_1(!rmax3_3.getValue(tempr));

  x.setValue(3.25);
  // should be known now
  assertTrue_1(rmax3_2.getValue(tempr));
  assertTrue_1(tempr == 4.5);
  assertTrue_1(rmax3_3.getValue(tempr));
  assertTrue_1(tempr == 3.25);


  // Mixed numeric
  // Binary
  BinaryFunction<double> mmax2_1(&maxr, too.getId(), tree.getId());
  BinaryFunction<double> mmax2_2(&maxr, i.getId(), too.getId());
  BinaryFunction<double> mmax2_3(&maxr, tree.getId(), x.getId());

  mmax2_1.activate();
  mmax2_2.activate();
  mmax2_3.activate();
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(mmax2_1.getValue(tempr));
  assertTrue_1(tempr == 3);

  // Should be unknown because i, x not initialized yet
  assertTrue_1(!mmax2_2.getValue(tempr));
  assertTrue_1(!mmax2_3.getValue(tempr));

  i.setValue(42);
  x.setValue(-0.5);
  // should be known now
  assertTrue_1(mmax2_2.getValue(tempr));
  assertTrue_1(tempr == 42);
  assertTrue_1(mmax2_3.getValue(tempr));
  assertTrue_1(tempr == 3);

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

  NaryFunction<double> mmax3_1(&maxr, rexprs1, garbage);
  NaryFunction<double> mmax3_2(&maxr, rexprs2, garbage);
  NaryFunction<double> mmax3_3(&maxr, rexprs3, garbage);

  mmax3_1.activate();
  mmax3_2.activate();
  mmax3_3.activate();
  
  i.setUnknown();
  x.setUnknown();

  assertTrue_1(mmax3_1.getValue(tempr));
  assertTrue_1(tempr == 4.5);

  // Should be unknown because i, x not initialized yet
  assertTrue_1(!mmax3_2.getValue(tempr));
  assertTrue_1(!mmax3_3.getValue(tempr));

  i.setValue(42);
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

  // Real

  // Mixed

  return true;
}

bool sqrtTest()
{
  // Real

  // Mixed

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
