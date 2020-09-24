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
#include <algorithm> //tranform

// Protects commands and lookups by guaranteeing the correc number of arguments
#define ARGCOUNT(a,b)  if(args.size()<a || args.size()>b){ \
      retval = Unknown; \
      cerr<<"Invalid number of arguments to "<<state_name<<endl;} else


using std::cerr;
using std::endl;
using std::string;
using std::map;
using std::string;
using std::vector;
using std::copy;

using namespace PLEXIL;


///////////////////////////// Conveniences //////////////////////////////////

// A preamble for error messages.
static string error = "Error in CheckpointAdapter: ";

// A prettier name for the "unknown" value.
static Value Unknown;

// Static member initialization
CheckpointSystem *CheckpointSystem::s_system = 0;

// An empty argument vector.
static vector<Value> EmptyArgs;

//////////////////////////// Helper functions ////////////////////////////////

// TODO: Use XPath
static string getChildWithAttribute(const pugi::xml_node& configXml,
			     const string& node_name,
			     const string& attribute_name){
  for (pugi::xml_node child = configXml.first_child(); child; child = child.next_sibling())
  {
    if(child.name()==node_name){
      for (pugi::xml_attribute attr = child.first_attribute(); attr; attr = attr.next_attribute())
      {
	if(attr.name()==attribute_name){
	  return attr.value();
	}
      }
    }
  }
  return "";
}

static pugi::xml_node getChildWithName(const pugi::xml_node& configXml,
			     const string& node_name){
  for (pugi::xml_node child = configXml.first_child(); child; child = child.next_sibling())
  {
    if(child.name()==node_name){
      return child;
    }
  }
  return pugi::xml_node(NULL);
}

static Integer get_boot(const vector<Value>& args,int pos){
  // Default is checkpoint in current boot (iteration 0)
  Integer which_boot = 0;
  if(!args.empty()){
    args[pos].getValue(which_boot);
    debugMsg("CheckpointAdapter:fetch","arg0="<<which_boot);
  }
  else{
    debugMsg("CheckpointAdapter:fetch","Defaulting boot to 0");
  }
  return which_boot;
}

///////////////////////////// State support //////////////////////////////////

// Queries the system for the value of a state and its arguments.
//
static Value fetch (const string& state_name, const vector<Value>& args)
{
  debugMsg("CheckpointAdapter:fetch",
           "Fetch called on " << state_name << " with " << args.size() << " args");
 
  Value retval;
  
  if (state_name == "DidCrash"){
    ARGCOUNT(0,0){
      retval = CheckpointSystem::getInstance()->didCrash();
    }
  }
  else if (state_name == "NumberOfAccessibleBoots"){
    ARGCOUNT(0,0){
      retval = CheckpointSystem::getInstance()->numAccessibleBoots();
    }
  }
  else if (state_name == "NumberOfUnhandledBoots"){
    ARGCOUNT(0,0){
      retval = CheckpointSystem::getInstance()->numUnhandledBoots();
    }
  }
  else if (state_name == "NumberOfTotalBoots"){
    ARGCOUNT(0,0){
      retval = CheckpointSystem::getInstance()->numTotalBoots();
    }
  }
  else if (state_name == "CheckpointWhen"){
    ARGCOUNT(1,1){
      string which_checkpoint;
      args[0].getValue(which_checkpoint);
      retval = CheckpointSystem::getInstance()->getCheckpointLastPassed(which_checkpoint);
    }
  }

  else{
    if (state_name == "TimeOfLastSave"){
      ARGCOUNT(0,1){
	retval = CheckpointSystem::getInstance()->getTimeOfCrash(get_boot(args,0));
      }
    }
    else if (state_name == "TimeOfBoot"){
      ARGCOUNT(0,1){
	retval = CheckpointSystem::getInstance()->getTimeOfBoot(get_boot(args,0));
      }
    }
    else if (state_name == "IsBootOK"){
      ARGCOUNT(0,1){
	retval = CheckpointSystem::getInstance()->getIsOK(get_boot(args,0));
      }
    }
    else{
      ARGCOUNT(1,2){
	Integer which_boot = get_boot(args,1);
	string which_checkpoint;
	args[0].getValue(which_checkpoint);
	if (state_name == "CheckpointState"){
	  retval = CheckpointSystem::getInstance()->getCheckpointState(which_checkpoint,which_boot);
	}
	else if (state_name == "CheckpointTime"){
	  retval = CheckpointSystem::getInstance()->getCheckpointTime(which_checkpoint,which_boot);
	}
	else if (state_name == "CheckpointInfo"){
	  retval = CheckpointSystem::getInstance()->getCheckpointInfo(which_checkpoint,which_boot);
	}
	//No match
	else {
	  cerr << error << "invalid state: " << state_name << endl;
	  retval = Unknown;
	}
      }
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

void CheckpointAdapter::receiveValue (const string& state_name, const Value& val)
{
  propagateValueChange (createState(state_name, EmptyArgs),
					       vector<Value> (1, val));
}


void CheckpointAdapter::receiveValue (const string& state_name, const Value& val, const Value& arg)
{
  propagateValueChange (createState(state_name, vector<Value> (1,arg)),
					       vector<Value> (1, val));
}


void CheckpointAdapter::receiveValue(const string& state_name, const Value& val, const Value& arg1,const Value& arg2)
{
  vector<Value> vec;
  vec.push_back (arg1);
  vec.push_back (arg2);
  propagateValueChange (createState(state_name, vec), vector<Value> (1, val));
}

void CheckpointAdapter::receiveCommandReceived(Command* cmd){
  if(cmd != NULL){
    m_execInterface.handleCommandAck(cmd, COMMAND_RCVD_BY_SYSTEM);
    m_execInterface.notifyOfExternalEvent();
  }
}
void CheckpointAdapter::receiveCommandSuccess   (Command* cmd){
  if(cmd != NULL){
    m_execInterface.handleCommandAck(cmd, COMMAND_SUCCESS);
    m_execInterface.notifyOfExternalEvent();
  }
}

///////////////////////////// Member functions //////////////////////////////////


CheckpointAdapter::CheckpointAdapter(AdapterExecInterface& execInterface,
                             const pugi::xml_node& configXml) :
    InterfaceAdapter(execInterface, configXml)
{

  
  // Reads save directory from configuration file
  const pugi::xml_node save_config = getChildWithName(configXml,"SaveConfiguration");

  // If SaveConfiguration not specified, will be a node_null that always returns empty strings
  CheckpointSystem::getInstance()->setSaveConfiguration(&save_config);

  string ok_on_exit = getChildWithAttribute(configXml,"AdapterConfiguration","OKOnExit");
  std::transform(ok_on_exit.begin(),ok_on_exit.end(),ok_on_exit.begin(), ::tolower);
  // Defaults to true for safety
  if(ok_on_exit == "false") m_ok_on_exit = false;
  else m_ok_on_exit = true;
  debugMsg("CheckpointAdapter", " created.");

  string flush_on_exit = getChildWithAttribute(configXml,"AdapterConfiguration","FlushOnExit");
  std::transform(flush_on_exit.begin(),flush_on_exit.end(),flush_on_exit.begin(), ::tolower);
  // Defaults to true for safety
  if(flush_on_exit == "false") m_flush_on_exit = false;
  else m_flush_on_exit = true;

  string flush_on_start = getChildWithAttribute(configXml,"AdapterConfiguration","FlushOnStart");
  std::transform(flush_on_start.begin(),flush_on_start.end(),flush_on_start.begin(), ::tolower);
  // Defaults to true for safety
  if(flush_on_start == "false") m_flush_on_start = false;
  else m_flush_on_start = true;

  string use_time_s = getChildWithAttribute(configXml,"AdapterConfiguration","UseTime");
  std::transform(use_time_s.begin(),use_time_s.end(),use_time_s.begin(), ::tolower);
  // Defaults to true for safety
  if(use_time_s == "false") CheckpointSystem::getInstance()->useTime(false);
  else CheckpointSystem::getInstance()->useTime(true);
  
  debugMsg("CheckpointAdapter", " created.");
}

bool CheckpointAdapter::initialize()
{
  g_configuration->registerLookupInterface("DidCrash", this);
  g_configuration->registerLookupInterface("IsBootOK", this);
  
  g_configuration->registerLookupInterface("NumberOfAccessibleBoots", this);
  g_configuration->registerLookupInterface("NumberOfTotalBoots", this);
  g_configuration->registerLookupInterface("NumberOfUnhandledBoots", this);
  g_configuration->registerLookupInterface("TimeOfLastSave", this);
  g_configuration->registerLookupInterface("TimeOfBoot", this);

  g_configuration->registerLookupInterface("CheckpointState", this);
  g_configuration->registerLookupInterface("CheckpointTime", this);
  g_configuration->registerLookupInterface("CheckpointInfo", this);
  g_configuration->registerLookupInterface("CheckpointWhen", this);

  g_configuration->registerCommandInterface("set_checkpoint", this);
  g_configuration->registerCommandInterface("set_boot_ok", this);
  g_configuration->registerCommandInterface("flush_checkpoints", this);

  // Register ourselves to receive updates
  setSubscriber(this);
  
  debugMsg("CheckpointAdapter", " initialized.");
  return true;
}

bool CheckpointAdapter::start()
{
  CheckpointSystem::getInstance()->start();
  if(m_flush_on_start) CheckpointSystem::getInstance()->flush();
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
  if(m_ok_on_exit) CheckpointSystem::getInstance()->setOK(true,0,NULL);
  if(m_flush_on_exit) CheckpointSystem::getInstance()->flush();
  debugMsg("CheckpointAdapter", " shut down.");
  return true;
}


// Sends a command (as invoked in a Plexil command node) to the system and sends
// the status, and return value if applicable, back to the executive.

void CheckpointAdapter::executeCommand(Command *cmd)
{

  const string &name = cmd->getName();
  debugMsg("CheckpointAdapter", "Received executeCommand for " << name);  

  
  Value retval = Unknown;
  const vector<Value>& args = cmd->getArgValues();
  
  // Each command in CheckpointSystem publishes to ReceiveCommandReceived or Success 
  // set_checkpoint and set_boot_ok to only return success when the change has been (possibly asycnrhonously)
  // written to disk

  // setOK and setCheckpoint handle the command acknowledgement themselves
    
  if (name == "flush_checkpoints"){
    retval = CheckpointSystem::getInstance()->flush();
    publishCommandSuccess(cmd); // publish success
  }
  else if (name == "set_checkpoint") {
    if(args.size()<1 || args.size()>3){
      cerr << error << "set_checkpoint invalid number of arguments" << endl;
    }
    
    else{
      string checkpoint_name = "";
      
      bool value = true;
      string info = "";
      args[0].getValue(checkpoint_name);

      if(args.size()==3){
	args[1].getValue(value);
	args[2].getValue(info);
      }
      if(args.size()==2){
	if(args[1].valueType()==BOOLEAN_TYPE) args[1].getValue(value);
	else args[1].getValue(info);
      }
      CheckpointSystem::getInstance()->setCheckpoint(checkpoint_name,value,info,cmd);
    }
  }

  else if (name == "set_boot_ok") {
    if(args.size()>2){
      cerr << error << "set_boot_ok invalid number of arguments" << endl;
    }
    
    else{      
      bool value = true;
      Integer boot_num = 0;

      if(args.size()==2){
	args[0].getValue(value);
	args[1].getValue(boot_num);
      }
      if(args.size()==1){
	if(args[0].valueType()==BOOLEAN_TYPE) args[0].getValue(value);
	else args[0].getValue(boot_num);
      }
      CheckpointSystem::getInstance()->setOK(value,boot_num,cmd);
    }
  }
  
  else{ 
    cerr << error << "invalid command: " << name << endl;
  }
  
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
  debugMsg("CheckpointAdapter:subscribe", " unsubscribing from state " << state.name());
  m_subscribedStates.erase(state);
}

  // Passes value onto executive, which makes no guarantees about non-modification so can't be const reference
void CheckpointAdapter::propagateValueChange (const State& state,
                                          const vector<Value>& vals) const
{
  // If subscribed
  if (m_subscribedStates.find(state) != m_subscribedStates.end()){
    m_execInterface.handleValueChange(state, vals.front());
    m_execInterface.notifyOfExternalEvent();
  }
}


// Necessary boilerplate
extern "C" {
  void initCheckpointAdapter() {
    REGISTER_ADAPTER(CheckpointAdapter, "CheckpointAdapter");
  }
}
