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
    InOutAlias *watroo = new InOutAlias(NULL, "watroo", troo);
    assertTrue_1(!atroo->isAssignable());
    assertTrue_1(watroo->isAssignable());
    assertTrue_1(atroo->isConstant() == troo->isConstant());
    assertTrue_1(watroo->isConstant() == troo->isConstant());
    assertTrue_1(atroo->valueType() == troo->valueType());
    assertTrue_1(watroo->valueType() == troo->valueType());
    assertTrue_1(!atroo->isActive());
    assertTrue_1(!watroo->isActive());
    assertTrue_1(!atroo->isKnown());
    assertTrue_1(!watroo->isKnown());
    troo->activate();
    atroo->activate();
    watroo->activate();
    assertTrue_1(atroo->isActive());
    assertTrue_1(watroo->isActive());
    assertTrue_1(atroo->isKnown());
    assertTrue_1(watroo->isKnown());

    bool tb, tba;
    assertTrue_1(troo->getValue(tb));
    assertTrue_1(atroo->getValue(tba));
    assertTrue_1(tb == tba);
    assertTrue_1(watroo->getValue(tba));
    assertTrue_1(tb == tba);

    watroo->setValue(false);
    assertTrue_1(troo->getValue(tb));
    assertTrue_1(!tb);
    assertTrue_1(watroo->getValue(tba));
    assertTrue_1(tb == tba);
    assertTrue_1(atroo->getValue(tba));
    assertTrue_1(tb == tba);

    watroo->setUnknown();
    assertTrue_1(!troo->getValue(tb));
    assertTrue_1(!watroo->getValue(tba));
    assertTrue_1(!atroo->getValue(tba));

    watroo->deactivate();
    watroo->reset();
    watroo->activate();

    assertTrue_1(!troo->getValue(tb));
    assertTrue_1(!watroo->getValue(tba));
    assertTrue_1(!atroo->getValue(tba));

    delete watroo;
    delete atroo;
    delete troo;
  }

  {
    IntegerVariable *tree = new IntegerVariable(3);
    Alias *atree = new Alias(NULL, "atree", tree);
    InOutAlias *watree = new InOutAlias(NULL, "watree", tree);
    assertTrue_1(!atree->isAssignable());
    assertTrue_1(watree->isAssignable());
    assertTrue_1(atree->isConstant() == tree->isConstant());
    assertTrue_1(watree->isConstant() == tree->isConstant());
    assertTrue_1(atree->valueType() == tree->valueType());
    assertTrue_1(watree->valueType() == tree->valueType());
    assertTrue_1(!atree->isActive());
    assertTrue_1(!watree->isActive());
    assertTrue_1(!atree->isKnown());
    assertTrue_1(!watree->isKnown());

    tree->activate();
    atree->activate();
    watree->activate();
    assertTrue_1(atree->isActive());
    assertTrue_1(watree->isActive());
    assertTrue_1(atree->isKnown());
    assertTrue_1(watree->isKnown());

    int32_t ti, tia;
    assertTrue_1(tree->getValue(ti));
    assertTrue_1(atree->getValue(tia));
    assertTrue_1(ti == tia);
    assertTrue_1(watree->getValue(tia));
    assertTrue_1(ti == tia);

    watree->setValue((int32_t) 42);
    assertTrue_1(tree->getValue(ti));
    assertTrue_1(ti == 42);
    assertTrue_1(watree->getValue(tia));
    assertTrue_1(ti == tia);
    assertTrue_1(atree->getValue(tia));
    assertTrue_1(ti == tia);

    watree->setUnknown();
    assertTrue_1(!tree->getValue(ti));
    assertTrue_1(!watree->getValue(tia));
    assertTrue_1(!atree->getValue(tia));

    watree->deactivate();
    watree->reset();
    watree->activate();
    assertTrue_1(!tree->getValue(ti));
    assertTrue_1(!watree->getValue(tia));
    assertTrue_1(!atree->getValue(tia));

    delete watree;
    delete atree;
    delete tree;
  }

  {
    RealVariable *pie = new RealVariable(3.14);
    Alias *apie = new Alias(NULL, "apie",  pie);
    InOutAlias *wapie = new InOutAlias(NULL, "wapie",  pie);
    assertTrue_1(!apie->isAssignable());
    assertTrue_1(wapie->isAssignable());
    assertTrue_1(apie->isConstant()  == pie->isConstant());
    assertTrue_1(wapie->isConstant()  == pie->isConstant());
    assertTrue_1(apie->valueType()  == pie->valueType());
    assertTrue_1(wapie->valueType()  == pie->valueType());
    assertTrue_1(!apie->isActive());
    assertTrue_1(!wapie->isActive());
    assertTrue_1(!apie->isKnown());
    assertTrue_1(!wapie->isKnown());

    pie->activate();
    apie->activate();
    wapie->activate();
    assertTrue_1(apie->isActive());
    assertTrue_1(wapie->isActive());
    assertTrue_1(apie->isKnown());
    assertTrue_1(wapie->isKnown());

    double td, tda;
    assertTrue_1(pie->getValue(td));
    assertTrue_1(apie->getValue(tda));
    assertTrue_1(td == tda);
    assertTrue_1(wapie->getValue(tda));
    assertTrue_1(td == tda);

    wapie->setValue(2.5);
    assertTrue_1(pie->getValue(td));
    assertTrue_1(td == 2.5);
    assertTrue_1(wapie->getValue(tda));
    assertTrue_1(td == tda);
    assertTrue_1(apie->getValue(tda));
    assertTrue_1(td == tda);

    wapie->setUnknown();
    assertTrue_1(!pie->getValue(td));
    assertTrue_1(!wapie->getValue(tda));
    assertTrue_1(!apie->getValue(tda));

    wapie->deactivate();
    wapie->reset();
    wapie->activate();
    assertTrue_1(!pie->getValue(td));
    assertTrue_1(!wapie->getValue(tda));
    assertTrue_1(!apie->getValue(tda));

    delete wapie;
    delete apie;
    delete pie;
  }

  {
    StringVariable *fore = new StringVariable("four");
    Alias *afore = new Alias(NULL, "afore", fore);
    InOutAlias *wafore = new InOutAlias(NULL, "wafore", fore);
    assertTrue_1(!afore->isAssignable());
    assertTrue_1(wafore->isAssignable());
    assertTrue_1(afore->isConstant() == fore->isConstant());
    assertTrue_1(wafore->isConstant() == fore->isConstant());
    assertTrue_1(afore->valueType() == fore->valueType());
    assertTrue_1(wafore->valueType() == fore->valueType());
    assertTrue_1(!afore->isActive());
    assertTrue_1(!wafore->isActive());
    assertTrue_1(!afore->isKnown());
    assertTrue_1(!wafore->isKnown());

    // Activate
    fore->activate();
    afore->activate();
    wafore->activate();
    assertTrue_1(afore->isActive());
    assertTrue_1(wafore->isActive());
    assertTrue_1(afore->isKnown());
    assertTrue_1(wafore->isKnown());
  
    std::string ts, tsa;
    const std::string *tsp, *tspa;
    assertTrue_1(fore->getValue(ts));
    assertTrue_1(afore->getValue(tsa));
    assertTrue_1(ts == tsa);
    assertTrue_1(fore->getValuePointer(tsp));
    assertTrue_1(afore->getValuePointer(tspa));
    assertTrue_1((*tsp) == (*tspa));
    assertTrue_1(wafore->getValue(tsa));
    assertTrue_1(ts == tsa);
    assertTrue_1(wafore->getValuePointer(tspa));
    assertTrue_1((*tsp) == (*tspa));

    wafore->setValue(std::string("yow!"));
    assertTrue_1(fore->getValue(ts));
    assertTrue_1(ts == std::string("yow!"));
    assertTrue_1(wafore->getValue(tsa));
    assertTrue_1(ts == tsa);
    assertTrue_1(afore->getValue(tsa));
    assertTrue_1(ts == tsa);

    wafore->setUnknown();
    assertTrue_1(!fore->getValue(ts));
    assertTrue_1(!wafore->getValue(tsa));
    assertTrue_1(!afore->getValue(tsa));

    wafore->deactivate();
    wafore->reset();
    wafore->activate();
  
    assertTrue_1(!fore->getValue(ts));
    assertTrue_1(!wafore->getValue(tsa));
    assertTrue_1(!afore->getValue(tsa));

    delete wafore;
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
    InOutAlias *wabc = new InOutAlias(NULL, "wabc", bc);
    BooleanArray const *pab = NULL, *paab = NULL;
    assertTrue_1(!abc->isAssignable());
    assertTrue_1(wabc->isAssignable());
    assertTrue_1(abc->isConstant() == bc->isConstant());
    assertTrue_1(wabc->isConstant() == bc->isConstant());
    assertTrue_1(abc->valueType() == bc->valueType());
    assertTrue_1(wabc->valueType() == bc->valueType());
    assertTrue_1(!abc->isActive());
    assertTrue_1(!wabc->isActive());
    assertTrue_1(!abc->isKnown());
    assertTrue_1(!wabc->isKnown());

    bc->activate();
    abc->activate();
    wabc->activate();
    assertTrue_1(abc->isActive());
    assertTrue_1(wabc->isActive());
    assertTrue_1(abc->isKnown());
    assertTrue_1(wabc->isKnown());
    assertTrue_1(bc->getValuePointer(pab));
    assertTrue_1(abc->getValuePointer(paab));
    assertTrue_1(*pab == *paab);
    assertTrue_1(wabc->getValuePointer(paab));
    assertTrue_1(*pab == *paab);

    ArrayReference *bar = new ArrayReference(bc, ix);
    ArrayReference *abar = new ArrayReference(abc, ix);
    MutableArrayReference *wabar = new MutableArrayReference(wabc, ix);
    bool bt1, bt2;
    ix->activate();
    bar->activate();
    abar->activate();
    wabar->activate();

    ix->setValue((int32_t) 0);
    assertTrue_1(bar->getValue(bt1));
    assertTrue_1(!bt1);
    assertTrue_1(abar->getValue(bt2));
    assertTrue_1(bt1 == bt2);
    assertTrue_1(wabar->getValue(bt2));
    assertTrue_1(bt1 == bt2);

    std::vector<bool> vb2(3);
    vb2[0] = true;
    vb2[1] = false;
    vb2[2] = true;
    std::vector<bool> const *pvb;
    wabc->setValue(vb2);
    assertTrue_1(bc->getValuePointer(pab));
    assertTrue_1(pab != NULL);
    pab->getContentsVector(pvb);
    assertTrue_1(pvb != NULL);
    assertTrue_1(*pvb == vb2);
    assertTrue_1(abc->getValuePointer(paab));
    assertTrue_1(*pab == *paab);

    wabar->setValue(false);
    assertTrue_1(bar->getValue(bt1));
    assertTrue_1(!bt1);
    assertTrue_1(abar->getValue(bt2));
    assertTrue_1(bt1 == bt2);
    assertTrue_1(wabar->getValue(bt2));
    assertTrue_1(bt1 == bt2);

    delete wabar;
    delete abar;
    delete bar;
    delete wabc;
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
    InOutAlias *waic = new InOutAlias(NULL, "waic", ic);
    IntegerArray const *pai = NULL, *paai = NULL;
    assertTrue_1(!aic->isAssignable());
    assertTrue_1(waic->isAssignable());
    assertTrue_1(aic->isConstant() == ic->isConstant());
    assertTrue_1(waic->isConstant() == ic->isConstant());
    assertTrue_1(aic->valueType() == ic->valueType());
    assertTrue_1(waic->valueType() == ic->valueType());
    assertTrue_1(!aic->isActive());
    assertTrue_1(!waic->isActive());
    assertTrue_1(!aic->isKnown());
    assertTrue_1(!waic->isKnown());

    ic->activate();
    aic->activate();
    waic->activate();
    assertTrue_1(aic->isActive());
    assertTrue_1(waic->isActive());
    assertTrue_1(aic->isKnown());
    assertTrue_1(waic->isKnown());
    assertTrue_1(ic->getValuePointer(pai));
    assertTrue_1(aic->getValuePointer(paai));
    assertTrue_1(*pai == *paai);
    assertTrue_1(waic->getValuePointer(paai));
    assertTrue_1(*pai == *paai);

    ArrayReference *iar = new ArrayReference(ic, ix);
    ArrayReference *aiar= new ArrayReference(aic, ix);
    MutableArrayReference *waiar = new MutableArrayReference(waic, ix);
    int32_t it1, it2;
    ix->activate();
    iar->activate();
    aiar->activate();
    waiar->activate();
    ix->setValue((int32_t) 0);
    assertTrue_1(iar->getValue(it1));
    assertTrue_1(it1 == 0);
    assertTrue_1(aiar->getValue(it2));
    assertTrue_1(it1 == it2);
    assertTrue_1(waiar->getValue(it2));
    assertTrue_1(it1 == it2);

    std::vector<int32_t> vi2(3);
    vi2[0] = 3;
    vi2[1] = 2;
    vi2[2] = 1;
    std::vector<int32_t> const *pvi;
    waic->setValue(vi2);
    assertTrue_1(ic->getValuePointer(pai));
    assertTrue_1(pai != NULL);
    pai->getContentsVector(pvi);
    assertTrue_1(pvi != NULL);
    assertTrue_1(*pvi == vi2);
    assertTrue_1(aic->getValuePointer(paai));
    assertTrue_1(*pai == *paai);

    waiar->setValue((int32_t) 42);
    assertTrue_1(iar->getValue(it1));
    assertTrue_1(it1 == 42);
    assertTrue_1(aiar->getValue(it2));
    assertTrue_1(it1 == it2);
    assertTrue_1(waiar->getValue(it2));
    assertTrue_1(it1 == it2);

    delete waiar;
    delete aiar;
    delete iar;
    delete waic;
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
    InOutAlias *wadc = new InOutAlias(NULL, "wadc", dc);
    RealArray const *pad = NULL, *paad = NULL;
    assertTrue_1(!adc->isAssignable());
    assertTrue_1(wadc->isAssignable());
    assertTrue_1(adc->isConstant() == dc->isConstant());
    assertTrue_1(wadc->isConstant() == dc->isConstant());
    assertTrue_1(adc->valueType() == dc->valueType());
    assertTrue_1(wadc->valueType() == dc->valueType());
    assertTrue_1(!adc->isActive());
    assertTrue_1(!wadc->isActive());
    assertTrue_1(!adc->isKnown());
    assertTrue_1(!wadc->isKnown());

    dc->activate();
    adc->activate();
    wadc->activate();
    assertTrue_1(adc->isActive());
    assertTrue_1(wadc->isActive());
    assertTrue_1(adc->isKnown());
    assertTrue_1(wadc->isKnown());
    assertTrue_1(dc->getValuePointer(pad));
    assertTrue_1(adc->getValuePointer(paad));
    assertTrue_1(*pad == *paad);
    assertTrue_1(wadc->getValuePointer(paad));
    assertTrue_1(*pad == *paad);

    ArrayReference *dar = new ArrayReference(dc, ix);
    ArrayReference *adar = new ArrayReference(adc, ix);
    MutableArrayReference *wadar = new MutableArrayReference(wadc, ix);
    double dt1, dt2;
    ix->activate();
    dar->activate();
    adar->activate();
    wadar->activate();
    ix->setValue((int32_t) 0);
    assertTrue_1(dar->getValue(dt1));
    assertTrue_1(dt1 == 0);
    assertTrue_1(adar->getValue(dt2));
    assertTrue_1(dt1 == dt2);
    assertTrue_1(wadar->getValue(dt2));
    assertTrue_1(dt1 == dt2);

    std::vector<double> vd2(3);
    vd2[0] = 5;
    vd2[1] = 4;
    vd2[2] = 3;
    std::vector<double> const *pvd;
    wadc->setValue(vd2);
    assertTrue_1(dc->getValuePointer(pad));
    assertTrue_1(pad != NULL);
    pad->getContentsVector(pvd);
    assertTrue_1(pvd != NULL);
    assertTrue_1(*pvd == vd2);
    assertTrue_1(adc->getValuePointer(paad));
    assertTrue_1(*pad == *paad);

    wadar->setValue(22.5);
    assertTrue_1(dar->getValue(dt1));
    assertTrue_1(dt1 == 22.5);
    assertTrue_1(adar->getValue(dt2));
    assertTrue_1(dt1 == dt2);
    assertTrue_1(wadar->getValue(dt2));
    assertTrue_1(dt1 == dt2);

    delete wadar;
    delete adar;
    delete dar;
    delete wadc;
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
    InOutAlias *wasc = new InOutAlias(NULL, "wasc", sc);
    StringArray const *pas = NULL, *paas = NULL;
    assertTrue_1(!asc->isAssignable());
    assertTrue_1(wasc->isAssignable());
    assertTrue_1(asc->isConstant() == sc->isConstant());
    assertTrue_1(wasc->isConstant() == sc->isConstant());
    assertTrue_1(asc->valueType() == sc->valueType());
    assertTrue_1(wasc->valueType() == sc->valueType());
    assertTrue_1(!asc->isActive());
    assertTrue_1(!wasc->isActive());
    assertTrue_1(!asc->isKnown());
    assertTrue_1(!wasc->isKnown());

    sc->activate();
    asc->activate();
    wasc->activate();
    assertTrue_1(asc->isActive());
    assertTrue_1(wasc->isActive());
    assertTrue_1(asc->isKnown());
    assertTrue_1(wasc->isKnown());
    assertTrue_1(sc->getValuePointer(pas));
    assertTrue_1(asc->getValuePointer(paas));
    assertTrue_1(*pas == *paas);
    assertTrue_1(wasc->getValuePointer(paas));
    assertTrue_1(*pas == *paas);

    ArrayReference *sar = new ArrayReference(sc, ix);
    ArrayReference *asar = new ArrayReference(asc, ix);
    MutableArrayReference *wasar = new MutableArrayReference(wasc, ix);
    std::string st1, st2;
    std::string const *stp1, *stp2;
    ix->activate();
    sar->activate();
    asar->activate();
    wasar->activate();
    ix->setValue((int32_t) 0);
    assertTrue_1(sar->getValue(st1));
    assertTrue_1(st1 == std::string("zero"));
    assertTrue_1(asar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(wasar->getValue(st2));
    assertTrue_1(st1 == st2);

    assertTrue_1(sar->getValuePointer(stp1));
    assertTrue_1(*stp1 == std::string("zero"));
    assertTrue_1(asar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);
    assertTrue_1(wasar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);

    std::vector<std::string> vs2(3);
    vs2[0] = std::string("seven");
    vs2[1] = std::string("six");
    vs2[2] = std::string("eight");
    std::vector<std::string> const *pvs;
    wasc->setValue(vs2);
    assertTrue_1(sc->getValuePointer(pas));
    assertTrue_1(pas != NULL);
    pas->getContentsVector(pvs);
    assertTrue_1(pvs != NULL);
    assertTrue_1(*pvs == vs2);
    assertTrue_1(asc->getValuePointer(paas));
    assertTrue_1(*pas == *paas);

    wasar->setValue(std::string("bogus"));
    assertTrue_1(sar->getValue(st1));
    assertTrue_1(st1 == std::string("bogus"));
    assertTrue_1(asar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(wasar->getValue(st2));
    assertTrue_1(st1 == st2);

    assertTrue_1(sar->getValuePointer(stp1));
    assertTrue_1(*stp1 == std::string("bogus"));
    assertTrue_1(asar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);
    assertTrue_1(wasar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);

    delete wasar;
    delete asar;
    delete sar;
    delete wasc;
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
    InOutAlias *wawbar = new InOutAlias(NULL, "wawbar", wbar);
    bool bt1, bt2;
    ix->activate();
    bc->activate();
    bar->activate();
    wbar->activate();
    ix->setValue((int32_t) 1);
    assertTrue_1(!abar->isActive());
    assertTrue_1(!awbar->isActive());
    assertTrue_1(!wawbar->isActive());
    assertTrue_1(!abar->isKnown());
    assertTrue_1(!awbar->isKnown());
    assertTrue_1(!wawbar->isKnown());

    abar->activate();
    awbar->activate();
    wawbar->activate();
    assertTrue_1(abar->isActive());
    assertTrue_1(awbar->isActive());
    assertTrue_1(wawbar->isActive());
    assertTrue_1(abar->isKnown());
    assertTrue_1(awbar->isKnown());
    assertTrue_1(wawbar->isKnown());
    assertTrue_1(bar->getValue(bt1));
    assertTrue_1(bt1);
    assertTrue_1(abar->getValue(bt2));
    assertTrue_1(bt1 == bt2);
    assertTrue_1(awbar->getValue(bt2));
    assertTrue_1(bt1 == bt2);
    assertTrue_1(wawbar->getValue(bt2));
    assertTrue_1(bt1 == bt2);

    ix->setValue((int32_t) 0);
    assertTrue_1(bar->getValue(bt1));
    assertTrue_1(!bt1);
    assertTrue_1(abar->getValue(bt2));
    assertTrue_1(bt1 == bt2);
    assertTrue_1(awbar->getValue(bt2));
    assertTrue_1(bt1 == bt2);
    assertTrue_1(wawbar->getValue(bt2));
    assertTrue_1(bt1 == bt2);

    wawbar->setValue(true);
    assertTrue_1(bar->getValue(bt1));
    assertTrue_1(bt1);
    assertTrue_1(abar->getValue(bt2));
    assertTrue_1(bt1 == bt2);
    assertTrue_1(awbar->getValue(bt2));
    assertTrue_1(bt1 == bt2);
    // assertTrue_1(wawbar->getValue(bt2));
    // assertTrue_1(bt1 == bt2);

    wawbar->deactivate();
    awbar->deactivate();
    abar->deactivate();
    wbar->deactivate();
    bar->deactivate();
    bc->deactivate();
    ix->deactivate();

    delete wawbar;
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
    InOutAlias *wawiar = new InOutAlias(NULL, "wawiar", wiar);
    int32_t it1, it2;
    ix->activate();
    ic->activate();
    iar->activate();
    wiar->activate();
    ix->setValue((int32_t) 1);
    assertTrue_1(!aiar->isActive());
    assertTrue_1(!awiar->isActive());
    assertTrue_1(!wawiar->isActive());
    assertTrue_1(!aiar->isKnown());
    assertTrue_1(!awiar->isKnown());
    assertTrue_1(!wawiar->isKnown());

    aiar->activate();
    awiar->activate();
    wawiar->activate();
    assertTrue_1(aiar->isActive());
    assertTrue_1(awiar->isActive());
    assertTrue_1(wawiar->isActive());
    assertTrue_1(aiar->isKnown());
    assertTrue_1(awiar->isKnown());
    assertTrue_1(wawiar->isKnown());
    assertTrue_1(iar->getValue(it1));
    assertTrue_1(it1 == 2);
    assertTrue_1(aiar->getValue(it2));
    assertTrue_1(it1 == it2);
    assertTrue_1(awiar->getValue(it2));
    assertTrue_1(it1 == it2);
    assertTrue_1(wawiar->getValue(it2));
    assertTrue_1(it1 == it2);

    ix->setValue((int32_t) 0);
    assertTrue_1(iar->getValue(it1));
    assertTrue_1(it1 == 3);
    assertTrue_1(aiar->getValue(it2));
    assertTrue_1(it1 == it2);
    assertTrue_1(awiar->getValue(it2));
    assertTrue_1(it1 == it2);
    assertTrue_1(wawiar->getValue(it2));
    assertTrue_1(it1 == it2);

    wawiar->setValue((int32_t) 69);
    assertTrue_1(iar->getValue(it1));
    assertTrue_1(it1 == 69);
    assertTrue_1(aiar->getValue(it2));
    assertTrue_1(it1 == it2);
    assertTrue_1(awiar->getValue(it2));
    assertTrue_1(it1 == it2);
    assertTrue_1(wawiar->getValue(it2));
    assertTrue_1(it1 == it2);

    wawiar->deactivate();
    awiar->deactivate();
    aiar->deactivate();
    wiar->deactivate();
    iar->deactivate();
    ic->deactivate();
    ix->deactivate();

    delete wawiar;
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
    InOutAlias *wawdar = new InOutAlias(NULL, "wawdar", wdar);
    double dt1, dt2;
    ix->activate();
    dc->activate();
    dar->activate();
    wdar->activate();
    ix->setValue((int32_t) 1);
    assertTrue_1(!adar->isActive());
    assertTrue_1(!awdar->isActive());
    assertTrue_1(!wawdar->isActive());
    assertTrue_1(!adar->isKnown());
    assertTrue_1(!awdar->isKnown());
    assertTrue_1(!wawdar->isKnown());

    adar->activate();
    awdar->activate();
    wawdar->activate();
    assertTrue_1(adar->isActive());
    assertTrue_1(awdar->isActive());
    assertTrue_1(wawdar->isActive());
    assertTrue_1(adar->isKnown());
    assertTrue_1(awdar->isKnown());
    assertTrue_1(wawdar->isKnown());
    assertTrue_1(dar->getValue(dt1));
    assertTrue_1(dt1 == 6);
    assertTrue_1(adar->getValue(dt2));
    assertTrue_1(dt1 == dt2);
    assertTrue_1(awdar->getValue(dt2));
    assertTrue_1(dt1 == dt2);
    assertTrue_1(wawdar->getValue(dt2));
    assertTrue_1(dt1 == dt2);

    ix->setValue((int32_t) 0);
    assertTrue_1(dar->getValue(dt1));
    assertTrue_1(dt1 == 7);
    assertTrue_1(adar->getValue(dt2));
    assertTrue_1(dt1 == dt2);
    assertTrue_1(awdar->getValue(dt2));
    assertTrue_1(dt1 == dt2);
    assertTrue_1(wawdar->getValue(dt2));
    assertTrue_1(dt1 == dt2);

    wawdar->setValue(-3.5);
    assertTrue_1(dar->getValue(dt1));
    assertTrue_1(dt1 == -3.5);
    assertTrue_1(adar->getValue(dt2));
    assertTrue_1(dt1 == dt2);
    assertTrue_1(awdar->getValue(dt2));
    assertTrue_1(dt1 == dt2);
    assertTrue_1(wawdar->getValue(dt2));
    assertTrue_1(dt1 == dt2);

    wawdar->deactivate();
    awdar->deactivate();
    adar->deactivate();
    wdar->deactivate();
    dar->deactivate();
    dc->deactivate();
    ix->deactivate();

    delete wawdar;
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
    InOutAlias *wawsar = new InOutAlias(NULL, "wawsar", wsar, false);
    std::string st1, st2;
    std::string const *stp1, *stp2;
    ix->activate();
    sc->activate();
    sar->activate();
    wsar->activate();
    ix->setValue((int32_t) 1);
    assertTrue_1(!asar->isActive());
    assertTrue_1(!awsar->isActive());
    assertTrue_1(!wawsar->isActive());
    assertTrue_1(!asar->isKnown());
    assertTrue_1(!awsar->isKnown());
    assertTrue_1(!wawsar->isKnown());

    asar->activate();
    awsar->activate();
    wawsar->activate();
    assertTrue_1(asar->isActive());
    assertTrue_1(awsar->isActive());
    assertTrue_1(wawsar->isActive());
    assertTrue_1(asar->isKnown());
    assertTrue_1(awsar->isKnown());
    assertTrue_1(wawsar->isKnown());
    assertTrue_1(sar->getValue(st1));
    assertTrue_1(st1 == std::string("one"));
    assertTrue_1(asar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(awsar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(wawsar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(sar->getValuePointer(stp1));
    assertTrue_1(*stp1 == std::string("one"));
    assertTrue_1(asar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);
    assertTrue_1(awsar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);
    assertTrue_1(wawsar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);

    ix->setValue((int32_t) 0);
    assertTrue_1(sar->getValue(st1));
    assertTrue_1(st1 == std::string("zero"));
    assertTrue_1(asar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(awsar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(wawsar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(sar->getValuePointer(stp1));
    assertTrue_1(*stp1 == std::string("zero"));
    assertTrue_1(asar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);
    assertTrue_1(awsar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);
    assertTrue_1(wawsar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);

    wawsar->setValue(std::string("minus one"));
    assertTrue_1(sar->getValue(st1));
    assertTrue_1(st1 == std::string("minus one"));
    assertTrue_1(asar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(awsar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(wawsar->getValue(st2));
    assertTrue_1(st1 == st2);
    assertTrue_1(sar->getValuePointer(stp1));
    assertTrue_1(*stp1 == std::string("minus one"));
    assertTrue_1(asar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);
    assertTrue_1(awsar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);
    assertTrue_1(wawsar->getValuePointer(stp2));
    assertTrue_1(*stp1 == *stp2);

    wawsar->deactivate();
    awsar->deactivate();
    asar->deactivate();
    wsar->deactivate();
    sar->deactivate();
    sc->deactivate();
    ix->deactivate();

    delete wawsar;
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
  InOutAlias *watree = new InOutAlias(NULL, "watree", tree);
  bool treeChanged, atreeChanged, watreeChanged;
  TrivialListener *treeListener = new TrivialListener(treeChanged);
  TrivialListener *atreeListener = new TrivialListener(atreeChanged);
  TrivialListener *watreeListener = new TrivialListener(watreeChanged);
  tree->addListener(treeListener);
  atree->addListener(atreeListener);
  watree->addListener(watreeListener);
  treeChanged = atreeChanged = watreeChanged = false;

  // Test that notifying while inactive does nothing
  atree->notifyChanged(tree);
  assertTrue_1(!atreeChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!watreeChanged);

  watree->notifyChanged(tree);
  assertTrue_1(!watreeChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);

  // Activate
  watree->activate();
  atree->activate();
  tree->activate();

  // Test that initial activation causes propagation
  assertTrue_1(treeChanged);
  assertTrue_1(atreeChanged);
  assertTrue_1(watreeChanged);

  // Test that notifying alias doesn't notify origin
  treeChanged = atreeChanged = watreeChanged = false;

  atree->notifyChanged(tree);
  assertTrue_1(atreeChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!watreeChanged);
  atreeChanged = false;

  watree->notifyChanged(tree);
  assertTrue_1(watreeChanged);
  assertTrue_1(!treeChanged);
  assertTrue_1(!atreeChanged);
  watreeChanged = false;


  // Test change propagation from origin to/through alias
  tree->notifyChanged(&dummy);
  assertTrue_1(treeChanged);
  assertTrue_1(atreeChanged);
  assertTrue_1(watreeChanged);
  treeChanged = atreeChanged = watreeChanged = false;

  // Assign to writable alias, ensure that the change propagates
  watree->setValue((int32_t) 2);
  int32_t temp;
  assertTrue_1(tree->getValue(temp));
  assertTrue_1(temp == 2);
  assertTrue_1(atree->getValue(temp));
  assertTrue_1(temp == 2);
  assertTrue_1(watree->getValue(temp));
  assertTrue_1(temp == 2);
  assertTrue_1(treeChanged);
  assertTrue_1(atreeChanged);
  assertTrue_1(watreeChanged);
  treeChanged = atreeChanged = watreeChanged = false;

  // Clean up
  tree->removeListener(treeListener);
  atree->removeListener(atreeListener);
  watree->removeListener(watreeListener);

  delete watreeListener;
  delete atreeListener;
  delete treeListener;
  delete watree;
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
  InOutAlias *waary = new InOutAlias(NULL, "waary", ary);
  bool aryChanged, aaryChanged, waaryChanged;
  TrivialListener *aryListener = new TrivialListener(aryChanged);
  TrivialListener *aaryListener = new TrivialListener(aaryChanged);
  TrivialListener *waaryListener = new TrivialListener (waaryChanged);
  ary->addListener(aryListener);
  aary->addListener(aaryListener);
  waary->addListener(waaryListener);
  aryChanged = aaryChanged = waaryChanged = false;

  // Test that notifying while inactive does nothing
  aary->notifyChanged(ary);
  assertTrue_1(!aaryChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!waaryChanged);
  waary->notifyChanged(ary);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);

  // Activate
  waary->activate();
  aary->activate();
  ary->activate();

  // Test that initial activation causes propagation
  assertTrue_1(aryChanged);
  assertTrue_1(aaryChanged);
  assertTrue_1(waaryChanged);

  // Test that notifying alias doesn't notify origin
  aryChanged = aaryChanged = waaryChanged = false;
  aary->notifyChanged(ary);
  assertTrue_1(aaryChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!waaryChanged);
  aaryChanged = false;

  waary->notifyChanged(ary);
  assertTrue_1(waaryChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!aaryChanged);
  waaryChanged = false;

  // Test change propagation from origin to/through alias
  ary->notifyChanged(&dummy);
  assertTrue_1(aryChanged);
  assertTrue_1(aaryChanged);
  assertTrue_1(waaryChanged);
  aryChanged = aaryChanged = waaryChanged = false;

  // Assign to writable aliases, ensure that the change propagates
  std::vector<int32_t> vi2(4);
  vi2[0] = 3;
  vi2[1] = 6;
  vi2[2] = 9;
  vi2[3] = 12;
  waary->setValue(vi2);
  IntegerArray const *atemp;
  std::vector<int32_t> const *pvi;
  assertTrue_1(ary->getValuePointer(atemp));
  assertTrue_1(atemp != NULL);
  atemp->getContentsVector(pvi);
  assertTrue_1(pvi != NULL);
  assertTrue_1(vi2 == *pvi);
  assertTrue_1(aary->getValuePointer(atemp));
  assertTrue_1(atemp != NULL);
  atemp->getContentsVector(pvi);
  assertTrue_1(pvi != NULL);
  assertTrue_1(vi2 == *pvi);
  assertTrue_1(waary->getValuePointer(atemp));
  assertTrue_1(atemp != NULL);
  atemp->getContentsVector(pvi);
  assertTrue_1(pvi != NULL);
  assertTrue_1(vi2 == *pvi);
  assertTrue_1(aryChanged);
  assertTrue_1(aaryChanged);
  assertTrue_1(waaryChanged);
  aryChanged = aaryChanged = waaryChanged = false;

  // Clean up
  ary->removeListener(aryListener);
  aary->removeListener(aaryListener);
  waary->removeListener(waaryListener);

  delete waaryListener;
  delete aaryListener;
  delete aryListener;
  delete waary;
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
  InOutAlias *waary = new InOutAlias(NULL, "waary", ary);
  MutableArrayReference *ref = new MutableArrayReference(waary, atree);
  Alias *aref = new Alias(NULL, "aref", ref);
  InOutAlias *waref = new InOutAlias(NULL, "waref", ref);

  bool aryChanged, waaryChanged;
  bool atreeChanged;
  bool refChanged, arefChanged, warefChanged;

  TrivialListener *aryListener = new TrivialListener(aryChanged);
  TrivialListener *waaryListener = new TrivialListener(waaryChanged);
  TrivialListener *atreeListener = new TrivialListener(atreeChanged);
  TrivialListener *refListener = new TrivialListener(refChanged);
  TrivialListener *arefListener = new TrivialListener(arefChanged);
  TrivialListener *warefListener = new TrivialListener(warefChanged);

  ary->addListener(aryListener);
  waary->addListener(waaryListener);
  atree->addListener(atreeListener);
  ref->addListener(refListener);
  aref->addListener(arefListener);
  waref->addListener(warefListener);

  aryChanged = waaryChanged = atreeChanged = false;
  refChanged = arefChanged = warefChanged = false;

  // Test that notifying while inactive does nothing
  atree->notifyChanged(tree);
  assertTrue_1(!atreeChanged);

  waary->notifyChanged(ary);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!aryChanged);

  aref->notifyChanged(ref);
  assertTrue_1(!arefChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!warefChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!atreeChanged);

  waref->notifyChanged(ref);
  assertTrue_1(!warefChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!arefChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!atreeChanged);

  // Activate all
  waref->activate();
  aref->activate();
  ref->activate();
  waary->activate();
  ary->activate();
  atree->activate();
  tree->activate();

  // Test that initial activation causes propagation
  assertTrue_1(aryChanged);
  assertTrue_1(refChanged);
  assertTrue_1(atreeChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(waaryChanged);
  assertTrue_1(warefChanged);

  // Test that notifying alias doesn't notify origin
  aryChanged = waaryChanged = atreeChanged = false;
  refChanged = arefChanged = warefChanged = false;

  atree->notifyChanged(tree);
  assertTrue_1(atreeChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(refChanged);   // these depend on atree
  assertTrue_1(arefChanged);  //
  assertTrue_1(warefChanged); //
  atreeChanged = refChanged = arefChanged = warefChanged = false;

  waary->notifyChanged(ary);
  assertTrue_1(waaryChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(refChanged);   // these depend on waary
  assertTrue_1(arefChanged);  //
  assertTrue_1(warefChanged); //
  waaryChanged = refChanged = arefChanged = warefChanged = false;

  aref->notifyChanged(ref);
  assertTrue_1(arefChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!warefChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!atreeChanged);
  arefChanged = false;

  waref->notifyChanged(ref);
  assertTrue_1(warefChanged);
  assertTrue_1(!refChanged);
  assertTrue_1(!arefChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!atreeChanged);
  warefChanged = false;

  // Test change propagation from origin to/through alias
  ary->notifyChanged(&dummy);
  assertTrue_1(aryChanged);
  assertTrue_1(waaryChanged);
  assertTrue_1(!atreeChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);
  aryChanged = waaryChanged = false;
  refChanged = arefChanged = warefChanged = false;

  tree->notifyChanged(&dummy);
  assertTrue_1(atreeChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);
  atreeChanged = refChanged = arefChanged = warefChanged = false;

  ref->notifyChanged(&dummy);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);
  assertTrue_1(!aryChanged);
  assertTrue_1(!waaryChanged);
  assertTrue_1(!atreeChanged);
  refChanged = arefChanged = warefChanged = false;

  // Assign to writable aliases, ensure that the change propagates
  {
    std::vector<int32_t> vi2(4);
    vi2[0] = 3;
    vi2[1] = 6;
    vi2[2] = 9;
    vi2[3] = 12;
    waary->setValue(vi2);
    IntegerArray const *atemp;
    std::vector<int32_t> const *pvi;
    assertTrue_1(ary->getValuePointer(atemp));
    assertTrue_1(atemp != NULL);
    atemp->getContentsVector(pvi);
    assertTrue_1(pvi != NULL);
    assertTrue_1(vi2 == *pvi);
    assertTrue_1(waary->getValuePointer(atemp));
    assertTrue_1(atemp != NULL);
    atemp->getContentsVector(pvi);
    assertTrue_1(pvi != NULL);
    assertTrue_1(vi2 == *pvi);
    assertTrue_1(!atreeChanged);
    assertTrue_1(aryChanged);
    assertTrue_1(waaryChanged);
    assertTrue_1(refChanged);
    assertTrue_1(arefChanged);
    assertTrue_1(warefChanged);
    aryChanged = waaryChanged = false;
    refChanged = arefChanged = warefChanged = false;
  }

  {
    IntegerConstant *luftballoons = new IntegerConstant(99);
    waref->setValue(luftballoons);
    delete luftballoons;
  }
  int32_t temp;
  assertTrue_1(ref->getValue(temp));
  assertTrue_1(temp == 99);
  assertTrue_1(aref->getValue(temp));
  assertTrue_1(temp == 99);
  assertTrue_1(waref->getValue(temp));
  assertTrue_1(temp == 99);
  assertTrue_1(!atreeChanged);
  assertTrue_1(aryChanged);
  assertTrue_1(waaryChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);
  aryChanged = waaryChanged = false;
  refChanged = arefChanged = warefChanged = false;

  waref->setValue((int32_t) 42);
  assertTrue_1(ref->getValue(temp));
  assertTrue_1(temp == 42);
  assertTrue_1(aref->getValue(temp));
  assertTrue_1(temp == 42);
  assertTrue_1(waref->getValue(temp));
  assertTrue_1(temp == 42);
  assertTrue_1(!atreeChanged);
  assertTrue_1(aryChanged);
  assertTrue_1(waaryChanged);
  assertTrue_1(refChanged);
  assertTrue_1(arefChanged);
  assertTrue_1(warefChanged);

  // Clean up
  ary->removeListener(aryListener);
  waary->removeListener(waaryListener);
  atree->removeListener(atreeListener);
  ref->removeListener(refListener);
  aref->removeListener(arefListener);
  waref->removeListener(warefListener);

  delete warefListener;
  delete arefListener;
  delete refListener;
  delete atreeListener;
  delete waaryListener;
  delete aryListener;

  delete waref;
  delete aref;
  delete ref;
  delete waary;
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
