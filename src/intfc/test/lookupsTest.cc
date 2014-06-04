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

#include "ExternalInterface.hh"
#include "Constant.hh"
#include "Lookup.hh"
#include "PlexilLookup.hh"
#include "StateCacheEntry.hh"
#include "StateCacheMap.hh"
#include "TestSupport.hh"
#include "UserVariable.hh"

#include <map>
#include <set>

using namespace PLEXIL;

class TestInterface : public ExternalInterface 
{
public:
  TestInterface()
    : ExternalInterface(),
      m_listener(*this) 
  {
  }

  ~TestInterface() 
  {
    for (std::set<ExpressionId>::iterator it = m_exprs.begin(); it != m_exprs.end(); ++it)
      (*it)->removeListener(m_listener.getId());
  }

  void lookupNow(const State& state, StateCacheEntry &entry) 
  {
    if (state.name() == "test1") {
      entry.update((double) 0.0);
      return;
    }
    else if (state.name() == "test2") {
      check_error(state.parameters().size() == 1);
      std::string const *param = NULL;
      state.parameters()[0].getValuePointer(param);
      if (*param == "high") {
        entry.update((double) 1.0);
        return;
      }
      else if (*param == "low") {
        entry.update((double) -1.0);
        return;
      }
      assertTrue_2(ALWAYS_FAIL, "ERROR: no matching param for TestInterface::lookupNow, state name = \"test2\"");
    }
    else if (state.name() == "time") {
      entry.update((double) 0.0);
      return;
    }
    else {
      entry.update(m_changingExprs[state.name()]->toValue());
      return;
    }
    assertTrue_2(ALWAYS_FAIL, "ERROR: reached end of TestInterface::lookupNow()");
    entry.update((double) 0.0);
  }

  void subscribe(const State& /* state */)
  {
  }

  void unsubscribe(const State& /* state */)
  {
  }

  void setThresholds(const State& /* state */, double /* hi */, double /* lo */)
  {
  }

  void setThresholds(const State& /* state */, int32_t /* hi */, int32_t /* lo */)
  {
  }

  double currentTime()
  {
    return 0.0;
  }

  void watch(const char* name, ExpressionId expr)
  {
    if (m_exprs.find(expr) == m_exprs.end()) {
      expr->addListener(m_listener.getId());
      m_exprs.insert(expr);
    }
    std::string nameStr(name);
    m_changingExprs.insert(std::pair<std::string, ExpressionId>(nameStr, expr));
    m_exprsToStateName.insert(std::make_pair(expr, nameStr));
  }

  void unwatch(const char* name, ExpressionId expr)
  {
    if (m_exprs.find(expr) != m_exprs.end()) {
      m_exprs.erase(expr);
      expr->removeListener(m_listener.getId());
    }
    std::string nameStr(name);
    m_changingExprs.erase(nameStr);
    m_exprsToStateName.erase(expr);
  }

protected:
  friend class ChangeListener;

  void executeCommand(CommandId const & /* cmd */)
  {}

  void invokeAbort(CommandId const & /* cmd */)
  {}

  void executeUpdate(UpdateId const & /* cmd */)
  {}

  void notifyChanged(ExpressionId expression)
  {
    std::multimap<ExpressionId, std::string>::const_iterator it = m_exprsToStateName.find(expression);
    while (it != m_exprsToStateName.end() && it->first == expression) {
      State st(it->second, std::vector<Value>());
      StateCacheMap::instance().ensureStateCacheEntry(st, expression->valueType())->update(expression->toValue());
      ++it;
    }
  }

private:
  class ChangeListener : public ExpressionListener 
  {
  public:
    ChangeListener(TestInterface& intf)
    : ExpressionListener(),
      m_intf(intf)
    {
    }

    void notifyChanged(ExpressionId src)
    {
      m_intf.notifyChanged(src);
    }

  private:
    TestInterface& m_intf;
  };

  std::set<ExpressionId> m_exprs;
  std::map<std::string, ExpressionId> m_changingExprs; //map of names to expressions being watched
  std::multimap<ExpressionId, std::string> m_exprsToStateName; //make of watched expressions to their state names
  std::multimap<ExpressionId, ExpressionId> m_listeningExprs; //map of changing expressions to listening expressions
  std::map<ExpressionId, double> m_tolerances; //map of dest expressions to tolerances
  std::map<ExpressionId, Value> m_cachedValues; //cache of the previously returned values (dest expression, value pairs)
  ChangeListener m_listener;
};

static TestInterface *theInterface = NULL;

static bool testLookupNow() 
{
  StringConstant test1("test1");
  std::vector<ExpressionId> const emptyArglist;
  std::vector<bool> const emptyGarbageList;

  StringConstant test2("test2");
  std::vector<ExpressionId> test2Args(1, (new StringConstant("high"))->getId());
  std::vector<bool> test2garbage(1, false);

  std::vector<ExpressionId> test3Args(1, (new StringConstant("low"))->getId());

  LookupImpl<double> l1(test1.getId(), false, emptyArglist, emptyGarbageList);
  LookupImpl<double> l2(test2.getId(), false, test2Args, test2garbage);
  LookupImpl<double> l3(test2.getId(), false, test3Args, test2garbage);

  // Bump the cycle count
  theInterface->incrementCycleCount();

  l1.activate();
  l2.activate();
  l3.activate();

  double temp;
  assertTrue_1(l1.getValue(temp));
  assertTrue_1(temp == 0.0);
  assertTrue_1(l2.getValue(temp));
  assertTrue_1(temp == 1.0);
  assertTrue_1(l3.getValue(temp));
  assertTrue_1(temp == -1.0);

  return true;
}

static bool testLookupOnChange() 
{
  StringConstant changeTest("changeTest");
  StringConstant changeWithToleranceTest("changeWithToleranceTest");

  RealVariable watchVar(0.0);
  watchVar.activate();
  theInterface->watch("changeTest", watchVar.getId());
  theInterface->watch("changeWithToleranceTest", watchVar.getId());

  RealConstant tolerance(0.5);
  tolerance.activate();
  std::vector<ExpressionId> const emptyArglist;
  std::vector<bool> const emptyGarbageList;

  double temp;

  LookupOnChange<double> l1(changeTest.getId(), false,
                            emptyArglist, emptyGarbageList,
                            ExpressionId::noId(), false);
  LookupOnChange<double> l2(changeWithToleranceTest.getId(), false,
                            emptyArglist, emptyGarbageList,
                            tolerance.getId(), false);

  assertTrue_1(!l1.isKnown());
  assertTrue_1(!l2.isKnown());


  // Bump the cycle count
  theInterface->incrementCycleCount();

  l1.activate();
  assertTrue_1(l1.getValue(temp));
  assertTrue_1(temp == 0.0);
  l2.activate();
  assertTrue_1(l2.getValue(temp));
  assertTrue_1(temp == 0.0);

  watchVar.setValue(0.1);

  // Bump the cycle count
  theInterface->incrementCycleCount();

  assertTrue_1(l1.getValue(temp));
  assertTrue_1(temp == 0.1);
  assertTrue_1(l2.getValue(temp));
  assertTrue_1(temp == 0.0);

  watchVar.setValue(0.6);

  // Bump the cycle count
  theInterface->incrementCycleCount();

  assertTrue_1(l1.getValue(temp));
  assertTrue_1(temp == 0.6);
  assertTrue_1(l2.getValue(temp));
  assertTrue_1(temp == 0.6);

  l1.deactivate();

  watchVar.setValue(0.7);

  // Bump the cycle count
  theInterface->incrementCycleCount();

  assertTrue_1(!l1.isKnown());
  assertTrue_1(l2.getValue(temp));
  assertTrue_1(temp == 0.6);

  watchVar.setValue(1.1);

  // Bump the cycle count
  theInterface->incrementCycleCount();

  assertTrue_1(!l1.isKnown());
  assertTrue_1(l2.getValue(temp));
  assertTrue_1(temp == 1.1);

  theInterface->unwatch("changeTest", watchVar.getId());
  theInterface->unwatch("changeWithToleranceTest", watchVar.getId());

  return true;
}

bool lookupsTest()
{
  TestInterface foo;
  theInterface = &foo;
  g_interface = foo.getId();

  runTest(testLookupNow);
  runTest(testLookupOnChange);
  return true;
}
