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
#include "CheckpointAdapter.hh"

#include "Subscriber.hh"
#include "CheckpointSystem.hh"

#include "AdapterConfiguration.hh"
#include "AdapterFactory.hh"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Expression.hh"
#include "StateCacheEntry.hh"

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::string;
using std::vector;
using std::copy;


///////////////////////////// Conveniences //////////////////////////////////

// A preamble for error messages.
static string error = "Error in CheckpointAdapter: ";

// A prettier name for the "unknown" value.
static Value Unknown;

// Static member initialization
CheckpointSystem *CheckpointSystem::m_system = 0;
CheckpointAdapter *CheckpointAdapter::m_adapter = 0;

// An empty argument vector.
static vector<Value> EmptyArgs;


///////////////////////////// State support //////////////////////////////////

// Queries the system for the value of a state and its arguments.
//
static Value fetch (const string& state_name, const vector<Value>& args)
{
  debugMsg("CheckpointAdapter:fetch",
           "Fetch called on " << state_name << " with " << args.size() << " args");
  Value retval;

  
  if (state_name == "DidCrash"){
    retval = CheckpointSystem::getInstance()->didCrash();
  }
  
  else if (state_name == "NumberOfActiveCrashes"){
    retval = CheckpointSystem::getInstance()-> numActiveCrashes();
  }
  else if (state_name == "NumberOfTotalCrashes"){
    retval = CheckpointSystem::getInstance()->numTotalCrashes();
  }

  else{
    // Default is checkpoint in current boot (iteration 0)
    int32_t which_boot = 0;
    if(!args.empty()){
      args[1].getValue(which_boot);
    }

    
    if (state_name == "Checkpoint"){
      string which_checkpoint;
      args[0].getValue(which_checkpoint);
      retval = CheckpointSystem::getInstance()->getCheckpointState(which_checkpoint,which_boot);
    }
    else if (state_name == "TimeOfCheckpoint"){
      string which_checkpoint;
      args[0].getValue(which_checkpoint);
      retval = CheckpointSystem::getInstance()->getCheckpointTime(which_checkpoint,which_boot);
    }
    else if (state_name == "TimeOfCrash"){
      retval = CheckpointSystem::getInstance()->getTimeOfCrash(which_boot);
    }
    else if (state_name == "TimeOfBoot"){
      retval = CheckpointSystem::getInstance()->getTimeOfBoot(which_boot);
    }
    //No match
    else {
      cerr << error << "invalid state: " << state_name << endl;
      retval = Unknown;
    }

  }

  debugMsg("CheckpointAdapter:fetch", "Fetch returning " << retval);
  return retval;
}

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

static void receiveBool (const string& state_name, bool val)
{
  CheckpointSystem::getInstance()->propagate (createState(state_name, EmptyArgs),
             vector<Value> (1, val));
}

static void receiveInt (const string& state_name, int val)
{
  CheckpointSystem::getInstance()->propagate (createState(state_name, EmptyArgs),
             vector<Value> (1, val));
}

static void receiveBoolInt (const string& state_name, bool val,  int arg)
{
  CheckpointSystem::getInstance()->propagate (createState(state_name, vector<Value> (1, arg)),
             vector<Value> (1, val));
}

static void receiveIntInt (const string& state_name, int val,  int arg)
{
  CheckpointSystem::getInstance()->propagate (createState(state_name, vector<Value> (1, arg)),
             vector<Value> (1, val));
}

static void receiveBoolStringInt (const string& state_name, bool val, const string& arg1, int arg2)
{
  vector<Value> vec;
  vec.push_back (arg1);
  vec.push_back (arg2);
  CheckpointSystem::getInstance()->propagate (createState(state_name, vec), vector<Value> (1, val));
}

static void receiveIntStringInt (const string& state_name, int val, const string& arg1, int arg2)
{
  vector<Value> vec;
  vec.push_back (arg1);
  vec.push_back (arg2);
  CheckpointSystem::getInstance()->propagate (createState(state_name, vec), vector<Value> (1, val));
}


///////////////////////////// Member functions //////////////////////////////////


CheckpointAdapter::CheckpointAdapter(AdapterExecInterface& execInterface,
                             const pugi::xml_node& configXml) :
    InterfaceAdapter(execInterface, configXml)
{
  m_adapter = this;
  debugMsg("CheckpointAdapter", " created.");
}

CheckpointAdapter::~CheckpointAdapter ()
{
  m_adapter = nullptr;
}

bool CheckpointAdapter::initialize()
{
  g_configuration->defaultRegisterAdapter(this);

  g_configuration->registerLookupInterface("DidCrash", this);
  g_configuration->registerLookupInterface("NumberOfActiveCrashes", this);
  g_configuration->registerLookupInterface("NumberOfTotalCrashes", this);
  g_configuration->registerLookupInterface("TimeOfCrash", this);
  g_configuration->registerLookupInterface("TimeOfBoot", this);
  g_configuration->registerLookupInterface("Checkpoint", this);
  g_configuration->registerLookupInterface("TimeOfCheckpoint", this);

  g_configuration->registerCommandInterface("SetCheckpoint", this);
  g_configuration->registerCommandInterface("SetSafeReboot", this);
  g_configuration->registerCommandInterface("DeleteCrash", this);

  setSubscriber (receiveInt);
  setSubscriber (receiveBool);
  setSubscriber (receiveBoolInt);
  setSubscriber (receiveIntInt);
  setSubscriber (receiveBoolStringInt);
  setSubscriber (receiveIntStringInt);
  
  debugMsg("CheckpointAdapter", " initialized.");
  return true;
}

bool CheckpointAdapter::start()
{
  debugMsg("CheckpointAdapter", " started.");
  return true;
}

bool CheckpointAdapter::stop()
{
  debugMsg("CheckpointAdapter", " stopped.");
  return true;
}

bool CheckpointAdapter::reset()
{
  debugMsg("CheckpointAdapter", " reset.");
  return true;
}

bool CheckpointAdapter::shutdown()
{
  debugMsg("CheckpointAdapter", " shut down.");
  return true;
}


// Sends a command (as invoked in a Plexil command node) to the system and sends
// the status, and return value if applicable, back to the executive.
//
void CheckpointAdapter::executeCommand(Command *cmd)
{
  const string &name = cmd->getName();
  debugMsg("CheckpointAdapter", "Received executeCommand for " << name);  

  Value retval = Unknown;
  const vector<Value>& args = cmd->getArgValues();
  
  if (name == "SetCheckpoint") {
    if(args.size()<1 || args.size()>2){
      cerr << error << "SetCheckpoint invalid number of arguments" << end;
    }
    
    else{
      string &checkpoint_name;
      // Default is set checkpoint to true
      bool value = true;
      args[0].getValue(checkpoint_name);
      if(args.size()==2){
	args[1].getValue(value);
      }
      retval = setCheckpoint(checkpoint_name,value);
    }
  }
  else if (name == "SetSafeReboot") {
    if(args.size()!=1){
      cerr << error << "SetSafeReboot invalid number of arguments" << end;
    }
    else{
      bool b;
      args[0].getValue(b);
      retval = setSafeReboot(b);
    }
  }
  else if (name == "DeleteCrash"){
    if(args.size()>1){
      cerr << error << "DeleteCrash invalid number of arguments" << end;
    }
    int32_t crash_number = num_active_crashes();
    if(!args.empty()){
      args[0].getValue(crash_number);
    }
    retval = delete_crash(crash_number);
  }
  else{ 
    cerr << error << "invalid command: " << name << endl;
  }
  
  // This sends a command handle back to the executive.

  //TODO: incorporate COMMAND_SUCCEEDED after IO completed
  m_execInterface.handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  // This sends the command's return value (if expected) to the executive.
  if (retval != Unknown){
    m_execInterface.handleCommandReturn(cmd, retval);
  }
  m_execInterface.notifyOfExternalEvent();
}

void CheckpointAdapter::lookupNow (const State& state, StateCacheEntry &entry)
{
  entry.update(fetch(state.name(), state.parameters()));
}


void CheckpointAdapter::subscribe(const State& state)
{
  debugMsg("CheckpointAdapter:subscribe", " processing state " << state.name());
  m_subscribedStates.insert(state);
}


void CheckpointAdapter::unsubscribe (const State& state)
{
  debugMsg("CheckpointAdapter:subscribe", " from state " << state.name());
  m_subscribedStates.erase(state);
}

// Does nothing.
void CheckpointAdapter::setThresholds (const State& state, double hi, double lo)
{
}

void CheckpointAdapter::setThresholds (const State& state, int32_t hi, int32_t lo)
{
}

void CheckpointAdapter::propagate (const State& state, const vector<Value>& value)
{
  CheckpointAdapter::propagateValueChange(state, value);
}

void CheckpointAdapter::propagateValueChange (const State& state,
                                          const vector<Value>& vals) const
{
  if (!isStateSubscribed(state))
    return; 
  m_execInterface.handleValueChange(state, vals.front());
  m_execInterface.notifyOfExternalEvent();
}


bool CheckpointAdapter::isStateSubscribed(const State& state) const
{
  return m_subscribedStates.find(state) != m_subscribedStates.end();
}

// Necessary boilerplate
extern "C" {
  void initCheckpointAdapter() {
    REGISTER_ADAPTER(CheckpointAdapter, "CheckpointAdapter");
  }
}
