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

#include "Constant.hh"
#include "ExpressionListener.hh"
#include "lifecycle-utils.h"
#include "TestSupport.hh"
#include "UserVariable.hh"


using namespace PLEXIL;

//
// ExpressionListener tests
//

class TrivialListener : public ExpressionListener
{
public:
  TrivialListener(bool& changed) 
  : ExpressionListener(), m_changed(changed)
  {
  }

protected:
  void notifyChanged()
  {
    m_changed = true;
  }

private:
  bool& m_changed;
};

class PropagatingListener : public ExpressionListener
{
public:
  PropagatingListener(const ExpressionId& owner)
  : ExpressionListener(),
    m_owner(owner)
  {
  }

protected:
  void notifyChanged()
  {
    m_owner->notifyChanged();
  }

private:
  ExpressionId m_owner;
};


class TrivialExpression : public Mutable
{
public:
  TrivialExpression()
    : Mutable(),
      changed(false)
  {
  }

  void setListener(const ExpressionListenerId & l)
  {
    m_listener = l;
  }

  void handleChange()
  {
    if (isActive()) {
      changed = true;
      publishChange();
    }
  }

  const char *exprName() const { return "trivial"; }
  const char *typeName() const { return "notype"; }

  void print(std::ostream& s) const {}
  void printValue(std::ostream& s) const {}

  bool isConstant() const { return true; }
  bool isKnown() const { return false; }
  bool isUnknown() const { return true; }

private:
  ExpressionListenerId m_listener;

public:
  bool changed;
};

class ListenerTest
{
public:
  static bool test()
  {
    runTest(testListenerPropagation);
    runTest(testDirectPropagation);
    return true;
  }

private:
  static bool testListenerPropagation()
  {
    // Test setup
    TrivialExpression source;
    TrivialExpression dest;
    PropagatingListener p(dest.getId());
    source.addListener(p.getId());
    dest.setListener(p.getId());
    bool transitiveChanged = false;
    TrivialListener t(transitiveChanged);
    dest.addListener(t.getId());

    // Test that all are initialized to inactive,
    // not assignable, and changed is false
    assertTrue_1(!source.isActive());
    assertTrue_1(!source.isAssignable());
    assertTrue_1(!source.changed);
    assertTrue_1(!dest.isActive());
    assertTrue_1(!dest.isAssignable());
    assertTrue_1(!dest.changed);

    // Test that notifications do nothing when expressions inactive
    source.notifyChanged();
    assertTrue_1(!source.changed);
    assertTrue_1(!dest.changed);
    dest.notifyChanged();
    assertTrue_1(!dest.changed);
    assertTrue_1(!transitiveChanged);

    // Activate dest, ensure it is active
    dest.activate();
    assertTrue_1(dest.isActive());
    assertTrue_1(!dest.changed);

    // Test that handleChange works locally
    dest.notifyChanged();
    assertTrue_1(dest.changed);
    assertTrue_1(transitiveChanged);

    // Reset changed flags
    dest.changed = transitiveChanged = false;

    // Activate source
    source.activate();
    assertTrue_1(source.isActive());

    // Test propagation
    source.notifyChanged();
    assertTrue_1(source.changed);
    assertTrue_1(dest.changed);
    assertTrue_1(transitiveChanged);

    // Reset changed flags
    dest.changed = transitiveChanged = false;

    // Test no propagation through dest when inactive
    dest.deactivate();
    source.notifyChanged();
    assertTrue_1(!dest.changed);
    assertTrue_1(!transitiveChanged);

    // Clean up
    dest.removeListener(t.getId());
    source.removeListener(p.getId());

    return true;
  }

  static bool testDirectPropagation()
  {
    // Test setup
    TrivialExpression source;
    TrivialExpression dest;
    source.addListener(dest.getId());

    // Test that all are initialized to inactive,
    // not assignable, and changed is false
    assertTrue_1(!source.isActive());
    assertTrue_1(!source.isAssignable());
    assertTrue_1(!source.changed);
    assertTrue_1(!dest.isActive());
    assertTrue_1(!dest.isAssignable());
    assertTrue_1(!dest.changed);

    // Test that notifications do nothing when expressions inactive
    source.notifyChanged();
    assertTrue_1(!source.changed);
    assertTrue_1(!dest.changed);
    dest.notifyChanged();
    assertTrue_1(!dest.changed);

    // Activate dest, ensure it is active
    dest.activate();
    assertTrue_1(dest.isActive());
    assertTrue_1(!dest.changed);

    // Test that handleChange works locally
    dest.notifyChanged();
    assertTrue_1(dest.changed);

    // Reset changed flag
    dest.changed = false;

    // Activate source
    source.activate();
    assertTrue_1(source.isActive());

    // Test propagation
    source.notifyChanged();
    assertTrue_1(source.changed);
    assertTrue_1(dest.changed);

    // Reset changed flags
    dest.changed = false;

    // Test no propagation through dest when inactive
    dest.deactivate();
    source.notifyChanged();
    assertTrue_1(!dest.changed);

    // Clean up
    source.removeListener(dest.getId());

    return true;
  }

};

//
// Constants
//

class ConstantsTest
{
public:
  static bool test()
  {
    runTest(testBasics);

    return true;
  }

private:
  static bool testBasics()
  {
    // Default constructor check
    Constant<bool> ub;
    Constant<int32_t> ui;
    Constant<double> ud;
    Constant<std::string> us;

    // Direct access
    assertTrue_1(ub.isConstant());
    assertTrue_1(ui.isConstant());
    assertTrue_1(ud.isConstant());
    assertTrue_1(us.isConstant());
    assertTrue_1(!ub.isAssignable());
    assertTrue_1(!ui.isAssignable());
    assertTrue_1(!ud.isAssignable());
    assertTrue_1(!us.isAssignable());
    assertTrue_1(!ub.isKnown());
    assertTrue_1(!ui.isKnown());
    assertTrue_1(!ud.isKnown());
    assertTrue_1(!us.isKnown());
    assertTrue_1(ub.isUnknown());
    assertTrue_1(ui.isUnknown());
    assertTrue_1(ud.isUnknown());
    assertTrue_1(us.isUnknown());

    // Access through ExpressionId
    ExpressionId iub = ub.getId();
    ExpressionId iui = ui.getId();
    ExpressionId iud = ud.getId();
    ExpressionId ius = us.getId();
    assertTrue_1(iub->isConstant());
    assertTrue_1(iui->isConstant());
    assertTrue_1(iud->isConstant());
    assertTrue_1(ius->isConstant());
    assertTrue_1(!iub->isAssignable());
    assertTrue_1(!iui->isAssignable());
    assertTrue_1(!iud->isAssignable());
    assertTrue_1(!ius->isAssignable());
    assertTrue_1(!iub->isKnown());
    assertTrue_1(!iui->isKnown());
    assertTrue_1(!iud->isKnown());
    assertTrue_1(!ius->isKnown());
    assertTrue_1(iub->isUnknown());
    assertTrue_1(iui->isUnknown());
    assertTrue_1(iud->isUnknown());
    assertTrue_1(ius->isUnknown());

    // getValue on unknowns
    double food;
    std::string foos;
    int32_t fooi;
    bool foob;
    assertTrue_1(!ub.getValue(foob));
    assertTrue_1(!ui.getValue(fooi));
    assertTrue_1(!ud.getValue(food));
    assertTrue_1(!us.getValue(foos));
    assertTrue_1(!iub->getValue(foob));
    assertTrue_1(!iui->getValue(fooi));
    assertTrue_1(!iud->getValue(food));
    assertTrue_1(!ius->getValue(foos));

    // Constants with values
    Constant<bool> troo = true;
    Constant<int32_t> too = 2;
    Constant<double> doo = 2.718;
    Constant<std::string> soo = std::string("Sue");

    assertTrue_1(troo.isConstant());
    assertTrue_1(too.isConstant());
    assertTrue_1(doo.isConstant());
    assertTrue_1(soo.isConstant());
    assertTrue_1(troo.isKnown());
    assertTrue_1(too.isKnown());
    assertTrue_1(doo.isKnown());
    assertTrue_1(soo.isKnown());
    assertTrue_1(!troo.isUnknown());
    assertTrue_1(!too.isUnknown());
    assertTrue_1(!doo.isUnknown());
    assertTrue_1(!soo.isUnknown());
    
    // getValue() test
    assertTrue_1(troo.getValue(foob));
    assertTrue_1(foob == true);
    assertTrue_1(too.getValue(fooi));
    assertTrue_1(fooi == 2);
    assertTrue_1(doo.getValue(food));
    assertTrue_1(food == 2.718);
    assertTrue_1(soo.getValue(foos));
    assertTrue_1(foos == std::string("Sue"));
    // Numeric conversion
    assertTrue_1(too.getValue(food));
    assertTrue_1(food == 2);

    return true;
  }
};

//
// Variables
//

class VariablesTest
{
public:
  static bool test()
  {
    runTest(testUninitialized);
    runTest(testInitialValue);
    runTest(testReset);
    runTest(testSavedValue);
    runTest(testNotification);

    return true;
  }

private:
  static bool testUninitialized()
  {
    // Default constructors
    UserVariable<bool> vub;
    UserVariable<int32_t> vui;
    UserVariable<double> vud;
    UserVariable<std::string> vus;

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
    assertTrue_1(vub.isUnknown());
    assertTrue_1(vui.isUnknown());
    assertTrue_1(vud.isUnknown());
    assertTrue_1(vus.isUnknown());

    double food;
    std::string foos;
    int32_t fooi;
    bool foob;

    assertTrue_1(!vub.getValue(foob));
    assertTrue_1(!vui.getValue(fooi));
    assertTrue_1(!vud.getValue(food));
    assertTrue_1(!vus.getValue(foos));
    // Numeric conversion
    assertTrue_1(!vui.getValue(food));

    // Activate and confirm they are unknown
    vub.activate();
    vui.activate();
    vud.activate();
    vus.activate();
    assertTrue_1(!vub.isKnown());
    assertTrue_1(!vui.isKnown());
    assertTrue_1(!vud.isKnown());
    assertTrue_1(!vus.isKnown());
    assertTrue_1(vub.isUnknown());
    assertTrue_1(vui.isUnknown());
    assertTrue_1(vud.isUnknown());
    assertTrue_1(vus.isUnknown());
    assertTrue_1(!vub.getValue(foob));
    assertTrue_1(!vui.getValue(fooi));
    assertTrue_1(!vud.getValue(food));
    assertTrue_1(!vus.getValue(foos));
    // Numeric conversion
    assertTrue_1(!vui.getValue(food));

    // Assign and check result
    vub.setValue(true);
    vui.setValue(42);
    vud.setValue(3.14);
    vus.setValue("yoohoo");
    assertTrue_1(vub.isKnown());
    assertTrue_1(vui.isKnown());
    assertTrue_1(vud.isKnown());
    assertTrue_1(vus.isKnown());
    assertTrue_1(!vub.isUnknown());
    assertTrue_1(!vui.isUnknown());
    assertTrue_1(!vud.isUnknown());
    assertTrue_1(!vus.isUnknown());
    
    assertTrue_1(vub.getValue(foob));
    assertTrue_1(foob == true);
    assertTrue_1(vui.getValue(fooi));
    assertTrue_1(fooi == 42);
    assertTrue_1(vud.getValue(food));
    assertTrue_1(food == 3.14);
    assertTrue_1(vus.getValue(foos));
    assertTrue_1(foos == std::string("yoohoo"));
    // Numeric conversion
    assertTrue_1(vui.getValue(food));
    assertTrue_1(food == 42);

    return true;
  }

  static bool testInitialValue()
  {
    // Default constructors
    UserVariable<bool> vb(false);
    UserVariable<int32_t> vi(69);
    UserVariable<double> vd(1.414);
    UserVariable<std::string> vs(std::string("yo"));

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
    assertTrue_1(vb.isUnknown());
    assertTrue_1(vi.isUnknown());
    assertTrue_1(vd.isUnknown());
    assertTrue_1(vs.isUnknown());

    // Activate and confirm they are known
    vb.activate();
    vi.activate();
    vd.activate();
    vs.activate();
    assertTrue_1(vb.isKnown());
    assertTrue_1(vi.isKnown());
    assertTrue_1(vd.isKnown());
    assertTrue_1(vs.isKnown());
    assertTrue_1(!vb.isUnknown());
    assertTrue_1(!vi.isUnknown());
    assertTrue_1(!vd.isUnknown());
    assertTrue_1(!vs.isUnknown());

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

    // Save current values
    vb.saveCurrentValue();
    vi.saveCurrentValue();
    vd.saveCurrentValue();
    vs.saveCurrentValue();

    // Assign and check result
    vb.setValue(true);
    vi.setValue(42);
    vd.setValue(3.14);
    vs.setValue("yoohoo");
    assertTrue_1(vb.isKnown());
    assertTrue_1(vi.isKnown());
    assertTrue_1(vd.isKnown());
    assertTrue_1(vs.isKnown());
    assertTrue_1(!vb.isUnknown());
    assertTrue_1(!vi.isUnknown());
    assertTrue_1(!vd.isUnknown());
    assertTrue_1(!vs.isUnknown());
    
    assertTrue_1(vb.getValue(foob));
    assertTrue_1(foob == true);
    assertTrue_1(vi.getValue(fooi));
    assertTrue_1(fooi == 42);
    assertTrue_1(vd.getValue(food));
    assertTrue_1(food == 3.14);
    assertTrue_1(vs.getValue(foos));
    assertTrue_1(foos == std::string("yoohoo"));
    // Numeric conversion
    assertTrue_1(vi.getValue(food));
    assertTrue_1(food == 42);

    // Test setValue type conversion
    int32_t fiftySeven = 57;
    vd.setValue(fiftySeven);
    assertTrue_1(vd.getValue(food));
    assertTrue_1(food == 57);

    // Restore saved values and check result
    vb.restoreSavedValue();
    vi.restoreSavedValue();
    vd.restoreSavedValue();
    vs.restoreSavedValue();
    assertTrue_1(vb.isKnown());
    assertTrue_1(vi.isKnown());
    assertTrue_1(vd.isKnown());
    assertTrue_1(vs.isKnown());
    assertTrue_1(!vb.isUnknown());
    assertTrue_1(!vi.isUnknown());
    assertTrue_1(!vd.isUnknown());
    assertTrue_1(!vs.isUnknown());

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

    return true;
  }

  static bool testReset()
  {
    UserVariable<bool> vub;
    UserVariable<int32_t> vui;
    UserVariable<double> vud;
    UserVariable<std::string> vus;

    // Assign
    vub.setValue(true);
    vui.setValue(42);
    vud.setValue(3.14);
    vus.setValue("yoohoo");

    // Test that they are unknown while inactive
    assertTrue_1(!vub.isKnown());
    assertTrue_1(!vui.isKnown());
    assertTrue_1(!vud.isKnown());
    assertTrue_1(!vus.isKnown());
    assertTrue_1(vub.isUnknown());
    assertTrue_1(vui.isUnknown());
    assertTrue_1(vud.isUnknown());
    assertTrue_1(vus.isUnknown());

    double food;
    std::string foos;
    int32_t fooi;
    bool foob;

    assertTrue_1(!vub.getValue(foob));
    assertTrue_1(!vui.getValue(fooi));
    assertTrue_1(!vud.getValue(food));
    assertTrue_1(!vus.getValue(foos));
    // Numeric conversion
    assertTrue_1(!vui.getValue(food));

    // Activate and check result
    vub.activate();
    vui.activate();
    vud.activate();
    vus.activate();
    assertTrue_1(vub.isKnown());
    assertTrue_1(vui.isKnown());
    assertTrue_1(vud.isKnown());
    assertTrue_1(vus.isKnown());
    assertTrue_1(!vub.isUnknown());
    assertTrue_1(!vui.isUnknown());
    assertTrue_1(!vud.isUnknown());
    assertTrue_1(!vus.isUnknown());
    
    assertTrue_1(vub.getValue(foob));
    assertTrue_1(foob == true);
    assertTrue_1(vui.getValue(fooi));
    assertTrue_1(fooi == 42);
    assertTrue_1(vud.getValue(food));
    assertTrue_1(food == 3.14);
    assertTrue_1(vus.getValue(foos));
    assertTrue_1(foos == std::string("yoohoo"));
    // Numeric conversion
    assertTrue_1(vui.getValue(food));
    assertTrue_1(food == 42);

    // Reset and check result
    vub.reset();
    vui.reset();
    vud.reset();
    vus.reset();
    assertTrue_1(!vub.isKnown());
    assertTrue_1(!vui.isKnown());
    assertTrue_1(!vud.isKnown());
    assertTrue_1(!vus.isKnown());
    assertTrue_1(vub.isUnknown());
    assertTrue_1(vui.isUnknown());
    assertTrue_1(vud.isUnknown());
    assertTrue_1(vus.isUnknown());
    assertTrue_1(!vub.getValue(foob));
    assertTrue_1(!vui.getValue(fooi));
    assertTrue_1(!vud.getValue(food));
    assertTrue_1(!vus.getValue(foos));
    // Numeric conversion
    assertTrue_1(!vui.getValue(food));

    // Assign again
    vub.setValue(true);
    assertTrue_1(!vub.isUnknown());
    vui.setValue(42);
    assertTrue_1(!vui.isUnknown());
    vud.setValue(3.14);
    assertTrue_1(!vud.isUnknown());
    vus.setValue("yoohoo");
    assertTrue_1(!vus.isUnknown());

    assertTrue_1(vub.getValue(foob));
    assertTrue_1(foob == true);
    assertTrue_1(vui.getValue(fooi));
    assertTrue_1(fooi == 42);
    assertTrue_1(vud.getValue(food));
    assertTrue_1(food == 3.14);
    assertTrue_1(vus.getValue(foos));
    assertTrue_1(foos == std::string("yoohoo"));
    // Numeric conversion
    assertTrue_1(vui.getValue(food));
    assertTrue_1(food == 42);

    return true;
  }

  static bool testSavedValue()
  {
    UserVariable<bool> vub;
    UserVariable<int32_t> vui;
    UserVariable<double> vud;
    UserVariable<std::string> vus;

    // Assign and check result
    vub.setValue(true);
    vui.setValue(42);
    vud.setValue(3.14);
    vus.setValue("yoohoo");
    
    // Should be unknown while inactive
    assertTrue_1(!vub.isKnown());
    assertTrue_1(!vui.isKnown());
    assertTrue_1(!vud.isKnown());
    assertTrue_1(!vus.isKnown());
    assertTrue_1(vub.isUnknown());
    assertTrue_1(vui.isUnknown());
    assertTrue_1(vud.isUnknown());
    assertTrue_1(vus.isUnknown());

    double food;
    std::string foos;
    int32_t fooi;
    bool foob;

    assertTrue_1(!vub.getValue(foob));
    assertTrue_1(!vui.getValue(fooi));
    assertTrue_1(!vud.getValue(food));
    assertTrue_1(!vus.getValue(foos));

    // Activate and recheck
    vub.activate();
    vui.activate();
    vud.activate();
    vus.activate();
    assertTrue_1(vub.isKnown());
    assertTrue_1(vui.isKnown());
    assertTrue_1(vud.isKnown());
    assertTrue_1(vus.isKnown());
    assertTrue_1(!vub.isUnknown());
    assertTrue_1(!vui.isUnknown());
    assertTrue_1(!vud.isUnknown());
    assertTrue_1(!vus.isUnknown());
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
    assertTrue_1(vub.isUnknown());
    assertTrue_1(vui.isUnknown());
    assertTrue_1(vud.isUnknown());
    assertTrue_1(vus.isUnknown());
    assertTrue_1(!vub.getValue(foob));
    assertTrue_1(!vui.getValue(fooi));
    assertTrue_1(!vud.getValue(food));
    assertTrue_1(!vus.getValue(foos));

    // Assign again
    vub.setValue(true);
    vui.setValue(42);
    vud.setValue(3.14);
    vus.setValue("yoohoo");
    assertTrue_1(vub.isKnown());
    assertTrue_1(vui.isKnown());
    assertTrue_1(vud.isKnown());
    assertTrue_1(vus.isKnown());
    assertTrue_1(!vub.isUnknown());
    assertTrue_1(!vui.isUnknown());
    assertTrue_1(!vud.isUnknown());
    assertTrue_1(!vus.isUnknown());
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
    assertTrue_1(vub.isUnknown());
    assertTrue_1(vui.isUnknown());
    assertTrue_1(vud.isUnknown());
    assertTrue_1(vus.isUnknown());
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
    assertTrue_1(!vub.isUnknown());
    assertTrue_1(!vui.isUnknown());
    assertTrue_1(!vud.isUnknown());
    assertTrue_1(!vus.isUnknown());
    assertTrue_1(vub.getValue(foob));
    assertTrue_1(vui.getValue(fooi));
    assertTrue_1(vud.getValue(food));
    assertTrue_1(vus.getValue(foos));
    assertTrue_1(foob == true);
    assertTrue_1(fooi == 42);
    assertTrue_1(food == 3.14);
    assertTrue_1(foos == std::string("yoohoo"));

    // Reset
    vub.reset();
    vui.reset();
    vud.reset();
    vus.reset();
    assertTrue_1(!vub.isKnown());
    assertTrue_1(!vui.isKnown());
    assertTrue_1(!vud.isKnown());
    assertTrue_1(!vus.isKnown());
    assertTrue_1(vub.isUnknown());
    assertTrue_1(vui.isUnknown());
    assertTrue_1(vud.isUnknown());
    assertTrue_1(vus.isUnknown());
    assertTrue_1(!vub.getValue(foob));
    assertTrue_1(!vui.getValue(fooi));
    assertTrue_1(!vud.getValue(food));
    assertTrue_1(!vus.getValue(foos));

    // Restore and confirm still unknown
    vub.restoreSavedValue();
    vui.restoreSavedValue();
    vud.restoreSavedValue();
    vus.restoreSavedValue();
    assertTrue_1(!vub.isKnown());
    assertTrue_1(!vui.isKnown());
    assertTrue_1(!vud.isKnown());
    assertTrue_1(!vus.isKnown());
    assertTrue_1(vub.isUnknown());
    assertTrue_1(vui.isUnknown());
    assertTrue_1(vud.isUnknown());
    assertTrue_1(vus.isUnknown());
    assertTrue_1(!vub.getValue(foob));
    assertTrue_1(!vui.getValue(fooi));
    assertTrue_1(!vud.getValue(food));
    assertTrue_1(!vus.getValue(foos));

    return true;
  }

  static bool testNotification()
  {
    UserVariable<bool> vub;
    UserVariable<int32_t> vui;
    UserVariable<double> vud;
    UserVariable<std::string> vus;
    bool bchanged = false,
      ichanged = false, 
      dchanged = false,
      schanged = false;
    TrivialListener lb(bchanged);
    TrivialListener li(ichanged);
    TrivialListener ld(dchanged);
    TrivialListener ls(schanged);
    vub.addListener(lb.getId());
    vui.addListener(li.getId());
    vud.addListener(ld.getId());
    vus.addListener(ls.getId());

    // Assign and check whether the listeners were notified
    vub.setValue(true);
    vui.setValue(42);
    vud.setValue(3.14);
    vus.setValue("yoohoo");
    assertTrue_1(!bchanged);
    assertTrue_1(!ichanged);
    assertTrue_1(!dchanged);
    assertTrue_1(!schanged);

    // Activation alone should not notify
    vub.activate();
    vui.activate();
    vud.activate();
    vus.activate();
    assertTrue_1(!bchanged);
    assertTrue_1(!ichanged);
    assertTrue_1(!dchanged);
    assertTrue_1(!schanged);

    // Reset should notify
    vub.reset();
    vui.reset();
    vud.reset();
    vus.reset();
    assertTrue_1(bchanged);
    assertTrue_1(ichanged);
    assertTrue_1(dchanged);
    assertTrue_1(schanged);

    // Assign should notify this time
    bchanged = ichanged = dchanged = schanged = false;
    vub.setValue(true);
    vui.setValue(42);
    vud.setValue(3.14);
    vus.setValue("yoohoo");
    assertTrue_1(bchanged);
    assertTrue_1(ichanged);
    assertTrue_1(dchanged);
    assertTrue_1(schanged);

    // Save current value shouldn't notify
    bchanged = ichanged = dchanged = schanged = false;
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
    vub.removeListener(lb.getId());
    vui.removeListener(li.getId());
    vud.removeListener(ld.getId());
    vus.removeListener(ls.getId());

    return true;
  }
};

class ExprModuleTests {
public:
  static void runTests()
  {
    runTestSuite(ListenerTest::test);
    runTestSuite(ConstantsTest::test);
    runTestSuite(VariablesTest::test);

    // clean up
    runFinalizers();

    std::cout << "Finished" << std::endl;
  }
};

int main(int argc, char *argv[])
{
  ExprModuleTests::runTests();
  return 0;
}
