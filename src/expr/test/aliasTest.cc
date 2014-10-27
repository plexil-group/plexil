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
#include "test/TrivialListener.hh"
#include "UserVariable.hh"

using namespace PLEXIL;

static bool testAliasToScalarConstant()
{
  {
    BooleanConstant *troo = new BooleanConstant(true);
    Alias *atroo = new Alias(NULL, "atroo", troo);
    assertTrue_1(!atroo->isAssignable());
    assertTrue_1(atroo->isConstant() == troo->isConstant());
    assertTrue_1(atroo->valueType() == troo->valueType());
    assertTrue_1(!atroo->isActive());
    assertTrue_1(!atroo->isKnown());
    atroo->activate();
    assertTrue_1(atroo->isActive());
    assertTrue_1(atroo->isKnown());

    bool tb, tba;
    assertTrue_1(troo->getValue(tb));
    assertTrue_1(atroo->getValue(tba));
    assertTrue_1(tb == tba);

    delete atroo;
    delete troo;
  }

  {
    IntegerConstant *tree = new IntegerConstant(3);
    Alias *atree = new Alias(NULL, "atree", tree);
    assertTrue_1(!atree->isAssignable());
    assertTrue_1(atree->isConstant() == tree->isConstant());
    assertTrue_1(atree->valueType() == tree->valueType());
    assertTrue_1(!atree->isActive());
    assertTrue_1(!atree->isKnown());
    atree->activate();
    assertTrue_1(atree->isActive());
    assertTrue_1(atree->isKnown());

    int32_t ti, tia;
    assertTrue_1(tree->getValue(ti));
    assertTrue_1(atree->getValue(tia));
    assertTrue_1(ti == tia);
 
    delete atree;
    delete tree;
  }

  {
    RealConstant *pie = new RealConstant(3.14);
    Alias *apie = new Alias(NULL, "apie", pie);
    assertTrue_1(!apie->isAssignable());
    assertTrue_1(apie->isConstant()   == pie->isConstant());
    assertTrue_1(apie->valueType()   == pie->valueType());
    assertTrue_1(!apie->isActive());
    assertTrue_1(!apie->isKnown());
    apie->activate();
    assertTrue_1(apie->isActive());
    assertTrue_1(apie->isKnown());

    double td, tda;
    assertTrue_1(pie->getValue(td));
    assertTrue_1(apie->getValue(tda));
    assertTrue_1(td == tda);

    delete apie;
    delete pie;
  }

  {
    StringConstant *fore = new StringConstant("four");
    Alias *afore = new Alias(NULL, "afore", fore);
    assertTrue_1(!afore->isAssignable());
    assertTrue_1(afore->isConstant() == fore->isConstant());
    assertTrue_1(afore->valueType() == fore->valueType());
    assertTrue_1(!afore->isActive());
    assertTrue_1(!afore->isKnown());
    afore->activate();
    assertTrue_1(afore->isActive());
    assertTrue_1(afore->isKnown());

    std::string ts, tsa;
    const std::string *tsp, *tspa;
    assertTrue_1(fore->getValue(ts));
    assertTrue_1(afore->getValue(tsa));
    assertTrue_1(ts == tsa);
    assertTrue_1(fore->getValuePointer(tsp));
    assertTrue_1(afore->getValuePointer(tspa));
    assertTrue_1((*tsp) == (*tspa));

    delete afore;
    delete fore;
  }

  return true;
}

static bool testAliasToArrayConstant()
{
  {
    std::vector<bool>        vb(2);
    vb[0] = false;
    vb[1] = true;
    BooleanArrayConstant *bc = new BooleanArrayConstant(vb);
    Alias *abc = new Alias(NULL, "abc", bc);
    BooleanArray const *pab = NULL, *paab = NULL;
    assertTrue_1(!abc->isAssignable());
    assertTrue_1(abc->isConstant() == bc->isConstant());
    assertTrue_1(abc->valueType() == bc->valueType());
    assertTrue_1(!abc->isActive());
    assertTrue_1(!abc->isKnown());

    abc->activate();
    assertTrue_1(abc->isActive());
    assertTrue_1(abc->isKnown());
    assertTrue_1(bc->getValuePointer(pab));
    assertTrue_1(abc->getValuePointer(paab));
    assertTrue_1(*pab == *paab);

    delete abc;
    delete bc;
  }

  {
    std::vector<int32_t> vi(4);
    vi[0] = 0;
    vi[1] = 1;
    vi[2] = 2;
    vi[3] = 3;
    IntegerArrayConstant *ic = new IntegerArrayConstant(vi);
    Alias *aic = new Alias(NULL, "aic", ic);
    IntegerArray const *pai = NULL, *paai = NULL;
    assertTrue_1(!aic->isAssignable());
    assertTrue_1(aic->isConstant() == ic->isConstant());
    assertTrue_1(aic->valueType() == ic->valueType());
    assertTrue_1(!aic->isActive());
    assertTrue_1(!aic->isKnown());
    aic->activate();
    assertTrue_1(aic->isActive());
    assertTrue_1(aic->isKnown());
    assertTrue_1(ic->getValuePointer(pai));
    assertTrue_1(aic->getValuePointer(paai));
    assertTrue_1(*pai == *paai);

    delete aic;
    delete ic;
  }

  {
    std::vector<double> vd(4);
    vd[0] = 0;
    vd[1] = 1;
    vd[2] = 2;
    vd[3] = 3;
    RealArrayConstant *dc = new RealArrayConstant(vd);
    Alias *adc = new Alias(NULL, "adc", dc);
    RealArray const *pad = NULL, *paad = NULL;
    assertTrue_1(!adc->isAssignable());
    assertTrue_1(adc->isConstant() == dc->isConstant());
    assertTrue_1(adc->valueType() == dc->valueType());
    assertTrue_1(!adc->isActive());
    assertTrue_1(!adc->isKnown());
    adc->activate();
    assertTrue_1(adc->isActive());
    assertTrue_1(adc->isKnown());
    assertTrue_1(dc->getValuePointer(pad));
    assertTrue_1(adc->getValuePointer(paad));
    assertTrue_1(*pad == *paad);

    delete adc;
    delete dc;
  }

  {
    std::vector<std::string> vs(4);
    vs[0] = std::string("zero");
    vs[1] = std::string("one");
    vs[2] = std::string("two");
    vs[3] = std::string("three");
    StringArrayConstant *sc = new StringArrayConstant(vs);
    Alias *asc = new Alias(NULL, "asc", sc);
    StringArray const *pas = NULL, *paas = NULL;
    assertTrue_1(!asc->isAssignable());
    assertTrue_1(asc->isConstant() == sc->isConstant());
    assertTrue_1(asc->valueType() == sc->valueType());
    assertTrue_1(!asc->isActive());
    assertTrue_1(!asc->isKnown());
    asc->activate();
    assertTrue_1(asc->isActive());
    assertTrue_1(asc->isKnown());
    assertTrue_1(sc->getValuePointer(pas));
    assertTrue_1(asc->getValuePointer(paas));
    assertTrue_1(*pas == *paas);

    delete asc;
    delete sc;
}

  return true;
}

static bool testAliasToScalarVariable()
{
  {
    BooleanVariable *troo = new BooleanVariable(true);
    Alias *atroo = new Alias(NULL, "atroo", troo);
    assertTrue_1(!atroo->isAssignable());
    assertTrue_1(atroo->isConstant() == troo->isConstant());
    assertTrue_1(atroo->valueType() == troo->valueType());
    assertTrue_1(!atroo->isActive());
    assertTrue_1(!atroo->isKnown());
    troo->activate();
    atroo->activate();
    assertTrue_1(atroo->isActive());
    assertTrue_1(atroo->isKnown());

    bool tb, tba;
    assertTrue_1(troo->getValue(tb));
    assertTrue_1(atroo->getValue(tba));
    assertTrue_1(tb == tba);

    delete atroo;
    delete troo;
  }

  {
    IntegerVariable *tree = new IntegerVariable(3);
    Alias *atree = new Alias(NULL, "atree", tree);
    assertTrue_1(!atree->isAssignable());
    assertTrue_1(atree->isConstant() == tree->isConstant());
    assertTrue_1(atree->valueType() == tree->valueType());
    assertTrue_1(!atree->isActive());
    assertTrue_1(!atree->isKnown());

    tree->activate();
    atree->activate();
    assertTrue_1(atree->isActive());
    assertTrue_1(atree->isKnown());

    int32_t ti, tia;
    assertTrue_1(tree->getValue(ti));
    assertTrue_1(atree->getValue(tia));
    assertTrue_1(ti == tia);

    delete atree;
    delete tree;
  }

  {
    RealVariable *pie = new RealVariable(3.14);
    Alias *apie = new Alias(NULL, "apie",  pie);
    assertTrue_1(!apie->isAssignable());
    assertTrue_1(apie->isConstant()  == pie->isConstant());
    assertTrue_1(apie->valueType()  == pie->valueType());
    assertTrue_1(!apie->isActive());
    assertTrue_1(!apie->isKnown());

    pie->activate();
    apie->activate();
    assertTrue_1(apie->isActive());
    assertTrue_1(apie->isKnown());

    double td, tda;
    assertTrue_1(pie->getValue(td));
    assertTrue_1(apie->getValue(tda));
    assertTrue_1(td == tda);

    delete apie;
    delete pie;
  }

  {
    StringVariable *fore = new StringVariable("four");
    Alias *afore = new Alias(NULL, "afore", fore);
    assertTrue_1(!afore->isAssignable());
    assertTrue_1(afore->isConstant() == fore->isConstant());
    assertTrue_1(afore->valueType() == fore->valueType());
    assertTrue_1(!afore->isActive());
    assertTrue_1(!afore->isKnown());

    // Activate
    fore->activate();
    afore->activate();
    assertTrue_1(afore->isActive());
    assertTrue_1(afore->isKnown());
  
    std::string ts, tsa;
    const std::string *tsp, *tspa;
    assertTrue_1(fore->getValue(ts));
    assertTrue_1(afore->getValue(tsa));
    assertTrue_1(ts == tsa);
    assertTrue_1(fore->getValuePointer(tsp));
    assertTrue_1(afore->getValuePointer(tspa));
    assertTrue_1((*tsp) == (*tspa));

    delete afore;
    delete fore;
  }

  return true;
}

static bool testAliasToArrayVariable()
{
  IntegerVariable *ix = new IntegerVariable;

  {
    std::vector<bool> vb(2);
    vb[0] = false;
    vb[1] = true;
    BooleanArrayVariable *bc = new BooleanArrayVariable(vb);
    Alias *abc = new Alias(NULL, "abc", bc);
    BooleanArray const *pab = NULL, *paab = NULL;
    assertTrue_1(!abc->isAssignable());
    assertTrue_1(abc->isConstant() == bc->isConstant());
    assertTrue_1(abc->valueType() == bc->valueType());
    assertTrue_1(!abc->isActive());
    assertTrue_1(!abc->isKnown());

    bc->activate();
    abc->activate();
    assertTrue_1(abc->isActive());
    assertTrue_1(abc->isKnown());
    assertTrue_1(bc->getValuePointer(pab));
    assertTrue_1(abc->getValuePointer(paab));
    assertTrue_1(*pab == *paab);

    ArrayReference *bar = new ArrayReference(bc, ix);
    ArrayReference *abar = new ArrayReference(abc, ix);
    bool bt1, bt2;
    ix->activate();
    bar->activate();
    abar->activate();

    ix->setValue((int32_t) 0);
    assertTrue_1(bar->getValue(bt1));
    assertTrue_1(!bt1);
    assertTrue_1(abar->getValue(bt2));
    assertTrue_1(bt1 == bt2);

    delete abar;
    delete bar;
    delete abc;
    delete bc;
  }

  {
    std::vector<int32_t>     vi(4);
    vi[0] = 0;
    vi[1] = 1;
    vi[2] = 2;
    vi[3] = 3;
    IntegerArrayVariable *ic = new IntegerArrayVariable(vi);
    Alias *aic = new Alias(NULL, "aic", ic);
    IntegerArray const *pai = NULL, *paai = NULL;
    assertTrue_1(!aic->isAssignable());
    assertTrue_1(aic->isConstant() == ic->isConstant());
    assertTrue_1(aic->valueType() == ic->valueType());
    assertTrue_1(!aic->isActive());
    assertTrue_1(!aic->isKnown());

    ic->activate();
    aic->activate();
    assertTrue_1(aic->isActive());
    assertTrue_1(aic->isKnown());
    assertTrue_1(ic->getValuePointer(pai));
    assertTrue_1(aic->getValuePointer(paai));
    assertTrue_1(*pai == *paai);

    ArrayReference *iar = new ArrayReference(ic, ix);
    ArrayReference *aiar= new ArrayReference(aic, ix);
    int32_t it1, it2;
    ix->activate();
    iar->activate();
    aiar->activate();
    ix->setValue((int32_t) 0);
    assertTrue_1(iar->getValue(it1));
    assertTrue_1(it1 == 0);
    assertTrue_1(aiar->getValue(it2));
    assertTrue_1(it1 == it2);

    delete aiar;
    delete iar;
    delete aic;
    delete ic;
  }

  {
    std::vector<double> vd(4);
    vd[0] = 0;
    vd[1] = 1;
    vd[2] = 2;
    vd[3] = 3;
    RealArrayVariable *dc = new RealArrayVariable(vd);
    Alias *adc = new Alias(NULL, "adc", dc);
    RealArray const *pad = NULL, *paad = NULL;
    assertTrue_1(!adc->isAssignable());
    assertTrue_1(adc->isConstant() == dc->isConstant());
    assertTrue_1(adc->valueType() == dc->valueType());
    assertTrue_1(!adc->isActive());
    assertTrue_1(!adc->isKnown());

    dc->activate();
    adc->activate();
    assertTrue_1(adc->isActive());
    assertTrue_1(adc->isKnown());
    assertTrue_1(dc->getValuePointer(pad));
    assertTrue_1(adc->getValuePointer(paad));
    assertTrue_1(*pad == *paad);

    ArrayReference *dar = new ArrayReference(dc, ix);
    ArrayReference *adar = new ArrayReference(adc, ix);
    double dt1, dt2;
    ix->activate();
    dar->activate();
    adar->activate();
    ix->setValue((int32_t) 0);
    assertTrue_1(dar->getValue(dt1));
    assertTrue_1(dt1 == 0);
    assertTrue_1(adar->getValue(dt2));
    assertTrue_1(dt1 == dt2);

    delete adar;
    delete dar;
    delete adc;
    delete dc;
  }

  {
    std::vector<std::string> vs(4);
    vs[0] = std::string("zero");
    vs[1] = std::string("one");
    vs[2] = std::string("two");
    vs[3] = std::string("three");
    StringArrayVariable *sc = new StringArrayVariable(vs);
    Alias *asc = new Alias(NULL, "asc", sc);
    StringArray const *pas = NULL, *paas = NULL;
    assertTrue_1(!asc->isAssignable());
    assertTrue_1(asc->isConstant() == sc->isConstant());
    assertTrue_1(asc->valueType() == sc->valueType());
    assertTrue_1(!asc->isActive());
    assertTrue_1(!asc->isKnown());

    sc->activate();
    asc->activate();
    assertTrue_1(asc->isActive());
    assertTrue_1(asc->isKnown());
    assertTrue_1(sc->getValuePointer(pas));
    assertTrue_1(asc->getValuePointer(paas));
    assertTrue_1(*pas == *paas);

    ArrayReference *sar = new ArrayReference(sc, ix);
    ArrayReference *asar = new ArrayReference(asc, ix);
    std::string st1, st2;
    std::string const *stp1, *stp2;
    ix->activate();
    sar->activate();
    asar->activate();
    ix->setValue((int32_t) 0);
    assertTrue_1(sar->getValue(st1));
    assertTrue_1(st1 == std::string("zero"));
    assertTrue_1(asar->getValue(st2));
    assertTrue_1(st1 == st2);

    assertTrue_1(sar->getValuePointer(stp1));
    assertTrue_1(*stp1 == std::string("zero"));
    assertTrue_1(asar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);

    delete asar;
    delete sar;
    delete asc;
    delete sc;
  }

  delete ix;

  return true;
}

static bool testAliasToArrayReference()
{
  IntegerVariable *ix = new IntegerVariable;

  {
    std::vector<bool> vb(2);
    vb[0] = false;
    vb[1] = true;
    BooleanArrayVariable *bc = new BooleanArrayVariable(vb);
    ArrayReference *bar = new ArrayReference(bc, ix);
    MutableArrayReference *wbar = new MutableArrayReference(bc, ix);
    Alias *abar = new Alias(NULL, "abar", bar);
    Alias *awbar = new Alias(NULL, "awbar", wbar);
    bool bt1, bt2;
    ix->activate();
    bc->activate();
    bar->activate();
    wbar->activate();
    ix->setValue((int32_t) 1);
    assertTrue_1(!abar->isActive());
    assertTrue_1(!awbar->isActive());
    assertTrue_1(!abar->isKnown());
    assertTrue_1(!awbar->isKnown());

    abar->activate();
    awbar->activate();
    assertTrue_1(abar->isActive());
    assertTrue_1(awbar->isActive());
    assertTrue_1(abar->isKnown());
    assertTrue_1(awbar->isKnown());
    assertTrue_1(bar->getValue(bt1));
    assertTrue_1(bt1);
    assertTrue_1(abar->getValue(bt2));
    assertTrue_1(bt1 == bt2);
    assertTrue_1(awbar->getValue(bt2));
    assertTrue_1(bt1 == bt2);

    ix->setValue((int32_t) 0);
    assertTrue_1(bar->getValue(bt1));
    assertTrue_1(!bt1);
    assertTrue_1(abar->getValue(bt2));
    assertTrue_1(bt1 == bt2);
    assertTrue_1(awbar->getValue(bt2));
    assertTrue_1(bt1 == bt2);

    awbar->deactivate();
    abar->deactivate();
    wbar->deactivate();
    bar->deactivate();
    bc->deactivate();
    ix->deactivate();

    delete awbar;
    delete abar;
    delete wbar;
    delete bar;
    delete bc;
  }

  {
    std::vector<int32_t> vi(4);
    vi[0] = 3;
    vi[1] = 2;
    vi[2] = 1;
    vi[3] = 0;
    IntegerArrayVariable *ic = new IntegerArrayVariable(vi);
    ArrayReference *iar = new ArrayReference(ic, ix);
    MutableArrayReference *wiar = new MutableArrayReference(ic, ix);
    Alias *aiar = new Alias(NULL, "aiar", iar);
    Alias *awiar = new Alias(NULL, "awiar", wiar);
    int32_t it1, it2;
    ix->activate();
    ic->activate();
    iar->activate();
    wiar->activate();
    ix->setValue((int32_t) 1);
    assertTrue_1(!aiar->isActive());
    assertTrue_1(!awiar->isActive());
    assertTrue_1(!aiar->isKnown());
    assertTrue_1(!awiar->isKnown());

    aiar->activate();
    awiar->activate();
    assertTrue_1(aiar->isActive());
    assertTrue_1(awiar->isActive());
    assertTrue_1(aiar->isKnown());
    assertTrue_1(awiar->isKnown());
    assertTrue_1(iar->getValue(it1));
    assertTrue_1(it1 == 2);
    assertTrue_1(aiar->getValue(it2));
    assertTrue_1(it1 == it2);
    assertTrue_1(awiar->getValue(it2));
    assertTrue_1(it1 == it2);

    ix->setValue((int32_t) 0);
    assertTrue_1(iar->getValue(it1));
    assertTrue_1(it1 == 3);
    assertTrue_1(aiar->getValue(it2));
    assertTrue_1(it1 == it2);
    assertTrue_1(awiar->getValue(it2));
    assertTrue_1(it1 == it2);

    awiar->deactivate();
    aiar->deactivate();
    wiar->deactivate();
    iar->deactivate();
    ic->deactivate();
    ix->deactivate();

    delete awiar;
    delete aiar;
    delete wiar;
    delete iar;
    delete ic;
  }

  {
    std::vector<double> vd(4);
    vd[0] = 7;
    vd[1] = 6;
    vd[2] = 5;
    vd[3] = 4;
    RealArrayVariable *dc = new RealArrayVariable(vd);
    ArrayReference *dar = new ArrayReference(dc, ix);
    MutableArrayReference *wdar = new MutableArrayReference(dc, ix);
    Alias *adar = new Alias(NULL, "adar", dar);
    Alias *awdar = new Alias(NULL, "awdar", wdar);
    double dt1, dt2;
    ix->activate();
    dc->activate();
    dar->activate();
    wdar->activate();
    ix->setValue((int32_t) 1);
    assertTrue_1(!adar->isActive());
    assertTrue_1(!awdar->isActive());
    assertTrue_1(!adar->isKnown());
    assertTrue_1(!awdar->isKnown());

    adar->activate();
    awdar->activate();
    assertTrue_1(adar->isActive());
    assertTrue_1(awdar->isActive());
    assertTrue_1(adar->isKnown());
    assertTrue_1(awdar->isKnown());
    assertTrue_1(dar->getValue(dt1));
    assertTrue_1(dt1 == 6);
    assertTrue_1(adar->getValue(dt2));
    assertTrue_1(dt1 == dt2);
    assertTrue_1(awdar->getValue(dt2));
    assertTrue_1(dt1 == dt2);

    ix->setValue((int32_t) 0);
    assertTrue_1(dar->getValue(dt1));
    assertTrue_1(dt1 == 7);
    assertTrue_1(adar->getValue(dt2));
    assertTrue_1(dt1 == dt2);
    assertTrue_1(awdar->getValue(dt2));
    assertTrue_1(dt1 == dt2);

    awdar->deactivate();
    adar->deactivate();
    wdar->deactivate();
    dar->deactivate();
    dc->deactivate();
    ix->deactivate();

    delete awdar;
    delete adar;
    delete wdar;
    delete dar;
    delete dc;
  }

  {
    std::vector<std::string> vs(4);
    vs[0] = std::string("zero");
    vs[1] = std::string("one");
    vs[2] = std::string("two");
    vs[3] = std::string("three");
    StringArrayVariable *sc = new StringArrayVariable(vs);
    ArrayReference *sar = new ArrayReference(sc, ix);
    MutableArrayReference *wsar = new MutableArrayReference(sc, ix);
    Alias *asar = new Alias(NULL, "asar", sar);
    Alias *awsar = new Alias(NULL, "awsar", wsar);
    std::string st1, st2;
    std::string const *stp1, *stp2;
    ix->activate();
    sc->activate();
    sar->activate();
    wsar->activate();
    ix->setValue((int32_t) 1);
    assertTrue_1(!asar->isActive());
    assertTrue_1(!awsar->isActive());
    assertTrue_1(!asar->isKnown());
    assertTrue_1(!awsar->isKnown());

    asar->activate();
    awsar->activate();
    assertTrue_1(asar->isActive());
    assertTrue_1(awsar->isActive());
    assertTrue_1(asar->isKnown());
    assertTrue_1(awsar->isKnown());
    assertTrue_1(sar->getValue(st1));
    assertTrue_1(st1 == std::string("one"));
    assertTrue_1(asar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(awsar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(sar->getValuePointer(stp1));
    assertTrue_1(*stp1 == std::string("one"));
    assertTrue_1(asar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);
    assertTrue_1(awsar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);

    ix->setValue((int32_t) 0);
    assertTrue_1(sar->getValue(st1));
    assertTrue_1(st1 == std::string("zero"));
    assertTrue_1(asar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(awsar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(sar->getValuePointer(stp1));
    assertTrue_1(*stp1 == std::string("zero"));
    assertTrue_1(asar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);
    assertTrue_1(awsar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);

    awsar->deactivate();
    asar->deactivate();
    wsar->deactivate();
    sar->deactivate();
    sc->deactivate();
    ix->deactivate();

    delete awsar;
    delete asar;
    delete wsar;
    delete sar;
    delete sc;
  }

  delete ix;

  return true;
}

static BooleanConstant dummy; // used as prop source

static bool testVariableAliasPropagation()
{
  IntegerVariable *tree = new IntegerVariable(3);
  Alias *atree = new Alias(NULL, "atree", tree);
  bool treeChanged, atreeChanged;
  TrivialListener *treeListener = new TrivialListener(treeChanged);
  TrivialListener *atreeListener = new TrivialListener(atreeChanged);
  tree->addListener(treeListener);
  atree->addListener(atreeListener);
  treeChanged = atreeChanged = false;

  // Test that notifying while inactive does nothing
  atree->notifyChanged(tree);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!treeChanged);

  // Activate
  atree->activate();
  tree->activate();

  // Test that initial activation causes propagation
  assertTrue_1(treeChanged);
  assertTrue_1(atreeChanged);

  // Test that notifying alias doesn't notify origin
  treeChanged = atreeChanged = false;

  atree->notifyChanged(tree);
  assertTrue_1(atreeChanged);
  assertTrue_1(!treeChanged);
  atreeChanged = false;

  // Test change propagation from origin to/through alias
  tree->notifyChanged(&dummy);
  assertTrue_1(treeChanged);
  assertTrue_1(atreeChanged);

  // Clean up
  tree->removeListener(treeListener);
  atree->removeListener(atreeListener);

  delete atreeListener;
  delete treeListener;
  delete atree;
  delete tree;

  return true;
}

static bool testArrayAliasPropagation()
{
  std::vector<int32_t> vi(4);
  vi[0] = 2;
  vi[1] = 4;
  vi[2] = 6;
  vi[3] = 8;
  IntegerArrayVariable *ary = new IntegerArrayVariable(vi);
  Alias *aary = new Alias(NULL, "aary", ary);
  bool aryChanged, aaryChanged;
  TrivialListener *aryListener = new TrivialListener(aryChanged);
  TrivialListener *aaryListener = new TrivialListener(aaryChanged);
  ary->addListener(aryListener);
  aary->addListener(aaryListener);
  aryChanged = aaryChanged = false;

  // Test that notifying while inactive does nothing
  aary->notifyChanged(ary);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!aryChanged);

  // Activate
  aary->activate();
  ary->activate();

  // Test that initial activation causes propagation
  assertTrue_1(aryChanged);
  assertTrue_1(aaryChanged);

  // Test that notifying alias doesn't notify origin
  aryChanged = aaryChanged = false;
  aary->notifyChanged(ary);
  assertTrue_1(aaryChanged);
  assertTrue_1(!aryChanged);
  aaryChanged = false;

  // Test change propagation from origin to/through alias
  ary->notifyChanged(&dummy);
  assertTrue_1(aryChanged);
  assertTrue_1(aaryChanged);

  // Clean up
  ary->removeListener(aryListener);
  aary->removeListener(aaryListener);

  delete aaryListener;
  delete aryListener;
  delete aary;
  delete ary;

  return true;
}

static bool testArrayRefAliasPropagation()
{
  std::vector<int32_t> vi(4);
  vi[0] = 2;
  vi[1] = 4;
  vi[2] = 6;
  vi[3] = 8;
  IntegerArrayVariable *ary = new IntegerArrayVariable(vi);
  IntegerVariable *tree = new IntegerVariable(3);
  Alias *atree = new Alias(NULL, "atree", tree);
  MutableArrayReference *ref = new MutableArrayReference(ary, atree);
  Alias *aref = new Alias(NULL, "aref", ref);

  bool aryChanged, atreeChanged, refChanged, arefChanged;

  TrivialListener *aryListener = new TrivialListener(aryChanged);
  TrivialListener *atreeListener = new TrivialListener(atreeChanged);
  TrivialListener *refListener = new TrivialListener(refChanged);
  TrivialListener *arefListener = new TrivialListener(arefChanged);

  ary->addListener(aryListener);
  atree->addListener(atreeListener);
  ref->addListener(refListener);
  aref->addListener(arefListener);

  aryChanged = atreeChanged = refChanged = arefChanged = false;

  // Test that notifying while inactive does nothing
  atree->notifyChanged(tree);
  assertTrue_1(!atreeChanged);

  aref->notifyChanged(ref);
  assertTrue_1(!arefChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!atreeChanged);

  // Activate all
  aref->activate();
  ref->activate();
  ary->activate();
  atree->activate();
  tree->activate();

  // Test that initial activation causes propagation
  assertTrue_1(aryChanged);
  assertTrue_1(refChanged);
  assertTrue_1(atreeChanged);
  assertTrue_1(arefChanged);

  // Test that notifying alias doesn't notify origin
  aryChanged = atreeChanged = refChanged = arefChanged = false;

  atree->notifyChanged(tree);
  assertTrue_1(atreeChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(refChanged);   // these depend on atree
  assertTrue_1(arefChanged);  //
  atreeChanged = refChanged = arefChanged = false;

  aref->notifyChanged(ref);
  assertTrue_1(arefChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!atreeChanged);
  arefChanged = false;

  // Test change propagation from origin to/through alias
  ary->notifyChanged(&dummy);
  assertTrue_1(aryChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  aryChanged = refChanged = arefChanged = false;

  tree->notifyChanged(&dummy);
  assertTrue_1(atreeChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  atreeChanged = refChanged = arefChanged = false;

  ref->notifyChanged(&dummy);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!atreeChanged);

  // Clean up
  ary->removeListener(aryListener);
  atree->removeListener(atreeListener);
  ref->removeListener(refListener);
  aref->removeListener(arefListener);

  delete arefListener;
  delete refListener;
  delete atreeListener;
  delete aryListener;

  delete aref;
  delete ref;
  delete atree;
  delete tree;
  delete ary;

  return true;
}

bool aliasTest()
{
  runTest(testAliasToScalarConstant);
  runTest(testAliasToArrayConstant);
  runTest(testAliasToScalarVariable);
  runTest(testVariableAliasPropagation);
  runTest(testAliasToArrayVariable);
  runTest(testArrayAliasPropagation);
  runTest(testAliasToArrayReference);
  runTest(testArrayRefAliasPropagation);

  return true;
}
