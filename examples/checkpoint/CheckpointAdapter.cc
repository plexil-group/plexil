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
#include "CheckpointAdapter.hh"

#include "Subscriber.hh"
#include "CheckpointSystem.hh"

#include "AdapterConfiguration.hh"
#include "AdapterFactory.hh"
#include "AdapterExecInterface.hh"
#include "CommandHandle.hh"
#include "CommandHandler.hh"
#include "Debug.hh"
#include "Expression.hh"
#include "LookupHandler.hh"
#include "LookupReceiver.hh"

#include <iostream>
#include <algorithm> //transform

using std::cerr;
using std::endl;
using std::string;
using std::map;
using std::string;
using std::vector;
using std::copy;

using namespace PLEXIL;

///////////////////////////// Conveniences //////////////////////////////////

// Handler boilerplate
#define LOOKUP_HANDLER(MIN_ARGS, MAX_ARGS, RETURN_VALUE)                \
  [this](const State& state, LookupReceiver *rcvr) -> void              \
  { if (state.parameterCount() < MIN_ARGS || state.parameterCount() > MAX_ARGS) { \
      cerr << "Invalid number of arguments to " << state.name() << endl; } \
    else { rcvr->update(RETURN_VALUE);                                  \
      m_subscribedStates.insert(state);}}

#define COMMAND_HANDLER(BODY)                               \
  [](Command* cmd, AdapterExecInterface* intf) -> void  \
  { const vector<Value>& args = cmd->getArgValues(); \
    BODY }

// Protects commands and lookups by checking the number of arguments
#define ARGCOUNT(a,b) \
  if (state.parameterCount() < a || state.parameterCount() > b) {       \
    retval.setUnknown();                                                \
    cerr << "Invalid number of arguments to " << state.name() << endl;} \
  else

// A preamble for error messages.
static string error = "Error in CheckpointAdapter: ";

// An empty argument vector.
static vector<Value> EmptyArgs;

//////////////////////////// Helper functions ////////////////////////////////

static string getChildWithAttribute(const pugi::xml_node& configXml,
                                    const string& node_name,
                                    const string& attribute_name)
{
  return configXml.child(node_name.c_str()).attribute(attribute_name.c_str()).value();
}

static Integer get_boot(const vector<Value>& args,int pos){
  // Default is checkpoint in current boot (iteration 0)
  Integer which_boot = 0;
  if(!args.empty()){
    args[pos].getValue(which_boot);
    debugMsg("CheckpointAdapter:fetch", "arg0=" << which_boot);
  }
  else{
    debugMsg("CheckpointAdapter:fetch", "Defaulting boot to 0");
  }
  return which_boot;
}

// The 'receive' functions are the subscribers for system state updates.  They
// receive the name of the state whose value has changed in the system.  Then
// they propagate the state's new value to the executive.

void CheckpointAdapter::receiveValue (const string& state_name, const Value& val)
{
  propagateValueChange (State(state_name, EmptyArgs),
					       vector<Value> (1, val));
}


void CheckpointAdapter::receiveValue (const string& state_name, const Value& val, const Value& arg)
{
  propagateValueChange (State(state_name, vector<Value> (1,arg)),
					       vector<Value> (1, val));
}

void CheckpointAdapter::receiveValue(const string& state_name, const Value& val, const Value& arg1, const Value& arg2)
{
  vector<Value> vec;
  vec.push_back (arg1);
  vec.push_back (arg2);
  propagateValueChange (State(state_name, vec), vector<Value> (1, val));
}

void CheckpointAdapter::receiveCommandReceived(Command* cmd) {
  if (cmd != NULL) {
    getInterface().handleCommandAck(cmd, COMMAND_RCVD_BY_SYSTEM);
    getInterface().notifyOfExternalEvent();
  }
}

void CheckpointAdapter::receiveCommandSuccess(Command* cmd) {
  if (cmd != NULL) {
    getInterface().handleCommandAck(cmd, COMMAND_SUCCESS);
    getInterface().notifyOfExternalEvent();
  }
}

///////////////////////////// Member functions //////////////////////////////////


CheckpointAdapter::CheckpointAdapter(AdapterExecInterface& execInterface,
                                     AdapterConf* conf) :
  InterfaceAdapter(execInterface, conf),
  Subscriber()
{
  
  // Reads save directory from configuration file
  const pugi::xml_node save_config = conf->xml.child("SaveConfiguration");

  // If SaveConfiguration not specified, will be a node_null that always returns empty strings
  CheckpointSystem::getInstance()->setSaveConfiguration(&save_config);

  string ok_on_exit = getChildWithAttribute(conf->xml,"AdapterConfiguration","OKOnExit");
  std::transform(ok_on_exit.begin(),ok_on_exit.end(),ok_on_exit.begin(), ::tolower);
  // Defaults to true for safety
  if(ok_on_exit == "false") m_ok_on_exit = false;
  else m_ok_on_exit = true;
  debugMsg("CheckpointAdapter", " created.");

  string flush_on_exit = getChildWithAttribute(conf->xml,"AdapterConfiguration","FlushOnExit");
  std::transform(flush_on_exit.begin(),flush_on_exit.end(),flush_on_exit.begin(), ::tolower);
  // Defaults to true for safety
  if(flush_on_exit == "false") m_flush_on_exit = false;
  else m_flush_on_exit = true;

  string flush_on_start = getChildWithAttribute(conf->xml,"AdapterConfiguration","FlushOnStart");
  std::transform(flush_on_start.begin(),flush_on_start.end(),flush_on_start.begin(), ::tolower);
  // Defaults to true for safety
  if(flush_on_start == "false") m_flush_on_start = false;
  else m_flush_on_start = true;

  string use_time_s = getChildWithAttribute(conf->xml,"AdapterConfiguration","UseTime");
  std::transform(use_time_s.begin(),use_time_s.end(),use_time_s.begin(), ::tolower);
  // Defaults to true for safety
  if(use_time_s == "false") CheckpointSystem::getInstance()->useTime(false);
  else CheckpointSystem::getInstance()->useTime(true);
  
  debugMsg("CheckpointAdapter", " created.");
}

bool CheckpointAdapter::initialize(AdapterConfiguration *config)
{
  // Construct and register our handlers
  config->registerLookupHandlerFunction("DidCrash",
                                        LOOKUP_HANDLER(0, 0,
                                                       CheckpointSystem::getInstance()->didCrash()));
  config->registerLookupHandlerFunction("IsBootOK",
                                        LOOKUP_HANDLER(0, 1,
                                                       CheckpointSystem::getInstance()->getIsOK(get_boot(state.parameters(), 0))));
  config->registerLookupHandlerFunction("NumberOfAccessibleBoots",
                                        LOOKUP_HANDLER(0, 0,
                                                       CheckpointSystem::getInstance()->numAccessibleBoots()));
  config->registerLookupHandlerFunction("NumberOfTotalBoots",
                                        LOOKUP_HANDLER(0, 0,
                                                       CheckpointSystem::getInstance()->numTotalBoots()));
  config->registerLookupHandlerFunction("NumberOfUnhandledBoots",
                                        LOOKUP_HANDLER(0, 0,
                                                       CheckpointSystem::getInstance()->numUnhandledBoots()));
  config->registerLookupHandlerFunction("TimeOfLastSave",
                                        LOOKUP_HANDLER(0, 1,
                                                       CheckpointSystem::getInstance()->getTimeOfCrash(get_boot(state.parameters(), 0))));
  config->registerLookupHandlerFunction("TimeOfBoot",
                                        LOOKUP_HANDLER(0, 1,
                                                       CheckpointSystem::getInstance()->getTimeOfBoot(get_boot(state.parameters(), 0))));

  config->registerLookupHandlerFunction("CheckpointState",
                                        LOOKUP_HANDLER(1, 2,
                                                       CheckpointSystem::getInstance()->getCheckpointState(state.parameter(0).valueToString(), get_boot(state.parameters(), 1))));
  config->registerLookupHandlerFunction("CheckpointTime",
                                        LOOKUP_HANDLER(1, 2,
                                                       CheckpointSystem::getInstance()->getCheckpointTime(state.parameter(0).valueToString(), get_boot(state.parameters(), 1))));
  config->registerLookupHandlerFunction("CheckpointInfo", 
                                        LOOKUP_HANDLER(1, 2,
                                                       CheckpointSystem::getInstance()->getCheckpointInfo(state.parameter(0).valueToString(), get_boot(state.parameters(), 1))));
  config->registerLookupHandlerFunction("CheckpointWhen",
                                        LOOKUP_HANDLER(1, 1,
                                                       CheckpointSystem::getInstance()->getCheckpointLastPassed(state.parameter(0).valueToString())));

  config->registerCommandHandlerFunction("set_checkpoint",
                                         COMMAND_HANDLER(string checkpoint_name;
                                                         bool flag = true;
                                                         string info;
                                                         args[0].getValue(checkpoint_name);
                                                         switch (args.size()) {
                                                         case 3:
                                                           args[1].getValue(flag);
                                                           args[2].getValue(info);
                                                           break;
                                                         case 2:
                                                           if (args[1].valueType() == BOOLEAN_TYPE)
                                                             args[1].getValue(flag);
                                                           else
                                                             args[1].getValue(info);
                                                           break;
                                                         case 1:
                                                           break;
                                                         default:
                                                           cerr << error << "set_checkpoint invalid number of arguments" << endl;
                                                           intf->handleCommandAck(cmd, COMMAND_FAILED);
                                                           return;
                                                         }
                                                         CheckpointSystem::getInstance()->setCheckpoint(checkpoint_name, flag, info, cmd);));
  config->registerCommandHandlerFunction("set_boot_ok",
                                         COMMAND_HANDLER(bool flag = true;
                                                         Integer boot_num = 0;
                                                         switch (args.size()) {
                                                         case 2:
                                                           args[0].getValue(flag);
                                                           args[1].getValue(boot_num);
                                                           break;
                                                         case 1:
                                                           if (args[0].valueType() == BOOLEAN_TYPE)
                                                             args[0].getValue(flag);
                                                           else
                                                             args[0].getValue(boot_num);
                                                           break;
                                                         default:
                                                           cerr << error << "set_boot_ok invalid number of arguments" << endl;
                                                           intf->handleCommandAck(cmd, COMMAND_FAILED);
                                                           return;
                                                         }
                                                         CheckpointSystem::getInstance()->setOK(flag, boot_num, cmd);));
  config->registerCommandHandlerFunction("flush_checkpoints",
                                         COMMAND_HANDLER(intf->handleCommandReturn(cmd,
                                                                                   Value((Boolean) CheckpointSystem::getInstance()->flush()));
                                                         intf->handleCommandAck(cmd, COMMAND_SUCCESS);));

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

void CheckpointAdapter::stop()
{
  if(m_ok_on_exit) CheckpointSystem::getInstance()->setOK(true,0,NULL);
  if(m_flush_on_exit) CheckpointSystem::getInstance()->flush();
  debugMsg("CheckpointAdapter", " stopped.");
}

// Passes value onto executive, which makes no guarantees about non-modification so can't be const reference
void CheckpointAdapter::propagateValueChange (const State& state,
                                              const vector<Value>& vals)
{
  // If subscribed
  if (m_subscribedStates.find(state) != m_subscribedStates.end()) {
    getInterface().handleValueChange(state, vals.front());
    getInterface().notifyOfExternalEvent();
  }
}

// Necessary boilerplate
extern "C" {
  void initCheckpointAdapter() {
    REGISTER_ADAPTER(CheckpointAdapter, "CheckpointAdapter");
  }
}
