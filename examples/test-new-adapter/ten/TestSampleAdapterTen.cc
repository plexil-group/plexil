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

#include "TestSampleAdapterTen.hh"

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
TestSampleAdapterTen *TestSampleAdapterTen::m_adapter = 0;

///////////////////////////// Member functions //////////////////////////////////

TestSampleAdapterTen::TestSampleAdapterTen(AdapterExecInterface& execInterface,
                             const pugi::xml_node& configXml) :
    InterfaceAdapter(execInterface, configXml)
{
  m_adapter = this;
  m_lookups = 0;
  debugMsg("TestSampleAdapterTen", " created.");
}

void TestSampleAdapterTen::executeCommand(Command *cmd) {
  return;
}

TestSampleAdapterTen::~TestSampleAdapterTen()
{
  m_adapter = nullptr;
}

bool TestSampleAdapterTen::initialize()
{
  g_configuration->registerLookupHandler("lookups1", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterTen::getLookups1));
  debugMsg("TestSampleAdapterTen", " initialized.");
  g_configuration->registerCommandHandler("resetLookups1", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterTen::resetLookups1));
  g_configuration->registerLookupHandler("lookups2", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterTen::getLookups2));
  debugMsg("TestSampleAdapterTen", " initialized.");
  g_configuration->registerCommandHandler("resetLookups2", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterTen::resetLookups2));
  g_configuration->registerLookupHandler("lookups3", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterTen::getLookups3));
  debugMsg("TestSampleAdapterTen", " initialized.");
  g_configuration->registerCommandHandler("resetLookups3", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterTen::resetLookups3));
  g_configuration->registerLookupHandler("lookups4", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterTen::getLookups4));
  debugMsg("TestSampleAdapterTen", " initialized.");
  g_configuration->registerCommandHandler("resetLookups4", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterTen::resetLookups4));
  g_configuration->registerLookupHandler("lookups5", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterTen::getLookups5));
  debugMsg("TestSampleAdapterTen", " initialized.");
  g_configuration->registerCommandHandler("resetLookups5", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterTen::resetLookups5));
  g_configuration->registerLookupHandler("lookups6", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterTen::getLookups6));
  debugMsg("TestSampleAdapterTen", " initialized.");
  g_configuration->registerCommandHandler("resetLookups6", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterTen::resetLookups6));
  g_configuration->registerLookupHandler("lookups7", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterTen::getLookups7));
  debugMsg("TestSampleAdapterTen", " initialized.");
  g_configuration->registerCommandHandler("resetLookups7", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterTen::resetLookups7));
  g_configuration->registerLookupHandler("lookups8", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterTen::getLookups8));
  debugMsg("TestSampleAdapterTen", " initialized.");
  g_configuration->registerCommandHandler("resetLookups8", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterTen::resetLookups8));
  g_configuration->registerLookupHandler("lookups9", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterTen::getLookups9));
  debugMsg("TestSampleAdapterTen", " initialized.");
  g_configuration->registerCommandHandler("resetLookups9", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterTen::resetLookups9));
  g_configuration->registerLookupHandler("lookups10", *this, 
      (AdapterConfiguration::LookupNowHandler)(&TestSampleAdapterTen::getLookups10));
  debugMsg("TestSampleAdapterTen", " initialized.");
  g_configuration->registerCommandHandler("resetLookups10", *this, m_execInterface,
    (AdapterConfiguration::ExecuteCommandHandler)(&TestSampleAdapterTen::resetLookups10));
  return true;
}

void TestSampleAdapterTen::getLookups1(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterTen::resetLookups1(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterTen::getLookups2(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterTen::resetLookups2(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterTen::getLookups3(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterTen::resetLookups3(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterTen::getLookups4(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterTen::resetLookups4(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterTen::getLookups5(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterTen::resetLookups5(Command *cmd) {
  m_lookups = 0;
  return Value();
}

void TestSampleAdapterTen::getLookups6(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterTen::resetLookups6(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterTen::getLookups7(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterTen::resetLookups7(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterTen::getLookups8(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterTen::resetLookups8(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterTen::getLookups9(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterTen::resetLookups9(Command *cmd) {
  m_lookups = 0;
  return Value();
}
void TestSampleAdapterTen::getLookups10(State const &state, StateCacheEntry &cacheEntry) {
  m_lookups++;
  cacheEntry.update(m_lookups);
}

Value TestSampleAdapterTen::resetLookups10(Command *cmd) {
  m_lookups = 0;
  return Value();
}

bool TestSampleAdapterTen::start()
{
  debugMsg("TestSampleAdapterTen", " started.");
  return true;
}

bool TestSampleAdapterTen::stop()
{
  debugMsg("TestSampleAdapterTen", " stopped.");
  return true;
}

bool TestSampleAdapterTen::reset()
{
  debugMsg("TestSampleAdapterTen", " reset.");
  return true;
}
bool TestSampleAdapterTen::shutdown()
{
  debugMsg("TestSampleAdapterTen", " shut down.");
  return true;
}


void TestSampleAdapterTen::subscribe(const State& state)
{
  debugMsg("TestSampleAdapterTen:subscribe", " processing state " << state.name());
}


void TestSampleAdapterTen::unsubscribe (const State& state)
{
  debugMsg("TestSampleAdapterTen:unsubscribe", " from state " << state.name());
}

// Does nothing.
void TestSampleAdapterTen::setThresholds (const State& state, double hi, double lo)
{
  debugMsg("TestSampleAdapterTen:setThresholds", " from state " << state.name() << " with lo: " << lo << ", hi: " << hi);
}
void TestSampleAdapterTen::setThresholds (const State& state, int32_t hi, int32_t lo)
{
  debugMsg("TestSampleAdapterTen:setThresholds", " from state " << state.name() << " with lo: " << lo << ", hi: " << hi);
}

// Necessary boilerplate
extern "C" {
  void initTestSampleAdapterTen() {
    //print("Registering Adapter...");
    REGISTER_ADAPTER(TestSampleAdapterTen, "TestSampleAdapterTen");
  }
}

