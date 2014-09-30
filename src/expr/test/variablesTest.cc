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
#include "Constant.hh"
#include "TestSupport.hh"
#include "test/TrivialListener.hh"

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

  // Test that they are assignable and not constant
  assertTrue_1(vub.isAssignable());
  assertTrue_1(vui.isAssignable());
  assertTrue_1(vud.isAssignable());
  assertTrue_1(vus.isAssignable());

  assertTrue_1(!vub.isConstant());
  assertTrue_1(!vui.isConstant());
  assertTrue_1(!vud.isConstant());
  assertTrue_1(!vus.isConstant());

  // Test that they are created inactive
  assertTrue_1(!vub.isActive());
  assertTrue_1(!vui.isActive());
  assertTrue_1(!vud.isActive());
  assertTrue_1(!vus.isActive());

  // Test that they are unknown while inactive
  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());

  bool foob;
  int32_t fooi;
  double food;
  std::string foos;

  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));
  // Numeric conversion
  assertTrue_1(!vui.getValue(food));

  // Activate and confirm they are still unknown
  vub.activate();
  vui.activate();
  vud.activate();
  vus.activate();

  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());

  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));
  // Numeric conversion
  assertTrue_1(!vui.getValue(food));

  // Assign and check result
  vub.setValue(true);
  vui.setValue((int32_t) 42);
  vud.setValue(3.14);
  vus.setValue(std::string("yoohoo"));

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());
    
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
  // Test setValue type conversion
  const int32_t fiftySeven = 57;
  vud.setValue(fiftySeven);
  assertTrue_1(vud.getValue(food));
  assertTrue_1(food == 57);
  vus.setValue("yoho");
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foos == std::string("yoho"));

  // Reset and check that value is now unknown
  // Can't reset while active
  vub.deactivate();
  vui.deactivate();
  vud.deactivate();
  vus.deactivate();

  vub.reset();
  vui.reset();
  vud.reset();
  vus.reset();

  vub.activate();
  vui.activate();
  vud.activate();
  vus.activate();

  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());

  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));

  // Set value and check result
  vub.setValue(true);
  vui.setValue((int32_t) 42);
  vud.setValue(3.14);
  vus.setValue(std::string("yoohoo"));

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());

  assertTrue_1(vub.getValue(foob));
  assertTrue_1(foob == true);
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(fooi == 42);
  assertTrue_1(vud.getValue(food));
  assertTrue_1(food == 3.14);
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foos == std::string("yoohoo"));

  // Set values and check that they changed
  vub.setValue(false);
  vui.setValue((int32_t) 69);
  vud.setValue(1.414);
  vus.setValue(std::string("yo"));

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());

  assertTrue_1(vub.getValue(foob));
  assertTrue_1(foob == false);
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(fooi == 69);
  assertTrue_1(vud.getValue(food));
  assertTrue_1(food == 1.414);
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foos == std::string("yo"));
  // Arrays

  return true;
}

// *** TODO: This test should be superseded by testInitializers() below.
static bool testInitialValue()
{
  BooleanVariable vb(false);
  IntegerVariable vi(69);
  RealVariable vd(1.414);
  StringVariable vs(std::string("yo"));

  // Test that they are assignable and not constant
  assertTrue_1(vb.isAssignable());
  assertTrue_1(vi.isAssignable());
  assertTrue_1(vd.isAssignable());
  assertTrue_1(vs.isAssignable());

  assertTrue_1(!vb.isConstant());
  assertTrue_1(!vi.isConstant());
  assertTrue_1(!vd.isConstant());
  assertTrue_1(!vs.isConstant());

  // Test that they are created inactive
  assertTrue_1(!vb.isActive());
  assertTrue_1(!vi.isActive());
  assertTrue_1(!vd.isActive());
  assertTrue_1(!vs.isActive());

  // Test that they are unknown while inactive
  assertTrue_1(!vb.isKnown());
  assertTrue_1(!vi.isKnown());
  assertTrue_1(!vd.isKnown());
  assertTrue_1(!vs.isKnown());

  // Activate and confirm they are known
  vb.activate();
  vi.activate();
  vd.activate();
  vs.activate();

  assertTrue_1(vb.isKnown());
  assertTrue_1(vi.isKnown());
  assertTrue_1(vd.isKnown());
  assertTrue_1(vs.isKnown());

  // Check values
  double food;
  std::string foos;
  int32_t fooi;
  bool foob;
    
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

  // Set unknown
  vb.setUnknown();
  vi.setUnknown();
  vd.setUnknown();
  vs.setUnknown();

  // Confirm that they are now unknown
  assertTrue_1(!vb.isKnown());
  assertTrue_1(!vi.isKnown());
  assertTrue_1(!vd.isKnown());
  assertTrue_1(!vs.isKnown());

  assertTrue_1(!vb.getValue(foob));
  assertTrue_1(!vi.getValue(fooi));
  assertTrue_1(!vd.getValue(food));
  assertTrue_1(!vs.getValue(foos));

  // Reset and confirm unknown
  vb.deactivate();
  vi.deactivate();
  vd.deactivate();
  vs.deactivate();

  vb.reset();
  vi.reset();
  vd.reset();
  vs.reset();

  assertTrue_1(!vb.isKnown());
  assertTrue_1(!vi.isKnown());
  assertTrue_1(!vd.isKnown());
  assertTrue_1(!vs.isKnown());

  // Activate and check that initial value is restored
  vb.activate();
  vi.activate();
  vd.activate();
  vs.activate();

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

  // Set values and check
  vb.setValue(true);
  vi.setValue((int32_t) 42);
  vd.setValue(2.718);
  vs.setValue(std::string("mama"));

  assertTrue_1(vb.isKnown());
  assertTrue_1(vi.isKnown());
  assertTrue_1(vd.isKnown());
  assertTrue_1(vs.isKnown());
  assertTrue_1(vb.getValue(foob));
  assertTrue_1(foob == true);
  assertTrue_1(vi.getValue(fooi));
  assertTrue_1(fooi == 42);
  assertTrue_1(vd.getValue(food));
  assertTrue_1(food == 2.718);
  assertTrue_1(vs.getValue(foos));
  assertTrue_1(foos == std::string("mama"));

  return true;
}

// Confirm that initializer expression is only invoked
// on an inactive to active transition.
static bool testInitializers()
{
  BooleanVariable vb;
  BooleanConstant cb(false);
  vb.setInitializer(&cb, false);

  IntegerVariable vi;
  IntegerConstant ci(69);
  vi.setInitializer(&ci, false);

  RealVariable vd;
  RealConstant cd(1.414);
  vd.setInitializer(&cd, false);

  StringVariable vs;
  StringConstant cs("yo");
  vs.setInitializer(&cs, false);

  // Test that they are assignable and not constant
  assertTrue_1(vb.isAssignable());
  assertTrue_1(vi.isAssignable());
  assertTrue_1(vd.isAssignable());
  assertTrue_1(vs.isAssignable());

  assertTrue_1(!vb.isConstant());
  assertTrue_1(!vi.isConstant());
  assertTrue_1(!vd.isConstant());
  assertTrue_1(!vs.isConstant());

  // Test that they are created inactive
  assertTrue_1(!vb.isActive());
  assertTrue_1(!vi.isActive());
  assertTrue_1(!vd.isActive());
  assertTrue_1(!vs.isActive());

  // Test that they are unknown while inactive
  assertTrue_1(!vb.isKnown());
  assertTrue_1(!vi.isKnown());
  assertTrue_1(!vd.isKnown());
  assertTrue_1(!vs.isKnown());

  // Activate and confirm they are known
  vb.activate();
  vi.activate();
  vd.activate();
  vs.activate();

  assertTrue_1(vb.isKnown());
  assertTrue_1(vi.isKnown());
  assertTrue_1(vd.isKnown());
  assertTrue_1(vs.isKnown());

  // Check values
  double food;
  std::string foos;
  int32_t fooi;
  bool foob;
    
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

  // Set unknown
  vb.setUnknown();
  vi.setUnknown();
  vd.setUnknown();
  vs.setUnknown();

  // Confirm that they are now unknown
  assertTrue_1(!vb.isKnown());
  assertTrue_1(!vi.isKnown());
  assertTrue_1(!vd.isKnown());
  assertTrue_1(!vs.isKnown());

  assertTrue_1(!vb.getValue(foob));
  assertTrue_1(!vi.getValue(fooi));
  assertTrue_1(!vd.getValue(food));
  assertTrue_1(!vs.getValue(foos));

  // Reset and confirm unknown
  vb.deactivate();
  vi.deactivate();
  vd.deactivate();
  vs.deactivate();

  vb.reset();
  vi.reset();
  vd.reset();
  vs.reset();

  assertTrue_1(!vb.isKnown());
  assertTrue_1(!vi.isKnown());
  assertTrue_1(!vd.isKnown());
  assertTrue_1(!vs.isKnown());

  // Activate and check that initial value is restored
  vb.activate();
  vi.activate();
  vd.activate();
  vs.activate();

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

  // Set values and check
  vb.setValue(true);
  vi.setValue((int32_t) 42);
  vd.setValue(2.718);
  vs.setValue(std::string("mama"));

  assertTrue_1(vb.isKnown());
  assertTrue_1(vi.isKnown());
  assertTrue_1(vd.isKnown());
  assertTrue_1(vs.isKnown());
  assertTrue_1(vb.getValue(foob));
  assertTrue_1(foob == true);
  assertTrue_1(vi.getValue(fooi));
  assertTrue_1(fooi == 42);
  assertTrue_1(vd.getValue(food));
  assertTrue_1(food == 2.718);
  assertTrue_1(vs.getValue(foos));
  assertTrue_1(foos == std::string("mama"));

  return true;
}

static bool testSavedValue()
{
  BooleanVariable vub;
  IntegerVariable vui;
  RealVariable vud;
  StringVariable vus;

  double food;
  std::string foos;
  int32_t fooi;
  bool foob;

  // Activate
  vub.activate();
  vui.activate();
  vud.activate();
  vus.activate();

  // Confirm that they are unknown
  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());

  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));

  // Save current value (should be unknown)
  vub.saveCurrentValue();
  vui.saveCurrentValue();
  vud.saveCurrentValue();
  vus.saveCurrentValue();

  // Assign and check result
  vub.setValue(true);
  vui.setValue((int32_t) 42);
  vud.setValue(3.14);
  vus.setValue("yoohoo");

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());

  assertTrue_1(vub.getValue(foob));
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(vud.getValue(food));
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foob == true);
  assertTrue_1(fooi == 42);
  assertTrue_1(food == 3.14);
  assertTrue_1(foos == std::string("yoohoo"));

  // Restore saved value and check result
  vub.restoreSavedValue();
  vui.restoreSavedValue();
  vud.restoreSavedValue();
  vus.restoreSavedValue();

  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());

  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));

  // Assign again
  vub.setValue(true);
  vui.setValue((int32_t) 42);
  vud.setValue(3.14);
  vus.setValue("yoohoo");

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());

  assertTrue_1(vub.getValue(foob));
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(vud.getValue(food));
  assertTrue_1(vus.getValue(foos));
  assertTrue_1(foob == true);
  assertTrue_1(fooi == 42);
  assertTrue_1(food == 3.14);
  assertTrue_1(foos == std::string("yoohoo"));

  // Save current value
  vub.saveCurrentValue();
  vui.saveCurrentValue();
  vud.saveCurrentValue();
  vus.saveCurrentValue();

  // Make unknown
  vub.setUnknown();
  vui.setUnknown();
  vud.setUnknown();
  vus.setUnknown();

  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());

  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));

  // Restore saved and check that it has returned
  vub.restoreSavedValue();
  vui.restoreSavedValue();
  vud.restoreSavedValue();
  vus.restoreSavedValue();

  assertTrue_1(vub.isKnown());
  assertTrue_1(vui.isKnown());
  assertTrue_1(vud.isKnown());
  assertTrue_1(vus.isKnown());

  assertTrue_1(vub.getValue(foob));
  assertTrue_1(vui.getValue(fooi));
  assertTrue_1(vud.getValue(food));
  assertTrue_1(vus.getValue(foos));

  assertTrue_1(foob == true);
  assertTrue_1(fooi == 42);
  assertTrue_1(food == 3.14);
  assertTrue_1(foos == std::string("yoohoo"));

  // Reset
  vub.deactivate();
  vui.deactivate();
  vud.deactivate();
  vus.deactivate();

  vub.reset();
  vui.reset();
  vud.reset();
  vus.reset();

  vub.activate();
  vui.activate();
  vud.activate();
  vus.activate();

  assertTrue_1(!vub.isKnown());
  assertTrue_1(!vui.isKnown());
  assertTrue_1(!vud.isKnown());
  assertTrue_1(!vus.isKnown());

  assertTrue_1(!vub.getValue(foob));
  assertTrue_1(!vui.getValue(fooi));
  assertTrue_1(!vud.getValue(food));
  assertTrue_1(!vus.getValue(foos));

  return true;
}

// Confirm that we can do all the Assignable operations
// through an Assignable * pointer.
static bool testAssignablePointer()
{
  BooleanVariable vb(false);
  IntegerVariable vi(69);
  RealVariable vd(1.414);
  StringVariable vs(std::string("yo"));

  Assignable *eb(vb.asAssignable());
  Assignable *ei(vi.asAssignable());
  Assignable *ed(vd.asAssignable());
  Assignable *es(vs.asAssignable());

  // Confirm that we actually got pointers
  assertTrue_1(eb != NULL);
  assertTrue_1(ei != NULL);
  assertTrue_1(ed != NULL);
  assertTrue_1(es != NULL);

  // Test that they are assignable and not constant
  assertTrue_1(eb->isAssignable());
  assertTrue_1(ei->isAssignable());
  assertTrue_1(ed->isAssignable());
  assertTrue_1(es->isAssignable());

  assertTrue_1(!eb->isConstant());
  assertTrue_1(!ei->isConstant());
  assertTrue_1(!ed->isConstant());
  assertTrue_1(!es->isConstant());

  // Test that they are created inactive
  assertTrue_1(!eb->isActive());
  assertTrue_1(!ei->isActive());
  assertTrue_1(!ed->isActive());
  assertTrue_1(!es->isActive());

  // Test that values are unknown while inactive
  assertTrue_1(!eb->isKnown());
  assertTrue_1(!ei->isKnown());
  assertTrue_1(!ed->isKnown());
  assertTrue_1(!es->isKnown());

  // Activate
  eb->activate();
  ei->activate();
  ed->activate();
  es->activate();

  // Test that they are now known
  assertTrue_1(eb->isKnown());
  assertTrue_1(ei->isKnown());
  assertTrue_1(ed->isKnown());
  assertTrue_1(es->isKnown());

  // Check values
  double food;
  std::string foos;
  int32_t fooi;
  bool foob;
    
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

  // Set values
  eb->setValue(true);
  ei->setValue((int32_t) 42);
  ed->setValue(3.14);
  es->setValue(std::string("yoohoo"));

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

  // Save
  eb->saveCurrentValue();
  ei->saveCurrentValue();
  ed->saveCurrentValue();
  es->saveCurrentValue();

  // Set unknown
  eb->setUnknown();
  ei->setUnknown();
  ed->setUnknown();
  es->setUnknown();

  // Test that values are now unknown
  assertTrue_1(!eb->isKnown());
  assertTrue_1(!ei->isKnown());
  assertTrue_1(!ed->isKnown());
  assertTrue_1(!es->isKnown());

  assertTrue_1(!eb->getValue(foob));
  assertTrue_1(!ei->getValue(fooi));
  assertTrue_1(!ed->getValue(food));
  assertTrue_1(!es->getValue(foos));

  // Restore
  eb->restoreSavedValue();
  ei->restoreSavedValue();
  ed->restoreSavedValue();
  es->restoreSavedValue();

  // Check that saved values are restored
  assertTrue_1(eb->isKnown());
  assertTrue_1(ei->isKnown());
  assertTrue_1(ed->isKnown());
  assertTrue_1(es->isKnown());

  assertTrue_1(eb->getValue(foob));
  assertTrue_1(ei->getValue(fooi));
  assertTrue_1(ed->getValue(food));
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foob == true);
  assertTrue_1(fooi == 42);
  assertTrue_1(food == 3.14);
  assertTrue_1(foos == std::string("yoyo"));

  // Reset
  eb->deactivate();
  ei->deactivate();
  ed->deactivate();
  es->deactivate();

  eb->reset();
  ei->reset();
  ed->reset();
  es->reset();
    
  eb->activate();
  ei->activate();
  ed->activate();
  es->activate();

  // Check initial values are restored
  assertTrue_1(eb->getValue(foob));
  assertTrue_1(foob == false);
  assertTrue_1(ei->getValue(fooi));
  assertTrue_1(fooi == 69);
  assertTrue_1(ed->getValue(food));
  assertTrue_1(food == 1.414);
  assertTrue_1(es->getValue(foos));
  assertTrue_1(foos == std::string("yo"));

  return true;
}

static bool testNotification()
{
  BooleanVariable vub;
  IntegerVariable vui;
  RealVariable vud;
  StringVariable vus;

  bool bchanged = false,
    ichanged = false, 
    dchanged = false,
    schanged = false;

  TrivialListener lb(bchanged);
  TrivialListener li(ichanged);
  TrivialListener ld(dchanged);
  TrivialListener ls(schanged);

  vub.addListener(&lb);
  vui.addListener(&li);
  vud.addListener(&ld);
  vus.addListener(&ls);

  // Activation alone should not notify
  vub.activate();
  vui.activate();
  vud.activate();
  vus.activate();

  assertTrue_1(!bchanged);
  assertTrue_1(!ichanged);
  assertTrue_1(!dchanged);
  assertTrue_1(!schanged);

  // Reset for uninitialized should NOT notify
  vub.deactivate();
  vui.deactivate();
  vud.deactivate();
  vus.deactivate();

  vub.reset();
  vui.reset();
  vud.reset();
  vus.reset();

  vub.activate();
  vui.activate();
  vud.activate();
  vus.activate();

  assertTrue_1(!bchanged);
  assertTrue_1(!ichanged);
  assertTrue_1(!dchanged);
  assertTrue_1(!schanged);

  // Assign should notify this time
  vub.setValue(false);
  vui.setValue((int32_t) 69);
  vud.setValue(1.414);
  vus.setValue(std::string("jojo"));

  assertTrue_1(bchanged);
  assertTrue_1(ichanged);
  assertTrue_1(dchanged);
  assertTrue_1(schanged);

  // Save current value shouldn't notify
  bchanged = ichanged = dchanged = schanged =
    false;
  vub.saveCurrentValue();
  vui.saveCurrentValue();
  vud.saveCurrentValue();
  vus.saveCurrentValue();

  assertTrue_1(!bchanged);
  assertTrue_1(!ichanged);
  assertTrue_1(!dchanged);
  assertTrue_1(!schanged);

  // Restoring same value shouldn't notify
  vub.restoreSavedValue();
  vui.restoreSavedValue();
  vud.restoreSavedValue();
  vus.restoreSavedValue();

  assertTrue_1(!bchanged);
  assertTrue_1(!ichanged);
  assertTrue_1(!dchanged);
  assertTrue_1(!schanged);

  // Making unknown should notify
  vub.setUnknown();
  vui.setUnknown();
  vud.setUnknown();
  vus.setUnknown();

  assertTrue_1(bchanged);
  assertTrue_1(ichanged);
  assertTrue_1(dchanged);
  assertTrue_1(schanged);

  // Now restoring should notify
  bchanged = ichanged = dchanged = schanged = false;
  vub.restoreSavedValue();
  vui.restoreSavedValue();
  vud.restoreSavedValue();
  vus.restoreSavedValue();

  assertTrue_1(bchanged);
  assertTrue_1(ichanged);
  assertTrue_1(dchanged);
  assertTrue_1(schanged);

  // Clean up
  vub.removeListener(&lb);
  vui.removeListener(&li);
  vud.removeListener(&ld);
  vus.removeListener(&ls);

  return true;
}

bool variablesTest()
{
  runTest(testUninitialized);
  runTest(testInitialValue);
  runTest(testInitializers);
  runTest(testSavedValue);
  runTest(testAssignablePointer);
  runTest(testNotification);

  return true;
}
