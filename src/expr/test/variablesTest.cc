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

#include "UserVariable.hh"
#include "TestSupport.hh"
#include "TrivialListener.hh"

using namespace PLEXIL;

//
// Variables
//

static bool testUninitialized()
{
  // Default constructors
  BooleanVariable vub;
  IntegerVariable vui;
  RealVariable vud;
  StringVariable vus;
  BooleanArrayVariable vuba;
  IntegerArrayVariable vuia;
  RealArrayVariable vuda;
  StringArrayVariable vusa;

  // Test that they are assignable and not constant
  assertTrue_1(vub.isAssignable());
  assertTrue_1(vui.isAssignable());
  assertTrue_1(vud.isAssignable());
  assertTrue_1(vus.isAssignable());
  assertTrue_1(vuba.isAssignable());
  assertTrue_1(vuia.isAssignable());
  assertTrue_1(vuda.isAssignable());
  assertTrue_1(vusa.isAssignable());

  assertTrue_1(!vub.isConstant());
  assertTrue_1(!vui.isConstant());
  assertTrue_1(!vud.isConstant());
  assertTrue_1(!vus.isConstant());
  assertTrue_1(!vuba.isConstant());
  assertTrue_1(!vuia.isConstant());
  assertTrue_1(!vuda.isConstant());
  assertTrue_1(!vusa.isConstant());

  // Test that they are created inactive
  assertTrue_1(!vub.isActive());
  assertTrue_1(!vui.isActive());
  assertTrue_1(!vud.isActive());
  assertTrue_1(!vus.isActive());
  assertTrue_1(!vuba.isActive());
  assertTrue_1(!vuia.isActive());
  assertTrue_1(!vuda.isActive());
  assertTrue_1(!vusa.isActive());

  // Test that they are unknown while inactive
  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());
  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());

  bool foob;
  int32_t fooi;
  double food;
  std::string foos;
  std::vector<bool> const *pfooba = NULL;
  std::vector<int32_t> const *pfooia = NULL;
  std::vector<double> const *pfooda = NULL;
  std::vector<std::string> const *pfoosa = NULL;

  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));
  // Numeric conversion
  assertTrue_1(!vui.getValue(food));

  // Arrays
  assertTrue_1(!vuba.getValuePointer(pfooba));
  assertTrue_1(!vuia.getValuePointer(pfooia));
  assertTrue_1(!vuda.getValuePointer(pfooda));
  assertTrue_1(!vusa.getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);


  // Activate and confirm they are still unknown
  vub.activate();
  vui.activate();
  vud.activate();
  vus.activate();
  vuba.activate();
  vuia.activate();
  vuda.activate();
  vusa.activate();

  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());
  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());

  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));
  // Numeric conversion
  assertTrue_1(!vui.getValue(food));

  // Arrays
  assertTrue_1(!vuba.getValuePointer(pfooba));
  assertTrue_1(!vuia.getValuePointer(pfooia));
  assertTrue_1(!vuda.getValuePointer(pfooda));
  assertTrue_1(!vusa.getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);

  // Assign and check result
  vub.setValue(true);
  vui.setValue(42);
  vud.setValue(3.14);
  vus.setValue(std::string("yoohoo"));

  std::vector<bool> vb(1, true);
  std::vector<int32_t> vi(1, 42);
  std::vector<double> vd(1, 3.1416);
  std::vector<std::string> vs(1, std::string("Foo"));
  vuba.setValue(vb);
  vuia.setValue(vi);
  vuda.setValue(vd);
  vusa.setValue(vs);

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());
  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());
    
  assertTrue_1(vub.getValue(foob));
  assertTrue_1(foob == true);
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(fooi == 42);
  assertTrue_1(vud.getValue(food));
  assertTrue_1(food == 3.14);
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foos == std::string("yoohoo"));
  // Test getValue type conversion
  assertTrue_1(vui.getValue(food));
  assertTrue_1(food == 42);

  // Arrays
  assertTrue_1(vuba.getValuePointer(pfooba));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(vb == *pfooba);
  assertTrue_1(vuia.getValuePointer(pfooia));
  assertTrue_1(pfooia != NULL);
  assertTrue_1(vi == *pfooia);
  assertTrue_1(vuda.getValuePointer(pfooda));
  assertTrue_1(pfooda != NULL);
  assertTrue_1(vd == *pfooda);
  assertTrue_1(vusa.getValuePointer(pfoosa));
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(vs == *pfoosa);

  // Test setValue type conversion
  const int32_t fiftySeven = 57;
  vud.setValue(fiftySeven);
  assertTrue_1(vud.getValue(food));
  assertTrue_1(food == 57);
  vus.setValue("yoho");
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foos == std::string("yoho"));

  // Reset and check that initial value is now unknown
  vub.reset();
  vui.reset();
  vud.reset();
  vus.reset();
  vuba.reset();
  vuia.reset();
  vuda.reset();
  vusa.reset();
  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());
  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());
  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;
  assertTrue_1(!vuba.getValuePointer(pfooba));
  assertTrue_1(!vuia.getValuePointer(pfooia));
  assertTrue_1(!vuda.getValuePointer(pfooda));
  assertTrue_1(!vusa.getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);

  // Set initial and current values, and check result
  vub.setInitialValue(true);
  vui.setInitialValue(42);
  vud.setInitialValue(3.14);
  vus.setInitialValue(std::string("yoohoo"));
  vuba.setInitialValue(vb);
  vuia.setInitialValue(vi);
  vuda.setInitialValue(vd);
  vusa.setInitialValue(vs);

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());
  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());

  assertTrue_1(vub.getValue(foob));
  assertTrue_1(foob == true);
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(fooi == 42);
  assertTrue_1(vud.getValue(food));
  assertTrue_1(food == 3.14);
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foos == std::string("yoohoo"));
  // Arrays
  assertTrue_1(vuba.getValuePointer(pfooba));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(vb == *pfooba);
  assertTrue_1(vuia.getValuePointer(pfooia));
  assertTrue_1(pfooia != NULL);
  assertTrue_1(vi == *pfooia);
  assertTrue_1(vuda.getValuePointer(pfooda));
  assertTrue_1(pfooda != NULL);
  assertTrue_1(vd == *pfooda);
  assertTrue_1(vusa.getValuePointer(pfoosa));
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(vs == *pfoosa);

  // Set values and check that they changed
  vub.setValue(false);
  vui.setValue(69);
  vud.setValue(1.414);
  vus.setValue(std::string("yo"));

  std::vector<bool> vab(2, false);
  std::vector<int32_t> vai(2, 69);
  std::vector<double> vad(2, 2.718);
  std::vector<std::string> vas(2, std::string("bar"));
  vuba.setValue(vab);
  vuia.setValue(vai);
  vuda.setValue(vad);
  vusa.setValue(vas);
  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());
  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());

  assertTrue_1(vub.getValue(foob));
  assertTrue_1(foob == false);
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(fooi == 69);
  assertTrue_1(vud.getValue(food));
  assertTrue_1(food == 1.414);
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foos == std::string("yo"));
  // Arrays
  assertTrue_1(vuba.getValuePointer(pfooba));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(vab == *pfooba);
  assertTrue_1(vuia.getValuePointer(pfooia));
  assertTrue_1(pfooia != NULL);
  assertTrue_1(vai == *pfooia);
  assertTrue_1(vuda.getValuePointer(pfooda));
  assertTrue_1(pfooda != NULL);
  assertTrue_1(vad == *pfooda);
  assertTrue_1(vusa.getValuePointer(pfoosa));
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(vas == *pfoosa);

  // Reset and check that the new initial values are back
  vub.reset();
  vui.reset();
  vud.reset();
  vus.reset();
  vuba.reset();
  vuia.reset();
  vuda.reset();
  vusa.reset();

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());
  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());

  assertTrue_1(vub.getValue(foob));
  assertTrue_1(foob == true);
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(fooi == 42);
  assertTrue_1(vud.getValue(food));
  assertTrue_1(food == 3.14);
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foos == std::string("yoohoo"));
  // Arrays
  assertTrue_1(vuba.getValuePointer(pfooba));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(vb == *pfooba);
  assertTrue_1(vuia.getValuePointer(pfooia));
  assertTrue_1(pfooia != NULL);
  assertTrue_1(vi == *pfooia);
  assertTrue_1(vuda.getValuePointer(pfooda));
  assertTrue_1(pfooda != NULL);
  assertTrue_1(vd == *pfooda);
  assertTrue_1(vusa.getValuePointer(pfoosa));
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(vs == *pfoosa);

  return true;
}

static bool testInitialValue()
{
  BooleanVariable vb(false);
  IntegerVariable vi(69);
  RealVariable vd(1.414);
  StringVariable vs(std::string("yo"));

  std::vector<bool> bv(2, true);
  std::vector<int32_t> iv(2, 56);
  std::vector<double> dv(2, 1.414);
  std::vector<std::string> sv(2, std::string("yahoo"));
  BooleanArrayVariable vba(bv);
  IntegerArrayVariable via(iv);
  RealArrayVariable vda(dv);
  StringArrayVariable vsa(sv);

  // Test that they are assignable and not constant
  assertTrue_1(vb.isAssignable());
  assertTrue_1(vi.isAssignable());
  assertTrue_1(vd.isAssignable());
  assertTrue_1(vs.isAssignable());
  assertTrue_1(vba.isAssignable());
  assertTrue_1(via.isAssignable());
  assertTrue_1(vda.isAssignable());
  assertTrue_1(vsa.isAssignable());
  assertTrue_1(!vb.isConstant());
  assertTrue_1(!vi.isConstant());
  assertTrue_1(!vd.isConstant());
  assertTrue_1(!vs.isConstant());
  assertTrue_1(!vba.isConstant());
  assertTrue_1(!via.isConstant());
  assertTrue_1(!vda.isConstant());
  assertTrue_1(!vsa.isConstant());

  // Test that they are created inactive
  assertTrue_1(!vb.isActive());
  assertTrue_1(!vi.isActive());
  assertTrue_1(!vd.isActive());
  assertTrue_1(!vs.isActive());
  assertTrue_1(!vba.isActive());
  assertTrue_1(!via.isActive());
  assertTrue_1(!vda.isActive());
  assertTrue_1(!vsa.isActive());

  // Test that they are unknown while inactive
  assertTrue_1(!vb.isKnown());
  assertTrue_1(!vi.isKnown());
  assertTrue_1(!vd.isKnown());
  assertTrue_1(!vs.isKnown());
  assertTrue_1(!vba.isKnown());
  assertTrue_1(!via.isKnown());
  assertTrue_1(!vda.isKnown());
  assertTrue_1(!vsa.isKnown());

  // Activate and confirm they are known
  vb.activate();
  vi.activate();
  vd.activate();
  vs.activate();
  vba.activate();
  via.activate();
  vda.activate();
  vsa.activate();
  assertTrue_1(vb.isKnown());
  assertTrue_1(vi.isKnown());
  assertTrue_1(vd.isKnown());
  assertTrue_1(vs.isKnown());
  assertTrue_1(vba.isKnown());
  assertTrue_1(via.isKnown());
  assertTrue_1(vda.isKnown());
  assertTrue_1(vsa.isKnown());

  // Check values
  double food;
  std::string foos;
  int32_t fooi;
  bool foob;
  std::vector<bool> const *pfooba = NULL;
  std::vector<int32_t> const *pfooia = NULL;
  std::vector<double> const *pfooda = NULL;
  std::vector<std::string> const *pfoosa = NULL;
    
  assertTrue_1(vb.getValue(foob));
  assertTrue_1(foob == false);
  assertTrue_1(vi.getValue(fooi));
  assertTrue_1(fooi == 69);
  assertTrue_1(vd.getValue(food));
  assertTrue_1(food == 1.414);
  assertTrue_1(vs.getValue(foos));
  assertTrue_1(foos == std::string("yo"));
  // Numeric conversion
  assertTrue_1(vi.getValue(food));
  assertTrue_1(food == 69);

  // Arrays
  assertTrue_1(vba.getValuePointer(pfooba));
  assertTrue_1(via.getValuePointer(pfooia));
  assertTrue_1(vda.getValuePointer(pfooda));
  assertTrue_1(vsa.getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv == *pfooba);
  assertTrue_1(iv == *pfooia);
  assertTrue_1(dv == *pfooda);
  assertTrue_1(sv == *pfoosa);
  
  // Set unknown
  vb.setUnknown();
  vi.setUnknown();
  vd.setUnknown();
  vs.setUnknown();
  vba.setUnknown();
  via.setUnknown();
  vda.setUnknown();
  vsa.setUnknown();

  // Confirm that they are now unknown
  assertTrue_1(!vb.isKnown());
  assertTrue_1(!vi.isKnown());
  assertTrue_1(!vd.isKnown());
  assertTrue_1(!vs.isKnown());
  assertTrue_1(!vba.isKnown());
  assertTrue_1(!via.isKnown());
  assertTrue_1(!vda.isKnown());
  assertTrue_1(!vsa.isKnown());

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(!vb.getValue(foob));
  assertTrue_1(!vi.getValue(fooi));
  assertTrue_1(!vd.getValue(food));
  assertTrue_1(!vs.getValue(foos));
  assertTrue_1(!vba.getValuePointer(pfooba));
  assertTrue_1(!via.getValuePointer(pfooia));
  assertTrue_1(!vda.getValuePointer(pfooda));
  assertTrue_1(!vsa.getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);

  // Reset and check that initial value is restored
  vb.reset();
  vi.reset();
  vd.reset();
  vs.reset();
  vba.reset();
  via.reset();
  vda.reset();
  vsa.reset();

  assertTrue_1(vb.isKnown());
  assertTrue_1(vi.isKnown());
  assertTrue_1(vd.isKnown());
  assertTrue_1(vs.isKnown());
  assertTrue_1(vba.isKnown());
  assertTrue_1(via.isKnown());
  assertTrue_1(vda.isKnown());
  assertTrue_1(vsa.isKnown());

  assertTrue_1(vb.getValue(foob));
  assertTrue_1(foob == false);
  assertTrue_1(vi.getValue(fooi));
  assertTrue_1(fooi == 69);
  assertTrue_1(vd.getValue(food));
  assertTrue_1(food == 1.414);
  assertTrue_1(vs.getValue(foos));
  assertTrue_1(foos == std::string("yo"));

  assertTrue_1(vba.getValuePointer(pfooba));
  assertTrue_1(via.getValuePointer(pfooia));
  assertTrue_1(vda.getValuePointer(pfooda));
  assertTrue_1(vsa.getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv == *pfooba);
  assertTrue_1(iv == *pfooia);
  assertTrue_1(dv == *pfooda);
  assertTrue_1(sv == *pfoosa);

  // Set initial values to unknown
  vb.setInitialUnknown();
  vi.setInitialUnknown();
  vd.setInitialUnknown();
  vs.setInitialUnknown();
  vba.setInitialUnknown();
  via.setInitialUnknown();
  vda.setInitialUnknown();
  vsa.setInitialUnknown();

  // Confirm that the current values are now unknown
  assertTrue_1(!vb.isKnown());
  assertTrue_1(!vi.isKnown());
  assertTrue_1(!vd.isKnown());
  assertTrue_1(!vs.isKnown());
  assertTrue_1(!vba.isKnown());
  assertTrue_1(!via.isKnown());
  assertTrue_1(!vda.isKnown());
  assertTrue_1(!vsa.isKnown());

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(!vb.getValue(foob));
  assertTrue_1(!vi.getValue(fooi));
  assertTrue_1(!vd.getValue(food));
  assertTrue_1(!vs.getValue(foos));
  assertTrue_1(!vba.getValuePointer(pfooba));
  assertTrue_1(!via.getValuePointer(pfooia));
  assertTrue_1(!vda.getValuePointer(pfooda));
  assertTrue_1(!vsa.getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);

  // Set values and check
  vb.setValue(false);
  vi.setValue(69);
  vd.setValue(1.414);
  vs.setValue(std::string("yo"));
  vba.setValue(bv);
  via.setValue(iv);
  vda.setValue(dv);
  vsa.setValue(sv);

  assertTrue_1(vb.isKnown());
  assertTrue_1(vi.isKnown());
  assertTrue_1(vd.isKnown());
  assertTrue_1(vs.isKnown());
  assertTrue_1(vb.getValue(foob));
  assertTrue_1(foob == false);
  assertTrue_1(vi.getValue(fooi));
  assertTrue_1(fooi == 69);
  assertTrue_1(vd.getValue(food));
  assertTrue_1(food == 1.414);
  assertTrue_1(vs.getValue(foos));
  assertTrue_1(foos == std::string("yo"));

  assertTrue_1(vba.getValuePointer(pfooba));
  assertTrue_1(via.getValuePointer(pfooia));
  assertTrue_1(vda.getValuePointer(pfooda));
  assertTrue_1(vsa.getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv == *pfooba);
  assertTrue_1(iv == *pfooia);
  assertTrue_1(dv == *pfooda);
  assertTrue_1(sv == *pfoosa);

  // Reset and check that initial value is now unknown
  vb.reset();
  vi.reset();
  vd.reset();
  vs.reset();
  vba.reset();
  via.reset();
  vda.reset();
  vsa.reset();
  assertTrue_1(!vb.isKnown());
  assertTrue_1(!vi.isKnown());
  assertTrue_1(!vd.isKnown());
  assertTrue_1(!vs.isKnown());
  assertTrue_1(!vba.isKnown());
  assertTrue_1(!via.isKnown());
  assertTrue_1(!vda.isKnown());
  assertTrue_1(!vsa.isKnown());

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(!vb.getValue(foob));
  assertTrue_1(!vi.getValue(fooi));
  assertTrue_1(!vd.getValue(food));
  assertTrue_1(!vs.getValue(foos));
  assertTrue_1(!vba.getValuePointer(pfooba));
  assertTrue_1(!via.getValuePointer(pfooia));
  assertTrue_1(!vda.getValuePointer(pfooda));
  assertTrue_1(!vsa.getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);

  return true;
}

// Confirm that we can do all the read-only operations
// through a pointer of type ExpressionId.
static bool testExpressionId()
{
  BooleanVariable vb(false);
  IntegerVariable vi(69);
  RealVariable vd(1.414);
  StringVariable vs(std::string("yo"));
  std::vector<bool> bv(2, true);
  std::vector<int32_t> iv(2, 56);
  std::vector<double> dv(2, 1.414);
  std::vector<std::string> sv(2, std::string("yahoo"));
  BooleanArrayVariable vba(bv);
  IntegerArrayVariable via(iv);
  RealArrayVariable vda(dv);
  StringArrayVariable vsa(sv);

  ExpressionId eb(vb.getId());
  ExpressionId ei(vi.getId());
  ExpressionId ed(vd.getId());
  ExpressionId es(vs.getId());
  ExpressionId eba(vba.getId());
  ExpressionId eia(via.getId());
  ExpressionId eda(vda.getId());
  ExpressionId esa(vsa.getId());

  // Test that they are assignable and not constant
  assertTrue_1(eb->isAssignable());
  assertTrue_1(ei->isAssignable());
  assertTrue_1(ed->isAssignable());
  assertTrue_1(es->isAssignable());
  assertTrue_1(eba->isAssignable());
  assertTrue_1(eia->isAssignable());
  assertTrue_1(eda->isAssignable());
  assertTrue_1(esa->isAssignable());

  assertTrue_1(!eb->isConstant());
  assertTrue_1(!ei->isConstant());
  assertTrue_1(!ed->isConstant());
  assertTrue_1(!es->isConstant());
  assertTrue_1(!eba->isConstant());
  assertTrue_1(!eia->isConstant());
  assertTrue_1(!eda->isConstant());
  assertTrue_1(!esa->isConstant());

  // Test that they are created inactive
  assertTrue_1(!eb->isActive());
  assertTrue_1(!ei->isActive());
  assertTrue_1(!ed->isActive());
  assertTrue_1(!es->isActive());
  assertTrue_1(!eba->isActive());
  assertTrue_1(!eia->isActive());
  assertTrue_1(!eda->isActive());
  assertTrue_1(!esa->isActive());

  // Test that they are unknown while inactive
  assertTrue_1(!eb->isKnown());
  assertTrue_1(!ei->isKnown());
  assertTrue_1(!ed->isKnown());
  assertTrue_1(!es->isKnown());
  assertTrue_1(!eba->isKnown());
  assertTrue_1(!eia->isKnown());
  assertTrue_1(!eda->isKnown());
  assertTrue_1(!esa->isKnown());

  // Activate and confirm they are known
  eb->activate();
  ei->activate();
  ed->activate();
  es->activate();
  eba->activate();
  eia->activate();
  eda->activate();
  esa->activate();

  assertTrue_1(eb->isKnown());
  assertTrue_1(ei->isKnown());
  assertTrue_1(ed->isKnown());
  assertTrue_1(es->isKnown());
  assertTrue_1(eba->isKnown());
  assertTrue_1(eia->isKnown());
  assertTrue_1(eda->isKnown());
  assertTrue_1(esa->isKnown());

  // Check values
  bool foob;
  int32_t fooi;
  double food;
  std::string foos;
  std::vector<bool> const *pfooba = NULL;
  std::vector<int32_t> const *pfooia = NULL;
  std::vector<double> const *pfooda = NULL;
  std::vector<std::string> const *pfoosa = NULL;
    
  assertTrue_1(eb->getValue(foob));
  assertTrue_1(foob == false);
  assertTrue_1(ei->getValue(fooi));
  assertTrue_1(fooi == 69);
  assertTrue_1(ed->getValue(food));
  assertTrue_1(food == 1.414);
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foos == std::string("yo"));
  // Numeric conversion
  assertTrue_1(ei->getValue(food));
  assertTrue_1(food == 69);

  assertTrue_1(eba->getValuePointer(pfooba));
  assertTrue_1(eia->getValuePointer(pfooia));
  assertTrue_1(eda->getValuePointer(pfooda));
  assertTrue_1(esa->getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv == *pfooba);
  assertTrue_1(iv == *pfooia);
  assertTrue_1(dv == *pfooda);
  assertTrue_1(sv == *pfoosa);

  return true;
}

static bool testSavedValue()
{
  BooleanVariable vub;
  IntegerVariable vui;
  RealVariable vud;
  StringVariable vus;
  BooleanArrayVariable vuba;
  IntegerArrayVariable vuia;
  RealArrayVariable vuda;
  StringArrayVariable vusa;

  double food;
  std::string foos;
  int32_t fooi;
  bool foob;
  std::vector<bool> const *pfooba = NULL;
  std::vector<int32_t> const *pfooia = NULL;
  std::vector<double> const *pfooda = NULL;
  std::vector<std::string> const *pfoosa = NULL;

  // Activate
  vub.activate();
  vui.activate();
  vud.activate();
  vus.activate();
  vuba.activate();
  vuia.activate();
  vuda.activate();
  vusa.activate();

  // Confirm that they are unknown
  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());
  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());

  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));
  assertTrue_1(!vuba.getValuePointer(pfooba));
  assertTrue_1(!vuia.getValuePointer(pfooia));
  assertTrue_1(!vuda.getValuePointer(pfooda));
  assertTrue_1(!vusa.getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);

  // Save current value (should be unknown)
  vub.saveCurrentValue();
  vui.saveCurrentValue();
  vud.saveCurrentValue();
  vus.saveCurrentValue();
  vuba.saveCurrentValue();
  vuia.saveCurrentValue();
  vuda.saveCurrentValue();
  vusa.saveCurrentValue();

  // Assign and check result
  vub.setValue(true);
  vui.setValue(42);
  vud.setValue(3.14);
  vus.setValue("yoohoo");
  std::vector<bool> bv(2, true);
  std::vector<int32_t> iv(2, 56);
  std::vector<double> dv(2, 1.414);
  std::vector<std::string> sv(2, std::string("yahoo"));
  vuba.setValue(bv);
  vuia.setValue(iv);
  vuda.setValue(dv);
  vusa.setValue(sv);

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());
  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());
  assertTrue_1(vub.getValue(foob));
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(vud.getValue(food));
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foob == true);
  assertTrue_1(fooi == 42);
  assertTrue_1(food == 3.14);
  assertTrue_1(foos == std::string("yoohoo"));

  assertTrue_1(vuba.getValuePointer(pfooba));
  assertTrue_1(vuia.getValuePointer(pfooia));
  assertTrue_1(vuda.getValuePointer(pfooda));
  assertTrue_1(vusa.getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv == *pfooba);
  assertTrue_1(iv == *pfooia);
  assertTrue_1(dv == *pfooda);
  assertTrue_1(sv == *pfoosa);

  // Restore saved value and check result
  vub.restoreSavedValue();
  vui.restoreSavedValue();
  vud.restoreSavedValue();
  vus.restoreSavedValue();
  vuba.restoreSavedValue();
  vuia.restoreSavedValue();
  vuda.restoreSavedValue();
  vusa.restoreSavedValue();
  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());
  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());
  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;
  assertTrue_1(!vuba.getValuePointer(pfooba));
  assertTrue_1(!vuia.getValuePointer(pfooia));
  assertTrue_1(!vuda.getValuePointer(pfooda));
  assertTrue_1(!vusa.getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);

  // Assign again
  vub.setValue(true);
  vui.setValue(42);
  vud.setValue(3.14);
  vus.setValue("yoohoo");
  vuba.setValue(bv);
  vuia.setValue(iv);
  vuda.setValue(dv);
  vusa.setValue(sv);

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());
  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());

  assertTrue_1(vub.getValue(foob));
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(vud.getValue(food));
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foob == true);
  assertTrue_1(fooi == 42);
  assertTrue_1(food == 3.14);
  assertTrue_1(foos == std::string("yoohoo"));

  assertTrue_1(vuba.getValuePointer(pfooba));
  assertTrue_1(vuia.getValuePointer(pfooia));
  assertTrue_1(vuda.getValuePointer(pfooda));
  assertTrue_1(vusa.getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv == *pfooba);
  assertTrue_1(iv == *pfooia);
  assertTrue_1(dv == *pfooda);
  assertTrue_1(sv == *pfoosa);

  // Save current value
  vub.saveCurrentValue();
  vui.saveCurrentValue();
  vud.saveCurrentValue();
  vus.saveCurrentValue();
  vuba.saveCurrentValue();
  vuia.saveCurrentValue();
  vuda.saveCurrentValue();
  vusa.saveCurrentValue();

  // Make unknown
  vub.setUnknown();
  vui.setUnknown();
  vud.setUnknown();
  vus.setUnknown();
  vuba.setUnknown();
  vuia.setUnknown();
  vuda.setUnknown();
  vusa.setUnknown();
  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());
  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());
  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;
  assertTrue_1(!vuba.getValuePointer(pfooba));
  assertTrue_1(!vuia.getValuePointer(pfooia));
  assertTrue_1(!vuda.getValuePointer(pfooda));
  assertTrue_1(!vusa.getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);

  // Restore saved and check that it has returned
  vub.restoreSavedValue();
  vui.restoreSavedValue();
  vud.restoreSavedValue();
  vus.restoreSavedValue();
  vuba.restoreSavedValue();
  vuia.restoreSavedValue();
  vuda.restoreSavedValue();
  vusa.restoreSavedValue();

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());
  assertTrue_1(vuba.isKnown());
  assertTrue_1(vuia.isKnown());
  assertTrue_1(vuda.isKnown());
  assertTrue_1(vusa.isKnown());

  assertTrue_1(vub.getValue(foob));
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(vud.getValue(food));
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foob == true);
  assertTrue_1(fooi == 42);
  assertTrue_1(food == 3.14);
  assertTrue_1(foos == std::string("yoohoo"));

  assertTrue_1(vuba.getValuePointer(pfooba));
  assertTrue_1(vuia.getValuePointer(pfooia));
  assertTrue_1(vuda.getValuePointer(pfooda));
  assertTrue_1(vusa.getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv == *pfooba);
  assertTrue_1(iv == *pfooia);
  assertTrue_1(dv == *pfooda);
  assertTrue_1(sv == *pfoosa);

  // Reset
  vub.reset();
  vui.reset();
  vud.reset();
  vus.reset();
  vuba.reset();
  vuia.reset();
  vuda.reset();
  vusa.reset();

  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());
  assertTrue_1(!vuba.isKnown());
  assertTrue_1(!vuia.isKnown());
  assertTrue_1(!vuda.isKnown());
  assertTrue_1(!vusa.isKnown());

  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;
  assertTrue_1(!vuba.getValuePointer(pfooba));
  assertTrue_1(!vuia.getValuePointer(pfooia));
  assertTrue_1(!vuda.getValuePointer(pfooda));
  assertTrue_1(!vusa.getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);

  return true;
}

// Confirm that we can do all the Assignable operations
// through a pointer of type AssignableId.
static bool testAssignableId()
{
  BooleanVariable vb(false);
  IntegerVariable vi(69);
  RealVariable vd(1.414);
  StringVariable vs(std::string("yo"));
  std::vector<bool> bv(2, true);
  std::vector<int32_t> iv(2, 56);
  std::vector<double> dv(2, 1.414);
  std::vector<std::string> sv(2, std::string("yahoo"));
  BooleanArrayVariable vba(bv);
  IntegerArrayVariable via(iv);
  RealArrayVariable vda(dv);
  StringArrayVariable vsa(sv);

  AssignableId eb(vb.getAssignableId());
  AssignableId ei(vi.getAssignableId());
  AssignableId ed(vd.getAssignableId());
  AssignableId es(vs.getAssignableId());
  AssignableId eba(vba.getAssignableId());
  AssignableId eia(via.getAssignableId());
  AssignableId eda(vda.getAssignableId());
  AssignableId esa(vsa.getAssignableId());

  // Test that they are assignable and not constant
  assertTrue_1(eb->isAssignable());
  assertTrue_1(ei->isAssignable());
  assertTrue_1(ed->isAssignable());
  assertTrue_1(es->isAssignable());
  assertTrue_1(eba->isAssignable());
  assertTrue_1(eia->isAssignable());
  assertTrue_1(eda->isAssignable());
  assertTrue_1(esa->isAssignable());
  assertTrue_1(!eb->isConstant());
  assertTrue_1(!ei->isConstant());
  assertTrue_1(!ed->isConstant());
  assertTrue_1(!es->isConstant());
  assertTrue_1(!eba->isConstant());
  assertTrue_1(!eia->isConstant());
  assertTrue_1(!eda->isConstant());
  assertTrue_1(!esa->isConstant());

  // Test that they are created inactive
  assertTrue_1(!eb->isActive());
  assertTrue_1(!ei->isActive());
  assertTrue_1(!ed->isActive());
  assertTrue_1(!es->isActive());
  assertTrue_1(!eba->isActive());
  assertTrue_1(!eia->isActive());
  assertTrue_1(!eda->isActive());
  assertTrue_1(!esa->isActive());

  // Test that values are unknown while inactive
  assertTrue_1(!eb->isKnown());
  assertTrue_1(!ei->isKnown());
  assertTrue_1(!ed->isKnown());
  assertTrue_1(!es->isKnown());
  assertTrue_1(!eba->isKnown());
  assertTrue_1(!eia->isKnown());
  assertTrue_1(!eda->isKnown());
  assertTrue_1(!esa->isKnown());

  // Activate
  eb->activate();
  ei->activate();
  ed->activate();
  es->activate();
  eba->activate();
  eia->activate();
  eda->activate();
  esa->activate();

  // Test that they are now known
  assertTrue_1(eb->isKnown());
  assertTrue_1(ei->isKnown());
  assertTrue_1(ed->isKnown());
  assertTrue_1(es->isKnown());
  assertTrue_1(eba->isKnown());
  assertTrue_1(eia->isKnown());
  assertTrue_1(eda->isKnown());
  assertTrue_1(esa->isKnown());

  // Check values
  double food;
  std::string foos;
  int32_t fooi;
  bool foob;
  std::vector<bool> const *pfooba = NULL;
  std::vector<int32_t> const *pfooia = NULL;
  std::vector<double> const *pfooda = NULL;
  std::vector<std::string> const *pfoosa = NULL;
    
  assertTrue_1(eb->getValue(foob));
  assertTrue_1(foob == false);
  assertTrue_1(ei->getValue(fooi));
  assertTrue_1(fooi == 69);
  assertTrue_1(ed->getValue(food));
  assertTrue_1(food == 1.414);
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foos == std::string("yo"));
  // Numeric conversion
  assertTrue_1(ei->getValue(food));
  assertTrue_1(food == 69);

  assertTrue_1(eba->getValuePointer(pfooba));
  assertTrue_1(eia->getValuePointer(pfooia));
  assertTrue_1(eda->getValuePointer(pfooda));
  assertTrue_1(esa->getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv == *pfooba);
  assertTrue_1(iv == *pfooia);
  assertTrue_1(dv == *pfooda);
  assertTrue_1(sv == *pfoosa);

  // Set values
  eb->setValue(true);
  ei->setValue(42);
  ed->setValue(3.14);
  es->setValue(std::string("yoohoo"));

  std::vector<bool> bv2(1, false);
  std::vector<int32_t> iv2(1, 99);
  std::vector<double> dv2(1, 2.718);
  std::vector<std::string> sv2(1, std::string("yeehaw"));
  eba->setValue(bv2);
  eia->setValue(iv2);
  eda->setValue(dv2);
  esa->setValue(sv2);

  assertTrue_1(eb->getValue(foob));
  assertTrue_1(foob == true);
  assertTrue_1(ei->getValue(fooi));
  assertTrue_1(fooi == 42);
  assertTrue_1(ed->getValue(food));
  assertTrue_1(food == 3.14);
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foos == std::string("yoohoo"));
  // Test setValue type conversions
  es->setValue("yoyo");
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foos == std::string("yoyo"));

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(eba->getValuePointer(pfooba));
  assertTrue_1(eia->getValuePointer(pfooia));
  assertTrue_1(eda->getValuePointer(pfooda));
  assertTrue_1(esa->getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv2 == *pfooba);
  assertTrue_1(iv2 == *pfooia);
  assertTrue_1(dv2 == *pfooda);
  assertTrue_1(sv2 == *pfoosa);

  // Save
  eb->saveCurrentValue();
  ei->saveCurrentValue();
  ed->saveCurrentValue();
  es->saveCurrentValue();
  eba->saveCurrentValue();
  eia->saveCurrentValue();
  eda->saveCurrentValue();
  esa->saveCurrentValue();

  // Set unknown
  eb->setUnknown();
  ei->setUnknown();
  ed->setUnknown();
  es->setUnknown();
  eba->setUnknown();
  eia->setUnknown();
  eda->setUnknown();
  esa->setUnknown();

  // Test that values are now unknown
  assertTrue_1(!eb->isKnown());
  assertTrue_1(!ei->isKnown());
  assertTrue_1(!ed->isKnown());
  assertTrue_1(!es->isKnown());
  assertTrue_1(!eba->isKnown());
  assertTrue_1(!eia->isKnown());
  assertTrue_1(!eda->isKnown());
  assertTrue_1(!esa->isKnown());

  assertTrue_1(!eb->getValue(foob));
  assertTrue_1(!ei->getValue(fooi));
  assertTrue_1(!ed->getValue(food));
  assertTrue_1(!es->getValue(foos));

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(!eba->getValuePointer(pfooba));
  assertTrue_1(!eia->getValuePointer(pfooia));
  assertTrue_1(!eda->getValuePointer(pfooda));
  assertTrue_1(!esa->getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);

  // Restore
  eb->restoreSavedValue();
  ei->restoreSavedValue();
  ed->restoreSavedValue();
  es->restoreSavedValue();
  eba->restoreSavedValue();
  eia->restoreSavedValue();
  eda->restoreSavedValue();
  esa->restoreSavedValue();

  // Check that saved values are restored
  assertTrue_1(eb->isKnown());
  assertTrue_1(ei->isKnown());
  assertTrue_1(ed->isKnown());
  assertTrue_1(es->isKnown());
  assertTrue_1(eba->isKnown());
  assertTrue_1(eia->isKnown());
  assertTrue_1(eda->isKnown());
  assertTrue_1(esa->isKnown());

  assertTrue_1(eb->getValue(foob));
  assertTrue_1(ei->getValue(fooi));
  assertTrue_1(ed->getValue(food));
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foob == true);
  assertTrue_1(fooi == 42);
  assertTrue_1(food == 3.14);
  assertTrue_1(foos == std::string("yoyo"));

  assertTrue_1(eba->getValuePointer(pfooba));
  assertTrue_1(eia->getValuePointer(pfooia));
  assertTrue_1(eda->getValuePointer(pfooda));
  assertTrue_1(esa->getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv2 == *pfooba);
  assertTrue_1(iv2 == *pfooia);
  assertTrue_1(dv2 == *pfooda);
  assertTrue_1(sv2 == *pfoosa);

  // Reset
  eb->reset();
  ei->reset();
  ed->reset();
  es->reset();
  eba->reset();
  eia->reset();
  eda->reset();
  esa->reset();
    
  // Check initial values are restored
  assertTrue_1(eb->getValue(foob));
  assertTrue_1(foob == false);
  assertTrue_1(ei->getValue(fooi));
  assertTrue_1(fooi == 69);
  assertTrue_1(ed->getValue(food));
  assertTrue_1(food == 1.414);
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foos == std::string("yo"));

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(eba->getValuePointer(pfooba));
  assertTrue_1(eia->getValuePointer(pfooia));
  assertTrue_1(eda->getValuePointer(pfooda));
  assertTrue_1(esa->getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv == *pfooba);
  assertTrue_1(iv == *pfooia);
  assertTrue_1(dv == *pfooda);
  assertTrue_1(sv == *pfoosa);

  // setInitialValue tests
  eb->setInitialValue(true);
  ei->setInitialValue(42);
  ed->setInitialValue(3.14);
  es->setInitialValue(std::string("yoohoo"));
  eba->setInitialValue(bv2);
  eia->setInitialValue(iv2);
  eda->setInitialValue(dv2);
  esa->setInitialValue(sv2);

  assertTrue_1(eb->isKnown());
  assertTrue_1(ei->isKnown());
  assertTrue_1(ed->isKnown());
  assertTrue_1(es->isKnown());
  assertTrue_1(eba->isKnown());
  assertTrue_1(eia->isKnown());
  assertTrue_1(eda->isKnown());
  assertTrue_1(esa->isKnown());

  assertTrue_1(eb->getValue(foob));
  assertTrue_1(ei->getValue(fooi));
  assertTrue_1(ed->getValue(food));
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foob == true);
  assertTrue_1(fooi == 42);
  assertTrue_1(food == 3.14);
  assertTrue_1(foos == std::string("yoohoo"));

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(eba->getValuePointer(pfooba));
  assertTrue_1(eia->getValuePointer(pfooia));
  assertTrue_1(eda->getValuePointer(pfooda));
  assertTrue_1(esa->getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv2 == *pfooba);
  assertTrue_1(iv2 == *pfooia);
  assertTrue_1(dv2 == *pfooda);
  assertTrue_1(sv2 == *pfoosa);

  // Set unknown
  eb->setUnknown();
  ei->setUnknown();
  ed->setUnknown();
  es->setUnknown();
  eba->setUnknown();
  eia->setUnknown();
  eda->setUnknown();
  esa->setUnknown();

  // Test that values are now unknown
  assertTrue_1(!eb->isKnown());
  assertTrue_1(!ei->isKnown());
  assertTrue_1(!ed->isKnown());
  assertTrue_1(!es->isKnown());
  assertTrue_1(!eba->isKnown());
  assertTrue_1(!eia->isKnown());
  assertTrue_1(!eda->isKnown());
  assertTrue_1(!esa->isKnown());

  assertTrue_1(!eb->getValue(foob));
  assertTrue_1(!ei->getValue(fooi));
  assertTrue_1(!ed->getValue(food));
  assertTrue_1(!es->getValue(foos));

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(!eba->getValuePointer(pfooba));
  assertTrue_1(!eia->getValuePointer(pfooia));
  assertTrue_1(!eda->getValuePointer(pfooda));
  assertTrue_1(!esa->getValuePointer(pfoosa));
  assertTrue_1(pfooba == NULL);
  assertTrue_1(pfooia == NULL);
  assertTrue_1(pfooda == NULL);
  assertTrue_1(pfoosa == NULL);

  // Reset
  eb->reset();
  ei->reset();
  ed->reset();
  es->reset();
  eba->reset();
  eia->reset();
  eda->reset();
  esa->reset();
    
  // Check initial values are restored
  assertTrue_1(eb->isKnown());
  assertTrue_1(ei->isKnown());
  assertTrue_1(ed->isKnown());
  assertTrue_1(es->isKnown());
  assertTrue_1(eba->isKnown());
  assertTrue_1(eia->isKnown());
  assertTrue_1(eda->isKnown());
  assertTrue_1(esa->isKnown());

  assertTrue_1(eb->getValue(foob));
  assertTrue_1(ei->getValue(fooi));
  assertTrue_1(ed->getValue(food));
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foob == true);
  assertTrue_1(fooi == 42);
  assertTrue_1(food == 3.14);
  assertTrue_1(foos == std::string("yoohoo"));

  pfooba = NULL;
  pfooia = NULL;
  pfooda = NULL;
  pfoosa = NULL;

  assertTrue_1(eba->getValuePointer(pfooba));
  assertTrue_1(eia->getValuePointer(pfooia));
  assertTrue_1(eda->getValuePointer(pfooda));
  assertTrue_1(esa->getValuePointer(pfoosa));
  assertTrue_1(pfooba != NULL);
  assertTrue_1(pfooia != NULL);
  assertTrue_1(pfooda != NULL);
  assertTrue_1(pfoosa != NULL);
  assertTrue_1(bv2 == *pfooba);
  assertTrue_1(iv2 == *pfooia);
  assertTrue_1(dv2 == *pfooda);
  assertTrue_1(sv2 == *pfoosa);

  // setInitialValue type conversions
  es->setInitialValue("yoyo");
  assertTrue_1(es->isKnown());
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foos == std::string("yoyo"));

  // Set unknown and check
  es->setUnknown();
  assertTrue_1(!es->isKnown());
  assertTrue_1(!es->getValue(foos));

  // Reset and recheck
  es->reset();
  assertTrue_1(es->isKnown());
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foos == std::string("yoyo"));

  return true;
}

static bool testNotification()
{
  BooleanVariable vub;
  IntegerVariable vui;
  RealVariable vud;
  StringVariable vus;
  BooleanArrayVariable vuba;
  IntegerArrayVariable vuia;
  RealArrayVariable vuda;
  StringArrayVariable vusa;

  bool bchanged = false,
    ichanged = false, 
    dchanged = false,
    schanged = false,
    bachanged = false,
    iachanged = false, 
    dachanged = false,
    sachanged = false;
  TrivialListener lb(bchanged);
  TrivialListener li(ichanged);
  TrivialListener ld(dchanged);
  TrivialListener ls(schanged);
  TrivialListener lba(bachanged);
  TrivialListener lia(iachanged);
  TrivialListener lda(dachanged);
  TrivialListener lsa(sachanged);
  vub.addListener(lb.getId());
  vui.addListener(li.getId());
  vud.addListener(ld.getId());
  vus.addListener(ls.getId());
  vuba.addListener(lba.getId());
  vuia.addListener(lia.getId());
  vuda.addListener(lda.getId());
  vusa.addListener(lsa.getId());

  // setInitialValue shouldn't notify while inactive
  vub.setInitialValue(false);
  vui.setInitialValue(69);
  vud.setInitialValue(1.414);
  vus.setInitialValue(std::string("jojo"));
  std::vector<bool> bv(2, true);
  std::vector<int32_t> iv(2, 56);
  std::vector<double> dv(2, 1.414);
  std::vector<std::string> sv(2, std::string("yahoo"));
  vuba.setInitialValue(bv);
  vuia.setInitialValue(iv);
  vuda.setInitialValue(dv);
  vusa.setInitialValue(sv);

  assertTrue_1(!bchanged);
  assertTrue_1(!ichanged);
  assertTrue_1(!dchanged);
  assertTrue_1(!schanged);
  assertTrue_1(!bachanged);
  assertTrue_1(!iachanged);
  assertTrue_1(!dachanged);
  assertTrue_1(!sachanged);

  // Assign and check whether the listeners were notified
  vub.setValue(true);
  vui.setValue(42);
  vud.setValue(3.14);
  vus.setValue("yoohoo");

  std::vector<bool> bv2(1, false);
  std::vector<int32_t> iv2(1, 99);
  std::vector<double> dv2(1, 2.718);
  std::vector<std::string> sv2(1, std::string("yeehaw"));
  vuba.setValue(bv2);
  vuia.setValue(iv2);
  vuda.setValue(dv2);
  vusa.setValue(sv2);

  assertTrue_1(!bchanged);
  assertTrue_1(!ichanged);
  assertTrue_1(!dchanged);
  assertTrue_1(!schanged);
  assertTrue_1(!bachanged);
  assertTrue_1(!iachanged);
  assertTrue_1(!dachanged);
  assertTrue_1(!sachanged);

  // Activation alone should not notify
  vub.activate();
  vui.activate();
  vud.activate();
  vus.activate();
  vuba.activate();
  vuia.activate();
  vuda.activate();
  vusa.activate();
  assertTrue_1(!bchanged);
  assertTrue_1(!ichanged);
  assertTrue_1(!dchanged);
  assertTrue_1(!schanged);
  assertTrue_1(!bachanged);
  assertTrue_1(!iachanged);
  assertTrue_1(!dachanged);
  assertTrue_1(!sachanged);

  // Reset should notify
  vub.reset();
  vui.reset();
  vud.reset();
  vus.reset();
  vuba.reset();
  vuia.reset();
  vuda.reset();
  vusa.reset();
  assertTrue_1(bchanged);
  assertTrue_1(ichanged);
  assertTrue_1(dchanged);
  assertTrue_1(schanged);
  assertTrue_1(bachanged);
  assertTrue_1(iachanged);
  assertTrue_1(dachanged);
  assertTrue_1(sachanged);

  // Assign should notify this time
  bchanged = ichanged = dchanged = schanged =
    bachanged = iachanged = dachanged = sachanged =
    false;
  vub.setValue(true);
  vui.setValue(42);
  vud.setValue(3.14);
  vus.setValue("yoohoo");
  vuba.setValue(bv2);
  vuia.setValue(iv2);
  vuda.setValue(dv2);
  vusa.setValue(sv2);
  assertTrue_1(bchanged);
  assertTrue_1(ichanged);
  assertTrue_1(dchanged);
  assertTrue_1(schanged);
  assertTrue_1(bachanged);
  assertTrue_1(iachanged);
  assertTrue_1(dachanged);
  assertTrue_1(sachanged);

  // Save current value shouldn't notify
  bchanged = ichanged = dchanged = schanged =
    bachanged = iachanged = dachanged = sachanged =
    false;
  vub.saveCurrentValue();
  vui.saveCurrentValue();
  vud.saveCurrentValue();
  vus.saveCurrentValue();
  vuba.saveCurrentValue();
  vuia.saveCurrentValue();
  vuda.saveCurrentValue();
  vusa.saveCurrentValue();
  assertTrue_1(!bchanged);
  assertTrue_1(!ichanged);
  assertTrue_1(!dchanged);
  assertTrue_1(!schanged);
  assertTrue_1(!bachanged);
  assertTrue_1(!iachanged);
  assertTrue_1(!dachanged);
  assertTrue_1(!sachanged);

  // Restoring same value shouldn't notify
  vub.restoreSavedValue();
  vui.restoreSavedValue();
  vud.restoreSavedValue();
  vus.restoreSavedValue();
  vuba.restoreSavedValue();
  vuia.restoreSavedValue();
  vuda.restoreSavedValue();
  vusa.restoreSavedValue();
  assertTrue_1(!bchanged);
  assertTrue_1(!ichanged);
  assertTrue_1(!dchanged);
  assertTrue_1(!schanged);
  assertTrue_1(!bachanged);
  assertTrue_1(!iachanged);
  assertTrue_1(!dachanged);
  assertTrue_1(!sachanged);

  // Making unknown should notify
  vub.setUnknown();
  vui.setUnknown();
  vud.setUnknown();
  vus.setUnknown();
  vuba.setUnknown();
  vuia.setUnknown();
  vuda.setUnknown();
  vusa.setUnknown();
  assertTrue_1(bchanged);
  assertTrue_1(ichanged);
  assertTrue_1(dchanged);
  assertTrue_1(schanged);
  assertTrue_1(bachanged);
  assertTrue_1(iachanged);
  assertTrue_1(dachanged);
  assertTrue_1(sachanged);

  // Now restoring should notify
  bchanged = ichanged = dchanged = schanged =
    bachanged = iachanged = dachanged = sachanged =
    false;
  vub.restoreSavedValue();
  vui.restoreSavedValue();
  vud.restoreSavedValue();
  vus.restoreSavedValue();
  vuba.restoreSavedValue();
  vuia.restoreSavedValue();
  vuda.restoreSavedValue();
  vusa.restoreSavedValue();
  assertTrue_1(bchanged);
  assertTrue_1(ichanged);
  assertTrue_1(dchanged);
  assertTrue_1(schanged);
  assertTrue_1(bachanged);
  assertTrue_1(iachanged);
  assertTrue_1(dachanged);
  assertTrue_1(sachanged);

  // setInitialValue shouldn't notify
  bchanged = ichanged = dchanged = schanged =
    bachanged = iachanged = dachanged = sachanged =
    false;
  vub.setInitialValue(false);
  vui.setInitialValue(69);
  vud.setInitialValue(1.414);
  vus.setInitialValue(std::string("jojo"));
  vuba.setInitialValue(bv);
  vuia.setInitialValue(iv);
  vuda.setInitialValue(dv);
  vusa.setInitialValue(sv);
  assertTrue_1(!bchanged);
  assertTrue_1(!ichanged);
  assertTrue_1(!dchanged);
  assertTrue_1(!schanged);
  assertTrue_1(!bachanged);
  assertTrue_1(!iachanged);
  assertTrue_1(!dachanged);
  assertTrue_1(!sachanged);

  // Clean up
  vub.removeListener(lb.getId());
  vui.removeListener(li.getId());
  vud.removeListener(ld.getId());
  vus.removeListener(ls.getId());
  vuba.removeListener(lba.getId());
  vuia.removeListener(lia.getId());
  vuda.removeListener(lda.getId());
  vusa.removeListener(lsa.getId());

  return true;
}

bool variablesTest()
{
  runTest(testUninitialized);
  runTest(testInitialValue);
  runTest(testExpressionId);
  runTest(testSavedValue);
  runTest(testAssignableId);
  runTest(testNotification);

  return true;
}
