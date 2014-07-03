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

#include "BooleanOperators.hh"
#include "Constant.hh"
#include "Function.hh"
#include "TestSupport.hh"

using namespace PLEXIL;

static bool testBooleanNot()
{
  BooleanConstant unk;
  BooleanConstant troo(true);
  BooleanConstant falls(false);

  std::vector<bool> garbage1(1, false);
  std::vector<Expression *> unkv(1, &unk);
  std::vector<Expression *> troov(1, &troo);
  std::vector<Expression *> fallsv(1, &falls);

  // Test logic
  Function n1(BooleanNot::instance(), makeExprVec(unkv, garbage1));
  Function n2(BooleanNot::instance(), makeExprVec(troov, garbage1));
  Function n3(BooleanNot::instance(), makeExprVec(fallsv, garbage1));

  n1.activate();
  n2.activate();
  n3.activate();

  bool temp;
  assertTrue_1(!n1.getValue(temp));

  assertTrue_1(n2.getValue(temp));
  assertTrue_1(!temp);

  assertTrue_1(n3.getValue(temp));
  assertTrue_1(temp);

  return true;
}

static bool testBooleanAnd()
{
  BooleanConstant unk;
  BooleanConstant troo(true);
  BooleanConstant falls(false);

  std::vector<bool> garbage2(2, false);
  std::vector<Expression *> v1, v2, v3, v4, v5, v6, v7, v8, v9;
  v1.push_back(&falls);
  v1.push_back(&falls);

  v2.push_back(&falls);
  v2.push_back(&unk);

  v3.push_back(&falls);
  v3.push_back(&troo);

  v4.push_back(&unk);
  v4.push_back(&falls);

  v5.push_back(&unk);
  v5.push_back(&unk);

  v6.push_back(&unk);
  v6.push_back(&troo);

  v7.push_back(&troo);
  v7.push_back(&falls);

  v8.push_back(&troo);
  v8.push_back(&unk);

  v9.push_back(&troo);
  v9.push_back(&troo);

  // Basic three-valued AND semantics of two args
  Function c1(BooleanAnd::instance(), makeExprVec(v1, garbage2));
  Function c2(BooleanAnd::instance(), makeExprVec(v2, garbage2));
  Function c3(BooleanAnd::instance(), makeExprVec(v3, garbage2));
  Function c4(BooleanAnd::instance(), makeExprVec(v4, garbage2));
  Function c5(BooleanAnd::instance(), makeExprVec(v5, garbage2));
  Function c6(BooleanAnd::instance(), makeExprVec(v6, garbage2));
  Function c7(BooleanAnd::instance(), makeExprVec(v7, garbage2));
  Function c8(BooleanAnd::instance(), makeExprVec(v8, garbage2));
  Function c9(BooleanAnd::instance(), makeExprVec(v9, garbage2));

  c1.activate();
  c2.activate();
  c3.activate();
  c4.activate();
  c5.activate();
  c6.activate();
  c7.activate();
  c8.activate();
  c9.activate();

  bool temp;
  assertTrue_1(c1.getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(c2.getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(c3.getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(c4.getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(!c5.getValue(temp));
  assertTrue_1(!c6.getValue(temp));
  assertTrue_1(c7.getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(!c8.getValue(temp));
  assertTrue_1(c9.getValue(temp));
  assertTrue_1(temp);

  // TODO: 3 or more args

  return true;
}

static bool testBooleanOr()
{
  BooleanConstant unk;
  BooleanConstant troo(true);
  BooleanConstant falls(false);

  std::vector<bool> garbage2(2, false);
  std::vector<Expression *> v1, v2, v3, v4, v5, v6, v7, v8, v9;
  v1.push_back(&falls);
  v1.push_back(&falls);

  v2.push_back(&falls);
  v2.push_back(&unk);

  v3.push_back(&falls);
  v3.push_back(&troo);

  v4.push_back(&unk);
  v4.push_back(&falls);

  v5.push_back(&unk);
  v5.push_back(&unk);

  v6.push_back(&unk);
  v6.push_back(&troo);

  v7.push_back(&troo);
  v7.push_back(&falls);

  v8.push_back(&troo);
  v8.push_back(&unk);

  v9.push_back(&troo);
  v9.push_back(&troo);

  // Three-valued OR semantics of two args
  Function d1(BooleanOr::instance(), makeExprVec(v1, garbage2));
  Function d2(BooleanOr::instance(), makeExprVec(v2, garbage2));
  Function d3(BooleanOr::instance(), makeExprVec(v3, garbage2));
  Function d4(BooleanOr::instance(), makeExprVec(v4, garbage2));
  Function d5(BooleanOr::instance(), makeExprVec(v5, garbage2));
  Function d6(BooleanOr::instance(), makeExprVec(v6, garbage2));
  Function d7(BooleanOr::instance(), makeExprVec(v7, garbage2));
  Function d8(BooleanOr::instance(), makeExprVec(v8, garbage2));
  Function d9(BooleanOr::instance(), makeExprVec(v9, garbage2));

  d1.activate();
  d2.activate();
  d3.activate();
  d4.activate();
  d5.activate();
  d6.activate();
  d7.activate();
  d8.activate();
  d9.activate();

  bool temp;
  assertTrue_1(d1.getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(!d2.getValue(temp));
  assertTrue_1(d3.getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(!d4.getValue(temp));
  assertTrue_1(!d5.getValue(temp));
  assertTrue_1(d6.getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(d7.getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(d8.getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(d9.getValue(temp));
  assertTrue_1(temp);

  // TODO: 3 or more args

  return true;
}

static bool testBooleanXor()
{
  BooleanConstant unk;
  BooleanConstant troo(true);
  BooleanConstant falls(false);

  std::vector<bool> garbage2(2, false);
  std::vector<Expression *> v1, v2, v3, v4, v5, v6, v7, v8, v9;
  v1.push_back(&falls);
  v1.push_back(&falls);

  v2.push_back(&falls);
  v2.push_back(&unk);

  v3.push_back(&falls);
  v3.push_back(&troo);

  v4.push_back(&unk);
  v4.push_back(&falls);

  v5.push_back(&unk);
  v5.push_back(&unk);

  v6.push_back(&unk);
  v6.push_back(&troo);

  v7.push_back(&troo);
  v7.push_back(&falls);

  v8.push_back(&troo);
  v8.push_back(&unk);

  v9.push_back(&troo);
  v9.push_back(&troo);

  //test basic three-valued XOR semantics
  Function d1(BooleanXor::instance(), makeExprVec(v1, garbage2));
  Function d2(BooleanXor::instance(), makeExprVec(v2, garbage2));
  Function d3(BooleanXor::instance(), makeExprVec(v3, garbage2));
  Function d4(BooleanXor::instance(), makeExprVec(v4, garbage2));
  Function d5(BooleanXor::instance(), makeExprVec(v5, garbage2));
  Function d6(BooleanXor::instance(), makeExprVec(v6, garbage2));
  Function d7(BooleanXor::instance(), makeExprVec(v7, garbage2));
  Function d8(BooleanXor::instance(), makeExprVec(v8, garbage2));
  Function d9(BooleanXor::instance(), makeExprVec(v9, garbage2));

  d1.activate();
  d2.activate();
  d3.activate();
  d4.activate();
  d5.activate();
  d6.activate();
  d7.activate();
  d8.activate();
  d9.activate();

  bool temp;
  assertTrue_1(d1.getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(!d2.getValue(temp));
  assertTrue_1(d3.getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(!d4.getValue(temp));
  assertTrue_1(!d5.getValue(temp));
  assertTrue_1(!d6.getValue(temp));
  assertTrue_1(d7.getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(!d8.getValue(temp));
  assertTrue_1(d9.getValue(temp));
  assertTrue_1(!temp);

  // TODO: 3 or more args
    
  return true;
}

bool booleanOperatorsTest()
{
  runTest(testBooleanNot);
  runTest(testBooleanAnd);
  runTest(testBooleanOr);
  runTest(testBooleanXor);

  return true;
}
