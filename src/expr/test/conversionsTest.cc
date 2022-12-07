/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "plexil-config.h"

#include "ConversionOperators.hh"
#include "Function.hh"
#include "Constant.hh"
#include "TestSupport.hh"
#include "UserVariable.hh"

using namespace PLEXIL;


static bool testCeiling()
{
#ifdef HAVE_CEIL
  RealConstant three(3);
  RealConstant pi(3.14);
  RealConstant e(2.718);
  RealConstant minusthree(-3);
  RealConstant minuspi(-3.14);
  RealConstant minuse(-2.718);
  RealConstant toobig(3000000000.5);
  RealConstant toonegative(-3000000000.5);

  RealVariable x;

  Function *realCeiling = makeFunction(Ceiling<Real>::instance(), 1);
  realCeiling->setArgument(0, &x, false);
  Function *integerCeiling = makeFunction(Ceiling<Integer>::instance(), 1);
  integerCeiling->setArgument(0, &x, false);

  Real rtemp;
  Integer itemp;

  x.activate();
  realCeiling->activate();
  integerCeiling->activate();

  assertTrue_1(realCeiling->valueType() == REAL_TYPE);
  assertTrue_1(integerCeiling->valueType() == INTEGER_TYPE);
  assertTrue_1(!x.isKnown());
  assertTrue_1(!realCeiling->isKnown());
  assertTrue_1(!integerCeiling->isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realCeiling->getValue(rtemp));
  assertTrue_1(!integerCeiling->getValue(rtemp));

  x.setValue(three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(integerCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(integerCeiling->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(integerCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == 4);
  assertTrue_1(integerCeiling->getValue(itemp));
  assertTrue_1(itemp == 4);

  x.setValue(e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(integerCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(integerCeiling->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(integerCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(integerCeiling->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(integerCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(integerCeiling->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(integerCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == -2);
  assertTrue_1(integerCeiling->getValue(itemp));
  assertTrue_1(itemp == -2);

  x.setValue(toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(!integerCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == 3000000001.0);
  assertTrue_1(!integerCeiling->getValue(itemp));

  x.setValue(toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(!integerCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.0);
  assertTrue_1(!integerCeiling->getValue(itemp));
  
  delete integerCeiling;
  delete realCeiling;
#endif // HAVE_CEIL

  return true;
}

static bool testFloor()
{
#ifdef HAVE_FLOOR
  RealConstant three(3);
  RealConstant pi(3.14);
  RealConstant e(2.718);
  RealConstant minusthree(-3);
  RealConstant minuspi(-3.14);
  RealConstant minuse(-2.718);
  RealConstant toobig(3000000000.5);
  RealConstant toonegative(-3000000000.5);

  RealVariable x;

  Function *realFloor = makeFunction(Floor<Real>::instance(), 1);
  realFloor->setArgument(0, &x, false);
  Function *integerFloor = makeFunction(Floor<Integer>::instance(), 1);
  integerFloor->setArgument(0, &x, false);

  Real rtemp;
  Integer itemp;

  x.activate();
  realFloor->activate();
  integerFloor->activate();

  assertTrue_1(realFloor->valueType() == REAL_TYPE);
  assertTrue_1(integerFloor->valueType() == INTEGER_TYPE);
  assertTrue_1(!x.isKnown());
  assertTrue_1(!realFloor->isKnown());
  assertTrue_1(!integerFloor->isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realFloor->getValue(rtemp));
  assertTrue_1(!integerFloor->getValue(itemp));

  x.setValue(three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(integerFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(integerFloor->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(integerFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(integerFloor->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(integerFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == 2);
  assertTrue_1(integerFloor->getValue(itemp));
  assertTrue_1(itemp == 2);

  x.setValue(minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(integerFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(integerFloor->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(integerFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == -4);
  assertTrue_1(integerFloor->getValue(itemp));
  assertTrue_1(itemp == -4);

  x.setValue(minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(integerFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(integerFloor->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(!integerFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.0);
  assertTrue_1(!integerFloor->getValue(itemp));

  x.setValue(toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(!integerFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == -3000000001.0);
  assertTrue_1(!integerFloor->getValue(itemp));

  delete integerFloor;
  delete realFloor;
#endif // HAVE_FLOOR

  return true;
}

static bool testRound()
{
#ifdef HAVE_ROUND
  RealConstant three(3);
  RealConstant pi(3.14);
  RealConstant e(2.718);
  RealConstant minusthree(-3);
  RealConstant minuspi(-3.14);
  RealConstant minuse(-2.718);
  RealConstant toobig(3000000000.5);
  RealConstant toonegative(-3000000000.5);

  RealVariable x;

  Function *realRound = makeFunction(Round<Real>::instance(), 1);
  realRound->setArgument(0, &x, false);
  Function *integerRound = makeFunction(Round<Integer>::instance(), 1);
  integerRound->setArgument(0, &x, false);

  Real rtemp;
  Integer itemp;

  x.activate();
  realRound->activate();
  integerRound->activate();

  assertTrue_1(realRound->valueType() == REAL_TYPE);
  assertTrue_1(integerRound->valueType() == INTEGER_TYPE);
  assertTrue_1(!x.isKnown());
  assertTrue_1(!realRound->isKnown());
  assertTrue_1(!integerRound->isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realRound->getValue(rtemp));
  assertTrue_1(!integerRound->getValue(itemp));

  x.setValue(three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(integerRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(integerRound->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(integerRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(integerRound->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(integerRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(integerRound->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(integerRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(integerRound->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(integerRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(integerRound->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(integerRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(integerRound->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(!integerRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == 3000000001.0);
  assertTrue_1(!integerRound->getValue(itemp));

  x.setValue(toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(!integerRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == -3000000001.0);
  assertTrue_1(!integerRound->getValue(itemp));

  delete integerRound;
  delete realRound;
#endif // HAVE_ROUND

  return true;
}

static bool testTruncate()
{
#ifdef HAVE_TRUNC
  RealConstant three(3);
  RealConstant pi(3.14);
  RealConstant e(2.718);
  RealConstant minusthree(-3);
  RealConstant minuspi(-3.14);
  RealConstant minuse(-2.718);
  RealConstant toobig(3000000000.5);
  RealConstant toonegative(-3000000000.5);

  RealVariable x;

  Function *realTruncate = makeFunction(Truncate<Real>::instance(), 1);
  realTruncate->setArgument(0, &x, false);
  Function *integerTruncate = makeFunction(Truncate<Integer>::instance(), 1);
  integerTruncate->setArgument(0, &x, false);

  Real rtemp;
  Integer itemp;

  x.activate();
  realTruncate->activate();
  integerTruncate->activate();

  assertTrue_1(realTruncate->valueType() == REAL_TYPE);
  assertTrue_1(integerTruncate->valueType() == INTEGER_TYPE);
  assertTrue_1(!x.isKnown());
  assertTrue_1(!realTruncate->isKnown());
  assertTrue_1(!integerTruncate->isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realTruncate->getValue(rtemp));
  assertTrue_1(!integerTruncate->getValue(itemp));

  x.setValue(three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(integerTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(integerTruncate->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(integerTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(integerTruncate->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(integerTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == 2);
  assertTrue_1(integerTruncate->getValue(itemp));
  assertTrue_1(itemp == 2);

  x.setValue(minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(integerTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(integerTruncate->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(integerTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(integerTruncate->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(integerTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == -2);
  assertTrue_1(integerTruncate->getValue(itemp));
  assertTrue_1(itemp == -2);

  x.setValue(toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(!integerTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.0);
  assertTrue_1(!integerTruncate->getValue(itemp));

  x.setValue(toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(!integerTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.0);
  assertTrue_1(!integerTruncate->getValue(itemp));

  delete integerTruncate;
  delete realTruncate;
#endif // HAVE_TRUNC

  return true;
}

static bool testRealToInteger()
{
  RealConstant zero((Real) 0);
  RealConstant three(3);
  RealConstant pi(3.14);
  RealConstant minusthree(-3);
  RealConstant minuspi(-3.14);
  RealConstant toobig(3000000000.0);
  RealConstant toonegative(-3000000000.0);

  RealVariable x;

  Function *rti = makeFunction(RealToInteger::instance(), 1);
  rti->setArgument(0, &x, false);

  Real rtemp;
  Integer itemp;

  x.activate();
  rti->activate();

  // Uninitialized
  assertTrue_1(!x.isKnown());
  assertTrue_1(!rti->isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!rti->getValue(itemp));

  x.setValue(zero);
  assertTrue_1(x.isKnown());
  assertTrue_1(rti->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 0);
  assertTrue_1(rti->getValue(itemp));
  assertTrue_1(itemp == 0);

  x.setValue(three);
  assertTrue_1(x.isKnown());
  assertTrue_1(rti->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(rti->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(!rti->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(!rti->getValue(itemp));

  x.setValue(minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(rti->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(rti->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(!rti->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(!rti->getValue(itemp));

  x.setValue(toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(!rti->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.0);
  assertTrue_1(!rti->getValue(itemp));

  x.setValue(toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(!rti->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.0);
  assertTrue_1(!rti->getValue(itemp));

  delete rti;

  return true;
}

bool conversionsTest()
{
#ifdef HAVE_CEIL
  runTest(testCeiling);
#endif

#ifdef HAVE_FLOOR
  runTest(testFloor);
#endif

#ifdef HAVE_ROUND
  runTest(testRound);
#endif

#ifdef HAVE_TRUNC
  runTest(testTruncate);
#endif

  runTest(testRealToInteger);
  return true;
}
