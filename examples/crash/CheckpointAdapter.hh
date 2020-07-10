//TODO: What does this do?
#ifndef _H__CheckpointAdapter
#define _H__CheckpointAdapter

#include "Command.hh"
#include "CheckpointAdapter.hh"
#include "Value.hh"

using namespace PLEXIL;

class CheckpointAdapter : public InterfaceAdapter
{
public:
  CheckpointAdapter (AdapterExecInterface&, const pugi::xml_node&);

  bool initialize();
  bool start();
  bool stop();
  bool reset();
  bool shutdown();

  virtual void lookupNow (State const &state, StateCacheEntry &cacheEntry);
  virtual void subscribe(const State& state);
  virtual void unsubscribe(const State& state);
  virtual void setThresholds(const State& state, double hi, double lo);
  virtual void setThresholds(const State& state, int32_t hi, int32_t lo);
  virtual void executeCommand(Command *cmd);

  // The following member, not inherited from the base class, propagates a state
  // value change from the system to the executive.
  //
  void propagateValueChange (const State& state,
                             const std::vector<Value>& vals) const;

private:
  bool isStateSubscribed(const State& state) const;

  std::set<State> m_subscribedStates;

};

extern "C" {
  void initCheckpointAdapter();
}

#endif
