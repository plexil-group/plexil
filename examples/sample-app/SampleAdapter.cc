/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include "SampleAdapter.hh"

#include "Subscriber.hh"
#include "SampleSystem.hh"

#include "AdapterConfiguration.hh"
#include "AdapterFactory.hh"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Expression.hh"
#include "StateCacheEntry.hh"

#include <iostream>

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
static string error = "Error in SampleAdaptor: ";

// A prettier name for the "unknown" value.
static Value const Unknown;

// Static member initialization
// An empty argument vector.
static vector<Value> const EmptyArgs;

std::set<State> SampleAdapter::m_subscribedStates;

///////////////////////////// State support //////////////////////////////////

// The 'receive' functions are the subscribers for system state updates.  They
// receive the name of the state whose value has changed in the system.  Then
// they propagate the state's new value to the executive.

static State createState (const string& state_name, const vector<Value>& value)
{
  State state(state_name, value.size());
  if (value.size() > 0)
  {
    for(size_t i=0; i<value.size();i++)
    {
      state.setParameter(i, value[i]);
    }
  }
  return state;
}

void SampleAdapter::receiveValue (const string& state_name, Value val)
{
  propagate (createState(state_name, EmptyArgs),
                                        vector<Value> (1, val));
}

void SampleAdapter::receiveValue (const string& state_name, Value val, Value arg)
{
  propagate (createState(state_name, vector<Value> (1, arg)),
                                        vector<Value> (1, val));
}

void SampleAdapter::receiveValue (const string& state_name, Value val, Value arg1, Value arg2)
{
  vector<Value> vec;
  vec.push_back (arg1);
  vec.push_back (arg2);
  propagate (createState(state_name, vec),
                                        vector<Value> (1, val));
}


///////////////////////////// Member functions //////////////////////////////////

SampleAdapter::SampleAdapter(AdapterExecInterface& execInterface,
                             const pugi::xml_node& configXml) :
    InterfaceAdapter(execInterface, configXml)
{
  debugMsg("SampleAdapter", " created.");
}

bool SampleAdapter::initialize()
{
  // Register command handlers for each command
  g_configuration->registerCommandHandler("SetSize", SampleAdapter::setSize);
  g_configuration->registerCommandHandler("SetSpeed", SampleAdapter::setSpeed);
  g_configuration->registerCommandHandler("SetColor", SampleAdapter::setColor);
  g_configuration->registerCommandHandler("SetName", SampleAdapter::setName);
  g_configuration->registerCommandHandler("Move", SampleAdapter::move);
  g_configuration->registerCommandHandler("Hello", SampleAdapter::hello);
  g_configuration->registerCommandHandler("Square", SampleAdapter::square);
  g_configuration->registerCommandHandler("Cube", SampleAdapter::cube);
  // Register a default command handler
  g_configuration->setDefaultCommandHandler(SampleAdapter::defaultHandler);

  // Register lookup handlers for each state
  g_configuration->registerLookupHandler("Size", SampleAdapter::getSize, 
      SampleAdapter::setStateThresholds, SampleAdapter::setStateThresholds,
      SampleAdapter::subscribeToState, SampleAdapter::unsubscribeFromState);
  g_configuration->registerLookupHandler("Color", SampleAdapter::getColor, 
      SampleAdapter::setStateThresholds, SampleAdapter::setStateThresholds,
      SampleAdapter::subscribeToState, SampleAdapter::unsubscribeFromState);
  g_configuration->registerLookupHandler("Speed", SampleAdapter::getSpeed, 
      SampleAdapter::setStateThresholds, SampleAdapter::setStateThresholds,
      SampleAdapter::subscribeToState, SampleAdapter::unsubscribeFromState);
  g_configuration->registerLookupHandler("SystemName", SampleAdapter::getSystemName, 
      SampleAdapter::setStateThresholds, SampleAdapter::setStateThresholds,
      SampleAdapter::subscribeToState, SampleAdapter::unsubscribeFromState);
  g_configuration->registerLookupHandler("at", SampleAdapter::getAt, 
      SampleAdapter::setStateThresholds, SampleAdapter::setStateThresholds,
      SampleAdapter::subscribeToState, SampleAdapter::unsubscribeFromState);
  //Register a default lookup handler
  g_configuration->setDefaultLookupHandler(SampleAdapter::getDefault, 
      SampleAdapter::setStateThresholds, SampleAdapter::setStateThresholds,
      SampleAdapter::subscribeToState, SampleAdapter::unsubscribeFromState);

  setSubscriber(this);
  debugMsg("SampleAdapter", " initialized.");
  return true;
}

bool SampleAdapter::start()
{
  debugMsg("SampleAdapter", " started.");
  return true;
}

bool SampleAdapter::stop()
{
  debugMsg("SampleAdapter", " stopped.");
  return true;
}

bool SampleAdapter::reset()
{
  debugMsg("SampleAdapter", " reset.");
  return true;
}
bool SampleAdapter::shutdown()
{
  debugMsg("SampleAdapter", " shut down.");
  return true;
}

//////////////////////////// Command Handlers /////////////////////////////////

void SampleAdapter::setSize(Command *cmd) {
  string const &name = cmd->getName();
  debugMsg("SampleAdapter", "Received executeCommand for " << name); 
  double d;
  cmd->getArgValues()[0].getValue(d);
  SampleSystem::getInstance()->setSize (d);
  g_execInterface->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  g_execInterface->notifyOfExternalEvent();
}

void SampleAdapter::setSpeed(Command *cmd) {
  string const &name = cmd->getName();
  debugMsg("SampleAdapter", "Received executeCommand for " << name); 
  int32_t i = 0;
  cmd->getArgValues()[0].getValue(i);
    SampleSystem::getInstance()->setSpeed (i);
  g_execInterface->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  g_execInterface->notifyOfExternalEvent();
}

void SampleAdapter::setColor(Command *cmd) {
  string const &name = cmd->getName();
  debugMsg("SampleAdapter", "Received executeCommand for " << name); 
  string s;
  cmd->getArgValues()[0].getValue(s);
    SampleSystem::getInstance()->setColor (s);
  g_execInterface->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  g_execInterface->notifyOfExternalEvent();
}

void SampleAdapter::setName(Command *cmd) {
  string const &name = cmd->getName();
  debugMsg("SampleAdapter", "Received executeCommand for " << name); 
  string s;
  cmd->getArgValues()[0].getValue(s);
    SampleSystem::getInstance()->setName (s);
  g_execInterface->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  g_execInterface->notifyOfExternalEvent();
}

void SampleAdapter::move(Command *cmd) {
  string const &name = cmd->getName();
  debugMsg("SampleAdapter", "Received executeCommand for " << name); 
  string s;
  int32_t i1 = 0, i2 = 0;
  const vector<Value>& args = cmd->getArgValues();
  args[0].getValue(s);
  args[1].getValue(i1);
  args[2].getValue(i2);
  SampleSystem::getInstance()->move (s, i1, i2);
  g_execInterface->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  g_execInterface->notifyOfExternalEvent();
}

void SampleAdapter::hello(Command *cmd) {
  string const &name = cmd->getName();
  debugMsg("SampleAdapter", "Received executeCommand for " << name); 
  SampleSystem::getInstance()->hello ();
  g_execInterface->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  g_execInterface->notifyOfExternalEvent();
}

void SampleAdapter::square(Command *cmd) {
  string const &name = cmd->getName();
  debugMsg("SampleAdapter", "Received executeCommand for " << name);
  g_execInterface->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  int32_t i = 0;
  cmd->getArgValues()[0].getValue(i);
  g_execInterface->handleCommandReturn(cmd, SampleSystem::getInstance()->square (i));
  g_execInterface->notifyOfExternalEvent();
}

void SampleAdapter::cube(Command *cmd) {
  string const &name = cmd->getName();
  debugMsg("SampleAdapter", "Received executeCommand for " << name);
  g_execInterface->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  int32_t i = 0;
  cmd->getArgValues()[0].getValue(i);
  g_execInterface->handleCommandReturn(cmd, SampleSystem::getInstance()->cube (i));
  g_execInterface->notifyOfExternalEvent();
}

void SampleAdapter::defaultHandler(Command *cmd) {
  string const &name = cmd->getName();
  cerr << error << "invalid command: " << name << endl;
  g_execInterface->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  g_execInterface->notifyOfExternalEvent();
}

///////////////////////////// Lookup Handlers /////////////////////////////////

void SampleAdapter::getSize (const State& state, StateCacheEntry &entry) 
{
  debugMsg("SampleAdapter:getSize",
        "lookup called for " << state.name() << " with " << state.parameters().size() << " args");
  entry.update(SampleSystem::getInstance()->getSize());
}

void SampleAdapter::getSpeed (const State& state, StateCacheEntry &entry) 
{
  debugMsg("SampleAdapter:getSpeed",
        "lookup called for " << state.name() << " with " << state.parameters().size() << " args");
  entry.update(SampleSystem::getInstance()->getSpeed());
}

void SampleAdapter::getColor (const State& state, StateCacheEntry &entry) 
{
  debugMsg("SampleAdapter:getColor",
        "lookup called for " << state.name() << " with " << state.parameters().size() << " args");
  entry.update(SampleSystem::getInstance()->getColor());
}

void SampleAdapter::getSystemName (const State& state, StateCacheEntry &entry) 
{
  debugMsg("SampleAdapter:getStateName",
          "lookup called for " << state.name() << " with " << state.parameters().size() << " args");
  entry.update(SampleSystem::getInstance()->getName());
}

void SampleAdapter::getAt (const State& state, StateCacheEntry &entry) 
{
  string const &name = state.name();
  const vector<Value>& args = state.parameters();
  debugMsg("SampleAdapter:getAt",
          "lookup called for " << name << " with " << args.size() << " args");
  switch (args.size()) {
    case 0:
      entry.update(SampleSystem::getInstance()->at ());
      break;
    case 1: {
      string s;
      args[0].getValue(s);
      entry.update(SampleSystem::getInstance()->at(s));
      break;
    }
    case 2: {
      int32_t arg0 = 0, arg1 = 0;
      args[0].getValue(arg0);
      args[1].getValue(arg1);
      entry.update(SampleSystem::getInstance()->at (arg0, arg1));
      break;
    }
    default: {
      cerr << error << "invalid lookup of 'at'" << endl;
      entry.update(Unknown);
    }
    }
}

void SampleAdapter::getDefault (const State& state, StateCacheEntry &entry) 
{
  debugMsg("SampleAdapter:getDefault",
          "lookup called for " << state.name() << " with " << state.parameters().size() << " args");
  cerr << error << "invalid state: " << state.name() << endl;
  entry.update(Unknown);
}

void SampleAdapter::subscribeToState(const State& state)
{
  debugMsg("SampleAdapter:subscribe", " processing state " << state.name());
  m_subscribedStates.insert(state);
}


void SampleAdapter::unsubscribeFromState (const State& state)
{
  debugMsg("SampleAdapter:subscribe", " from state " << state.name());
  m_subscribedStates.erase(state);
}

// Does nothing.
void SampleAdapter::setStateThresholds (const State& state, double hi, double lo)
{
}
void SampleAdapter::setStateThresholds (const State& state, int32_t hi, int32_t lo)
{
}

void SampleAdapter::propagateValueChange (const State& state,
                                          const vector<Value>& vals) const
{
  if (!isStateSubscribed(state))
    return; 
  m_execInterface.handleValueChange (state, vals.front());
  m_execInterface.notifyOfExternalEvent();
}

void SampleAdapter::propagate (const State& state, const vector<Value>& value)
{
  SampleAdapter::propagateValueChange(state, value);
}

bool SampleAdapter::isStateSubscribed(const State& state) const
{
  return m_subscribedStates.find(state) != m_subscribedStates.end();
}

// Necessary boilerplate
extern "C" {
  void initSampleAdapter() {
    REGISTER_ADAPTER(SampleAdapter, "SampleAdapter");
  }
}

