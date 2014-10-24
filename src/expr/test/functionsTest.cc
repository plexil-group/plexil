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
#include "Constant.hh"
#include "Function.hh"
#include "TestSupport.hh"
#include "test/TrivialListener.hh"
#include "UserVariable.hh"

using namespace PLEXIL;

//
// Functions and operators
//

template <typename R>
class Passthrough : public OperatorImpl<R>
{
public:
  Passthrough()
    : OperatorImpl<R>("PT")
  {
  }

  ~Passthrough()
  {
  }

  bool checkArgCount(size_t count) const
  {
    return count == 1;
  }

  bool operator()(R &result, Expression const * arg) const
  {
    R temp;
    if (!arg->getValue(temp))
      return false;
    result = temp;
    return true;
  }
};

Passthrough<bool> ptb;
Passthrough<int32_t> pti;
Passthrough<double> ptd;
Passthrough<std::string> pts;

// TODO - test propagation of changes through variable and fn
static bool testUnaryBasics()
{
  {
    BooleanConstant treu(true);
    ExprVec *vecb = makeExprVec(1);
    vecb->setArgument(0, &treu, false);
    Function boule(&ptb, vecb);
    assertTrue_1(!boule.isKnown());
    boule.activate();
    bool tempb;
    assertTrue_1(boule.getValue(tempb));
    assertTrue_1(tempb == true);
  }

  {
    IntegerConstant fortytwo(42);
    {
      ExprVec *veci = makeExprVec(1);
      veci->setArgument(0, &fortytwo, false);
      Function inty(&pti, veci);
      assertTrue_1(!inty.isKnown());
      inty.activate();
      int32_t tempi;
      assertTrue_1(inty.getValue(tempi));
      assertTrue_1(tempi == 42);
    }

    {
      ExprVec *vecdi = makeExprVec(1);
      vecdi->setArgument(0, &fortytwo, false);
      Function intd(&ptd, vecdi);
      assertTrue_1(!intd.isKnown());
      intd.activate();
      double tempdi;
      assertTrue_1(intd.getValue(tempdi));
      assertTrue_1(tempdi == 42.0);
    }
  }

  {
    RealConstant pie(3.14);
    ExprVec *vecd = makeExprVec(1);
    vecd->setArgument(0, &pie, false);
    Function dub(&ptd, vecd);
    assertTrue_1(!dub.isKnown());
    dub.activate();
    double tempd;
    assertTrue_1(dub.getValue(tempd));
    assertTrue_1(tempd == 3.14);
  }

  {
    StringConstant fou("Foo");
    ExprVec *vecs = makeExprVec(1);
    vecs->setArgument(0, &fou, false);
    Function str(&pts, vecs);
    assertTrue_1(!str.isKnown());
    str.activate();
    std::string temps;
    assertTrue_1(str.getValue(temps));
    assertTrue_1(temps == std::string("Foo"));
  }

  return true;
}
 
static bool testUnaryPropagation()
{
  {
    BooleanVariable treu(true);
    ExprVec *vecb = makeExprVec(1);
    vecb->setArgument(0, &treu, false);
    Function boule(&ptb, vecb);
    bool bchanged = false;
    TrivialListener bl(bchanged);
    boule.addListener(&bl);

    treu.setValue(false);
    assertTrue_1(!bchanged);
    boule.activate();
    assertTrue_1(treu.isActive());

    treu.setValue(false);
    bool boolv;
    assertTrue_1(boule.getValue(boolv));
    assertTrue_1(!boolv);

    treu.setUnknown();
    assertTrue_1(bchanged);

    boule.removeListener(&bl);
  }

  {
    IntegerVariable fortytwo(42);

    {
      ExprVec *veci = makeExprVec(1);
      veci->setArgument(0, &fortytwo, false);
      Function inty(&pti, veci);
      bool ichanged = false;
      TrivialListener il(ichanged);
      inty.addListener(&il);

      fortytwo.setValue((int32_t) 43);
      assertTrue_1(!ichanged);
      inty.activate();
      assertTrue_1(fortytwo.isActive());

      fortytwo.setValue((int32_t) 43);
      int32_t intv;
      assertTrue_1(inty.getValue(intv));
      assertTrue_1(intv == 43);

      fortytwo.setUnknown();
      assertTrue_1(ichanged);

      inty.removeListener(&il);
      fortytwo.deactivate();
    }

    {
      ExprVec *vecdi = makeExprVec(1);
      vecdi->setArgument(0, &fortytwo, false);
      Function intd(&ptd, vecdi);
      bool r2changed = false;
      TrivialListener rl2(r2changed);
      intd.addListener(&rl2);

      fortytwo.setValue((int32_t) 43);
      assertTrue_1(!r2changed);
      intd.activate();
      assertTrue_1(fortytwo.isActive());

      fortytwo.setValue((int32_t) 43);
      double dubv;
      assertTrue_1(intd.getValue(dubv));
      assertTrue_1(dubv == 43);

      fortytwo.setUnknown();
      assertTrue_1(r2changed);
      intd.removeListener(&rl2);
    }
  }

  {
    RealVariable pie(3.14);
    ExprVec *vecd = makeExprVec(1);
    vecd->setArgument(0, &pie, false);
    Function dub(&ptd, vecd);
    bool rchanged = false;
    TrivialListener rl(rchanged);
    dub.addListener(&rl);

    pie.setValue(2.718);
    assertTrue_1(!rchanged);
    dub.activate();
    assertTrue_1(pie.isActive());

    pie.setValue(2.718);
    double dubv;
    assertTrue_1(dub.getValue(dubv));
    assertTrue_1(dubv == 2.718);

    pie.setUnknown();
    assertTrue_1(rchanged);

    dub.removeListener(&rl);
  }

  {
    StringVariable fou("Foo");
    ExprVec *vecs = makeExprVec(1);
    vecs->setArgument(0, &fou, false);
    Function str(&pts, vecs);
    bool schanged = false;
    TrivialListener sl(schanged);
    str.addListener(&sl);

    fou.setValue(std::string("fu"));
    assertTrue_1(!schanged);
    str.activate();
    assertTrue_1(fou.isActive());

    fou.setValue(std::string("fu"));
    std::string strv;
    assertTrue_1(str.getValue(strv));
    assertTrue_1(strv == std::string("fu"));

    fou.setUnknown();
    assertTrue_1(schanged);

    str.removeListener(&sl);
  }

  return true;
}

static bool testBinaryBasics()
{
  {
    Addition<int32_t> intAdd;
    IntegerVariable won(1);
    IntegerConstant too(2);
    ExprVec *vi = makeExprVec(2);
    vi->setArgument(0, &won, false);
    vi->setArgument(1, &too, false);
    Function intFn(&intAdd, vi);
    int32_t itemp;
    bool ichanged = false;
    TrivialListener il(ichanged);
    intFn.addListener(&il);

    assertTrue_1(!intFn.isActive());
    assertTrue_1(!won.isActive());
    assertTrue_1(!won.isKnown());
    assertTrue_1(!won.getValue(itemp));
    assertTrue_1(!intFn.isKnown());
    assertTrue_1(!intFn.getValue(itemp));

    intFn.activate();
    assertTrue_1(intFn.isActive());
    assertTrue_1(won.isActive());
    assertTrue_1(won.isKnown());
    assertTrue_1(intFn.isKnown());
    assertTrue_1(won.getValue(itemp));
    assertTrue_1(itemp == 1);
    assertTrue_1(intFn.getValue(itemp));
    assertTrue_1(itemp == 3);
    assertTrue_1(ichanged);

    ichanged = false;
    won.setUnknown();
    assertTrue_1(!won.isKnown());
    assertTrue_1(!won.getValue(itemp));
    assertTrue_1(!intFn.isKnown());
    assertTrue_1(!intFn.getValue(itemp));
    assertTrue_1(ichanged);

    ichanged = false;
    won.deactivate();
    won.reset();
    won.activate();
    assertTrue_1(won.isKnown());
    assertTrue_1(won.getValue(itemp));
    assertTrue_1(itemp == 1);
    assertTrue_1(intFn.getValue(itemp));
    assertTrue_1(itemp == 3);
    assertTrue_1(ichanged);
    intFn.removeListener(&il);
  }

  {
  Addition<double> realAdd;
  RealVariable tree(3);
  RealConstant fore(4);
  std::vector<Expression *> vr;
  vr.push_back(&tree);
  vr.push_back(&fore);
  std::vector<bool> garbage2(2, false);
  Function realFn(&realAdd, makeExprVec(vr, garbage2));
  double rtemp;
  bool rchanged = false;
  TrivialListener rl(rchanged);
  realFn.addListener(&rl);

  assertTrue_1(!realFn.isActive());
  assertTrue_1(!tree.isActive());
  assertTrue_1(!tree.isKnown());
  assertTrue_1(!tree.getValue(rtemp));
  assertTrue_1(!realFn.isKnown());
  assertTrue_1(!realFn.getValue(rtemp));

  realFn.activate();
  assertTrue_1(realFn.isActive());
  assertTrue_1(tree.isActive());

  assertTrue_1(tree.isKnown());
  assertTrue_1(realFn.isKnown());
  assertTrue_1(tree.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realFn.getValue(rtemp));
  assertTrue_1(rtemp == 7);
  assertTrue_1(rchanged);

  rchanged = false;
  tree.setUnknown();
  assertTrue_1(!tree.isKnown());
  assertTrue_1(!tree.getValue(rtemp));
  assertTrue_1(!realFn.isKnown());
  assertTrue_1(!realFn.getValue(rtemp));
  assertTrue_1(rchanged);

  rchanged = false;
  tree.deactivate();
  tree.reset();
  tree.activate();
  assertTrue_1(tree.isKnown());
    assertTrue_1(realFn.isKnown());
  assertTrue_1(tree.getValue(rtemp));
  assertTrue_1(rtemp == 3);
  assertTrue_1(realFn.getValue(rtemp));
  assertTrue_1(rtemp == 7);
  assertTrue_1(rchanged);

  realFn.removeListener(&rl);
  }
  
  return true;
}

static bool testNaryBasics()
{
  const std::vector<bool> garbage(3, false);

  {
    Addition<int32_t> intAdd;
    IntegerVariable won(1);
    IntegerConstant too(2);
    IntegerVariable tree(3);
    std::vector<Expression *> exprs;
    exprs.push_back(&won);
    exprs.push_back(&too);
    exprs.push_back(&tree);
    Function intFn(&intAdd, makeExprVec(exprs, garbage));
    int32_t itemp;
    bool ichanged = false;
    TrivialListener il(ichanged);
    intFn.addListener(&il);

    assertTrue_1(!intFn.isActive());
    assertTrue_1(!won.isActive());
    assertTrue_1(!tree.isActive());
    assertTrue_1(!intFn.isKnown());
    assertTrue_1(!intFn.getValue(itemp));

    intFn.activate();
    assertTrue_1(intFn.isActive());
    assertTrue_1(won.isActive());
    assertTrue_1(tree.isActive());
    assertTrue_1(intFn.isKnown());
    assertTrue_1(intFn.getValue(itemp));
    assertTrue_1(itemp == 6);
    assertTrue_1(ichanged);

    ichanged = false;
    tree.setUnknown();
    assertTrue_1(!tree.isKnown());
    assertTrue_1(!tree.getValue(itemp));
    assertTrue_1(!intFn.isKnown());
    assertTrue_1(!intFn.getValue(itemp));
    assertTrue_1(ichanged);
    ichanged = false;

    tree.deactivate();
    tree.reset();
    tree.activate();
    assertTrue_1(tree.isKnown());
    assertTrue_1(intFn.isKnown());
    assertTrue_1(tree.getValue(itemp));
    assertTrue_1(itemp == 3);
    assertTrue_1(intFn.getValue(itemp));
    assertTrue_1(itemp == 6);
    assertTrue_1(ichanged);
    intFn.removeListener(&il);
  }

  {
    Addition<double> realAdd;
    RealConstant fore(4);
    RealVariable fivefive(5.5);
    RealVariable sixfive(6.5);
    std::vector<Expression *> exprs;
    exprs.push_back(&fore);
    exprs.push_back(&fivefive);
    exprs.push_back(&sixfive);
    Function realFn(&realAdd, makeExprVec(exprs, garbage));
    double rtemp;
    bool rchanged = false;
    TrivialListener rl(rchanged);
    realFn.addListener(&rl);

    assertTrue_1(!realFn.isActive());
    assertTrue_1(!fivefive.isActive());
    assertTrue_1(!sixfive.isActive());
    assertTrue_1(!realFn.isKnown());
    assertTrue_1(!realFn.getValue(rtemp));

    realFn.activate();
    assertTrue_1(realFn.isActive());
    assertTrue_1(fivefive.isActive());
    assertTrue_1(sixfive.isActive());
    assertTrue_1(realFn.isKnown());
    assertTrue_1(realFn.getValue(rtemp));
    assertTrue_1(rtemp == 16);
    assertTrue_1(rchanged);

    rchanged = false;
    fivefive.setUnknown();
    assertTrue_1(!fivefive.isKnown());
    assertTrue_1(!fivefive.getValue(rtemp));
    assertTrue_1(!realFn.isKnown());
    assertTrue_1(!realFn.getValue(rtemp));
    assertTrue_1(rchanged);
    rchanged = false;

    // Reset variables, check that values are known and reasonable
    fivefive.deactivate();
    fivefive.reset();
    fivefive.activate();
    assertTrue_1(fivefive.isKnown());
    assertTrue_1(realFn.isKnown());
    assertTrue_1(fivefive.getValue(rtemp));
    assertTrue_1(rtemp == 5.5);
    assertTrue_1(realFn.getValue(rtemp));
    assertTrue_1(rtemp == 16);
    assertTrue_1(rchanged);

    realFn.removeListener(&rl);
  }

  return true;
}

bool functionsTest()
{
  runTest(testUnaryBasics);
  runTest(testUnaryPropagation);
  runTest(testBinaryBasics);
  runTest(testNaryBasics);
  return true;
}
