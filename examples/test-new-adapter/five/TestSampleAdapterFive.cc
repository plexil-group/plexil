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

#include "TestSampleAdapterFive.hh"

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
TestSampleAdapterFive *TestSampleAdapterFive::m_adapter = 0;

///////////////////////////// Member functions //////////////////////////////////

TestSampleAdapterFive::TestSampleAdapterFive(AdapterExecInterface& execInterface,
                             const pugi::xml_node& configXml) :
    InterfaceAdapter(execInterface, configXml)
{
  m_adapter = this;
  m_lookups = 0;
  debugMsg("TestSampleAdapterFive", " created.");
}

void TestSampleAdapterFive::executeCommand(Command *cmd) {
  return;
}

TestSampleAdapterFive::~TestSampleAdapterFive()
{
  m_adapter = nullptr;
}

bool TestSampleAdapterFive::initialize()
{
  g_configuration->registerLookupHandler("lookups1", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterFive::getLookups1));
  debugMsg("TestSampleAdapterFive", " initialized.");
  g_configuration->registerCommandHandler("resetLookups1", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterFive::resetLookups1));
  g_configuration->registerLookupHandler("lookups2", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterFive::getLookups2));
  debugMsg("TestSampleAdapterFive", " initialized.");
  g_configuration->registerCommandHandler("resetLookups2", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterFive::resetLookups2));
  g_configuration->registerLookupHandler("lookups3", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterFive::getLookups3));
  debugMsg("TestSampleAdapterFive", " initialized.");
  g_configuration->registerCommandHandler("resetLookups3", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterFive::resetLookups3));
  g_configuration->registerLookupHandler("lookups4", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterFive::getLookups4));
  debugMsg("TestSampleAdapterFive", " initialized.");
  g_configuration->registerCommandHandler("resetLookups4", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterFive::resetLookups4));
  g_configuration->registerLookupHandler("lookups5", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterFive::getLookups5));
  debugMsg("TestSampleAdapterFive", " initialized.");
  g_configuration->registerCommandHandler("resetLookups5", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterFive::resetLookups5));
  return true;
}

void TestSampleAdapterFive::getLookups1(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterFive::resetLookups1(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterFive::getLookups2(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterFive::resetLookups2(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterFive::getLookups3(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterFive::resetLookups3(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterFive::getLookups4(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterFive::resetLookups4(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterFive::getLookups5(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterFive::resetLookups5(Command *cmd) {
  m_lookups = 0;
  return Value();
}

bool TestSampleAdapterFive::start()
{
  debugMsg("TestSampleAdapterFive", " started.");
  return true;
}

bool TestSampleAdapterFive::stop()
{
  debugMsg("TestSampleAdapterFive", " stopped.");
  return true;
}

bool TestSampleAdapterFive::reset()
{
  debugMsg("TestSampleAdapterFive", " reset.");
  return true;
}
bool TestSampleAdapterFive::shutdown()
{
  debugMsg("TestSampleAdapterFive", " shut down.");
  return true;
}


void TestSampleAdapterFive::subscribe(const State& state)
{
  debugMsg("TestSampleAdapterFive:subscribe", " processing state " << state.name());
}


void TestSampleAdapterFive::unsubscribe (const State& state)
{
  debugMsg("TestSampleAdapterFive:unsubscribe", " from state " << state.name());
}

// Does nothing.
void TestSampleAdapterFive::setThresholds (const State& state, double hi, double lo)
{
  debugMsg("TestSampleAdapterFive:setThresholds", " from state " << state.name() << " with lo: " << lo << ", hi: " << hi);
}
void TestSampleAdapterFive::setThresholds (const State& state, int32_t hi, int32_t lo)
{
  debugMsg("TestSampleAdapterFive:setThresholds", " from state " << state.name() << " with lo: " << lo << ", hi: " << hi);
}

// Necessary boilerplate
extern "C" {
  void initTestSampleAdapterFive() {
    //print("Registering Adapter...");
    REGISTER_ADAPTER(TestSampleAdapterFive, "TestSampleAdapterFive");
  }
}

