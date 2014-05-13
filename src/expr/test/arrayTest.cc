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

#include "ArrayConstant.hh"
#include "ArrayVariable.hh" // includes UserVariable.hh
#include "TestSupport.hh"
#include "TrivialListener.hh"

using namespace PLEXIL;

static bool constantArrayReadTest()
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

  std::vector<bool> const        *pknown = NULL;
  std::vector<bool> const        *pvb = NULL;
  std::vector<int32_t> const     *pvi = NULL;
  std::vector<double> const      *pvd = NULL;
  std::vector<std::string> const *pvs = NULL;

  // Constants are always active
  assertTrue_1(bc.isKnown());
  assertTrue_1(bc.getArrayContents(pvb, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vb.size());
  // TODO: check all known
  assertTrue_1(pvb != NULL);
  assertTrue_1(vb == *pvb);

  pknown = NULL;
  assertTrue_1(ic.isKnown());
  assertTrue_1(ic.getArrayContents(pvi, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vi.size());
  // TODO: check all known
  assertTrue_1(pvi != NULL);
  assertTrue_1(vi == *pvi);

  pknown = NULL;
  assertTrue_1(dc.isKnown());
  assertTrue_1(dc.getArrayContents(pvd, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vd.size());
  // TODO: check all known
  assertTrue_1(pvd != NULL);
  assertTrue_1(vd == *pvd);

  pknown = NULL;
  assertTrue_1(sc.isKnown());
  assertTrue_1(sc.getArrayContents(pvs, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vs.size());
  // TODO: check all known
  assertTrue_1(pvs != NULL);
  assertTrue_1(vs == *pvs);

  return true;
}

static bool uninitializedVariableTest()
{
  BooleanArrayVariable vuba;
  IntegerArrayVariable vuia;
  RealArrayVariable vuda;
  StringArrayVariable vusa;

  // Test that they are assignable and not constant
  assertTrue_1(vuba.isAssignable());
  assertTrue_1(vuia.isAssignable());
  assertTrue_1(vuda.isAssignable());
  assertTrue_1(vusa.isAssignable());

  assertTrue_1(!vuba.isConstant());
  assertTrue_1(!vuia.isConstant());
  assertTrue_1(!vuda.isConstant());
  assertTrue_1(!vusa.isConstant());

  // Test that they are created inactive
  assertTrue_1(!vuba.isActive());
  assertTrue_1(!vuia.isActive());
  assertTrue_1(!vuda.isActive());
  assertTrue_1(!vusa.isActive());

  // Test that they are unknown while inactive
  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());

  std::vector<bool> const *pknown = NULL;
  std::vector<bool> const *pfooba = NULL;
  std::vector<int32_t> const *pfooia = NULL;
  std::vector<double> const *pfooda = NULL;
  std::vector<std::string> const *pfoosa = NULL;

  assertTrue_1(!vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooba == NULL);

  assertTrue_1(!vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooia == NULL);

  assertTrue_1(!vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooda == NULL);

  assertTrue_1(!vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfoosa == NULL);

  // Activate and confirm they are still unknown
  vuba.activate();
  vuia.activate();
  vuda.activate();
  vusa.activate();

  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());

  assertTrue_1(!vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooba == NULL);

  assertTrue_1(!vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooia == NULL);

  assertTrue_1(!vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooda == NULL);

  assertTrue_1(!vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfoosa == NULL);

  // Assign and check result
  std::vector<bool> vb(1, true);
  std::vector<int32_t> vi(1, 42);
  std::vector<double> vd(1, 3.1416);
  std::vector<std::string> vs(1, std::string("Foo"));
  vuba.setValue(vb);
  vuia.setValue(vi);
  vuda.setValue(vd);
  vusa.setValue(vs);

  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());

  assertTrue_1(vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vb.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooba != NULL);
  assertTrue_1(vb == *pfooba);

  pknown = NULL;
  assertTrue_1(vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vi.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooia != NULL);
  assertTrue_1(vi == *pfooia);

  pknown = NULL;
  assertTrue_1(vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vd.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooda != NULL);
  assertTrue_1(vd == *pfooda);

  pknown = NULL;
  assertTrue_1(vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vs.size());
  // TODO: check all known flags are true
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(vs == *pfoosa);

  pknown = NULL;

  // Reset and check that value is now unknown
  // Can't reset while active
  vuba.deactivate();
  vuia.deactivate();
  vuda.deactivate();
  vusa.deactivate();

  vuba.reset();
  vuia.reset();
  vuda.reset();
  vusa.reset();

  vuba.activate();
  vuia.activate();
  vuda.activate();
  vusa.activate();

  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(!vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooba == NULL);

  assertTrue_1(!vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooia == NULL);

  assertTrue_1(!vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooda == NULL);

  assertTrue_1(!vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfoosa == NULL);

  // Set value and check result
  vuba.setValue(vb);
  vuia.setValue(vi);
  vuda.setValue(vd);
  vusa.setValue(vs);

  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());

  assertTrue_1(vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vb.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooba != NULL);
  assertTrue_1(vb == *pfooba);

  pknown = NULL;
  assertTrue_1(vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vi.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooia != NULL);
  assertTrue_1(vi == *pfooia);

  pknown = NULL;
  assertTrue_1(vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vd.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooda != NULL);
  assertTrue_1(vd == *pfooda);

  pknown = NULL;
  assertTrue_1(vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vs.size());
  // TODO: check all known flags are true
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(vs == *pfoosa);

  pknown = NULL;

  // Set values and check that they changed
  std::vector<bool> vab(2, false);
  std::vector<int32_t> vai(2, 69);
  std::vector<double> vad(2, 2.718);
  std::vector<std::string> vas(2, std::string("bar"));

  vuba.setValue(vab);
  vuia.setValue(vai);
  vuda.setValue(vad);
  vusa.setValue(vas);

  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vab.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooba != NULL);
  assertTrue_1(vab == *pfooba);

  pknown = NULL;
  assertTrue_1(vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vai.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooia != NULL);
  assertTrue_1(vai == *pfooia);

  pknown = NULL;
  assertTrue_1(vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vad.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooda != NULL);
  assertTrue_1(vad == *pfooda);

  pknown = NULL;
  assertTrue_1(vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= vas.size());
  // TODO: check all known flags are true
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(vas == *pfoosa);

  return true;
}

static bool testVariableInitialValue()
{
  std::vector<bool> bv(2, true);
  std::vector<int32_t> iv(2, 56);
  std::vector<double> dv(2, 1.414);
  std::vector<std::string> sv(2, std::string("yahoo"));

  BooleanArrayVariable vba(bv);
  IntegerArrayVariable via(iv);
  RealArrayVariable vda(dv);
  StringArrayVariable vsa(sv);

  // Test that they are assignable and not constant
  assertTrue_1(vba.isAssignable());
  assertTrue_1(via.isAssignable());
  assertTrue_1(vda.isAssignable());
  assertTrue_1(vsa.isAssignable());

  assertTrue_1(!vba.isConstant());
  assertTrue_1(!via.isConstant());
  assertTrue_1(!vda.isConstant());
  assertTrue_1(!vsa.isConstant());

  // Test that they are created inactive
  assertTrue_1(!vba.isActive());
  assertTrue_1(!via.isActive());
  assertTrue_1(!vda.isActive());
  assertTrue_1(!vsa.isActive());

  // Test that they are unknown while inactive
  assertTrue_1(!vba.isKnown());
  assertTrue_1(!via.isKnown());
  assertTrue_1(!vda.isKnown());
  assertTrue_1(!vsa.isKnown());

  // Activate and confirm they are known
  vba.activate();
  via.activate();
  vda.activate();
  vsa.activate();

  assertTrue_1(vba.isKnown());
  assertTrue_1(via.isKnown());
  assertTrue_1(vda.isKnown());
  assertTrue_1(vsa.isKnown());

  // Check values
  std::vector<bool> const *pknown = NULL;
  std::vector<bool> const *pfooba = NULL;
  std::vector<int32_t> const *pfooia = NULL;
  std::vector<double> const *pfooda = NULL;
  std::vector<std::string> const *pfoosa = NULL;

  assertTrue_1(vba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= bv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooba != NULL);
  assertTrue_1(bv == *pfooba);

  pknown = NULL;
  assertTrue_1(via.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= iv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooia != NULL);
  assertTrue_1(iv == *pfooia);

  pknown = NULL;
  assertTrue_1(vda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= dv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooda != NULL);
  assertTrue_1(dv == *pfooda);

  pknown = NULL;
  assertTrue_1(vsa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= sv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(sv == *pfoosa);

  // Set unknown
  vba.setUnknown();
  via.setUnknown();
  vda.setUnknown();
  vsa.setUnknown();

  // Confirm that they are now unknown
  assertTrue_1(!vba.isKnown());
  assertTrue_1(!via.isKnown());
  assertTrue_1(!vda.isKnown());
  assertTrue_1(!vsa.isKnown());

  pknown = NULL;
  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(!vba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooba == NULL);

  assertTrue_1(!via.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooia == NULL);

  assertTrue_1(!vda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooda == NULL);

  assertTrue_1(!vsa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfoosa == NULL);

  // Reset and confirm unknown
  vba.deactivate();
  via.deactivate();
  vda.deactivate();
  vsa.deactivate();

  vba.reset();
  via.reset();
  vda.reset();
  vsa.reset();

  assertTrue_1(!vba.isKnown());
  assertTrue_1(!via.isKnown());
  assertTrue_1(!vda.isKnown());
  assertTrue_1(!vsa.isKnown());

  // Activate and check that initial value is restored
  vba.activate();
  via.activate();
  vda.activate();
  vsa.activate();

  assertTrue_1(vba.isKnown());
  assertTrue_1(via.isKnown());
  assertTrue_1(vda.isKnown());
  assertTrue_1(vsa.isKnown());

  assertTrue_1(vba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= bv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooba != NULL);
  assertTrue_1(bv == *pfooba);

  pknown = NULL;
  assertTrue_1(via.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= iv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooia != NULL);
  assertTrue_1(iv == *pfooia);

  pknown = NULL;
  assertTrue_1(vda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= dv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooda != NULL);
  assertTrue_1(dv == *pfooda);

  pknown = NULL;
  assertTrue_1(vsa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= sv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(sv == *pfoosa);

  // Set values and check
  std::vector<bool> bv2(1, true);
  std::vector<int32_t> iv2(1, 65);
  std::vector<double> dv2(1, 3.162);
  std::vector<std::string> sv2(1, std::string("yoohoo"));

  vba.setValue(bv2);
  via.setValue(iv2);
  vda.setValue(dv2);
  vsa.setValue(sv2);

  pknown = NULL;
  assertTrue_1(vba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= bv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooba != NULL);
  assertTrue_1(bv2 == *pfooba);

  pknown = NULL;
  assertTrue_1(via.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= iv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooia != NULL);
  assertTrue_1(iv2 == *pfooia);

  pknown = NULL;
  assertTrue_1(vda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= dv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooda != NULL);
  assertTrue_1(dv2 == *pfooda);

  pknown = NULL;
  assertTrue_1(vsa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= sv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(sv2 == *pfoosa);

  return true;
}

// Confirm that initializer expression is only invoked
// on an inactive to active transition.
static bool testVariableInitializers()
{
  // TODO
  return true;
}

// Confirm that we can do all the read-only operations
// through a pointer of type ExpressionId.

// *** TODO: merge in array constant tests ***
static bool testExpressionId()
{
  std::vector<bool> bv(2, true);
  std::vector<int32_t> iv(2, 56);
  std::vector<double> dv(2, 1.414);
  std::vector<std::string> sv(2, std::string("yahoo"));

  BooleanArrayVariable vba(bv);
  IntegerArrayVariable via(iv);
  RealArrayVariable vda(dv);
  StringArrayVariable vsa(sv);

  ExpressionId eba(vba.getId());
  ExpressionId eia(via.getId());
  ExpressionId eda(vda.getId());
  ExpressionId esa(vsa.getId());

  // Test that they are assignable and not constant
  assertTrue_1(eba->isAssignable());
  assertTrue_1(eia->isAssignable());
  assertTrue_1(eda->isAssignable());
  assertTrue_1(esa->isAssignable());

  assertTrue_1(!eba->isConstant());
  assertTrue_1(!eia->isConstant());
  assertTrue_1(!eda->isConstant());
  assertTrue_1(!esa->isConstant());

  // Test that they are created inactive
  assertTrue_1(!eba->isActive());
  assertTrue_1(!eia->isActive());
  assertTrue_1(!eda->isActive());
  assertTrue_1(!esa->isActive());

  // Test that they are unknown while inactive
  assertTrue_1(!eba->isKnown());
  assertTrue_1(!eia->isKnown());
  assertTrue_1(!eda->isKnown());
  assertTrue_1(!esa->isKnown());

  // Activate and confirm they are known
  eba->activate();
  eia->activate();
  eda->activate();
  esa->activate();

  // Check values
  std::vector<bool> const *pknown = NULL;
  std::vector<bool> const *pfooba = NULL;
  std::vector<int32_t> const *pfooia = NULL;
  std::vector<double> const *pfooda = NULL;
  std::vector<std::string> const *pfoosa = NULL;

  assertTrue_1(eba->getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= bv.size());
  // TODO: check that all known flags are set
  assertTrue_1(pfooba != NULL);
  assertTrue_1(bv == *pfooba);
  assertTrue_1(eba->isKnown());

  pknown = NULL;
  assertTrue_1(eia->getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >=iv.size());
  // TODO: check that all known flags are set
  assertTrue_1(pfooia != NULL);
  assertTrue_1(iv == *pfooia);
  assertTrue_1(eia->isKnown());

  pknown = NULL;
  assertTrue_1(eda->getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= dv.size());
  // TODO: check that all known flags are set
  assertTrue_1(pfooda != NULL);
  assertTrue_1(dv == *pfooda);
  assertTrue_1(eda->isKnown());

  pknown = NULL;
  assertTrue_1(esa->getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= sv.size());
  // TODO: check that all known flags are set
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(sv == *pfoosa);
  assertTrue_1(esa->isKnown());

  return true;
}

static bool testVariableSavedValue()
{
  BooleanArrayVariable vuba;
  IntegerArrayVariable vuia;
  RealArrayVariable vuda;
  StringArrayVariable vusa;

  std::vector<bool> const *pknown = NULL;
  std::vector<bool> const *pfooba = NULL;
  std::vector<int32_t> const *pfooia = NULL;
  std::vector<double> const *pfooda = NULL;
  std::vector<std::string> const *pfoosa = NULL;

  // Activate
  vuba.activate();
  vuia.activate();
  vuda.activate();
  vusa.activate();

  // Confirm that they are unknown
  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());

  assertTrue_1(!vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooba == NULL);

  assertTrue_1(!vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooia == NULL);

  assertTrue_1(!vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooda == NULL);

  assertTrue_1(!vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfoosa == NULL);

  // Save current value (should be unknown)
  vuba.saveCurrentValue();
  vuia.saveCurrentValue();
  vuda.saveCurrentValue();
  vusa.saveCurrentValue();

  // Assign and check result
  std::vector<bool> bv(2, true);
  std::vector<int32_t> iv(2, 56);
  std::vector<double> dv(2, 1.414);
  std::vector<std::string> sv(2, std::string("yahoo"));

  vuba.setValue(bv);
  vuia.setValue(iv);
  vuda.setValue(dv);
  vusa.setValue(sv);

  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());

  assertTrue_1(vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= bv.size());
  assertTrue_1(pfooba != NULL);
  assertTrue_1(bv == *pfooba);

  pknown = NULL;
  assertTrue_1(vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= iv.size());
  assertTrue_1(pfooia != NULL);
  assertTrue_1(iv == *pfooia);

  pknown = NULL;
  assertTrue_1(vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= dv.size());
  assertTrue_1(pfooda != NULL);
  assertTrue_1(dv == *pfooda);

  pknown = NULL;
  assertTrue_1(vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= sv.size());
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(sv == *pfoosa);

  // Restore saved value and check result
  vuba.restoreSavedValue();
  vuia.restoreSavedValue();
  vuda.restoreSavedValue();
  vusa.restoreSavedValue();

  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());

  pknown = NULL;
  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(!vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooba == NULL);

  assertTrue_1(!vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooia == NULL);

  assertTrue_1(!vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooda == NULL);

  assertTrue_1(!vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfoosa == NULL);

  // Assign again
  vuba.setValue(bv);
  vuia.setValue(iv);
  vuda.setValue(dv);
  vusa.setValue(sv);

  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());

  assertTrue_1(vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= bv.size());
  assertTrue_1(pfooba != NULL);
  assertTrue_1(bv == *pfooba);

  pknown = NULL;
  assertTrue_1(vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= iv.size());
  assertTrue_1(pfooia != NULL);
  assertTrue_1(iv == *pfooia);

  pknown = NULL;
  assertTrue_1(vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= dv.size());
  assertTrue_1(pfooda != NULL);
  assertTrue_1(dv == *pfooda);

  pknown = NULL;
  assertTrue_1(vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= sv.size());
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(sv == *pfoosa);

  // Save current value
  vuba.saveCurrentValue();
  vuia.saveCurrentValue();
  vuda.saveCurrentValue();
  vusa.saveCurrentValue();

  // Make unknown
  vuba.setUnknown();
  vuia.setUnknown();
  vuda.setUnknown();
  vusa.setUnknown();

  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());

  pknown = NULL;
  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(!vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooba == NULL);

  assertTrue_1(!vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooia == NULL);

  assertTrue_1(!vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooda == NULL);

  assertTrue_1(!vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfoosa == NULL);

  // Restore saved and check that it has returned
  vuba.restoreSavedValue();
  vuia.restoreSavedValue();
  vuda.restoreSavedValue();
  vusa.restoreSavedValue();

  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());

  assertTrue_1(vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= bv.size());
  assertTrue_1(pfooba != NULL);
  assertTrue_1(bv == *pfooba);

  pknown = NULL;
  assertTrue_1(vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= iv.size());
  assertTrue_1(pfooia != NULL);
  assertTrue_1(iv == *pfooia);

  pknown = NULL;
  assertTrue_1(vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= dv.size());
  assertTrue_1(pfooda != NULL);
  assertTrue_1(dv == *pfooda);

  pknown = NULL;
  assertTrue_1(vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= sv.size());
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(sv == *pfoosa);

  // Reset
  vuba.deactivate();
  vuia.deactivate();
  vuda.deactivate();
  vusa.deactivate();

  vuba.reset();
  vuia.reset();
  vuda.reset();
  vusa.reset();

  vuba.activate();
  vuia.activate();
  vuda.activate();
  vusa.activate();

  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());

  pknown = NULL;
  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(!vuba.getArrayContents(pfooba, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooba == NULL);

  assertTrue_1(!vuia.getArrayContents(pfooia, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooia == NULL);

  assertTrue_1(!vuda.getArrayContents(pfooda, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooda == NULL);

  assertTrue_1(!vusa.getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfoosa == NULL);

  return true;
}

// Confirm that we can do all the Assignable operations
// through a pointer of type AssignableId.
static bool testAssignableId()
{
  std::vector<bool> bv(2, true);
  std::vector<int32_t> iv(2, 56);
  std::vector<double> dv(2, 1.414);
  std::vector<std::string> sv(2, std::string("yahoo"));

  BooleanArrayVariable vba(bv);
  IntegerArrayVariable via(iv);
  RealArrayVariable vda(dv);
  StringArrayVariable vsa(sv);

  AssignableId eba(vba.getAssignableId());
  AssignableId eia(via.getAssignableId());
  AssignableId eda(vda.getAssignableId());
  AssignableId esa(vsa.getAssignableId());

  // Confirm that we actually got IDs
  assertTrue_1(eba.isId());
  assertTrue_1(eia.isId());
  assertTrue_1(eda.isId());
  assertTrue_1(esa.isId());

  // Test that they are assignable and not constant
  assertTrue_1(eba->isAssignable());
  assertTrue_1(eia->isAssignable());
  assertTrue_1(eda->isAssignable());
  assertTrue_1(esa->isAssignable());

  assertTrue_1(!eba->isConstant());
  assertTrue_1(!eia->isConstant());
  assertTrue_1(!eda->isConstant());
  assertTrue_1(!esa->isConstant());

  // Test that they are created inactive
  assertTrue_1(!eba->isActive());
  assertTrue_1(!eia->isActive());
  assertTrue_1(!eda->isActive());
  assertTrue_1(!esa->isActive());

  // Test that values are unknown while inactive
  assertTrue_1(!eba->isKnown());
  assertTrue_1(!eia->isKnown());
  assertTrue_1(!eda->isKnown());
  assertTrue_1(!esa->isKnown());

  // Activate
  eba->activate();
  eia->activate();
  eda->activate();
  esa->activate();

  // Test that they are now known
  assertTrue_1(eba->isKnown());
  assertTrue_1(eia->isKnown());
  assertTrue_1(eda->isKnown());
  assertTrue_1(esa->isKnown());

  // Check values
  std::vector<bool> const *pknown = NULL;
  std::vector<bool> const *pfooba = NULL;
  std::vector<int32_t> const *pfooia = NULL;
  std::vector<double> const *pfooda = NULL;
  std::vector<std::string> const *pfoosa = NULL;

  assertTrue_1(eba->getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= bv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooba != NULL);
  assertTrue_1(bv == *pfooba);

  pknown = NULL;
  assertTrue_1(eia->getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= iv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooia != NULL);
  assertTrue_1(iv == *pfooia);

  pknown = NULL;
  assertTrue_1(eda->getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= dv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooda != NULL);
  assertTrue_1(dv == *pfooda);

  pknown = NULL;
  assertTrue_1(esa->getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= sv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(sv == *pfoosa);

  // Set values

  std::vector<bool> bv2(1, false);
  std::vector<int32_t> iv2(1, 99);
  std::vector<double> dv2(1, 2.718);
  std::vector<std::string> sv2(1, std::string("yeehaw"));

  eba->setValue(bv2);
  eia->setValue(iv2);
  eda->setValue(dv2);
  esa->setValue(sv2);

  assertTrue_1(eba->getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= bv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooba != NULL);
  assertTrue_1(bv2 == *pfooba);

  pknown = NULL;
  assertTrue_1(eia->getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= iv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooia != NULL);
  assertTrue_1(iv2 == *pfooia);

  pknown = NULL;
  assertTrue_1(eda->getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= dv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooda != NULL);
  assertTrue_1(dv2 == *pfooda);

  pknown = NULL;
  assertTrue_1(esa->getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= sv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(sv2 == *pfoosa);

  // Save
  eba->saveCurrentValue();
  eia->saveCurrentValue();
  eda->saveCurrentValue();
  esa->saveCurrentValue();

  // Set unknown
  eba->setUnknown();
  eia->setUnknown();
  eda->setUnknown();
  esa->setUnknown();

  // Test that values are now unknown
  assertTrue_1(!eba->isKnown());
  assertTrue_1(!eia->isKnown());
  assertTrue_1(!eda->isKnown());
  assertTrue_1(!esa->isKnown());

  pknown = NULL;
  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(!eba->getArrayContents(pfooba, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooba == NULL);

  assertTrue_1(!eia->getArrayContents(pfooia, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooia == NULL);

  assertTrue_1(!eda->getArrayContents(pfooda, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfooda == NULL);

  assertTrue_1(!esa->getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown == NULL);
  assertTrue_1(pfoosa == NULL);

  // Restore
  eba->restoreSavedValue();
  eia->restoreSavedValue();
  eda->restoreSavedValue();
  esa->restoreSavedValue();

  // Check that saved values are restored
  assertTrue_1(eba->isKnown());
  assertTrue_1(eia->isKnown());
  assertTrue_1(eda->isKnown());
  assertTrue_1(esa->isKnown());

  assertTrue_1(eba->getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= bv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooba != NULL);
  assertTrue_1(bv2 == *pfooba);

  pknown = NULL;
  assertTrue_1(eia->getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= iv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooia != NULL);
  assertTrue_1(iv2 == *pfooia);

  pknown = NULL;
  assertTrue_1(eda->getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= dv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooda != NULL);
  assertTrue_1(dv2 == *pfooda);

  pknown = NULL;
  assertTrue_1(esa->getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= sv2.size());
  // TODO: check all known flags are true
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(sv2 == *pfoosa);

  // Reset
  eba->deactivate();
  eia->deactivate();
  eda->deactivate();
  esa->deactivate();

  eba->reset();
  eia->reset();
  eda->reset();
  esa->reset();

  eba->activate();
  eia->activate();
  eda->activate();
  esa->activate();

  // Check initial values are restored
  pknown = NULL;
  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(eba->isKnown());
  assertTrue_1(eia->isKnown());
  assertTrue_1(eda->isKnown());
  assertTrue_1(esa->isKnown());

  assertTrue_1(eba->getArrayContents(pfooba, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= bv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooba != NULL);
  assertTrue_1(bv == *pfooba);

  pknown = NULL;
  assertTrue_1(eia->getArrayContents(pfooia, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= iv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooia != NULL);
  assertTrue_1(iv == *pfooia);

  pknown = NULL;
  assertTrue_1(eda->getArrayContents(pfooda, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= dv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfooda != NULL);
  assertTrue_1(dv == *pfooda);

  pknown = NULL;
  assertTrue_1(esa->getArrayContents(pfoosa, pknown));
  assertTrue_1(pknown != NULL);
  assertTrue_1(pknown->size() >= sv.size());
  // TODO: check all known flags are true
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(sv == *pfoosa);

  return true;
}

static bool testVariableNotification()
{
  BooleanArrayVariable vuba;
  IntegerArrayVariable vuia;
  RealArrayVariable vuda;
  StringArrayVariable vusa;

  bool bachanged = false,
    iachanged = false, 
    dachanged = false,
    sachanged = false;

  TrivialListener lba(bachanged);
  TrivialListener lia(iachanged);
  TrivialListener lda(dachanged);
  TrivialListener lsa(sachanged);

  vuba.addListener(lba.getId());
  vuia.addListener(lia.getId());
  vuda.addListener(lda.getId());
  vusa.addListener(lsa.getId());

  // Activation alone should not notify
  vuba.activate();
  vuia.activate();
  vuda.activate();
  vusa.activate();

  assertTrue_1(!bachanged);
  assertTrue_1(!iachanged);
  assertTrue_1(!dachanged);
  assertTrue_1(!sachanged);

  // Reset for uninitialized should NOT notify
  vuba.deactivate();
  vuia.deactivate();
  vuda.deactivate();
  vusa.deactivate();

  vuba.reset();
  vuia.reset();
  vuda.reset();
  vusa.reset();

  vuba.activate();
  vuia.activate();
  vuda.activate();
  vusa.activate();

  assertTrue_1(!bachanged);
  assertTrue_1(!iachanged);
  assertTrue_1(!dachanged);
  assertTrue_1(!sachanged);

  // Assign should notify this time
  std::vector<bool> bv(2, true);
  std::vector<int32_t> iv(2, 56);
  std::vector<double> dv(2, 1.414);
  std::vector<std::string> sv(2, std::string("yahoo"));

  vuba.setValue(bv);
  vuia.setValue(iv);
  vuda.setValue(dv);
  vusa.setValue(sv);

  assertTrue_1(bachanged);
  assertTrue_1(iachanged);
  assertTrue_1(dachanged);
  assertTrue_1(sachanged);

  // Save current value shouldn't notify
  bachanged = iachanged = dachanged = sachanged =
    false;
  vuba.saveCurrentValue();
  vuia.saveCurrentValue();
  vuda.saveCurrentValue();
  vusa.saveCurrentValue();

  assertTrue_1(!bachanged);
  assertTrue_1(!iachanged);
  assertTrue_1(!dachanged);
  assertTrue_1(!sachanged);

  // Restoring same value shouldn't notify
  vuba.restoreSavedValue();
  vuia.restoreSavedValue();
  vuda.restoreSavedValue();
  vusa.restoreSavedValue();

  assertTrue_1(!bachanged);
  assertTrue_1(!iachanged);
  assertTrue_1(!dachanged);
  assertTrue_1(!sachanged);

  // Making unknown should notify
  vuba.setUnknown();
  vuia.setUnknown();
  vuda.setUnknown();
  vusa.setUnknown();

  assertTrue_1(bachanged);
  assertTrue_1(iachanged);
  assertTrue_1(dachanged);
  assertTrue_1(sachanged);

  // Now restoring should notify
  bachanged = iachanged = dachanged = sachanged =
    false;
  vuba.restoreSavedValue();
  vuia.restoreSavedValue();
  vuda.restoreSavedValue();
  vusa.restoreSavedValue();

  assertTrue_1(bachanged);
  assertTrue_1(iachanged);
  assertTrue_1(dachanged);
  assertTrue_1(sachanged);

  // Clean up
  vuba.removeListener(lba.getId());
  vuia.removeListener(lia.getId());
  vuda.removeListener(lda.getId());
  vusa.removeListener(lsa.getId());

  return true;
}

bool arrayTest()
{
  runTest(constantArrayReadTest);
  runTest(uninitializedVariableTest);
  runTest(testVariableInitialValue);
  runTest(testVariableInitializers);
  runTest(testExpressionId);
  runTest(testVariableSavedValue);
  runTest(testAssignableId);
  runTest(testVariableNotification);
  return true;
}
