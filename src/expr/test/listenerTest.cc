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


//
// ExpressionListener tests
//

#include "Error.hh"
#include "NotifierImpl.hh"
#include "TestSupport.hh"
#include "test/TrivialListener.hh"
#include "Value.hh"

class PropagatingListener : public PLEXIL::ExpressionListener
{
public:
  PropagatingListener(PLEXIL::Expression &owner)
  : PLEXIL::ExpressionListener(),
    m_owner(owner)
  {
  }

protected:
  void notifyChanged(PLEXIL::Expression const *src)
  {
    m_owner.notifyChanged(src);
  }

private:
  PLEXIL::Expression &m_owner;
};


class TrivialExpression : public PLEXIL::NotifierImpl
{
public:
  TrivialExpression()
    : PLEXIL::NotifierImpl(),
      changed(false)
  {
  }

  // Only ever called when active
  void handleChange(PLEXIL::Expression const * src)
  {
    changed = true;
    publishChange(src);
  }

  const char *exprName() const { return "trivial"; }
  const PLEXIL::ValueType valueType() const { return PLEXIL::UNKNOWN_TYPE; }

  void print(std::ostream& s) const {}
  void printValue(std::ostream& s) const {}

  bool isConstant() const { return true; }
  bool isKnown() const { return false; }

  PLEXIL::Value toValue() const { return PLEXIL::Value(); }

public:
  bool changed;
};

static bool testListenerPropagation()
{
  // Test setup
  TrivialExpression source;
  TrivialExpression dest;
  TrivialExpression dummy;
  PropagatingListener p(dest);
  source.addListener(&p);
  bool transitiveChanged = false;
  TrivialListener t(transitiveChanged);
  dest.addListener(&t);

  // Test that all are initialized to inactive,
  // not assignable, and changed is false
  assertTrue_1(!source.isActive());
  assertTrue_1(!source.isAssignable());
  assertTrue_1(!source.changed);
  assertTrue_1(!dest.isActive());
  assertTrue_1(!dest.isAssignable());
  assertTrue_1(!dest.changed);

  // Test that notifications do nothing when expressions inactive
  source.notifyChanged(&dummy);
  assertTrue_1(!source.changed);
  assertTrue_1(!dest.changed);
  dest.notifyChanged(&dummy);
  assertTrue_1(!dest.changed);
  assertTrue_1(!transitiveChanged);

  // Activate dest, ensure it is active
  dest.activate();
  assertTrue_1(dest.isActive());
  assertTrue_1(!dest.changed);

  // Test that handleChange works locally
  dest.notifyChanged(&dummy);
  assertTrue_1(dest.changed);
  assertTrue_1(transitiveChanged);

  // Reset changed flags
  dest.changed = transitiveChanged = false;

  // Activate source
  source.activate();
  assertTrue_1(source.isActive());

  // Test propagation
  source.notifyChanged(&dummy);
  assertTrue_1(source.changed);
  assertTrue_1(dest.changed);
  assertTrue_1(transitiveChanged);

  // Reset changed flags
  dest.changed = transitiveChanged = false;

  // Test no propagation through dest when inactive
  dest.deactivate();
  source.notifyChanged(&dummy);
  assertTrue_1(!dest.changed);
  assertTrue_1(!transitiveChanged);

  // Clean up
  dest.removeListener(&t);
  source.removeListener(&p);

  return true;
}

static bool testDirectPropagation()
{
  // Test setup
  TrivialExpression source;
  TrivialExpression dest;
  TrivialExpression dummy;
  source.addListener(&dest);

  // Test that all are initialized to inactive,
  // not assignable, and changed is false
  assertTrue_1(!source.isActive());
  assertTrue_1(!source.isAssignable());
  assertTrue_1(!source.changed);
  assertTrue_1(!dest.isActive());
  assertTrue_1(!dest.isAssignable());
  assertTrue_1(!dest.changed);

  // Test that notifications do nothing when expressions inactive
  source.notifyChanged(&dummy);
  assertTrue_1(!source.changed);
  assertTrue_1(!dest.changed);
  dest.notifyChanged(&dummy);
  assertTrue_1(!dest.changed);

  // Activate dest, ensure it is active
  dest.activate();
  assertTrue_1(dest.isActive());
  assertTrue_1(!dest.changed);

  // Test that handleChange works locally
  dest.notifyChanged(&dummy);
  assertTrue_1(dest.changed);

  // Reset changed flag
  dest.changed = false;

  // Activate source
  source.activate();
  assertTrue_1(source.isActive());

  // Test propagation
  source.notifyChanged(&dummy);
  assertTrue_1(source.changed);
  assertTrue_1(dest.changed);

  // Reset changed flags
  dest.changed = false;

  // Test no propagation through dest when inactive
  dest.deactivate();
  source.notifyChanged(&dummy);
  assertTrue_1(!dest.changed);

  // Clean up
  source.removeListener(&dest);

  return true;
}

bool listenerTest()
{
  runTest(testListenerPropagation);
  runTest(testDirectPropagation);
  return true;
}






