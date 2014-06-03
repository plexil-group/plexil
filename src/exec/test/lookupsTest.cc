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

using namespace PLEXIL;

class TestInterface : public ExternalInterface 
{
public:
  static Id<TestInterface>& instance()
  {
    return s_instanceTestInterface;
  }

  TestInterface()
    : ExternalInterface(),
      m_listener(*this) 
  {
    s_instanceTestInterface = (Id<TestInterface>) this->getId();
    m_listener.activate();
  }

  ~TestInterface() 
  {
    if (s_instanceTestInterface == this->getId())
      s_instanceTestInterface = Id<TestInterface>::noId();

    for (std::set<ExpressionId>::iterator it = m_exprs.begin(); it != m_exprs.end(); ++it)
      (*it)->removeListener(m_listener.getId());
  }

  Value lookupNow(const State& state) {
    if (state.first == "test1") {
      return Value(0.0);
    }
    else if (state.first == "test2") {
      check_error(state.second.size() == 1);
      const std::string& param = state.second[0].getStringValue();
      if (param == "high") return Value(1.0);
      else if (param == "low") return Value(-1.0);
    }
    else if (state.first == "time") {
      return Value(0.0);
    }
    else {
      return m_changingExprs[state.first]->getValue();
    }
    std::cerr << "ERROR (shouldn't happen): reached end of lookupNow()"
              << std::endl;
    return Value(0.0);
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

  void batchActions(std::list<CommandId>& /* commands */)
  {
  }

  void updatePlanner(std::list<UpdateId>& /* updates */)
  {
  }

  void invokeAbort(const CommandId& /* cmd */)
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
    LabelStr nameStr(name);
    m_changingExprs.insert(std::pair<LabelStr, ExpressionId>(nameStr, expr));
    m_exprsToStateName.insert(std::make_pair(expr, nameStr));
  }

  void unwatch(const char* name, ExpressionId expr)
  {
    if (m_exprs.find(expr) != m_exprs.end()) {
      m_exprs.erase(expr);
      expr->removeListener(m_listener.getId());
    }
    LabelStr nameStr(name);
    m_changingExprs.erase(nameStr);
    m_exprsToStateName.erase(expr);
  }

protected:
  friend class ChangeListener;

  void internalExecuteCommand(const LabelStr& /* name */,
                  const std::vector<Value>& /* args */,
                  ExpressionId /* dest */)
  {}

  void internalInvokeAbort(const LabelStr& /* name */,
               const std::vector<Value>& /* args */, 
               ExpressionId /* dest */)
  {}

  void notifyValueChanged(ExpressionId expression)
  {
    std::multimap<ExpressionId, LabelStr>::const_iterator it = m_exprsToStateName.find(expression);
    while(it != m_exprsToStateName.end() && it->first == expression) {
      State st(it->second.toString(), std::vector<Value>());
      m_cache->updateState(st, expression->getValue());
      ++it;
    }
  }

private:
  class ChangeListener : public ExpressionListener {
  public:
    ChangeListener(TestInterface& intf) : ExpressionListener(), m_intf(intf) {}
    void notifyValueChanged(const ExpressionId& expression) {m_intf.notifyValueChanged(expression);}
  protected:
  private:
    TestInterface& m_intf;
  };

  static Id<TestInterface> s_instanceTestInterface;

  std::set<ExpressionId> m_exprs;
  std::map<LabelStr, ExpressionId> m_changingExprs; //map of names to expressions being watched
  std::multimap<ExpressionId, LabelStr> m_exprsToStateName; //make of watched expressions to their state names
  std::multimap<ExpressionId, ExpressionId> m_listeningExprs; //map of changing expressions to listening expressions
  std::map<ExpressionId, double> m_tolerances; //map of dest expressions to tolerances
  std::map<ExpressionId, Value> m_cachedValues; //cache of the previously returned values (dest expression, value pairs)
  ChangeListener m_listener;
  StateCacheId m_cache;
};

Id<TestInterface> TestInterface::s_instanceTestInterface = Id<TestInterface>::noId();

class LookupTestExecConnector : public ExecConnector {
public:
  LookupTestExecConnector() : ExecConnector() 
  {
    m_cache.setExternalInterface(TestInterface::instance()->getId()); // static_cast didn't work here, grumble
  }

  void notifyNodeConditionChanged(NodeId /* node */) {}
  void handleConditionsChanged(const NodeId& /* node */, NodeState /* newState */) {}
  void enqueueAssignment(const AssignmentId& /* assign */) {}
  void enqueueAssignmentForRetraction(const AssignmentId& /* assign */) {}
  void enqueueCommand(const CommandId& /* cmd */) {}
  void enqueueUpdate(const UpdateId& /* update */) {}
  void notifyExecuted(const NodeId& /* node */) {}
  void markRootNodeFinished(const NodeId& /* node */) {}
  unsigned int getCycleCount() const { return 0; /* FIXME */ }
  const ExternalInterfaceId& getExternalInterface() 
  {
    return TestInterface::instance()->getId();
  }
  const ExecListenerHubId& getExecListenerHub() const { return ExecListenerHubId::noId(); }
};

static bool testLookupNow() 
{
  PlexilStateId state1 = (new PlexilState())->getId();
  state1->setName("test1");

  PlexilStateId state2 = (new PlexilState())->getId();
  state2->setName("test2");
  state2->addArg((new PlexilValue(PLEXIL::STRING, "high"))->getId());

  PlexilStateId state3 = (new PlexilState())->getId();
  state3->setName("test2");
  state3->addArg((new PlexilValue(PLEXIL::STRING, "low"))->getId());

  PlexilLookupNow test1;
  test1.setState(state1);

  PlexilLookupNow test2;
  test2.setState(state2);

  PlexilLookupNow test3;
  test3.setState(state3);

  LookupNow l1(test1.getId(), node.getId());
  LookupNow l2(test2.getId(), node.getId());
  LookupNow l3(test3.getId(), node.getId());

  l1.activate();
  l2.activate();
  l3.activate();

  assertTrue(l1.getValue().getDoubleValue() == 0.0);
  assertTrue(l2.getValue().getDoubleValue() == 1.0);
  assertTrue(l3.getValue().getDoubleValue() == -1.0);

  return true;
}

static bool testLookupOnChange() 
{
  PlexilStateId state1 = (new PlexilState())->getId();
  state1->setName("changeTest");
  PlexilChangeLookup test1;
  test1.setState(state1);

  PlexilStateId state2 = (new PlexilState())->getId();
  state2->setName("changeWithToleranceTest");
  PlexilChangeLookup test2;
  test2.setState(state2);
  test2.addTolerance((new PlexilValue(PLEXIL::REAL, "0.5"))->getId());


  RealVariable watchVar(0.0);
  watchVar.activate();
  TestInterface::instance()->watch("changeTest", watchVar.getId());
  TestInterface::instance()->watch("changeWithToleranceTest", watchVar.getId());

  LookupOnChange l1(test1.getId(), node.getId());
  LookupOnChange l2(test2.getId(), node.getId());

  assertTrue(l1.getValue().isUnknown());
  assertTrue(l2.getValue().isUnknown());

  l1.activate();
  assertTrue(l1.getValue().getDoubleValue() == 0.0);
  l2.activate();
  assertTrue(l2.getValue().getDoubleValue() == 0.0);

  watchVar.setValue(0.1);
  assertTrue(l1.getValue().getDoubleValue() == 0.1);
  assertTrue(l2.getValue().getDoubleValue() == 0.0);

  watchVar.setValue(0.6);
  assertTrue(l1.getValue().getDoubleValue() == 0.6);
  assertTrue(l2.getValue().getDoubleValue() == 0.6);

  l1.deactivate();

  watchVar.setValue(0.7);
  assertTrue(l1.getValue().isUnknown());
  assertTrue(l2.getValue().getDoubleValue() == 0.6);
  watchVar.setValue(1.1);
  assertTrue(l1.getValue().isUnknown());
  assertTrue(l2.getValue().getDoubleValue() == 1.1);

  TestInterface::instance()->unwatch("changeTest", watchVar.getId());
  TestInterface::instance()->unwatch("changeWithToleranceTest", watchVar.getId());

  return true;
}

bool lookupsTest()
{
  TestInterface foo;
  runTest(testLookupNow);
  runTest(testLookupOnChange);
  return true;
}
