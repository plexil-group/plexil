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


using namespace PLEXIL;

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
  debugMsg("StringAdapter", " created.");
}

bool StringAdapter::initialize()
{
  g_configuration->defaultRegisterAdapter(this);

  g_configuration->registerCommandInterface("ToString", this);
  g_configuration->registerCommandInterface("StringToInteger", this);
  g_configuration->registerCommandInterface("StringToReal", this);
  g_configuration->registerCommandInterface("StringToBoolean", this);
  
  g_configuration->registerCommandInterface("substr", this);
  g_configuration->registerCommandInterface("strlwr", this);
  g_configuration->registerCommandInterface("strupr", this);
  g_configuration->registerCommandInterface("strindex", this);
  g_configuration->registerCommandInterface("find_first_of", this);
  g_configuration->registerCommandInterface("find_last_of", this);
  
  debugMsg("StringAdapter", " initialized.");
  return true;
}

bool StringAdapter::start()
{
  debugMsg("StringAdapter", " started.");
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
    if(args.size()!=1){
      retval = Unknown;
      cerr<<"Invalid number of arguments to "<<name<<endl;
    }
    else{
      retval = args[0].valueToString();
    }
  }
  else if (name == "StringToInteger"){
    if(args.size()!=1) {
      retval = Unknown;
      cerr<<"Invalid number of arguments to "<<name<<endl;
    }
    else{
      int32_t i;
      std::istringstream(args[0].valueToString()) >> i; // Streams string to integer
      // This is the C++98 version of stoi
      retval = i;
    }
  }
  else if (name == "StringToReal"){
    if(args.size()!=1){
      retval = Unknown;
      cerr<<"Invalid number of arguments to "<<name<<endl;
    }
    else{
      double d;
      std::istringstream(args[0].valueToString()) >> d; // Streams string to double
      retval = d;
    }
  }
  else if (name == "StringToBoolean"){
    if(args.size()!=1){
      retval = Unknown;
      cerr<<"Invalid number of arguments to "<<name<<endl;
    }
    else{
      string data = args[0].valueToString();
      std::transform(data.begin(), data.end(), data.begin(), ::tolower);
      std::istringstream is(data);
      // Allows us to look for 1 or true, all other values return false
      bool b;
      bool b2;
      is >> std::boolalpha >> b;
      is.clear();
      is >> std::noboolalpha >> b2;
      retval = b||b2;
    }
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
    else{
      cerr<<"Invalid number of arguments to "<<name<<endl;
      retval = Unknown;
    }
  }
  else if (name == "strlwr"){
     if(args.size()!=1){
      retval = Unknown;
      cerr<<"Invalid number of arguments to "<<name<<endl;
    }
    else{
      string data = args[0].valueToString();
      std::transform(data.begin(), data.end(), data.begin(), ::tolower);
      retval = data;
    }
  }
  else if (name == "strupr"){
     if(args.size()!=1){
      retval = Unknown;
      cerr<<"Invalid number of arguments to "<<name<<endl;
    }
    else{
      string data = args[0].valueToString();
      std::transform(data.begin(), data.end(), data.begin(), ::toupper);
      retval = data;
    }
  }
  else if (name == "strupr"){
    if(args.size()!=1){
      retval = Unknown;
      cerr<<"Invalid number of arguments to "<<name<<endl;
    }
    else{
    string data = args[0].valueToString();
    std::transform(data.begin(), data.end(), data.begin(), ::toupper);
    retval = data;
    }
  }
  // strindex(s,i,[v]) acts the same as s[i] = v, or s[i] if v is not specified
  else if (name == "strindex"){
    if(args.size()<2 || args.size()>3){
      retval = Unknown;
      cerr<<"Invalid number of arguments to "<<name<<endl;
    }
    else{
      string data = args[0].valueToString();
      int pos;
      args[1].getValue(pos);
      if(args.size()==3){
	const string newval = args[2].valueToString();
	const string beginning = (pos>0)?data.substr(0,pos):"";
	const string end = (pos<data.length())?data.substr(pos+1):"";
	retval =  beginning + newval + end ;
      }
      else retval = string(1, data[pos]);
    }
  }
  else if (name == "find_first_of"){
    if(args.size()<2 || args.size()>3){
      retval = Unknown;
      cerr<<"Invalid number of arguments to "<<name<<endl;
    }
    else{
      string data = args[0].valueToString();
      string toSearchFor = args[1].valueToString();
      int pos = 0;
      if(args.size()==3){
	args[2].getValue(pos);
      }
      int i = data.find_first_of(toSearchFor,pos);
      retval = i;
    }
  }
  else if (name == "find_last_of"){
    if(args.size()<2 || args.size()>3){
      retval = Unknown;
      cerr<<"Invalid number of arguments to "<<name<<endl;
    }
    else{
      string data = args[0].valueToString();
      string toSearchFor = args[1].valueToString();
      int pos = 0;
      if(args.size()==3){
	args[2].getValue(pos);
      }
      int i = data.find_last_of(toSearchFor,pos);
      retval = i;
    }
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


// Necessary boilerplate
extern "C" {
  void initStringAdapter() {
    REGISTER_ADAPTER(StringAdapter, "StringAdapter");
  }
}
