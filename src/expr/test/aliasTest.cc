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
#include "Constant.hh"
#include "ArrayReference.hh"
#include "ArrayVariable.hh"
#include "Function.hh"
#include "TestSupport.hh"
#include "TrivialListener.hh"

using namespace PLEXIL;

static bool testAliasToScalarConstant()
{
  BooleanConstant troo(true);
  IntegerConstant tree(3);
  RealConstant pie(3.14);
  StringConstant fore("four");

  Alias atroo(NULL, "atroo", &troo);
  Alias atree(NULL, "atree", &tree);
  Alias apie (NULL, "apie",  &pie);
  Alias afore(NULL, "afore", &fore);

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

  Alias abc(NULL, "abc", &bc);
  Alias aic(NULL, "aic", &ic);
  Alias adc(NULL, "adc", &dc);
  Alias asc(NULL, "asc", &sc);

  BooleanArray const        *pab = NULL, *paab = NULL;
  IntegerArray const     *pai = NULL, *paai = NULL;
  RealArray const      *pad = NULL, *paad = NULL;
  StringArray const *pas = NULL, *paas = NULL;

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
  
  // Check that getValuePointer is forwarded
  assertTrue_1(bc.getValuePointer(pab));
  assertTrue_1(abc.getValuePointer(paab));
  assertTrue_1(*pab == *paab);

  assertTrue_1(ic.getValuePointer(pai));
  assertTrue_1(aic.getValuePointer(paai));
  assertTrue_1(*pai == *paai);

  assertTrue_1(dc.getValuePointer(pad));
  assertTrue_1(adc.getValuePointer(paad));
  assertTrue_1(*pad == *paad);

  assertTrue_1(sc.getValuePointer(pas));
  assertTrue_1(asc.getValuePointer(paas));
  assertTrue_1(*pas == *paas);

  return true;
}

static bool testAliasToScalarVariable()
{
  BooleanVariable troo(true);
  IntegerVariable tree(3);
  RealVariable pie(3.14);
  StringVariable fore("four");

  Alias atroo(NULL, "atroo", &troo);
  Alias atree(NULL, "atree", &tree);
  Alias apie (NULL, "apie",  &pie);
  Alias afore(NULL, "afore", &fore);

  InOutAlias watroo(NULL, "watroo", &troo);
  InOutAlias watree(NULL, "watree", &tree);
  InOutAlias wapie (NULL, "wapie",  &pie);
  InOutAlias wafore(NULL, "wafore", &fore);

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

  watree.setValue((int32_t) 42);
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

  BooleanArrayVariable bc(vb);
  IntegerArrayVariable ic(vi);
  RealArrayVariable    dc(vd);
  StringArrayVariable  sc(vs);

  Alias abc(NULL, "abc", &bc);
  Alias aic(NULL, "aic", &ic);
  Alias adc(NULL, "adc", &dc);
  Alias asc(NULL, "asc", &sc);

  InOutAlias wabc(NULL, "wabc", &bc);
  InOutAlias waic(NULL, "waic", &ic);
  InOutAlias wadc(NULL, "wadc", &dc);
  InOutAlias wasc(NULL, "wasc", &sc);

  BooleanArray const        *pab = NULL, *paab = NULL;
  IntegerArray const     *pai = NULL, *paai = NULL;
  RealArray const      *pad = NULL, *paad = NULL;
  StringArray const *pas = NULL, *paas = NULL;

  // Check that Alias is not assignable
  assertTrue_1(!abc.isAssignable());
  assertTrue_1(!aic.isAssignable());
  assertTrue_1(!adc.isAssignable());
  assertTrue_1(!asc.isAssignable());

  // Check that InOutAlias is
  assertTrue_1(wabc.isAssignable());
  assertTrue_1(waic.isAssignable());
  assertTrue_1(wadc.isAssignable());
  assertTrue_1(wasc.isAssignable());

  // Check that isConstant is forwarded
  assertTrue_1(abc.isConstant() == bc.isConstant());
  assertTrue_1(aic.isConstant() == ic.isConstant());
  assertTrue_1(adc.isConstant() == dc.isConstant());
  assertTrue_1(asc.isConstant() == sc.isConstant());
  assertTrue_1(wabc.isConstant() == bc.isConstant());
  assertTrue_1(waic.isConstant() == ic.isConstant());
  assertTrue_1(wadc.isConstant() == dc.isConstant());
  assertTrue_1(wasc.isConstant() == sc.isConstant());

  // Check that valueType is forwarded
  assertTrue_1(abc.valueType() == bc.valueType());
  assertTrue_1(aic.valueType() == ic.valueType());
  assertTrue_1(adc.valueType() == dc.valueType());
  assertTrue_1(asc.valueType() == sc.valueType());
  assertTrue_1(wabc.valueType() == bc.valueType());
  assertTrue_1(waic.valueType() == ic.valueType());
  assertTrue_1(wadc.valueType() == dc.valueType());
  assertTrue_1(wasc.valueType() == sc.valueType());

  // Check unknown when inactive
  assertTrue_1(!abc.isActive());
  assertTrue_1(!aic.isActive());
  assertTrue_1(!adc.isActive());
  assertTrue_1(!asc.isActive());
  assertTrue_1(!wabc.isActive());
  assertTrue_1(!waic.isActive());
  assertTrue_1(!wadc.isActive());
  assertTrue_1(!wasc.isActive());

  assertTrue_1(!abc.isKnown());
  assertTrue_1(!aic.isKnown());
  assertTrue_1(!adc.isKnown());
  assertTrue_1(!asc.isKnown());
  assertTrue_1(!wabc.isKnown());
  assertTrue_1(!waic.isKnown());
  assertTrue_1(!wadc.isKnown());
  assertTrue_1(!wasc.isKnown());

  // Activate
  bc.activate();
  ic.activate();
  dc.activate();
  sc.activate();
  abc.activate();
  aic.activate();
  adc.activate();
  asc.activate();
  wabc.activate();
  waic.activate();
  wadc.activate();
  wasc.activate();

  // Check that all are active and known
  assertTrue_1(abc.isActive());
  assertTrue_1(aic.isActive());
  assertTrue_1(adc.isActive());
  assertTrue_1(asc.isActive());
  assertTrue_1(wabc.isActive());
  assertTrue_1(waic.isActive());
  assertTrue_1(wadc.isActive());
  assertTrue_1(wasc.isActive());

  assertTrue_1(abc.isKnown());
  assertTrue_1(aic.isKnown());
  assertTrue_1(adc.isKnown());
  assertTrue_1(asc.isKnown());
  assertTrue_1(wabc.isKnown());
  assertTrue_1(waic.isKnown());
  assertTrue_1(wadc.isKnown());
  assertTrue_1(wasc.isKnown());
  
  // Check that getValuePointer is forwarded
  assertTrue_1(bc.getValuePointer(pab));
  assertTrue_1(abc.getValuePointer(paab));
  assertTrue_1(*pab == *paab);
  assertTrue_1(wabc.getValuePointer(paab));
  assertTrue_1(*pab == *paab);

  assertTrue_1(ic.getValuePointer(pai));
  assertTrue_1(aic.getValuePointer(paai));
  assertTrue_1(*pai == *paai);
  assertTrue_1(waic.getValuePointer(paai));
  assertTrue_1(*pai == *paai);

  assertTrue_1(dc.getValuePointer(pad));
  assertTrue_1(adc.getValuePointer(paad));
  assertTrue_1(*pad == *paad);
  assertTrue_1(wadc.getValuePointer(paad));
  assertTrue_1(*pad == *paad);

  assertTrue_1(sc.getValuePointer(pas));
  assertTrue_1(asc.getValuePointer(paas));
  assertTrue_1(*pas == *paas);
  assertTrue_1(wasc.getValuePointer(paas));
  assertTrue_1(*pas == *paas);

  // Test array reference through alias
  IntegerVariable ix;

  ArrayReference bar(&bc, &ix);
  ArrayReference abar(&abc, &ix);
  MutableArrayReference wabar(&wabc, &ix);

  ArrayReference iar(&ic, &ix);
  ArrayReference aiar(&aic, &ix);
  MutableArrayReference waiar(&waic, &ix);

  ArrayReference dar(&dc, &ix);
  ArrayReference adar(&adc, &ix);
  MutableArrayReference wadar(&wadc, &ix);

  ArrayReference sar(&sc, &ix);
  ArrayReference asar(&asc, &ix);
  MutableArrayReference wasar(&wasc, &ix);

  bool bt1, bt2;
  int32_t it1, it2;
  double dt1, dt2;
  std::string st1, st2;
  std::string const *stp1, *stp2;

  ix.activate();
  bar.activate();
  abar.activate();
  wabar.activate();
  iar.activate();
  aiar.activate();
  waiar.activate();
  dar.activate();
  adar.activate();
  wadar.activate();
  sar.activate();
  asar.activate();
  wasar.activate();

  ix.setValue((int32_t) 0);

  assertTrue_1(bar.getValue(bt1));
  assertTrue_1(!bt1);
  assertTrue_1(abar.getValue(bt2));
  assertTrue_1(bt1 == bt2);
  assertTrue_1(wabar.getValue(bt2));
  assertTrue_1(bt1 == bt2);

  assertTrue_1(iar.getValue(it1));
  assertTrue_1(it1 == 0);
  assertTrue_1(aiar.getValue(it2));
  assertTrue_1(it1 == it2);
  assertTrue_1(waiar.getValue(it2));
  assertTrue_1(it1 == it2);

  assertTrue_1(dar.getValue(dt1));
  assertTrue_1(dt1 == 0);
  assertTrue_1(adar.getValue(dt2));
  assertTrue_1(dt1 == dt2);
  assertTrue_1(wadar.getValue(dt2));
  assertTrue_1(dt1 == dt2);

  assertTrue_1(sar.getValue(st1));
  assertTrue_1(st1 == std::string("zero"));
  assertTrue_1(asar.getValue(st2));
  assertTrue_1(st1 == st2);
  assertTrue_1(wasar.getValue(st2));
  assertTrue_1(st1 == st2);

  assertTrue_1(sar.getValuePointer(stp1));
  assertTrue_1(*stp1 == std::string("zero"));
  assertTrue_1(asar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);
  assertTrue_1(wasar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);

  // Test setting whole arrays
  std::vector<bool> vb2(3);
  std::vector<int32_t> vi2(3);
  std::vector<double> vd2(3);
  std::vector<std::string> vs2(3);

  vb2[0] = true;
  vb2[1] = false;
  vb2[2] = true;

  vi2[0] = 3;
  vi2[1] = 2;
  vi2[2] = 1;

  vd2[0] = 5;
  vd2[1] = 4;
  vd2[2] = 3;

  vs2[0] = std::string("seven");
  vs2[1] = std::string("six");
  vs2[2] = std::string("eight");

  std::vector<bool> const *pvb;
  std::vector<int32_t> const *pvi;
  std::vector<double> const *pvd;
  std::vector<std::string> const *pvs;

  wabc.setValue(vb2);
  assertTrue_1(bc.getValuePointer(pab));
  assertTrue_1(pab != NULL);
  pab->getContentsVector(pvb);
  assertTrue_1(pvb != NULL);
  assertTrue_1(*pvb == vb2);
  assertTrue_1(abc.getValuePointer(paab));
  assertTrue_1(*pab == *paab);

  waic.setValue(vi2);
  assertTrue_1(ic.getValuePointer(pai));
  assertTrue_1(pai != NULL);
  pai->getContentsVector(pvi);
  assertTrue_1(pvi != NULL);
  assertTrue_1(*pvi == vi2);
  assertTrue_1(aic.getValuePointer(paai));
  assertTrue_1(*pai == *paai);

  wadc.setValue(vd2);
  assertTrue_1(dc.getValuePointer(pad));
  assertTrue_1(pad != NULL);
  pad->getContentsVector(pvd);
  assertTrue_1(pvd != NULL);
  assertTrue_1(*pvd == vd2);
  assertTrue_1(adc.getValuePointer(paad));
  assertTrue_1(*pad == *paad);

  wasc.setValue(vs2);
  assertTrue_1(sc.getValuePointer(pas));
  assertTrue_1(pas != NULL);
  pas->getContentsVector(pvs);
  assertTrue_1(pvs != NULL);
  assertTrue_1(*pvs == vs2);
  assertTrue_1(asc.getValuePointer(paas));
  assertTrue_1(*pas == *paas);

  // Test writable array reference through alias
  wabar.setValue(false);
  assertTrue_1(bar.getValue(bt1));
  assertTrue_1(!bt1);
  assertTrue_1(abar.getValue(bt2));
  assertTrue_1(bt1 == bt2);
  assertTrue_1(wabar.getValue(bt2));
  assertTrue_1(bt1 == bt2);

  waiar.setValue((int32_t) 42);
  assertTrue_1(iar.getValue(it1));
  assertTrue_1(it1 == 42);
  assertTrue_1(aiar.getValue(it2));
  assertTrue_1(it1 == it2);
  assertTrue_1(waiar.getValue(it2));
  assertTrue_1(it1 == it2);

  wadar.setValue(22.5);
  assertTrue_1(dar.getValue(dt1));
  assertTrue_1(dt1 == 22.5);
  assertTrue_1(adar.getValue(dt2));
  assertTrue_1(dt1 == dt2);
  assertTrue_1(wadar.getValue(dt2));
  assertTrue_1(dt1 == dt2);

  wasar.setValue(std::string("bogus"));
  assertTrue_1(sar.getValue(st1));
  assertTrue_1(st1 == std::string("bogus"));
  assertTrue_1(asar.getValue(st2));
  assertTrue_1(st1 == st2);
  assertTrue_1(wasar.getValue(st2));
  assertTrue_1(st1 == st2);

  assertTrue_1(sar.getValuePointer(stp1));
  assertTrue_1(*stp1 == std::string("bogus"));
  assertTrue_1(asar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);
  assertTrue_1(wasar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);

  return true;
}

static bool testAliasToArrayReference()
{
  // Set up test data
  std::vector<bool>        vb(2);
  std::vector<int32_t>     vi(4);
  std::vector<double>      vd(4);
  std::vector<std::string> vs(4);

  vb[0] = false;
  vb[1] = true;

  vi[0] = 3;
  vi[1] = 2;
  vi[2] = 1;
  vi[3] = 0;

  vd[0] = 7;
  vd[1] = 6;
  vd[2] = 5;
  vd[3] = 4;

  vs[0] = std::string("zero");
  vs[1] = std::string("one");
  vs[2] = std::string("two");
  vs[3] = std::string("three");

  BooleanArrayVariable bc(vb);
  IntegerArrayVariable ic(vi);
  RealArrayVariable    dc(vd);
  StringArrayVariable  sc(vs);

  IntegerVariable ix;

  ArrayReference bar(&bc, &ix);
  MutableArrayReference wbar(&bc, &ix);
  Alias abar(NULL, "abar", &bar);
  Alias awbar(NULL, "awbar", &wbar);
  InOutAlias wawbar(NULL, "wawbar", &wbar);

  ArrayReference iar(&ic, &ix);
  MutableArrayReference wiar(&ic, &ix);
  Alias aiar(NULL, "aiar", &iar);
  Alias awiar(NULL, "awiar", &wiar);
  InOutAlias wawiar(NULL, "wawiar", &wiar);

  ArrayReference dar(&dc, &ix);
  MutableArrayReference wdar(&dc, &ix);
  Alias adar(NULL, "adar", &dar);
  Alias awdar(NULL, "awdar", &wdar);
  InOutAlias wawdar(NULL, "wawdar", &wdar);

  ArrayReference sar(&sc, &ix);
  MutableArrayReference wsar(&sc, &ix);
  Alias asar(NULL, "asar", &sar);
  Alias awsar(NULL, "awsar", &wsar);
  InOutAlias wawsar(NULL, "wawsar", &wsar);

  bool bt1, bt2;
  int32_t it1, it2;
  double dt1, dt2;
  std::string st1, st2;
  std::string const *stp1, *stp2;

  // Activate the real objects
  ix.activate();
  bc.activate();
  ic.activate();
  dc.activate();
  sc.activate();
  bar.activate();
  iar.activate();
  dar.activate();
  sar.activate();
  wbar.activate();
  wiar.activate();
  wdar.activate();
  wsar.activate();

  // Set the index
  ix.setValue((int32_t) 1);

  // Check that aliases are inactive and unknown
  assertTrue_1(!abar.isActive());
  assertTrue_1(!awbar.isActive());
  assertTrue_1(!wawbar.isActive());
  assertTrue_1(!aiar.isActive());
  assertTrue_1(!awiar.isActive());
  assertTrue_1(!wawiar.isActive());
  assertTrue_1(!adar.isActive());
  assertTrue_1(!awdar.isActive());
  assertTrue_1(!wawdar.isActive());
  assertTrue_1(!asar.isActive());
  assertTrue_1(!awsar.isActive());
  assertTrue_1(!wawsar.isActive());

  assertTrue_1(!abar.isKnown());
  assertTrue_1(!awbar.isKnown());
  assertTrue_1(!wawbar.isKnown());
  assertTrue_1(!aiar.isKnown());
  assertTrue_1(!awiar.isKnown());
  assertTrue_1(!wawiar.isKnown());
  assertTrue_1(!adar.isKnown());
  assertTrue_1(!awdar.isKnown());
  assertTrue_1(!wawdar.isKnown());
  assertTrue_1(!asar.isKnown());
  assertTrue_1(!awsar.isKnown());
  assertTrue_1(!wawsar.isKnown());

  // Activate and check again
  abar.activate();
  awbar.activate();
  wawbar.activate();
  aiar.activate();
  awiar.activate();
  wawiar.activate();
  adar.activate();
  awdar.activate();
  wawdar.activate();
  asar.activate();
  awsar.activate();
  wawsar.activate();

  assertTrue_1(abar.isActive());
  assertTrue_1(awbar.isActive());
  assertTrue_1(wawbar.isActive());
  assertTrue_1(aiar.isActive());
  assertTrue_1(awiar.isActive());
  assertTrue_1(wawiar.isActive());
  assertTrue_1(adar.isActive());
  assertTrue_1(awdar.isActive());
  assertTrue_1(wawdar.isActive());
  assertTrue_1(asar.isActive());
  assertTrue_1(awsar.isActive());
  assertTrue_1(wawsar.isActive());

  assertTrue_1(abar.isKnown());
  assertTrue_1(awbar.isKnown());
  assertTrue_1(wawbar.isKnown());
  assertTrue_1(aiar.isKnown());
  assertTrue_1(awiar.isKnown());
  assertTrue_1(wawiar.isKnown());
  assertTrue_1(adar.isKnown());
  assertTrue_1(awdar.isKnown());
  assertTrue_1(wawdar.isKnown());
  assertTrue_1(asar.isKnown());
  assertTrue_1(awsar.isKnown());
  assertTrue_1(wawsar.isKnown());

  // Check values
  assertTrue_1(bar.getValue(bt1));
  assertTrue_1(bt1);
  assertTrue_1(abar.getValue(bt2));
  assertTrue_1(bt1 == bt2);
  assertTrue_1(awbar.getValue(bt2));
  assertTrue_1(bt1 == bt2);
  assertTrue_1(wawbar.getValue(bt2));
  assertTrue_1(bt1 == bt2);

  assertTrue_1(iar.getValue(it1));
  assertTrue_1(it1 == 2);
  assertTrue_1(aiar.getValue(it2));
  assertTrue_1(it1 == it2);
  assertTrue_1(awiar.getValue(it2));
  assertTrue_1(it1 == it2);
  assertTrue_1(wawiar.getValue(it2));
  assertTrue_1(it1 == it2);

  assertTrue_1(dar.getValue(dt1));
  assertTrue_1(dt1 == 6);
  assertTrue_1(adar.getValue(dt2));
  assertTrue_1(dt1 == dt2);
  assertTrue_1(awdar.getValue(dt2));
  assertTrue_1(dt1 == dt2);
  assertTrue_1(wawdar.getValue(dt2));
  assertTrue_1(dt1 == dt2);

  assertTrue_1(sar.getValue(st1));
  assertTrue_1(st1 == std::string("one"));
  assertTrue_1(asar.getValue(st2));
  assertTrue_1(st1 == st2);
  assertTrue_1(awsar.getValue(st2));
  assertTrue_1(st1 == st2);
  assertTrue_1(wawsar.getValue(st2));
  assertTrue_1(st1 == st2);

  assertTrue_1(sar.getValuePointer(stp1));
  assertTrue_1(*stp1 == std::string("one"));
  assertTrue_1(asar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);
  assertTrue_1(awsar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);
  assertTrue_1(wawsar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);

  // Change index, ensure value tracks
  ix.setValue((int32_t) 0);

  assertTrue_1(bar.getValue(bt1));
  assertTrue_1(!bt1);
  assertTrue_1(abar.getValue(bt2));
  assertTrue_1(bt1 == bt2);
  assertTrue_1(awbar.getValue(bt2));
  assertTrue_1(bt1 == bt2);
  assertTrue_1(wawbar.getValue(bt2));
  assertTrue_1(bt1 == bt2);

  assertTrue_1(iar.getValue(it1));
  assertTrue_1(it1 == 3);
  assertTrue_1(aiar.getValue(it2));
  assertTrue_1(it1 == it2);
  assertTrue_1(awiar.getValue(it2));
  assertTrue_1(it1 == it2);
  assertTrue_1(wawiar.getValue(it2));
  assertTrue_1(it1 == it2);

  assertTrue_1(dar.getValue(dt1));
  assertTrue_1(dt1 == 7);
  assertTrue_1(adar.getValue(dt2));
  assertTrue_1(dt1 == dt2);
  assertTrue_1(awdar.getValue(dt2));
  assertTrue_1(dt1 == dt2);
  assertTrue_1(wawdar.getValue(dt2));
  assertTrue_1(dt1 == dt2);

  assertTrue_1(sar.getValue(st1));
  assertTrue_1(st1 == std::string("zero"));
  assertTrue_1(asar.getValue(st2));
  assertTrue_1(st1 == st2);
  assertTrue_1(awsar.getValue(st2));
  assertTrue_1(st1 == st2);
  assertTrue_1(wawsar.getValue(st2));
  assertTrue_1(st1 == st2);

  assertTrue_1(sar.getValuePointer(stp1));
  assertTrue_1(*stp1 == std::string("zero"));
  assertTrue_1(asar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);
  assertTrue_1(awsar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);
  assertTrue_1(wawsar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);

  // Set element through InOutAlias
  wawbar.setValue(true);
  assertTrue_1(bar.getValue(bt1));
  assertTrue_1(bt1);
  assertTrue_1(abar.getValue(bt2));
  assertTrue_1(bt1 == bt2);
  assertTrue_1(awbar.getValue(bt2));
  assertTrue_1(bt1 == bt2);
  assertTrue_1(wawbar.getValue(bt2));
  assertTrue_1(bt1 == bt2);

  wawiar.setValue((int32_t) 69);
  assertTrue_1(iar.getValue(it1));
  assertTrue_1(it1 == 69);
  assertTrue_1(aiar.getValue(it2));
  assertTrue_1(it1 == it2);
  assertTrue_1(awiar.getValue(it2));
  assertTrue_1(it1 == it2);
  assertTrue_1(wawiar.getValue(it2));
  assertTrue_1(it1 == it2);

  wawdar.setValue(-3.5);
  assertTrue_1(dar.getValue(dt1));
  assertTrue_1(dt1 == -3.5);
  assertTrue_1(adar.getValue(dt2));
  assertTrue_1(dt1 == dt2);
  assertTrue_1(awdar.getValue(dt2));
  assertTrue_1(dt1 == dt2);
  assertTrue_1(wawdar.getValue(dt2));
  assertTrue_1(dt1 == dt2);

  wawsar.setValue(std::string("minus one"));
  assertTrue_1(sar.getValue(st1));
  assertTrue_1(st1 == std::string("minus one"));
  assertTrue_1(asar.getValue(st2));
  assertTrue_1(st1 == st2);
  assertTrue_1(awsar.getValue(st2));
  assertTrue_1(st1 == st2);
  assertTrue_1(wawsar.getValue(st2));
  assertTrue_1(st1 == st2);

  assertTrue_1(sar.getValuePointer(stp1));
  assertTrue_1(*stp1 == std::string("minus one"));
  assertTrue_1(asar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);
  assertTrue_1(awsar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);
  assertTrue_1(wawsar.getValuePointer(stp2));
  assertTrue_1(*stp1 == *stp2);

  return true;
}

static bool testAliasToFunction()
{
  // TODO - add some string, array functions
  return true;
}

static bool testAliasPropagation()
{
  std::vector<int32_t> vi(4);
  vi[0] = 2;
  vi[1] = 4;
  vi[2] = 6;
  vi[3] = 8;
  IntegerArrayVariable ary(vi);
  IntegerVariable tree(3);
  BooleanConstant dummy; // used as prop source

  Alias aary(NULL, "aary", &ary);
  Alias atree(NULL, "atree", &tree);
  InOutAlias waary(NULL, "waary", &ary);
  InOutAlias watree(NULL, "watree", &tree);

  MutableArrayReference ref(&waary, &atree);
  Alias aref(NULL, "aref", &ref);
  InOutAlias waref(NULL, "waref", &ref);

  bool aryChanged, aaryChanged, waaryChanged;
  bool treeChanged, atreeChanged, watreeChanged;
  bool refChanged, arefChanged, warefChanged;

  TrivialListener aryListener(aryChanged);
  TrivialListener aaryListener(aaryChanged);
  TrivialListener waaryListener(waaryChanged);
  TrivialListener treeListener(treeChanged);
  TrivialListener atreeListener(atreeChanged);
  TrivialListener watreeListener(watreeChanged);
  TrivialListener refListener(refChanged);
  TrivialListener arefListener(arefChanged);
  TrivialListener warefListener(warefChanged);

  ary.addListener(&aryListener);
  aary.addListener(&aaryListener);
  waary.addListener(&waaryListener);
  tree.addListener(&treeListener);
  atree.addListener(&atreeListener);
  watree.addListener(&watreeListener);
  ref.addListener(&refListener);
  aref.addListener(&arefListener);
  waref.addListener(&warefListener);

  aryChanged = aaryChanged = waaryChanged = false;
  treeChanged = atreeChanged = watreeChanged = false;
  refChanged = arefChanged = warefChanged = false;

  // Test that notifying while inactive does nothing
  atree.notifyChanged(&tree);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!watreeChanged);

  watree.notifyChanged(&tree);
  assertTrue_1(!watreeChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);

  aary.notifyChanged(&ary);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!waaryChanged);

  waary.notifyChanged(&ary);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);

  aref.notifyChanged(&ref);
  assertTrue_1(!arefChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!warefChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!watreeChanged);

  waref.notifyChanged(&ref);
  assertTrue_1(!warefChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!arefChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!watreeChanged);

  // Activate all
  waref.activate();
  aref.activate();
  ref.activate();
  waary.activate();
  aary.activate();
  ary.activate();
  watree.activate();
  atree.activate();
  tree.activate();

  // Test that initial activation causes propagation
  assertTrue_1(aryChanged);
  assertTrue_1(treeChanged);
  assertTrue_1(refChanged);
  assertTrue_1(aaryChanged);
  assertTrue_1(atreeChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(waaryChanged);
  assertTrue_1(watreeChanged);
  assertTrue_1(warefChanged);

  // Test that notifying alias doesn't notify origin
  aryChanged = aaryChanged = waaryChanged = false;
  treeChanged = atreeChanged = watreeChanged = false;
  refChanged = arefChanged = warefChanged = false;

  atree.notifyChanged(&tree);
  assertTrue_1(atreeChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!watreeChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(refChanged);   // these depend on atree
  assertTrue_1(arefChanged);  //
  assertTrue_1(warefChanged); //
  atreeChanged = refChanged = arefChanged = warefChanged = false;

  watree.notifyChanged(&tree);
  assertTrue_1(watreeChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!arefChanged);
  assertTrue_1(!warefChanged);
  watreeChanged = false;

  aary.notifyChanged(&ary);
  assertTrue_1(aaryChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!watreeChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!arefChanged);
  assertTrue_1(!warefChanged);
  aaryChanged = false;

  waary.notifyChanged(&ary);
  assertTrue_1(waaryChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!watreeChanged);
  assertTrue_1(refChanged);   // these depend on waary
  assertTrue_1(arefChanged);  //
  assertTrue_1(warefChanged); //
  waaryChanged = refChanged = arefChanged = warefChanged = false;

  aref.notifyChanged(&ref);
  assertTrue_1(arefChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!warefChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!watreeChanged);
  arefChanged = false;

  waref.notifyChanged(&ref);
  assertTrue_1(warefChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!arefChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!watreeChanged);
  warefChanged = false;

  // Test change propagation from origin to/through alias
  ary.notifyChanged(&dummy);
  assertTrue_1(aryChanged);
  assertTrue_1(aaryChanged);
  assertTrue_1(waaryChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!watreeChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);
  aryChanged = aaryChanged = waaryChanged = false;
  refChanged = arefChanged = warefChanged = false;

  tree.notifyChanged(&dummy);
  assertTrue_1(treeChanged);
  assertTrue_1(atreeChanged);
  assertTrue_1(watreeChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);
  treeChanged = atreeChanged = watreeChanged = false;
  refChanged = arefChanged = warefChanged = false;

  ref.notifyChanged(&dummy);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!watreeChanged);
  refChanged = arefChanged = warefChanged = false;

  // Assign to writable aliases, ensure that the change propagates
  watree.setValue((int32_t) 2);
  int32_t temp;
  assertTrue_1(tree.getValue(temp));
  assertTrue_1(temp == 2);
  assertTrue_1(atree.getValue(temp));
  assertTrue_1(temp == 2);
  assertTrue_1(watree.getValue(temp));
  assertTrue_1(temp == 2);
  assertTrue_1(treeChanged);
  assertTrue_1(atreeChanged);
  assertTrue_1(watreeChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);
  treeChanged = atreeChanged = watreeChanged = false;
  refChanged = arefChanged = warefChanged = false;

  std::vector<int32_t> vi2(4);
  vi2[0] = 3;
  vi2[1] = 6;
  vi2[2] = 9;
  vi2[3] = 12;
  waary.setValue(vi2);
  IntegerArray const *atemp;
  std::vector<int32_t> const *pvi;
  assertTrue_1(ary.getValuePointer(atemp));
  assertTrue_1(atemp != NULL);
  atemp->getContentsVector(pvi);
  assertTrue_1(pvi != NULL);
  assertTrue_1(vi2 == *pvi);
  assertTrue_1(aary.getValuePointer(atemp));
  assertTrue_1(atemp != NULL);
  atemp->getContentsVector(pvi);
  assertTrue_1(pvi != NULL);
  assertTrue_1(vi2 == *pvi);
  assertTrue_1(waary.getValuePointer(atemp));
  assertTrue_1(atemp != NULL);
  atemp->getContentsVector(pvi);
  assertTrue_1(pvi != NULL);
  assertTrue_1(vi2 == *pvi);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!watreeChanged);
  assertTrue_1(aryChanged);
  assertTrue_1(aaryChanged);
  assertTrue_1(waaryChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);
  aryChanged = aaryChanged = waaryChanged = false;
  refChanged = arefChanged = warefChanged = false;

  IntegerConstant luftballoons(99);
  waref.setValue(&luftballoons);
  assertTrue_1(ref.getValue(temp));
  assertTrue_1(temp == 99);
  assertTrue_1(aref.getValue(temp));
  assertTrue_1(temp == 99);
  assertTrue_1(waref.getValue(temp));
  assertTrue_1(temp == 99);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!watreeChanged);
  assertTrue_1(aryChanged);
  assertTrue_1(aaryChanged);
  assertTrue_1(waaryChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);
  aryChanged = aaryChanged = waaryChanged = false;
  refChanged = arefChanged = warefChanged = false;

  waref.setValue((int32_t) 42);
  assertTrue_1(ref.getValue(temp));
  assertTrue_1(temp == 42);
  assertTrue_1(aref.getValue(temp));
  assertTrue_1(temp == 42);
  assertTrue_1(waref.getValue(temp));
  assertTrue_1(temp == 42);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!watreeChanged);
  assertTrue_1(aryChanged);
  assertTrue_1(aaryChanged);
  assertTrue_1(waaryChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);

  // Clean up
  ary.removeListener(&aryListener);
  aary.removeListener(&aaryListener);
  waary.removeListener(&waaryListener);
  tree.removeListener(&treeListener);
  atree.removeListener(&atreeListener);
  watree.removeListener(&watreeListener);
  ref.removeListener(&refListener);
  aref.removeListener(&arefListener);
  waref.removeListener(&warefListener);

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
