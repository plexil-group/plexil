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

#include "TestSampleAdapterOne.hh"

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
TestSampleAdapterOne *TestSampleAdapterOne::m_adapter = 0;

///////////////////////////// Member functions //////////////////////////////////

TestSampleAdapterOne::TestSampleAdapterOne(AdapterExecInterface& execInterface,
                             const pugi::xml_node& configXml) :
    InterfaceAdapter(execInterface, configXml)
{
  m_adapter = this;
  m_lookups = 0;
  debugMsg("TestSampleAdapterOne", " created.");
}

void TestSampleAdapterOne::executeCommand(Command *cmd) {
  return;
}

TestSampleAdapterOne::~TestSampleAdapterOne()
{
  m_adapter = nullptr;
}

bool TestSampleAdapterOne::initialize()
{
  g_configuration->registerLookupHandler("lookups", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterOne::getLookups));
  debugMsg("TestSampleAdapterOne", " initialized.");
  g_configuration->registerCommandHandler("resetLookups", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterOne::resetLookups));
  return true;
}

void TestSampleAdapterOne::getLookups(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterOne::resetLookups(Command *cmd) {
  m_lookups = 0;
  return Value();
}

bool TestSampleAdapterOne::start()
{
  debugMsg("TestSampleAdapterOne", " started.");
  return true;
}

bool TestSampleAdapterOne::stop()
{
  debugMsg("TestSampleAdapterOne", " stopped.");
  return true;
}

bool TestSampleAdapterOne::reset()
{
  debugMsg("TestSampleAdapterOne", " reset.");
  return true;
}
bool TestSampleAdapterOne::shutdown()
{
  debugMsg("TestSampleAdapterOne", " shut down.");
  return true;
}


void TestSampleAdapterOne::subscribe(const State& state)
{
  debugMsg("TestSampleAdapterOne:subscribe", " processing state " << state.name());
}


void TestSampleAdapterOne::unsubscribe (const State& state)
{
  debugMsg("TestSampleAdapterOne:unsubscribe", " from state " << state.name());
}

// Does nothing.
void TestSampleAdapterOne::setThresholds (const State& state, double hi, double lo)
{
  debugMsg("TestSampleAdapterOne:setThresholds", " from state " << state.name() << " with lo: " << lo << ", hi: " << hi);
}
void TestSampleAdapterOne::setThresholds (const State& state, int32_t hi, int32_t lo)
{
  debugMsg("TestSampleAdapterOne:setThresholds", " from state " << state.name() << " with lo: " << lo << ", hi: " << hi);
}

// Necessary boilerplate
extern "C" {
  void initTestSampleAdapterOne() {
    //print("Registering Adapter...");
    REGISTER_ADAPTER(TestSampleAdapterOne, "TestSampleAdapterOne");
  }
}

