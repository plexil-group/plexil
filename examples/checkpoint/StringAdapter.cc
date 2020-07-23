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
#include "StringAdapter.hh"

#include "AdapterConfiguration.hh"
#include "AdapterFactory.hh"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Expression.hh"
#include "StateCacheEntry.hh"

#include <iostream>
#include <algorithm> //transform




using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::map;
using std::string;
using std::vector;
using std::copy;

///////////////////////////// Conveniences //////////////////////////////////

// A preamble for error messages.
static string error = "Error in StringAdapter: ";

// A prettier name for the "unknown" value.
static Value Unknown;

// Static member initialization
StringAdapter *StringAdapter::m_adapter = 0; 

// An empty argument vector.
static vector<Value> EmptyArgs;


///////////////////////////// State support //////////////////////////////////

// Queries the system for the value of a state and its arguments.
// No Lookups are supported
static Value fetch (const string& state_name, const vector<Value>& args)
{
  debugMsg("StringAdapter:fetch",
           "Fetch called on " << state_name << " with " << args.size() << " args");
  return Unknown;
}



///////////////////////////// Member functions //////////////////////////////////


StringAdapter::StringAdapter(AdapterExecInterface& execInterface,
                             const pugi::xml_node& configXml) :
    InterfaceAdapter(execInterface, configXml)
{
  m_adapter = this;
  debugMsg("StringAdapter", " created.");
}

StringAdapter::~StringAdapter ()
{
  m_adapter = nullptr;
}

bool StringAdapter::initialize()
{
  g_configuration->defaultRegisterAdapter(this);

  g_configuration->registerCommandInterface("ToString", this);
  g_configuration->registerCommandInterface("StringToInteger", this);
  g_configuration->registerCommandInterface("StringToReal", this);
  g_configuration->registerCommandInterface("StringToBoolean", this);
  
  g_configuration->registerCommandInterface("substr", this);
  g_configuration->registerCommandInterface("strlen", this);
  g_configuration->registerCommandInterface("strlwr", this);
  g_configuration->registerCommandInterface("strupr", this);
  
  debugMsg("StringAdapter", " initialized.");
  return true;
}

bool StringAdapter::start()
{
  debugMsg("StringAdapter", " started.");
  // TODO: verify that all adapters are intialized by this point
  return true;
}

bool StringAdapter::stop()
{
  debugMsg("StringAdapter", " stopped.");
  return true;
}

bool StringAdapter::reset()
{
  debugMsg("StringAdapter", " reset.");
  return true;
}

bool StringAdapter::shutdown()
{
  debugMsg("StringAdapter", " shut down.");
  return true;
}


// Sends a command (as invoked in a Plexil command node) to the system and sends
// the status, and return value if applicable, back to the executive.
//
void StringAdapter::executeCommand(Command *cmd)
{
  const string &name = cmd->getName();
  debugMsg("StringAdapter", "Received executeCommand for " << name);  

  
  Value retval = Unknown;
  const vector<Value>& args = cmd->getArgValues();


  // NOTE: many of these are restricted to <2GB strings which really shouldn't be an issue
  if (name == "ToString"){
    retval = args[0].valueToString();
  }
  else if (name == "StringToInteger"){
    int32_t i;
    std::istringstream(args[0].valueToString()) >> i; // Streams string to integer
    // This is the C++98 version of stoi
    retval = i;
  }
  else if (name == "StringToReal"){
    double d;
    std::istringstream(args[0].valueToString()) >> d; // Streams string to double
    retval = d;
  }
  else if (name == "StringToBoolean"){
    string data = args[0].valueToString();
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);
    std::istringstream is(data);
    bool b;
    is >> std::boolalpha >> b;
    retval = b;
  }
  else if (name == "substr"){
    string data = args[0].valueToString();
    if(args.size()==1){
      retval = data.substr();
    }
    if(args.size()==2){
      int32_t pos;
      args[1].getValue(pos);
      retval = data.substr(pos);
    }
    if(args.size()==3){
      int32_t pos;
      int32_t len;
      args[1].getValue(pos);
      args[2].getValue(len);
      retval = data.substr(pos,len);
    }
  }
  else if (name == "strlen"){
    int32_t len = args[0].valueToString().length();
    retval = len;
  }
  else if (name == "strlwr"){
    string data = args[0].valueToString();
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);
    retval = data;
  }
  else if (name == "strupr"){
    string data = args[0].valueToString();
    std::transform(data.begin(), data.end(), data.begin(), ::toupper);
    retval = data;
  }
  else{ 
    cerr << error << "invalid command: " << name << endl;
  }

  
  // This sends a command handle back to the executive.
  m_execInterface.handleCommandAck(cmd, COMMAND_SUCCESS);
  // This sends the command's return value (if expected) to the executive.
  if (retval != Unknown){
    m_execInterface.handleCommandReturn(cmd, retval);
  }
  m_execInterface.notifyOfExternalEvent();
}

void StringAdapter::lookupNow (const State& state, StateCacheEntry &entry)
{
  entry.update(fetch(state.name(), state.parameters()));
}


void StringAdapter::subscribe(const State& state)
{
  debugMsg("StringAdapter:subscribe", " processing state " << state.name());
  m_subscribedStates.insert(state);
}


void StringAdapter::unsubscribe (const State& state)
{
  debugMsg("StringAdapter:subscribe", " from state " << state.name());
  m_subscribedStates.erase(state);
}

// Does nothing.
void StringAdapter::setThresholds (const State& state, double hi, double lo)
{
}

void StringAdapter::setThresholds (const State& state, int32_t hi, int32_t lo)
{
}

void StringAdapter::propagate (const State& state, const vector<Value>& value)
{
  StringAdapter::propagateValueChange(state, value);
}

void StringAdapter::propagateValueChange (const State& state,
                                          const vector<Value>& vals) const
{
  if (!isStateSubscribed(state))
    return; 
  m_execInterface.handleValueChange(state, vals.front());
  m_execInterface.notifyOfExternalEvent();
}


bool StringAdapter::isStateSubscribed(const State& state) const
{
  return m_subscribedStates.find(state) != m_subscribedStates.end();
}

// Necessary boilerplate
extern "C" {
  void initStringAdapter() {
    REGISTER_ADAPTER(StringAdapter, "StringAdapter");
  }
}
