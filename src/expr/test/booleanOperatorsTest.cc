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

  // Test logic
  UnaryFunction<bool> n1(BooleanNot::instance(), unk.getId());
  UnaryFunction<bool> n2(BooleanNot::instance(), falls.getId());
  UnaryFunction<bool> n3(BooleanNot::instance(), troo.getId());

  bool temp;
  n1.activate();
  assertTrue_1(!n1.getValue(temp));
  n2.activate();
  assertTrue_1(n2.getValue(temp));
  assertTrue_1(temp);
  n3.activate();
  assertTrue_1(n3.getValue(temp));
  assertTrue_1(!temp);

  return true;
}

static bool testBooleanAnd()
{
  BooleanConstant unk;
  BooleanConstant troo(true);
  BooleanConstant falls(false);

  // Basic three-valued AND semantics of two args
  BinaryFunction<bool> c1(BooleanAnd::instance(), falls.getId(), falls.getId());
  c1.activate();
  BinaryFunction<bool> c2(BooleanAnd::instance(), falls.getId(), unk.getId());
  c2.activate();
  BinaryFunction<bool> c3(BooleanAnd::instance(), falls.getId(), troo.getId());
  c3.activate();
  BinaryFunction<bool> c4(BooleanAnd::instance(), unk.getId(), falls.getId());
  c4.activate();
  BinaryFunction<bool> c5(BooleanAnd::instance(), unk.getId(), unk.getId());
  c5.activate();
  BinaryFunction<bool> c6(BooleanAnd::instance(), unk.getId(), troo.getId());
  c6.activate();
  BinaryFunction<bool> c7(BooleanAnd::instance(), troo.getId(), falls.getId());
  c7.activate();
  BinaryFunction<bool> c8(BooleanAnd::instance(), troo.getId(), unk.getId());
  c8.activate();
  BinaryFunction<bool> c9(BooleanAnd::instance(), troo.getId(), troo.getId());
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

  // Three-valued OR semantics of two args
  BinaryFunction<bool> d1(BooleanOr::instance(), falls.getId(), falls.getId());
  BinaryFunction<bool> d2(BooleanOr::instance(), falls.getId(), unk.getId());
  BinaryFunction<bool> d3(BooleanOr::instance(), falls.getId(), troo.getId());
  BinaryFunction<bool> d4(BooleanOr::instance(), unk.getId(), falls.getId());
  BinaryFunction<bool> d5(BooleanOr::instance(), unk.getId(), unk.getId());
  BinaryFunction<bool> d6(BooleanOr::instance(), unk.getId(), troo.getId());
  BinaryFunction<bool> d7(BooleanOr::instance(), troo.getId(), falls.getId());
  BinaryFunction<bool> d8(BooleanOr::instance(), troo.getId(), unk.getId());
  BinaryFunction<bool> d9(BooleanOr::instance(), troo.getId(), troo.getId());

  bool temp;
  d1.activate();
  assertTrue_1(d1.getValue(temp));
  assertTrue_1(!temp);
  d2.activate();
  assertTrue_1(!d2.getValue(temp));
  d3.activate();
  assertTrue_1(d3.getValue(temp));
  assertTrue_1(temp);
  d4.activate();
  assertTrue_1(!d4.getValue(temp));
  d5.activate();
  assertTrue_1(!d5.getValue(temp));
  d6.activate();
  assertTrue_1(d6.getValue(temp));
  assertTrue_1(temp);
  d7.activate();
  assertTrue_1(d7.getValue(temp));
  assertTrue_1(temp);
  d8.activate();
  assertTrue_1(d8.getValue(temp));
  assertTrue_1(temp);
  d9.activate();
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

  //test basic three-valued XOR semantics
  BinaryFunction<bool> d1(BooleanXor::instance(), falls.getId(), falls.getId());
  BinaryFunction<bool> d2(BooleanXor::instance(), falls.getId(), unk.getId());
  BinaryFunction<bool> d3(BooleanXor::instance(), falls.getId(), troo.getId());
  BinaryFunction<bool> d4(BooleanXor::instance(), unk.getId(), falls.getId());
  BinaryFunction<bool> d5(BooleanXor::instance(), unk.getId(), unk.getId());
  BinaryFunction<bool> d6(BooleanXor::instance(), unk.getId(), troo.getId());
  BinaryFunction<bool> d7(BooleanXor::instance(), troo.getId(), falls.getId());
  BinaryFunction<bool> d8(BooleanXor::instance(), troo.getId(), unk.getId());
  BinaryFunction<bool> d9(BooleanXor::instance(), troo.getId(), troo.getId());

  bool temp;
  d1.activate();
  assertTrue_1(d1.getValue(temp));
  assertTrue_1(!temp);
  d2.activate();
  assertTrue_1(!d2.getValue(temp));
  d3.activate();
  assertTrue_1(d3.getValue(temp));
  assertTrue_1(temp);
  d4.activate();
  assertTrue_1(!d4.getValue(temp));
  d5.activate();
  assertTrue_1(!d5.getValue(temp));
  d6.activate();
  assertTrue_1(!d6.getValue(temp));
  d7.activate();
  assertTrue_1(d7.getValue(temp));
  assertTrue_1(temp);
  d8.activate();
  assertTrue_1(!d8.getValue(temp));
  d9.activate();
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
