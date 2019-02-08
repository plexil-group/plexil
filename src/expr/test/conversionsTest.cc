/* Copyright (c) 2006-2019, Universities Space Research Association (USRA).
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

#include "ConversionOperators.hh"
#include "Function.hh"
#include "Constant.hh"
#include "TestSupport.hh"
#include "UserVariable.hh"

using namespace PLEXIL;


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

  Ceiling ceiling;
  RealVariable x;

  Function *realCeiling = makeFunction(&ceiling, 1);
  realCeiling->setArgument(0, &x, false);

  Real rtemp;
  Integer itemp;

  x.activate();
  realCeiling->activate();

  assertTrue_1(realCeiling->valueType() == INTEGER_TYPE);
  assertTrue_1(!x.isKnown());
  assertTrue_1(!realCeiling->isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realCeiling->getValue(rtemp));

  x.setValue(three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realCeiling->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == 4);
  assertTrue_1(realCeiling->getValue(itemp));
  assertTrue_1(itemp == 4);

  x.setValue(e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realCeiling->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realCeiling->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realCeiling->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == -2);
  assertTrue_1(realCeiling->getValue(itemp));
  assertTrue_1(itemp == -2);

  x.setValue(toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == 3000000001.0);
  assertTrue_1(!realCeiling->getValue(itemp));

  x.setValue(toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realCeiling->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realCeiling->getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.0);
  assertTrue_1(!realCeiling->getValue(itemp));
  
  delete realCeiling;

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

  Floor floor;
  RealVariable x;

  Function *realFloor = makeFunction(&floor, 1);
  realFloor->setArgument(0, &x, false);

  Real rtemp;
  Integer itemp;

  x.activate();
  realFloor->activate();

  assertTrue_1(realFloor->valueType() == INTEGER_TYPE);
  assertTrue_1(!x.isKnown());
  assertTrue_1(!realFloor->isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realFloor->getValue(rtemp));
  assertTrue_1(!realFloor->getValue(itemp));

  x.setValue(three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realFloor->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realFloor->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == 2);
  assertTrue_1(realFloor->getValue(itemp));
  assertTrue_1(itemp == 2);

  x.setValue(minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realFloor->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == -4);
  assertTrue_1(realFloor->getValue(itemp));
  assertTrue_1(itemp == -4);

  x.setValue(minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realFloor->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.0);
  assertTrue_1(!realFloor->getValue(itemp));

  x.setValue(toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realFloor->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realFloor->getValue(rtemp));
  assertTrue_1(rtemp == -3000000001.0);
  assertTrue_1(!realFloor->getValue(itemp));

  delete realFloor;

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

  Round round;
  RealVariable x;

  Function *realRound = makeFunction(&round, 1);
  realRound->setArgument(0, &x, false);

  Real rtemp;
  Integer itemp;

  x.activate();
  realRound->activate();

  assertTrue_1(realRound->valueType() == INTEGER_TYPE);
  assertTrue_1(!x.isKnown());
  assertTrue_1(!realRound->isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realRound->getValue(rtemp));
  assertTrue_1(!realRound->getValue(itemp));

  x.setValue(three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realRound->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realRound->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realRound->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realRound->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realRound->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realRound->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == 3000000001.0);
  assertTrue_1(!realRound->getValue(itemp));

  x.setValue(toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realRound->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realRound->getValue(rtemp));
  assertTrue_1(rtemp == -3000000001.0);
  assertTrue_1(!realRound->getValue(itemp));

  delete realRound;

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

  Truncate truncate;
  RealVariable x;

  Function *realTruncate = makeFunction(&truncate, 1);
  realTruncate->setArgument(0, &x, false);

  Real rtemp;
  Integer itemp;

  x.activate();
  realTruncate->activate();

  assertTrue_1(realTruncate->valueType() == INTEGER_TYPE);
  assertTrue_1(!x.isKnown());
  assertTrue_1(!realTruncate->isKnown());
  assertTrue_1(!x.getValue(rtemp));
  assertTrue_1(!realTruncate->getValue(rtemp));
  assertTrue_1(!realTruncate->getValue(itemp));

  x.setValue(three);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realTruncate->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(pi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3.14);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realTruncate->getValue(itemp));
  assertTrue_1(itemp == 3);

  x.setValue(e);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 2.718);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == 2);
  assertTrue_1(realTruncate->getValue(itemp));
  assertTrue_1(itemp == 2);

  x.setValue(minusthree);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realTruncate->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuspi);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3.14);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == -3);
  assertTrue_1(realTruncate->getValue(itemp));
  assertTrue_1(itemp == -3);

  x.setValue(minuse);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -2.718);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == -2);
  assertTrue_1(realTruncate->getValue(itemp));
  assertTrue_1(itemp == -2);

  x.setValue(toobig);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.5);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == 3000000000.0);
  assertTrue_1(!realTruncate->getValue(itemp));

  x.setValue(toonegative);
  assertTrue_1(x.isKnown());
  assertTrue_1(realTruncate->isKnown());
  assertTrue_1(x.getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.5);
  assertTrue_1(realTruncate->getValue(rtemp));
  assertTrue_1(rtemp == -3000000000.0);
  assertTrue_1(!realTruncate->getValue(itemp));

  delete realTruncate;

  return true;
}

#endif // !defined(__VXWORKS__)

static bool testRealToInteger()
{
  RealConstant zero((Real) 0);
  RealConstant three(3);
  RealConstant pi(3.14);
  RealConstant minusthree(-3);
  RealConstant minuspi(-3.14);
  RealConstant toobig(3000000000.0);
  RealConstant toonegative(-3000000000.0);

  RealToInteger rtiOp;
  RealVariable x;

  Function *rti = makeFunction(&rtiOp, 1);
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
