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

#include "TestSampleAdapterOneOld.hh"

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
static string error = "Error in SampleAdaptor: ";

// A prettier name for the "unknown" value.
static Value const Unknown;

// Static member initialization
TestSampleAdapterOneOld *TestSampleAdapterOneOld::m_adapter = 0;

///////////////////////////// Member functions //////////////////////////////////

TestSampleAdapterOneOld::TestSampleAdapterOneOld(AdapterExecInterface& execInterface,
                             const pugi::xml_node& configXml) :
    InterfaceAdapter(execInterface, configXml)
{
  m_adapter = this;
  m_lookups = 0;
  debugMsg("TestSampleAdapterOneOld", " created.");
}

Value TestSampleAdapterOneOld::fetch (const string& state_name, const vector<Value>& args)
{
  // debugMsg("SampleAdapter:fetch",
  //          "Fetch called on " << state_name << " with " << args.size() << " args");
  Value retval;

  // NOTE: A more streamlined approach to dispatching on state name
  // would be nice.

  if (state_name == "lookups") {
    m_lookups++;
    retval = m_lookups;
  } else {
    cerr << error << "invalid state: " << state_name << endl;
    retval = Unknown;
  }
  //debugMsg("SampleAdapter:fetch", "Fetch returning " << retval);
  return retval;
}

void TestSampleAdapterOneOld::lookupNow (const State& state, StateCacheEntry &entry)
{
  entry.update(fetch(state.name(), state.parameters()));
}

void TestSampleAdapterOneOld::executeCommand(Command *cmd) {
  string const &name = cmd->getName();
  debugMsg("SampleAdapter", "Received executeCommand for " << name);  

  Value retval = Unknown;
  vector<Value> argv(10);
  const vector<Value>& args = cmd->getArgValues();
  copy (args.begin(), args.end(), argv.begin());

  if (name == "resetLookups") {
    this->resetLookups(cmd);
  }
  else
    cerr << error << "invalid command: " << name << endl;

  // This sends a command handle back to the executive.
  m_execInterface.handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
  // This sends the command's return value (if expected) to the executive.
  if (retval != Unknown) {
    m_execInterface.handleCommandReturn(cmd, retval);
  }
  m_execInterface.notifyOfExternalEvent();
}

TestSampleAdapterOneOld::~TestSampleAdapterOneOld()
{
  m_adapter = nullptr;
}

bool TestSampleAdapterOneOld::initialize()
{
  g_configuration->registerLookupInterface("lookups", this);
  g_configuration->registerCommandInterface("resetLookups", this);
  debugMsg("TestSampleAdapterOneOld", " initialized.");
  return true;
}

bool TestSampleAdapterOneOld::start()
{
  debugMsg("TestSampleAdapterOneOld", " started.");
  return true;
}

bool TestSampleAdapterOneOld::stop()
{
  debugMsg("TestSampleAdapterOneOld", " stopped.");
  return true;
}

bool TestSampleAdapterOneOld::reset()
{
  debugMsg("TestSampleAdapterOneOld", " reset.");
  return true;
}
bool TestSampleAdapterOneOld::shutdown()
{
  debugMsg("TestSampleAdapterOneOld", " shut down.");
  return true;
}


void TestSampleAdapterOneOld::subscribe(const State& state)
{
  debugMsg("TestSampleAdapterOneOld:subscribe", " processing state " << state.name());
}


void TestSampleAdapterOneOld::unsubscribe (const State& state)
{
  debugMsg("TestSampleAdapterOneOld:unsubscribe", " from state " << state.name());
}

// Does nothing.
void TestSampleAdapterOneOld::setThresholds (const State& state, double hi, double lo)
{
  debugMsg("TestSampleAdapterOneOld:setThresholds", " from state " << state.name() << " with lo: " << lo << ", hi: " << hi);
}
void TestSampleAdapterOneOld::setThresholds (const State& state, int32_t hi, int32_t lo)
{
  debugMsg("TestSampleAdapterOneOld:setThresholds", " from state " << state.name() << " with lo: " << lo << ", hi: " << hi);
}

// Necessary boilerplate
extern "C" {
  void initTestSampleAdapterOneOld() {
    //print("Registering Adapter...");
    REGISTER_ADAPTER(TestSampleAdapterOneOld, "TestSampleAdapterOneOld");
  }
}

