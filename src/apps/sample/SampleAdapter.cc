/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

// NOTE: this file contains a lot of static functions, which
// we might later choose to incorporate as class members.

#include <iostream>
#include "AdapterFactory.hh"
#include "CoreExpressions.hh"
#include "AdapterExecInterface.hh"
#include "Expression.hh"
#include "Debug.hh"
#include "subscriber.hh"
#include "sample_system.hh"
#include "SampleAdapter.hh"

using PLEXIL::LabelStr;
using PLEXIL::State;

using std::cout;
using std::cerr;
using std::endl;
using std::list;
using std::map;
using std::string;
using std::vector;
using std::copy;


///////////////////////////// Conveniences //////////////////////////////////

// A preamble for error messages.
static string error = "Error in SampleAdaptor: ";

// A prettier name for the "unknown" value.
static Any Unknown = PLEXIL::UNKNOWN();

// A localized handle on the adapter, which allows a
// decoupling between the sample system and adapter.
static SampleAdapter * Adapter;

// An empty argument vector.
static vector<Any> EmptyArgs;


///////////////////////////// State support //////////////////////////////////

// Queries the system for the value of a state and its arguments.
//
static Any fetch (const string& state_name, const vector<Any>& args)
{
  debugMsg("SampleAdapter:fetch",
           "Fetch called on " << state_name << " with " << args.size() << " args");
  Any retval;

  // Crufty, will try to generalize later...
  if (state_name == "Size") retval = encodeReal (getSize());
  else if (state_name == "Speed") retval = encodeInt (getSpeed());
  else if (state_name == "Color") retval = encodeString (getColor());
  else if (state_name == "at") {
    switch (args.size()) {
    case 0: retval = encodeString (at ()); break;
    case 1: retval = encodeBool (at (decodeString (args[0]))); break;
    case 2: {
      retval = encodeBool (at (decodeInt (args[0]), decodeInt (args[1])));
      break;
    }
    default: {
      cerr << error << "invalid lookup of 'at'" << endl;
      retval = Unknown;
    }
    }
  }
  else {
    cerr << error << "invalid state: " << state_name << endl;
    retval = Unknown;
  }
  debugMsg("SampleAdapter:fetch",
           "Fetch returning " << PLEXIL::Expression::valueToString (retval));
  return retval;
}


// The 'receive' functions are the subscribers for system state updates.  They
// receive the name of the state whose value has changed in the system.  Then
// they propagate the state's new value to the executive.

static void propagate (const State& state, const vector<Any>& value)
{
  Adapter->propagateValueChange (state, value);
}

static void receive (const string& state_name, int val)
{
  propagate (State (LabelStr (state_name), EmptyArgs),
             vector<Int> (1, encodeInt (val)));
}

static void receive (const string& state_name, float val)
{
  propagate (State (LabelStr (state_name), EmptyArgs),
             vector<Real> (1, encodeReal (val)));
}

static void receive (const string& state_name, const string& val)
{
  propagate (State (LabelStr (state_name), EmptyArgs),
             vector<String> (1, encodeString (val)));
}

static void receive (const string& state_name, bool val, const string& arg)
{
  State state (LabelStr (state_name), vector<String> (encodeString (arg)));
  propagate (state, vector<Bool> (1, encodeBool (val)));
}

static void receive (const string& state_name, bool val, int arg1, int arg2)
{
  vector<Int> vec;
  vec.push_back (encodeInt (arg1));
  vec.push_back (encodeInt (arg2));
  State state (LabelStr (state_name), vec);
  propagate (state, vector<Bool> (1, encodeBool (val)));
}


///////////////////////////// Member functions //////////////////////////////////


SampleAdapter::SampleAdapter(PLEXIL::AdapterExecInterface& execInterface,
                     const TiXmlElement*& configXml) :
    InterfaceAdapter(execInterface, configXml)
{
  debugMsg("SampleAdapter", " created.");
}

bool SampleAdapter::initialize()
{
  m_execInterface.defaultRegisterAdapter(getId());
  Adapter = this;
  setSubscriberInt (receive);
  setSubscriberReal (receive);
  setSubscriberString (receive);
  setSubscriberBoolString (receive);
  setSubscriberBoolIntInt (receive);
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


// Sends a command (as invoked in a Plexil command node) to the system and sends
// the status, and return value if applicable, back to the executive.
//
void SampleAdapter::executeCommand (const LabelStr& command_name,
                                    const list<Any>& args,
                                    PLEXIL::ExpressionId dest,
                                    PLEXIL::ExpressionId ack) 
{
  string name = command_name.toString();
  debugMsg("SampleAdapter", "Received executeCommand for " << name);  

  // Warning!  This is a crufty function, handling each known
  // command signature individually.

  Any retval = Unknown;
  vector<Any> argv(10);
  copy (args.begin(), args.end(), argv.begin());

  if (name == "SetSize") setSize (decodeReal (*args.begin()));
  else if (name == "SetSpeed") setSpeed (decodeInt (*args.begin()));
  else if (name == "SetColor") setColor (decodeString (*args.begin()));
  else if (name == "Move") move (decodeString (argv[0]),
                                 decodeInt (argv[1]),
                                 decodeInt (argv[2]));
  else if (name == "Hello") hello ();
  else if (name == "Square") retval = square (decodeInt (*args.begin()));
  else cerr << error << "invalid command: " << name << endl;

  // This sends a command handle back to the executive.
  m_execInterface.handleValueChange
    (ack, PLEXIL::CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());

  // This sends the command's return value (if expected) to the executive.
  if (dest != PLEXIL::ExpressionId::noId()) {
    m_execInterface.handleValueChange (dest, retval);
  }

  m_execInterface.notifyOfExternalEvent();
}


double SampleAdapter::lookupNow (const State& state)
{
  // This is the name of the state as given in the plan's LookupNow
  LabelStr name (state.first);
  const vector<Any>& args = state.second;
  return fetch(name.toString(), args);
}


void SampleAdapter::subscribe(const State& state)
{
  LabelStr nameLabel = LabelStr (state.first);
  debugMsg("SampleAdapter:subscribe", " processing state "
           << nameLabel.toString());
  m_subscribedStates.insert(state);
}


void SampleAdapter::unsubscribe (const State& state)
{
  LabelStr nameLabel = LabelStr (state.first);
  debugMsg("SampleAdapter:subscribe", " from state "
           << nameLabel.toString());
  m_subscribedStates.erase(state);
}

// Does nothing.
void SampleAdapter::setThresholds (const State& state, double hi, double lo)
{
}


void SampleAdapter::propagateValueChange (const State& state,
                                          const vector<Any>& vals) const
{
  if (!isStateSubscribed(state))
	return; 
  m_execInterface.handleValueChange (state, vals.front());
  m_execInterface.notifyOfExternalEvent();
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
