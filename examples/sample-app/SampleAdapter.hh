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

// This file defines an simple interface adapter for the example Plexil
// application in this directory.  See src/app-framework/InterfaceAdaptor.hh for
// brief documentation of the inherited class members.  See the implementation
// (.cc version) of this file for details on how this adapter works.

#ifndef _H__SampleAdapter
#define _H__SampleAdapter

#include "Command.hh"
#include "InterfaceAdapter.hh"
#include "Value.hh"

#include <set>

class SampleAdapter : public PLEXIL::InterfaceAdapter
{
public:

  SampleAdapter (PLEXIL::AdapterExecInterface&, const pugi::xml_node&);

  virtual bool initialize(PLEXIL::AdapterConfiguration *config);
  virtual bool start();
  virtual bool stop();
  virtual bool reset();
  virtual bool shutdown();

  // Command Handlers
  static void setSize(PLEXIL::Command *cmd, PLEXIL::AdapterExecInterface *intf);
  static void setSpeed(PLEXIL::Command *cmd, PLEXIL::AdapterExecInterface *intf);
  static void setColor(PLEXIL::Command *cmd, PLEXIL::AdapterExecInterface *intf);
  static void setName(PLEXIL::Command *cmd, PLEXIL::AdapterExecInterface *intf);
  static void move(PLEXIL::Command *cmd, PLEXIL::AdapterExecInterface *intf);
  static void hello(PLEXIL::Command *cmd, PLEXIL::AdapterExecInterface *intf);
  static void square(PLEXIL::Command *cmd, PLEXIL::AdapterExecInterface *intf);
  static void cube(PLEXIL::Command *cmd, PLEXIL::AdapterExecInterface *intf);
  static void defaultHandler(PLEXIL::Command *cmd, PLEXIL::AdapterExecInterface *intf);

  // Lookup handlers
  static void getSize (const PLEXIL::State& state, PLEXIL::StateCacheEntry &entry);
  static void getSpeed (const PLEXIL::State& state, PLEXIL::StateCacheEntry &entry);
  static void getColor (const PLEXIL::State& state, PLEXIL::StateCacheEntry &entry);
  static void getSystemName (const PLEXIL::State& state, PLEXIL::StateCacheEntry &entry);
  static void getAt (const PLEXIL::State& state, PLEXIL::StateCacheEntry &entry);
  static void getDefault (const PLEXIL::State& state, PLEXIL::StateCacheEntry &entry);

  static void subscribeToState(const PLEXIL::State& state, PLEXIL::AdapterExecInterface *intf);
  static void unsubscribeFromState(const PLEXIL::State& state);
  static void setStateThresholds(const PLEXIL::State& state, double hi, double lo);
  static void setStateThresholds(const PLEXIL::State& state, int32_t hi, int32_t lo);

  // The following member, not inherited from the base class, propagates a state
  // value change from the system to the executive.
  //

  void propagateValueChange (const PLEXIL::State& state,
                             const std::vector<PLEXIL::Value>& vals) const;

  void propagate (const PLEXIL::State& state, const std::vector<PLEXIL::Value>& value);

  void receiveValue (const std::string& state_name, PLEXIL::Value val);
  void receiveValue (const std::string& state_name, PLEXIL::Value val, PLEXIL::Value arg);
  void receiveValue (const std::string& state_name, PLEXIL::Value val, PLEXIL::Value arg1, PLEXIL::Value arg2);

private:
  // Deliberately private, delete is not C++98 compatible
  SampleAdapter();
  
  bool isStateSubscribed(const PLEXIL::State& state) const;
  static std::set<PLEXIL::State> m_subscribedStates;
};

extern "C" {
  void initSampleAdapter();
}

#endif
