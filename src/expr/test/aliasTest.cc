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

#include "Alias.hh"
#include "ArrayConstant.hh"
#include "ArrayReference.hh"
#include "ArrayVariable.hh"
#include "TestSupport.hh"
#include "TrivialListener.hh"

using namespace PLEXIL;

static bool testAliasToScalarConstant()
{
  BooleanConstant troo(true);
  IntegerConstant tree(3);
  RealConstant pie(3.14);
  StringConstant fore("four");

  Alias atroo(NodeId::noId(), "atroo", troo.getId());
  Alias atree(NodeId::noId(), "atree", tree.getId());
  Alias apie (NodeId::noId(), "apie",  pie.getId());
  Alias afore(NodeId::noId(), "afore", fore.getId());

  // Check that alias is not assignable
  assertTrue_1(!atroo.isAssignable());
  assertTrue_1(!atree.isAssignable());
  assertTrue_1(!apie.isAssignable());
  assertTrue_1(!afore.isAssignable());

  // Check that isConstant is forwarded
  assertTrue_1(atroo.isConstant() == troo.isConstant());
  assertTrue_1(atree.isConstant() == tree.isConstant());
  assertTrue_1(apie.isConstant()   == pie.isConstant());
  assertTrue_1(afore.isConstant() == fore.isConstant());

  // Check that valueType is forwarded
  assertTrue_1(atroo.valueType() == troo.valueType());
  assertTrue_1(atree.valueType() == tree.valueType());
  assertTrue_1(apie.valueType()   == pie.valueType());
  assertTrue_1(afore.valueType() == fore.valueType());

  // Check unknown when inactive
  assertTrue_1(!atroo.isActive());
  assertTrue_1(!atree.isActive());
  assertTrue_1(!apie.isActive());
  assertTrue_1(!afore.isActive());

  assertTrue_1(!atroo.isKnown());
  assertTrue_1(!atree.isKnown());
  assertTrue_1(!apie.isKnown());
  assertTrue_1(!afore.isKnown());

  // Activate
  atroo.activate();
  atree.activate();
  apie.activate();
  afore.activate();

  // Check that all are active and known
  assertTrue_1(atroo.isActive());
  assertTrue_1(atree.isActive());
  assertTrue_1(apie.isActive());
  assertTrue_1(afore.isActive());

  assertTrue_1(atroo.isKnown());
  assertTrue_1(atree.isKnown());
  assertTrue_1(apie.isKnown());
  assertTrue_1(afore.isKnown());
  
  // Check that getValue is forwarded
  bool tb, tba;
  int32_t ti, tia;
  double td, tda;
  std::string ts, tsa;
  const std::string *tsp, *tspa;

  assertTrue_1(troo.getValue(tb));
  assertTrue_1(atroo.getValue(tba));
  assertTrue_1(tb == tba);

  assertTrue_1(tree.getValue(ti));
  assertTrue_1(atree.getValue(tia));
  assertTrue_1(ti == tia);

  assertTrue_1(pie.getValue(td));
  assertTrue_1(apie.getValue(tda));
  assertTrue_1(td == tda);

  assertTrue_1(fore.getValue(ts));
  assertTrue_1(afore.getValue(tsa));
  assertTrue_1(ts == tsa);

  assertTrue_1(fore.getValuePointer(tsp));
  assertTrue_1(afore.getValuePointer(tspa));
  assertTrue_1((*tsp) == (*tspa));

  return true;
}

static bool testAliasToArrayConstant()
{
  // Set up test data
  std::vector<bool>        vb(2);
  std::vector<int32_t>     vi(4);
  std::vector<double>      vd(4);
  std::vector<std::string> vs(4);

  vb[0] = false;
  vb[1] = true;

  vi[0] = 0;
  vi[1] = 1;
  vi[2] = 2;
  vi[3] = 3;

  vd[0] = 0;
  vd[1] = 1;
  vd[2] = 2;
  vd[3] = 3;

  vs[0] = std::string("zero");
  vs[1] = std::string("one");
  vs[2] = std::string("two");
  vs[3] = std::string("three");

  BooleanArrayConstant bc(vb);
  IntegerArrayConstant ic(vi);
  RealArrayConstant    dc(vd);
  StringArrayConstant  sc(vs);

  Alias abc(NodeId::noId(), "abc", bc.getId());
  Alias aic(NodeId::noId(), "aic", ic.getId());
  Alias adc(NodeId::noId(), "adc", dc.getId());
  Alias asc(NodeId::noId(), "asc", sc.getId());

  std::vector<bool> const        *pknown = NULL, *paknown = NULL;
  std::vector<bool> const        *pvb = NULL, *pavb = NULL;
  std::vector<int32_t> const     *pvi = NULL, *pavi = NULL;
  std::vector<double> const      *pvd = NULL, *pavd = NULL;
  std::vector<std::string> const *pvs = NULL, *pavs = NULL;

  // Check that alias is not assignable
  assertTrue_1(!abc.isAssignable());
  assertTrue_1(!aic.isAssignable());
  assertTrue_1(!adc.isAssignable());
  assertTrue_1(!asc.isAssignable());

  // Check that isConstant is forwarded
  assertTrue_1(abc.isConstant() == bc.isConstant());
  assertTrue_1(aic.isConstant() == ic.isConstant());
  assertTrue_1(adc.isConstant() == dc.isConstant());
  assertTrue_1(asc.isConstant() == sc.isConstant());

  // Check that valueType is forwarded
  assertTrue_1(abc.valueType() == bc.valueType());
  assertTrue_1(aic.valueType() == ic.valueType());
  assertTrue_1(adc.valueType() == dc.valueType());
  assertTrue_1(asc.valueType() == sc.valueType());

  // Check unknown when inactive
  assertTrue_1(!abc.isActive());
  assertTrue_1(!aic.isActive());
  assertTrue_1(!adc.isActive());
  assertTrue_1(!asc.isActive());

  assertTrue_1(!abc.isKnown());
  assertTrue_1(!aic.isKnown());
  assertTrue_1(!adc.isKnown());
  assertTrue_1(!asc.isKnown());

  // Activate
  abc.activate();
  aic.activate();
  adc.activate();
  asc.activate();

  // Check that all are active and known
  assertTrue_1(abc.isActive());
  assertTrue_1(aic.isActive());
  assertTrue_1(adc.isActive());
  assertTrue_1(asc.isActive());

  assertTrue_1(abc.isKnown());
  assertTrue_1(aic.isKnown());
  assertTrue_1(adc.isKnown());
  assertTrue_1(asc.isKnown());
  
  // Check that getArrayContents is forwarded
  assertTrue(bc.getArrayContents(pvb, pknown));
  assertTrue(abc.getArrayContents(pavb, paknown));
  assertTrue(*pknown == *paknown);
  assertTrue(*pvb == *pavb);

  assertTrue(ic.getArrayContents(pvi, pknown));
  assertTrue(aic.getArrayContents(pavi, paknown));
  assertTrue(*pknown == *paknown);
  assertTrue(*pvi == *pavi);

  assertTrue(dc.getArrayContents(pvd, pknown));
  assertTrue(adc.getArrayContents(pavd, paknown));
  assertTrue(*pknown == *paknown);
  assertTrue(*pvd == *pavd);

  assertTrue(sc.getArrayContents(pvs, pknown));
  assertTrue(asc.getArrayContents(pavs, paknown));
  assertTrue(*pknown == *paknown);
  assertTrue(*pvs == *pavs);

  return true;
}

static bool testAliasToScalarVariable()
{
  BooleanVariable troo(true);
  IntegerVariable tree(3);
  RealVariable pie(3.14);
  StringVariable fore("four");

  Alias atroo(NodeId::noId(), "atroo", troo.getId());
  Alias atree(NodeId::noId(), "atree", tree.getId());
  Alias apie (NodeId::noId(), "apie",  pie.getId());
  Alias afore(NodeId::noId(), "afore", fore.getId());

  InOutAlias watroo(NodeId::noId(), "watroo", troo.getId());
  InOutAlias watree(NodeId::noId(), "watree", tree.getId());
  InOutAlias wapie (NodeId::noId(), "wapie",  pie.getId());
  InOutAlias wafore(NodeId::noId(), "wafore", fore.getId());

  // Check that Alias is not assignable
  assertTrue_1(!atroo.isAssignable());
  assertTrue_1(!atree.isAssignable());
  assertTrue_1(!apie.isAssignable());
  assertTrue_1(!afore.isAssignable());

  // Check that InOutAlias is not assignable
  assertTrue_1(watroo.isAssignable());
  assertTrue_1(watree.isAssignable());
  assertTrue_1(wapie.isAssignable());
  assertTrue_1(wafore.isAssignable());

  // Check that isConstant is forwarded
  assertTrue_1(atroo.isConstant() == troo.isConstant());
  assertTrue_1(atree.isConstant() == tree.isConstant());
  assertTrue_1(apie.isConstant()  == pie.isConstant());
  assertTrue_1(afore.isConstant() == fore.isConstant());

  assertTrue_1(watroo.isConstant() == troo.isConstant());
  assertTrue_1(watree.isConstant() == tree.isConstant());
  assertTrue_1(wapie.isConstant()  == pie.isConstant());
  assertTrue_1(wafore.isConstant() == fore.isConstant());

  // Check that valueType is forwarded
  assertTrue_1(atroo.valueType() == troo.valueType());
  assertTrue_1(atree.valueType() == tree.valueType());
  assertTrue_1(apie.valueType()  == pie.valueType());
  assertTrue_1(afore.valueType() == fore.valueType());

  assertTrue_1(watroo.valueType() == troo.valueType());
  assertTrue_1(watree.valueType() == tree.valueType());
  assertTrue_1(wapie.valueType()  == pie.valueType());
  assertTrue_1(wafore.valueType() == fore.valueType());

  // Check unknown when inactive
  assertTrue_1(!atroo.isActive());
  assertTrue_1(!atree.isActive());
  assertTrue_1(!apie.isActive());
  assertTrue_1(!afore.isActive());

  assertTrue_1(!watroo.isActive());
  assertTrue_1(!watree.isActive());
  assertTrue_1(!wapie.isActive());
  assertTrue_1(!wafore.isActive());

  assertTrue_1(!atroo.isKnown());
  assertTrue_1(!atree.isKnown());
  assertTrue_1(!apie.isKnown());
  assertTrue_1(!afore.isKnown());

  assertTrue_1(!watroo.isKnown());
  assertTrue_1(!watree.isKnown());
  assertTrue_1(!wapie.isKnown());
  assertTrue_1(!wafore.isKnown());

  // Activate
  troo.activate();
  tree.activate();
  pie.activate();
  fore.activate();
  atroo.activate();
  atree.activate();
  apie.activate();
  afore.activate();
  watroo.activate();
  watree.activate();
  wapie.activate();
  wafore.activate();

  // Check that all are active and known
  assertTrue_1(atroo.isActive());
  assertTrue_1(atree.isActive());
  assertTrue_1(apie.isActive());
  assertTrue_1(afore.isActive());

  assertTrue_1(watroo.isActive());
  assertTrue_1(watree.isActive());
  assertTrue_1(wapie.isActive());
  assertTrue_1(wafore.isActive());

  assertTrue_1(atroo.isKnown());
  assertTrue_1(atree.isKnown());
  assertTrue_1(apie.isKnown());
  assertTrue_1(afore.isKnown());

  assertTrue_1(watroo.isKnown());
  assertTrue_1(watree.isKnown());
  assertTrue_1(wapie.isKnown());
  assertTrue_1(wafore.isKnown());
  
  // Check that getValue is forwarded
  bool tb, tba;
  int32_t ti, tia;
  double td, tda;
  std::string ts, tsa;
  const std::string *tsp, *tspa;

  assertTrue_1(troo.getValue(tb));
  assertTrue_1(atroo.getValue(tba));
  assertTrue_1(tb == tba);

  assertTrue_1(tree.getValue(ti));
  assertTrue_1(atree.getValue(tia));
  assertTrue_1(ti == tia);

  assertTrue_1(pie.getValue(td));
  assertTrue_1(apie.getValue(tda));
  assertTrue_1(td == tda);

  assertTrue_1(fore.getValue(ts));
  assertTrue_1(afore.getValue(tsa));
  assertTrue_1(ts == tsa);

  assertTrue_1(fore.getValuePointer(tsp));
  assertTrue_1(afore.getValuePointer(tspa));
  assertTrue_1((*tsp) == (*tspa));

  assertTrue_1(watroo.getValue(tba));
  assertTrue_1(tb == tba);

  assertTrue_1(watree.getValue(tia));
  assertTrue_1(ti == tia);

  assertTrue_1(wapie.getValue(tda));
  assertTrue_1(td == tda);

  assertTrue_1(wafore.getValue(tsa));
  assertTrue_1(ts == tsa);

  assertTrue_1(wafore.getValuePointer(tspa));
  assertTrue_1((*tsp) == (*tspa));

  // Write tests
  watroo.setValue(false);
  assertTrue_1(troo.getValue(tb));
  assertTrue_1(!tb);
  assertTrue_1(watroo.getValue(tba));
  assertTrue_1(tb == tba);
  assertTrue_1(atroo.getValue(tba));
  assertTrue_1(tb == tba);

  watree.setValue(42);
  assertTrue_1(tree.getValue(ti));
  assertTrue_1(ti == 42);
  assertTrue_1(watree.getValue(tia));
  assertTrue_1(ti == tia);
  assertTrue_1(atree.getValue(tia));
  assertTrue_1(ti == tia);

  wapie.setValue(2.5);
  assertTrue_1(pie.getValue(td));
  assertTrue_1(td == 2.5);
  assertTrue_1(wapie.getValue(tda));
  assertTrue_1(td == tda);
  assertTrue_1(apie.getValue(tda));
  assertTrue_1(td == tda);

  wafore.setValue(std::string("yow!"));
  assertTrue_1(fore.getValue(ts));
  assertTrue_1(ts == std::string("yow!"));
  assertTrue_1(wafore.getValue(tsa));
  assertTrue_1(ts == tsa);
  assertTrue_1(afore.getValue(tsa));
  assertTrue_1(ts == tsa);

  watroo.setUnknown();
  assertTrue_1(!troo.getValue(tb));
  assertTrue_1(!watroo.getValue(tba));
  assertTrue_1(!atroo.getValue(tba));

  watree.setUnknown();
  assertTrue_1(!tree.getValue(ti));
  assertTrue_1(!watree.getValue(tia));
  assertTrue_1(!atree.getValue(tia));

  wapie.setUnknown();
  assertTrue_1(!pie.getValue(td));
  assertTrue_1(!wapie.getValue(tda));
  assertTrue_1(!apie.getValue(tda));

  wafore.setUnknown();
  assertTrue_1(!fore.getValue(ts));
  assertTrue_1(!wafore.getValue(tsa));
  assertTrue_1(!afore.getValue(tsa));

  // Test that reset is NOT propagated
  watroo.deactivate();
  watree.deactivate();
  wapie.deactivate();
  wafore.deactivate();

  // If these propagate, we'll get an error
  watroo.reset();
  watree.reset();
  wapie.reset();
  wafore.reset();

  watroo.activate();
  watree.activate();
  wapie.activate();
  wafore.activate();

  // Should still be unknown
  assertTrue_1(!troo.getValue(tb));
  assertTrue_1(!watroo.getValue(tba));
  assertTrue_1(!atroo.getValue(tba));

  assertTrue_1(!tree.getValue(ti));
  assertTrue_1(!watree.getValue(tia));
  assertTrue_1(!atree.getValue(tia));

  assertTrue_1(!pie.getValue(td));
  assertTrue_1(!wapie.getValue(tda));
  assertTrue_1(!apie.getValue(tda));
  
  assertTrue_1(!fore.getValue(ts));
  assertTrue_1(!wafore.getValue(tsa));
  assertTrue_1(!afore.getValue(tsa));

  return true;
}

static bool testAliasToArrayVariable()
{
  return true;
}

static bool testAliasToArrayReference()
{
  return true;
}

static bool testAliasToFunction()
{
  return true;
}

static bool testAliasPropagation()
{
  return true;
}

bool aliasTest()
{
  runTest(testAliasToScalarConstant);
  runTest(testAliasToArrayConstant);
  runTest(testAliasToScalarVariable);
  runTest(testAliasToArrayVariable);
  runTest(testAliasToArrayReference);
  runTest(testAliasToFunction);
  runTest(testAliasPropagation);

  return true;
}
