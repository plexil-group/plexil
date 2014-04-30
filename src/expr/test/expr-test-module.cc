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

#include "ArithmeticOperators.hh" // includes Operator
#include "Constant.hh"       // includes ExpressionListener, Expression
#include "Function.hh"
#include "lifecycle-utils.h"
#include "TestSupport.hh"
#include "UserVariable.hh"   // includes Mutable, Assignable

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
  const ValueType valueType() const { return UNKNOWN_TYPE; }

  void print(std::ostream& s) const {}
  void printValue(std::ostream& s) const {}

  bool isConstant() const { return true; }
  bool isKnown() const { return false; }

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
    BooleanConstant ub;
    IntegerConstant ui;
    RealConstant ud;
    StringConstant us;

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
    BooleanConstant troo = true;
    IntegerConstant too = 2;
    RealConstant doo = 2.718;
    StringConstant soo = std::string("Sue");

    assertTrue_1(troo.isConstant());
    assertTrue_1(too.isConstant());
    assertTrue_1(doo.isConstant());
    assertTrue_1(soo.isConstant());
    assertTrue_1(troo.isKnown());
    assertTrue_1(too.isKnown());
    assertTrue_1(doo.isKnown());
    assertTrue_1(soo.isKnown());
    
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
    
    // getValue() through ExpressionId test
    assertTrue_1((troo.getId())->getValue(foob));
    assertTrue_1(foob == true);
    assertTrue_1((too.getId())->getValue(fooi));
    assertTrue_1(fooi == 2);
    assertTrue_1((doo.getId())->getValue(food));
    assertTrue_1(food == 2.718);
    assertTrue_1((soo.getId())->getValue(foos));
    assertTrue_1(foos == std::string("Sue"));
    // Numeric conversion
    assertTrue_1((too.getId())->getValue(food));
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
    runTest(testExpressionId);
    runTest(testSavedValue);
    runTest(testAssignableId);
    runTest(testNotification);

    return true;
  }

private:
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
    assertTrue_1(!vub.isKnown());
    assertTrue_1(!vui.isKnown());
    assertTrue_1(!vud.isKnown());
    assertTrue_1(!vus.isKnown());
    assertTrue_1(!vub.getValue(foob));
    assertTrue_1(!vui.getValue(fooi));
    assertTrue_1(!vud.getValue(food));
    assertTrue_1(!vus.getValue(foos));

    // Set initial and current values, and check result
    vub.setInitialValue(true);
    vui.setInitialValue(42);
    vud.setInitialValue(3.14);
    vus.setInitialValue(std::string("yoohoo"));

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
    vui.setValue(69);
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

    // Reset and check that the new initial values are back
    vub.reset();
    vui.reset();
    vud.reset();
    vus.reset();

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

    return true;
  }

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

    // Reset and check that initial value is restored
    vb.reset();
    vi.reset();
    vd.reset();
    vs.reset();

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

    // Set initial values to unknown
    vb.setInitialUnknown();
    vi.setInitialUnknown();
    vd.setInitialUnknown();
    vs.setInitialUnknown();

    // Confirm that the current values are now unknown
    assertTrue_1(!vb.isKnown());
    assertTrue_1(!vi.isKnown());
    assertTrue_1(!vd.isKnown());
    assertTrue_1(!vs.isKnown());
    assertTrue_1(!vb.getValue(foob));
    assertTrue_1(!vi.getValue(fooi));
    assertTrue_1(!vd.getValue(food));
    assertTrue_1(!vs.getValue(foos));

    // Set values and check
    vb.setValue(false);
    vi.setValue(69);
    vd.setValue(1.414);
    vs.setValue(std::string("yo"));

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

    // Reset and check that initial value is now unknown
    vb.reset();
    vi.reset();
    vd.reset();
    vs.reset();
    assertTrue_1(!vb.isKnown());
    assertTrue_1(!vi.isKnown());
    assertTrue_1(!vd.isKnown());
    assertTrue_1(!vs.isKnown());
    assertTrue_1(!vb.getValue(foob));
    assertTrue_1(!vi.getValue(fooi));
    assertTrue_1(!vd.getValue(food));
    assertTrue_1(!vs.getValue(foos));

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

    ExpressionId eb(vb.getId());
    ExpressionId ei(vi.getId());
    ExpressionId ed(vd.getId());
    ExpressionId es(vs.getId());

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

    // Test that they are unknown while inactive
    assertTrue_1(!eb->isKnown());
    assertTrue_1(!ei->isKnown());
    assertTrue_1(!ed->isKnown());
    assertTrue_1(!es->isKnown());

    // Activate and confirm they are known
    eb->activate();
    ei->activate();
    ed->activate();
    es->activate();
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
    vui.setValue(42);
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
    vui.setValue(42);
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
    vub.reset();
    vui.reset();
    vud.reset();
    vus.reset();
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
  // through a pointer of type AssignableId.
  static bool testAssignableId()
  {
    BooleanVariable vb(false);
    IntegerVariable vi(69);
    RealVariable vd(1.414);
    StringVariable vs(std::string("yo"));

    AssignableId eb(vb.getId());
    AssignableId ei(vi.getId());
    AssignableId ed(vd.getId());
    AssignableId es(vs.getId());

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
    ei->setValue(42);
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
    const int32_t fiftySeven = 57;
    ed->setValue(fiftySeven);
    assertTrue_1(ed->getValue(food));
    assertTrue_1(food == 57);
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
    assertTrue_1(food == 57);
    assertTrue_1(foos == std::string("yoyo"));

    // Reset
    eb->reset();
    ei->reset();
    ed->reset();
    es->reset();
    
    // Check initial values are restored
    assertTrue_1(eb->getValue(foob));
    assertTrue_1(foob == false);
    assertTrue_1(ei->getValue(fooi));
    assertTrue_1(fooi == 69);
    assertTrue_1(ed->getValue(food));
    assertTrue_1(food == 1.414);
    assertTrue_1(es->getValue(foos));
    assertTrue_1(foos == std::string("yo"));

    // setInitialValue tests
    eb->setInitialValue(true);
    ei->setInitialValue(42);
    ed->setInitialValue(3.14);
    es->setInitialValue(std::string("yoohoo"));
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
    assertTrue_1(foos == std::string("yoohoo"));

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

    // Reset
    eb->reset();
    ei->reset();
    ed->reset();
    es->reset();
    
    // Check initial values are restored
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
    assertTrue_1(foos == std::string("yoohoo"));

    // setInitialValue type conversions
    ed->setInitialValue(fiftySeven);
    es->setInitialValue("yoyo");
    assertTrue_1(ed->isKnown());
    assertTrue_1(es->isKnown());
    assertTrue_1(ed->getValue(food));
    assertTrue_1(es->getValue(foos));
    assertTrue_1(food == 57);
    assertTrue_1(foos == std::string("yoyo"));

    // Set unknown and check
    ed->setUnknown();
    es->setUnknown();
    assertTrue_1(!ed->isKnown());
    assertTrue_1(!es->isKnown());
    assertTrue_1(!ed->getValue(food));
    assertTrue_1(!es->getValue(foos));

    // Reset and recheck
    ed->reset();
    es->reset();
    assertTrue_1(ed->isKnown());
    assertTrue_1(es->isKnown());
    assertTrue_1(ed->getValue(food));
    assertTrue_1(es->getValue(foos));
    assertTrue_1(food == 57);
    assertTrue_1(foos == std::string("yoyo"));

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
    vub.addListener(lb.getId());
    vui.addListener(li.getId());
    vud.addListener(ld.getId());
    vus.addListener(ls.getId());

    // setInitialValue shouldn't notify while inactive
    vub.setInitialValue(false);
    vui.setInitialValue(69);
    vud.setInitialValue(1.414);
    vus.setInitialValue(std::string("jojo"));
    assertTrue_1(!bchanged);
    assertTrue_1(!ichanged);
    assertTrue_1(!dchanged);
    assertTrue_1(!schanged);

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

    // setInitialValue shouldn't notify
    bchanged = ichanged = dchanged = schanged = false;
    vub.setInitialValue(false);
    vui.setInitialValue(69);
    vud.setInitialValue(1.414);
    vus.setInitialValue(std::string("jojo"));
    assertTrue_1(!bchanged);
    assertTrue_1(!ichanged);
    assertTrue_1(!dchanged);
    assertTrue_1(!schanged);

    // Clean up
    vub.removeListener(lb.getId());
    vui.removeListener(li.getId());
    vud.removeListener(ld.getId());
    vus.removeListener(ls.getId());

    return true;
  }
};

//
// Functions and operators
//

template <typename R>
class Passthrough : public Operator<R>
{
public:
  Passthrough()
  {
    Operator<R>::m_name = "PT";
  }

  ~Passthrough()
  {
  }

  bool operator()(R &result, const ExpressionId &arg) const
  {
    R temp;
    if (!arg->getValue(temp))
      return false;
    result = temp;
    return true;
  }
};

class FunctionsTest {
public:
  static bool test()
  {
    runTest(testUnaryBasics);
    runTest(testUnaryPropagation);
    runTest(testBinaryBasics);
    runTest(testNaryBasics);
    return true;
  }

private:
  static bool testUnaryBasics()
  {
    BooleanConstant treu(true);
    IntegerConstant fortytwo(42);
    RealConstant pie(3.14);
    StringConstant fou("Foo");

    Passthrough<bool> ptb;
    Passthrough<int32_t> pti;
    Passthrough<double> ptd;
    Passthrough<std::string> pts;

    UnaryFunction<bool> boule(&ptb, treu.getId());
    UnaryFunction<int32_t> inty(&pti, fortytwo.getId());
    UnaryFunction<double> dub(&ptd, pie.getId());
    UnaryFunction<double> intd(&ptd, fortytwo.getId());
    UnaryFunction<std::string> str(&pts, fou.getId());

    // Test that all are unknown when inactive
    assertTrue_1(!boule.isKnown());
    assertTrue_1(!inty.isKnown());
    assertTrue_1(!dub.isKnown());
    assertTrue_1(!intd.isKnown());
    assertTrue_1(!str.isKnown());

    // Activate and check values
    boule.activate();
    inty.activate();
    dub.activate();
    intd.activate();
    str.activate();

    bool tempb;
    int32_t tempi;
    double tempd, tempdi;
    std::string temps;
    assertTrue_1(boule.getValue(tempb));
    assertTrue_1(inty.getValue(tempi));
    assertTrue_1(dub.getValue(tempd));
    assertTrue_1(intd.getValue(tempdi));
    assertTrue_1(str.getValue(temps));

    assertTrue_1(tempb == true);
    assertTrue_1(tempi == 42);
    assertTrue_1(tempd == 3.14);
    assertTrue_1(tempdi == 42);
    assertTrue_1(temps == std::string("Foo"));

    // TODO - test propagation of changes through variable and fn

    return true;
  }
  static bool testUnaryPropagation()
  {
    BooleanVariable treu(true);
    IntegerVariable fortytwo(42);
    RealVariable pie(3.14);
    StringVariable fou("Foo");

    Passthrough<bool> ptb;
    Passthrough<int32_t> pti;
    Passthrough<double> ptd;
    Passthrough<std::string> pts;

    UnaryFunction<bool> boule(&ptb, treu.getId());
    UnaryFunction<int32_t> inty(&pti, fortytwo.getId());
    UnaryFunction<double> dub(&ptd, pie.getId());
    UnaryFunction<double> intd(&ptd, fortytwo.getId());
    UnaryFunction<std::string> str(&pts, fou.getId());

    bool bchanged = false;
    bool ichanged = false;
    bool rchanged = false;
    bool r2changed = false;
    bool schanged = false;

    TrivialListener bl(bchanged);
    TrivialListener il(ichanged);
    TrivialListener rl(rchanged);
    TrivialListener rl2(r2changed);
    TrivialListener sl(schanged);

    boule.addListener(bl.getId());
    inty.addListener(il.getId());
    dub.addListener(rl.getId());
    intd.addListener(rl2.getId());
    str.addListener(sl.getId());

    // Check propagation doesn't happen when inactive
    treu.setValue(false);
    fortytwo.setValue(43);
    pie.setValue(2.718);
    fou.setValue(std::string("fu"));

    assertTrue_1(!bchanged);
    assertTrue_1(!ichanged);
    assertTrue_1(!rchanged);
    assertTrue_1(!r2changed);
    assertTrue_1(!schanged);

    // Check that variables get activated when functions do
    boule.activate();
    assertTrue_1(treu.isActive());
    inty.activate();
    assertTrue_1(fortytwo.isActive());
    dub.activate();
    assertTrue_1(pie.isActive());
    // inty and intd share the same variable
    inty.deactivate();
    intd.activate();
    assertTrue_1(fortytwo.isActive());
    str.activate();
    assertTrue_1(fou.isActive());
    // reactivate inty
    inty.activate();

    bool boolv;
    int32_t intv;
    double dubv;
    std::string strv;

    // Check function values
    assertTrue_1(boule.getValue(boolv));
    assertTrue_1(!boolv);
    assertTrue_1(inty.getValue(intv));
    assertTrue_1(intv == 43);
    assertTrue_1(intd.getValue(dubv));
    assertTrue_1(dubv == 43);
    assertTrue_1(dub.getValue(dubv));
    assertTrue_1(dubv == 2.718);
    assertTrue_1(str.getValue(strv));
    assertTrue_1(strv == std::string("fu"));

    // Check propagation does happen when active
    treu.reset();
    fortytwo.reset();
    pie.reset();
    fou.reset();

    assertTrue_1(bchanged);
    assertTrue_1(ichanged);
    assertTrue_1(rchanged);
    assertTrue_1(r2changed);
    assertTrue_1(schanged);

    // Clean up
    boule.removeListener(bl.getId());
    inty.removeListener(il.getId());
    dub.removeListener(rl.getId());
    intd.removeListener(rl2.getId());
    str.removeListener(sl.getId());

    return true;
  }

  static bool testBinaryBasics()
  {
    Addition<int32_t> intAdd;
    Addition<double> realAdd;

    IntegerVariable won(1);
    IntegerConstant too(2);
    RealVariable tree(3);
    RealConstant fore(4);

    BinaryFunction<int32_t> intFn(&intAdd, won.getId(), too.getId());
    BinaryFunction<double> realFn(&realAdd, tree.getId(), fore.getId());

    int32_t itemp;
    double rtemp;

    bool ichanged = false;
    bool rchanged = false;

    TrivialListener il(ichanged);
    TrivialListener rl(rchanged);

    intFn.addListener(il.getId());
    realFn.addListener(rl.getId());

    // Check that variables and functions are inactive when created
    assertTrue_1(!intFn.isActive());
    assertTrue_1(!realFn.isActive());
    assertTrue_1(!won.isActive());
    assertTrue_1(!tree.isActive());

    // Check that values are unknown when inactive
    assertTrue_1(!won.isKnown());
    assertTrue_1(!won.getValue(itemp));
    assertTrue_1(!tree.isKnown());
    assertTrue_1(!tree.getValue(rtemp));
    assertTrue_1(!intFn.isKnown());
    assertTrue_1(!intFn.getValue(itemp));
    assertTrue_1(!realFn.isKnown());
    assertTrue_1(!realFn.getValue(rtemp));

    // Activate expressions, check that both they and their arguments are now active
    intFn.activate();
    realFn.activate();
    assertTrue_1(intFn.isActive());
    assertTrue_1(realFn.isActive());
    assertTrue_1(won.isActive());
    assertTrue_1(tree.isActive());

    // Check that values are known and reasonable
    assertTrue_1(won.isKnown());
    assertTrue_1(tree.isKnown());
    assertTrue_1(intFn.isKnown());
    assertTrue_1(realFn.isKnown());
    assertTrue_1(won.getValue(itemp));
    assertTrue_1(tree.getValue(rtemp));
    assertTrue_1(itemp == 1);
    assertTrue_1(rtemp == 3);
    assertTrue_1(intFn.getValue(itemp));
    assertTrue_1(realFn.getValue(rtemp));
    assertTrue_1(itemp == 3);
    assertTrue_1(rtemp == 7);

    // No notifications should have happened yet
    assertTrue_1(!ichanged);
    assertTrue_1(!rchanged);

    // Set the variables unknown and check that they and epxressions are now unknown
    won.setUnknown();
    tree.setUnknown();
    assertTrue_1(!won.isKnown());
    assertTrue_1(!won.getValue(itemp));
    assertTrue_1(!tree.isKnown());
    assertTrue_1(!tree.getValue(rtemp));
    assertTrue_1(!intFn.isKnown());
    assertTrue_1(!intFn.getValue(itemp));
    assertTrue_1(!realFn.isKnown());
    assertTrue_1(!realFn.getValue(rtemp));

    // Check that notifications have occurred, and clear them
    assertTrue_1(ichanged);
    assertTrue_1(rchanged);
    ichanged = rchanged = false;

    // Reset variables, check that values are known and reasonable
    won.reset();
    tree.reset();
    assertTrue_1(won.isKnown());
    assertTrue_1(tree.isKnown());
    assertTrue_1(intFn.isKnown());
    assertTrue_1(realFn.isKnown());
    assertTrue_1(won.getValue(itemp));
    assertTrue_1(tree.getValue(rtemp));
    assertTrue_1(itemp == 1);
    assertTrue_1(rtemp == 3);
    assertTrue_1(intFn.getValue(itemp));
    assertTrue_1(realFn.getValue(rtemp));
    assertTrue_1(itemp == 3);
    assertTrue_1(rtemp == 7);

    // Check that notifications have occurred
    assertTrue_1(ichanged);
    assertTrue_1(rchanged);

    // Clean up
    intFn.removeListener(il.getId());
    realFn.removeListener(rl.getId());

    return true;
  }

  static bool testNaryBasics()
  {
    Addition<int32_t> intAdd;
    Addition<double> realAdd;

    IntegerVariable won(1);
    IntegerConstant too(2);
    IntegerVariable tree(3);

    RealConstant fore(4);
    RealVariable fivefive(5.5);
    RealVariable sixfive(6.5);

    std::vector<ExpressionId> exprs;
    const std::vector<bool> garbage(3, false);

    exprs.push_back(won.getId());
    exprs.push_back(too.getId());
    exprs.push_back(tree.getId());

    NaryFunction<int32_t> intFn(&intAdd, exprs, garbage);

    exprs.clear();
    exprs.push_back(fore.getId());
    exprs.push_back(fivefive.getId());
    exprs.push_back(sixfive.getId());

    NaryFunction<double> realFn(&realAdd, exprs, garbage);

    int32_t itemp;
    double rtemp;

    bool ichanged = false;
    bool rchanged = false;

    TrivialListener il(ichanged);
    TrivialListener rl(rchanged);

    intFn.addListener(il.getId());
    realFn.addListener(rl.getId());

    // Check that variables and functions are inactive when created
    assertTrue_1(!intFn.isActive());
    assertTrue_1(!realFn.isActive());
    assertTrue_1(!won.isActive());
    assertTrue_1(!tree.isActive());
    assertTrue_1(!fivefive.isActive());
    assertTrue_1(!sixfive.isActive());

    // Check that values are unknown when inactive
    assertTrue_1(!intFn.isKnown());
    assertTrue_1(!intFn.getValue(itemp));
    assertTrue_1(!realFn.isKnown());
    assertTrue_1(!realFn.getValue(rtemp));

    // Activate expressions, check that both they and their arguments are now active
    intFn.activate();
    realFn.activate();
    assertTrue_1(intFn.isActive());
    assertTrue_1(realFn.isActive());
    assertTrue_1(won.isActive());
    assertTrue_1(tree.isActive());
    assertTrue_1(fivefive.isActive());
    assertTrue_1(sixfive.isActive());

    // Check that values are known and reasonable
    assertTrue_1(intFn.isKnown());
    assertTrue_1(realFn.isKnown());
    assertTrue_1(intFn.getValue(itemp));
    assertTrue_1(realFn.getValue(rtemp));
    assertTrue_1(itemp == 6);
    assertTrue_1(rtemp == 16);

    // No notifications should have happened yet
    assertTrue_1(!ichanged);
    assertTrue_1(!rchanged);

    // Set the variables unknown and check that they and epxressions are now unknown
    tree.setUnknown();
    fivefive.setUnknown();
    assertTrue_1(!tree.isKnown());
    assertTrue_1(!tree.getValue(itemp));
    assertTrue_1(!fivefive.isKnown());
    assertTrue_1(!fivefive.getValue(rtemp));
    assertTrue_1(!intFn.isKnown());
    assertTrue_1(!intFn.getValue(itemp));
    assertTrue_1(!realFn.isKnown());
    assertTrue_1(!realFn.getValue(rtemp));

    // Check that notifications have occurred, and clear them
    assertTrue_1(ichanged);
    assertTrue_1(rchanged);
    ichanged = rchanged = false;

    // Reset variables, check that values are known and reasonable
    tree.reset();
    fivefive.reset();
    assertTrue_1(tree.isKnown());
    assertTrue_1(fivefive.isKnown());
    assertTrue_1(intFn.isKnown());
    assertTrue_1(realFn.isKnown());
    assertTrue_1(tree.getValue(itemp));
    assertTrue_1(fivefive.getValue(rtemp));
    assertTrue_1(itemp == 3);
    assertTrue_1(rtemp == 5.5);
    assertTrue_1(intFn.getValue(itemp));
    assertTrue_1(realFn.getValue(rtemp));
    assertTrue_1(itemp == 6);
    assertTrue_1(rtemp == 16);

    // Check that notifications have occurred
    assertTrue_1(ichanged);
    assertTrue_1(rchanged);

    // Clean up
    intFn.removeListener(il.getId());
    realFn.removeListener(rl.getId());

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
    runTestSuite(FunctionsTest::test);

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
