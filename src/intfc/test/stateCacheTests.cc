
class CacheTestInterface : public ExternalInterface {
public:
  CacheTestInterface(const StateCacheId& cache)
  : ExternalInterface(), m_cache(cache), m_lookupNowCalled(false)
  {}

  Value lookupNow(const State& state) 
  {
    m_lookupNowCalled = true;
    return m_values[state];
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

  bool lookupNowCalled() {return m_lookupNowCalled;}
  void clearLookupNowCalled() {m_lookupNowCalled = false;}
  void setValue(const State& state, double value, StateCacheId cache, bool update = true) {
    std::map<State, Value>::iterator it = m_values.find(state);
    if (it == m_values.end())
      m_values.insert(std::make_pair(state, Value(value)));
    else
      it->second = Value(value);
    if (update)
      cache->updateState(state, Value(value));
  }
protected:
private:
  std::map<State, Value> m_values;
  StateCacheId m_cache;
  bool m_lookupNowCalled;
};

class StateCacheTest {
public:
  static bool test() {
    runTest(testLookupNow);
    runTest(testChangeLookup);
    return true;
  }
private:
  static bool testLookupNow() {
    StateCache cache;
    CacheTestInterface iface(cache.getId());
    cache.setExternalInterface(iface.getId());

    IntegerVariable destVar;
    destVar.activate();

    State st("foo", std::vector<Value>());

    iface.setValue(st, 1, cache.getId(), false);
    cache.handleQuiescenceStarted();

    //single lookup for new state
    assertTrue(destVar.getValue().isUnknown());
    cache.registerLookupNow(destVar.getId(), st);
    assertTrue(iface.lookupNowCalled());
    assertTrue(destVar.getValue().getDoubleValue() == 1);
    cache.unregisterLookupNow(destVar.getId());

    //re-lookup for same state in same quiescence
    iface.setValue(st, 2, cache.getId(), false);
    iface.clearLookupNowCalled();
    cache.registerLookupNow(destVar.getId(), st);
    assertTrue(!iface.lookupNowCalled());
    assertTrue(destVar.getValue().getDoubleValue() == 1);
    cache.unregisterLookupNow(destVar.getId());

    //re-lookup for same state in next quiescence
    cache.handleQuiescenceEnded();
    cache.handleQuiescenceStarted();
    cache.registerLookupNow(destVar.getId(), st);
    assertTrue(iface.lookupNowCalled());
    assertTrue(destVar.getValue().getDoubleValue() == 2);
    cache.unregisterLookupNow(destVar.getId());

    // *** TODO: Add test for updating LookupNow that 
    // *** remains active across multiple quiescence cycles

    return true;
  }

  static bool testChangeLookup() {
    StateCache cache;
    CacheTestInterface iface(cache.getId());
    cache.setExternalInterface(iface.getId());

    IntegerVariable destVar1, destVar2;
    destVar1.activate();
    destVar2.activate();

    State st("foo", std::vector<Value>());

    //lookup
    iface.setValue(st, 1, cache.getId(), false);
    cache.handleQuiescenceStarted();
    cache.registerChangeLookup(destVar1.getId(), st, 1);
    assertTrue(destVar1.getValue().getDoubleValue() == 1);
    cache.registerChangeLookup(destVar2.getId(), st, 2);
    assertTrue(destVar2.getValue().getDoubleValue() == 1);
    cache.handleQuiescenceEnded();

    //update value
    iface.setValue(st, 2, cache.getId());
    assertTrue(destVar1.getValue().getDoubleValue() == 2);
    assertTrue(destVar2.getValue().getDoubleValue() == 1);

    //lookupNow triggering change
    IntegerVariable nowDestVar;
    nowDestVar.activate();
    iface.setValue(st, 3, cache.getId(), false);
    cache.handleQuiescenceStarted();
    cache.handleQuiescenceEnded();
    cache.handleQuiescenceStarted();
    cache.registerLookupNow(nowDestVar.getId(), st);
    assertTrue(nowDestVar.getValue().getDoubleValue() == 3);
    assertTrue(destVar1.getValue().getDoubleValue() == 3);
    assertTrue(destVar2.getValue().getDoubleValue() == 3);

    //unregister
    cache.unregisterLookupNow(nowDestVar.getId());
    cache.unregisterChangeLookup(destVar2.getId());
    cache.handleQuiescenceEnded();
    iface.setValue(st, 5, cache.getId());
    assertTrue(destVar2.getValue().getDoubleValue() == 3);
    assertTrue(destVar1.getValue().getDoubleValue() == 5);
    return true;
  }
};
