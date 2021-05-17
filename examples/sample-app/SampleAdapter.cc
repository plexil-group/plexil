/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "SampleSystem.hh"
#include "Subscriber.hh"

// PLEXIL includes
#include "AdapterConfiguration.hh"
#include "AdapterFactory.hh"
#include "AdapterExecInterface.hh"
#include "Command.hh"
#include "Debug.hh"
//#include "Expression.hh"
#include "InterfaceAdapter.hh"
#include "LookupReceiver.hh"
#include "State.hh"

#include <iostream>
#include <set>

using std::cout;
using std::string;
using std::cerr;
using std::endl;
using std::map;
using std::string;
using std::vector;
using std::copy;

using namespace PLEXIL;

///////////////////////////// Conveniences //////////////////////////////////

// A preamble for error messages.
static string error = "Error in SampleAdapter: ";

// A prettier name for the "unknown" value.
static Value const Unknown;

// An empty argument vector.
static vector<Value> const EmptyArgs;


///////////////////////////// State support //////////////////////////////////

// The states for which SampleAdapter publishes updates
static std::set<State> subscribedStates;



class SampleAdapter : public PLEXIL::InterfaceAdapter, public Subscriber
{
public:

  SampleAdapter(AdapterExecInterface& execInterface,
                AdapterConf *conf) :
    InterfaceAdapter(execInterface, conf),
    Subscriber()
  {
    debugMsg("SampleAdapter", " created.");
  }

  virtual ~SampleAdapter() = default;

  ///////////////////////////// Member functions //////////////////////////////////

  //
  // InterfaceAdapter API
  //
  virtual bool initialize(AdapterConfiguration *config)
  {
    // Register command handlers for each command
    config->registerCommandHandlerFunction("SetSize", setSize);
    config->registerCommandHandlerFunction("SetSpeed", setSpeed);
    config->registerCommandHandlerFunction("SetColor", setColor);
    config->registerCommandHandlerFunction("SetName", setName);
    config->registerCommandHandlerFunction("Move", move);
    config->registerCommandHandlerFunction("Hello", hello);
    config->registerCommandHandlerFunction("Square", square);
    config->registerCommandHandlerFunction("Cube", cube);
    // Register a default command handler
    config->setDefaultCommandHandlerFunction(defaultHandler);

    // Register lookup handlers for each state
    config->registerLookupHandlerFunction("Size", getSize);
    config->registerLookupHandlerFunction("Color", getColor);
    config->registerLookupHandlerFunction("Speed", getSpeed);
    config->registerLookupHandlerFunction("SystemName", getSystemName);
    config->registerLookupHandlerFunction("at", getAt);
    // Register a default lookup handler
    config->setDefaultLookupHandler(getDefault);

    setSubscriber(this);
    debugMsg("SampleAdapter", " initialized.");
    return true;
  }

  virtual bool start()
  {
    debugMsg("SampleAdapter", " started.");
    return true;
  }

  virtual void stop()
  {
    debugMsg("SampleAdapter", " stopped.");
  }

  //
  // Subscriber API
  //

  // The 'receive' functions are the subscribers for system state updates.  They
  // receive the name of the state whose value has changed in the system.  Then
  // they propagate the state's new value to the executive.

  virtual void receiveValue (const string& state_name, Value val)
  {
    propagateValueChange (State(state_name), val);
  }

  virtual void receiveValue (const string& state_name, Value val, Value arg)
  {
    propagateValueChange (State(state_name, vector<Value> (1, arg)), val);
  }

  virtual void receiveValue (const string& state_name, Value val, Value arg1, Value arg2)
  {
    vector<Value> vec;
    vec.push_back (arg1);
    vec.push_back (arg2);
    propagateValueChange (State(state_name, vec), val);
  }

  private:

  //////////////////////////// Command Handlers /////////////////////////////////

  static void setSize(Command *cmd, AdapterExecInterface *intf) {
    string const &name = cmd->getName();
    debugMsg("SampleAdapter", " Received executeCommand for " << name); 
    double d;
    cmd->getArgValues()[0].getValue(d);
    SampleSystem::getInstance()->setSize (d);
    intf->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
    intf->notifyOfExternalEvent();
  }

  static void setSpeed(Command *cmd, AdapterExecInterface *intf) {
    string const &name = cmd->getName();
    debugMsg("SampleAdapter", " Received executeCommand for " << name); 
    int32_t i = 0;
    cmd->getArgValues()[0].getValue(i);
    SampleSystem::getInstance()->setSpeed (i);
    intf->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
    intf->notifyOfExternalEvent();
  }

  static void setColor(Command *cmd, AdapterExecInterface *intf) {
    string const &name = cmd->getName();
    debugMsg("SampleAdapter", " Received executeCommand for " << name); 
    string s;
    cmd->getArgValues()[0].getValue(s);
    SampleSystem::getInstance()->setColor (s);
    intf->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
    intf->notifyOfExternalEvent();
  }

  static void setName(Command *cmd, AdapterExecInterface *intf) {
    string const &name = cmd->getName();
    debugMsg("SampleAdapter", " Received executeCommand for " << name); 
    string s;
    cmd->getArgValues()[0].getValue(s);
    SampleSystem::getInstance()->setName (s);
    intf->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
    intf->notifyOfExternalEvent();
  }

  static void move(Command *cmd, AdapterExecInterface *intf) {
    string const &name = cmd->getName();
    debugMsg("SampleAdapter", " Received executeCommand for " << name); 
    string s;
    int32_t i1 = 0, i2 = 0;
    const vector<Value>& args = cmd->getArgValues();
    args[0].getValue(s);
    args[1].getValue(i1);
    args[2].getValue(i2);
    SampleSystem::getInstance()->move (s, i1, i2);
    intf->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
    intf->notifyOfExternalEvent();
  }

  static void hello(Command *cmd, AdapterExecInterface *intf) {
    string const &name = cmd->getName();
    debugMsg("SampleAdapter", " Received executeCommand for " << name); 
    SampleSystem::getInstance()->hello ();
    intf->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
    intf->notifyOfExternalEvent();
  }

  static void square(Command *cmd, AdapterExecInterface *intf) {
    string const &name = cmd->getName();
    debugMsg("SampleAdapter", " Received executeCommand for " << name);
    intf->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
    int32_t i = 0;
    cmd->getArgValues()[0].getValue(i);
    intf->handleCommandReturn(cmd, SampleSystem::getInstance()->square (i));
    intf->notifyOfExternalEvent();
  }

  static void cube(Command *cmd, AdapterExecInterface *intf) {
    string const &name = cmd->getName();
    debugMsg("SampleAdapter", " Received executeCommand for " << name);
    intf->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
    int32_t i = 0;
    cmd->getArgValues()[0].getValue(i);
    intf->handleCommandReturn(cmd, SampleSystem::getInstance()->cube (i));
    intf->notifyOfExternalEvent();
  }

  static void defaultHandler(Command *cmd, AdapterExecInterface *intf) {
    string const &name = cmd->getName();
    cerr << error << "invalid command: " << name << endl;
    intf->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
    intf->notifyOfExternalEvent();
  }

  ///////////////////////////// Lookup Handlers /////////////////////////////////

  static void getSize (const State& state, LookupReceiver *entry) 
  {
    debugMsg("SampleAdapter:getSize",
             " lookup called for " << state.name() << " with " << state.parameters().size() << " args");
    entry->update(SampleSystem::getInstance()->getSize());
    subscribedStates.insert(state);
  }

  static void getSpeed (const State& state, LookupReceiver *entry) 
  {
    debugMsg("SampleAdapter:getSpeed",
             " lookup called for " << state.name() << " with " << state.parameters().size() << " args");
    entry->update(SampleSystem::getInstance()->getSpeed());
    subscribedStates.insert(state);
  }

  static void getColor (const State& state, LookupReceiver *entry) 
  {
    debugMsg("SampleAdapter:getColor",
             " lookup called for " << state.name() << " with " << state.parameters().size() << " args");
    entry->update(SampleSystem::getInstance()->getColor());
    subscribedStates.insert(state);
  }

  static void getSystemName (const State& state, LookupReceiver *entry) 
  {
    debugMsg("SampleAdapter:getStateName",
             " lookup called for " << state.name() << " with " << state.parameters().size() << " args");
    entry->update(SampleSystem::getInstance()->getName());
    subscribedStates.insert(state);
  }

  static void getAt (const State& state, LookupReceiver *entry) 
  {
    string const &name = state.name();
    const vector<Value>& args = state.parameters();
    debugMsg("SampleAdapter:getAt",
             " lookup called for " << name << " with " << args.size() << " args");
    switch (args.size()) {
    case 0:
      entry->update(SampleSystem::getInstance()->at ());
      subscribedStates.insert(state);
      break;
    case 1: {
      string s;
      args[0].getValue(s);
      entry->update(SampleSystem::getInstance()->at(s));
      subscribedStates.insert(state);
      break;
    }
    case 2: {
      int32_t arg0 = 0, arg1 = 0;
      args[0].getValue(arg0);
      args[1].getValue(arg1);
      entry->update(SampleSystem::getInstance()->at (arg0, arg1));
      subscribedStates.insert(state);
      break;
    }
    default: {
      cerr << error << "invalid lookup of 'at'" << endl;
      entry->update(Unknown);
    }
    }
  }

  static void getDefault (const State& state, LookupReceiver *entry) 
  {
    debugMsg("SampleAdapter:getDefault",
             " lookup called for " << state.name() << " with " << state.parameters().size() << " args");
    cerr << error << "invalid state: " << state.name() << endl;
    entry->update(Unknown);
  }

  void propagateValueChange (const State& state, const Value &val)
  {
    if (!isStateSubscribed(state))
      return;
    debugMsg("SampleAdapter:propagateValueChange",
             " Propagating new value " << val << " for state " << state);
    getInterface().handleValueChange (state, val);
    getInterface().notifyOfExternalEvent();
  }

  bool isStateSubscribed(const State& state) const
  {
    return subscribedStates.find(state) != subscribedStates.end();
  }

}; // class SampleAdapter

// Necessary boilerplate
extern "C" {
  void initSampleAdapter() {
    REGISTER_ADAPTER(SampleAdapter, "SampleAdapter");
  }
}
