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
#include "Function.hh"     // includes Mutable.hh
#include "TestSupport.hh"  // includes Error.hh
#include "UserVariable.hh"

using namespace PLEXIL;

static bool testIsKnown()
{
  bool tempb;
  IsKnown iskop;

  BooleanVariable varb;
  IntegerVariable vari;
  RealVariable varr;
  StringVariable vars;

  UnaryFunction<bool> iskfnb(&iskop, varb.getId());
  UnaryFunction<bool> iskfni(&iskop, vari.getId());
  UnaryFunction<bool> iskfnr(&iskop, varr.getId());
  UnaryFunction<bool> iskfns(&iskop, vars.getId());

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
  vari.setValue(93);
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
  Equal<bool> eqb;
  NotEqual<bool> neqb;

  BinaryFunction<bool> boolEq1(&eqb, troo.getId(), troo.getId());
  BinaryFunction<bool> boolEq2(&eqb, falls.getId(), falls.getId());
  BinaryFunction<bool> boolEq3(&eqb, troo.getId(), falls.getId());
  BinaryFunction<bool> boolEq4(&eqb, falls.getId(), troo.getId());
  BinaryFunction<bool> boolEq5(&eqb, troo.getId(), varb.getId());
  BinaryFunction<bool> boolEq6(&eqb, varb.getId(), falls.getId());
  BinaryFunction<bool> boolNeq1(&neqb, troo.getId(), troo.getId());
  BinaryFunction<bool> boolNeq2(&neqb, falls.getId(), falls.getId());
  BinaryFunction<bool> boolNeq3(&neqb, troo.getId(), falls.getId());
  BinaryFunction<bool> boolNeq4(&neqb, falls.getId(), troo.getId());
  BinaryFunction<bool> boolNeq5(&neqb, troo.getId(), varb.getId());
  BinaryFunction<bool> boolNeq6(&neqb, varb.getId(), falls.getId());
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
  Equal<std::string> eqs;
  NotEqual<std::string> neqs;

  BinaryFunction<bool> strEq1(&eqs, yo.getId(), yo.getId());
  BinaryFunction<bool> strEq2(&eqs, mama.getId(), mama.getId());
  BinaryFunction<bool> strEq3(&eqs, yo.getId(), mama.getId());
  BinaryFunction<bool> strEq4(&eqs, mama.getId(), yo.getId());
  BinaryFunction<bool> strEq5(&eqs, yo.getId(), vars.getId());
  BinaryFunction<bool> strEq6(&eqs, vars.getId(), mama.getId());
  BinaryFunction<bool> strNeq1(&neqs, yo.getId(), yo.getId());
  BinaryFunction<bool> strNeq2(&neqs, mama.getId(), mama.getId());
  BinaryFunction<bool> strNeq3(&neqs, yo.getId(), mama.getId());
  BinaryFunction<bool> strNeq4(&neqs, mama.getId(), yo.getId());
  BinaryFunction<bool> strNeq5(&neqs, yo.getId(), vars.getId());
  BinaryFunction<bool> strNeq6(&neqs, vars.getId(), mama.getId());
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

  assertTrue_1(strEq1.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strEq2.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strEq3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strEq4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strNeq4.getValue(tempb));
  assertTrue_1(tempb);

  // These should be unknown because vars is uninitialized
  assertTrue_1(!strEq5.getValue(tempb));
  assertTrue_1(!strEq6.getValue(tempb));
  assertTrue_1(!strNeq5.getValue(tempb));
  assertTrue_1(!strNeq6.getValue(tempb));

  // Set vars and try again
  vars.setValue(std::string("Mama"));
  assertTrue_1(strEq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strEq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strNeq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strNeq6.getValue(tempb));
  assertTrue_1(!tempb);

  vars.setValue(std::string("Yo!"));
  assertTrue_1(strEq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq6.getValue(tempb));
  assertTrue_1(tempb);

  vars.setValue(std::string("Yo Mama!"));
  assertTrue_1(strEq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(strNeq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(strNeq6.getValue(tempb));
  assertTrue_1(tempb);

  return true;
}

static bool testInteger()
{
  bool tempb;

  // Integer
  IntegerConstant won(1);
  IntegerConstant too(2);
  IntegerVariable vari;
  Equal<int32_t> eqi;
  NotEqual<int32_t> neqi;
  GreaterThan<int32_t> gti;
  GreaterEqual<int32_t> gei;
  LessThan<int32_t> lti;
  LessEqual<int32_t> lei;

  BinaryFunction<bool> intEq1(&eqi, won.getId(), won.getId());
  BinaryFunction<bool> intEq2(&eqi, too.getId(), too.getId());
  BinaryFunction<bool> intEq3(&eqi, won.getId(), too.getId());
  BinaryFunction<bool> intEq4(&eqi, too.getId(), won.getId());
  BinaryFunction<bool> intEq5(&eqi, won.getId(), vari.getId());
  BinaryFunction<bool> intEq6(&eqi, vari.getId(), too.getId());
  BinaryFunction<bool> intNeq1(&neqi, won.getId(), won.getId());
  BinaryFunction<bool> intNeq2(&neqi, too.getId(), too.getId());
  BinaryFunction<bool> intNeq3(&neqi, won.getId(), too.getId());
  BinaryFunction<bool> intNeq4(&neqi, too.getId(), won.getId());
  BinaryFunction<bool> intNeq5(&neqi, won.getId(), vari.getId());
  BinaryFunction<bool> intNeq6(&neqi, vari.getId(), too.getId());
  BinaryFunction<bool> intGt1(&gti, won.getId(), won.getId());
  BinaryFunction<bool> intGt2(&gti, too.getId(), too.getId());
  BinaryFunction<bool> intGt3(&gti, won.getId(), too.getId());
  BinaryFunction<bool> intGt4(&gti, too.getId(), won.getId());
  BinaryFunction<bool> intGt5(&gti, won.getId(), vari.getId());
  BinaryFunction<bool> intGt6(&gti, vari.getId(), too.getId());
  BinaryFunction<bool> intGe1(&gei, won.getId(), won.getId());
  BinaryFunction<bool> intGe2(&gei, too.getId(), too.getId());
  BinaryFunction<bool> intGe3(&gei, won.getId(), too.getId());
  BinaryFunction<bool> intGe4(&gei, too.getId(), won.getId());
  BinaryFunction<bool> intGe5(&gei, won.getId(), vari.getId());
  BinaryFunction<bool> intGe6(&gei, vari.getId(), too.getId());
  BinaryFunction<bool> intLt1(&lti, won.getId(), won.getId());
  BinaryFunction<bool> intLt2(&lti, too.getId(), too.getId());
  BinaryFunction<bool> intLt3(&lti, won.getId(), too.getId());
  BinaryFunction<bool> intLt4(&lti, too.getId(), won.getId());
  BinaryFunction<bool> intLt5(&lti, won.getId(), vari.getId());
  BinaryFunction<bool> intLt6(&lti, vari.getId(), too.getId());
  BinaryFunction<bool> intLe1(&lei, won.getId(), won.getId());
  BinaryFunction<bool> intLe2(&lei, too.getId(), too.getId());
  BinaryFunction<bool> intLe3(&lei, won.getId(), too.getId());
  BinaryFunction<bool> intLe4(&lei, too.getId(), won.getId());
  BinaryFunction<bool> intLe5(&lei, won.getId(), vari.getId());
  BinaryFunction<bool> intLe6(&lei, vari.getId(), too.getId());
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
  vari.setValue(2);
    
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

  vari.setValue(1);

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

  vari.setValue(-1);

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
  Equal<double> eqr;
  NotEqual<double> neqr;

  BinaryFunction<bool> dblEq1(&eqr, wontoo.getId(), wontoo.getId());
  BinaryFunction<bool> dblEq2(&eqr, tootoo.getId(), tootoo.getId());
  BinaryFunction<bool> dblEq3(&eqr, wontoo.getId(), tootoo.getId());
  BinaryFunction<bool> dblEq4(&eqr, tootoo.getId(), wontoo.getId());
  BinaryFunction<bool> dblEq5(&eqr, wontoo.getId(), varr.getId());
  BinaryFunction<bool> dblEq6(&eqr, varr.getId(), tootoo.getId());
  BinaryFunction<bool> dblNeq1(&neqr, wontoo.getId(), wontoo.getId());
  BinaryFunction<bool> dblNeq2(&neqr, tootoo.getId(), tootoo.getId());
  BinaryFunction<bool> dblNeq3(&neqr, wontoo.getId(), tootoo.getId());
  BinaryFunction<bool> dblNeq4(&neqr, tootoo.getId(), wontoo.getId());
  BinaryFunction<bool> dblNeq5(&neqr, wontoo.getId(), varr.getId());
  BinaryFunction<bool> dblNeq6(&neqr, varr.getId(), tootoo.getId());
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

  assertTrue_1(dblEq1.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblEq2.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblEq3.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblEq4.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq1.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq2.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq3.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblNeq4.getValue(tempb));
  assertTrue_1(tempb);

  // These should be unknown because varr is uninitialized
  assertTrue_1(!dblEq5.getValue(tempb));
  assertTrue_1(!dblEq6.getValue(tempb));
  assertTrue_1(!dblNeq5.getValue(tempb));
  assertTrue_1(!dblNeq6.getValue(tempb));

  // Set varr and try again
  varr.setValue(2);
  assertTrue_1(dblEq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblEq6.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblNeq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblNeq6.getValue(tempb));
  assertTrue_1(!tempb);

  varr.setValue(1);
  assertTrue_1(dblEq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq6.getValue(tempb));
  assertTrue_1(tempb);

  varr.setValue(-1);
  assertTrue_1(dblEq5.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblEq6.getValue(tempb));
  assertTrue_1(!tempb);
  assertTrue_1(dblNeq5.getValue(tempb));
  assertTrue_1(tempb);
  assertTrue_1(dblNeq6.getValue(tempb));
  assertTrue_1(tempb);

  return true;
}

static bool testMixedNumerics()
{
  bool tempb;
    
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
