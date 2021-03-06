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

// This file defines an interface to set/lookup checkpoints which are persistent
// between crashes. It also provides information about crashes

// See src/app-framework/InterfaceAdapter.hh for
// brief documentation of the inherited class members.  See the implementation
// (.cc version) of this file for details on how this adapter works.


#ifndef _H__CheckpointAdapter
#define _H__CheckpointAdapter

#include "Subscriber.hh"

#include "InterfaceAdapter.hh"
#include "Value.hh"

#include <set>

namespace PLEXIL
{
  class State;
}

class CheckpointAdapter : public PLEXIL::InterfaceAdapter, public Subscriber
{
public:  
  CheckpointAdapter (PLEXIL::AdapterExecInterface&, PLEXIL::AdapterConf*);
  virtual ~CheckpointAdapter() = default;

  //
  // InterfaceAdapter public API
  //

  virtual bool initialize(PLEXIL::AdapterConfiguration *);
  virtual bool start();
  virtual void stop();

  //
  // Subscriber public API
  //

  void receiveValue (const std::string& state_name,
		     const PLEXIL::Value& val);
  
  void receiveValue (const std::string& state_name,
		     const PLEXIL::Value& val,
		     const PLEXIL::Value& arg);
  
  void receiveValue (const std::string& state_name,
		     const PLEXIL::Value& val,
		     const PLEXIL::Value& arg1,
		     const PLEXIL::Value& arg2);

  void receiveCommandReceived(PLEXIL::Command* cmd);
  void receiveCommandSuccess(PLEXIL::Command* cmd);

private:

  CheckpointAdapter() = delete;
  CheckpointAdapter(const CheckpointAdapter&) = delete;
  CheckpointAdapter(CheckpointAdapter&&) = delete;
  CheckpointAdapter & operator=(const CheckpointAdapter&) = delete;
  CheckpointAdapter & operator=(CheckpointAdapter&&) = delete;
  
  // The following member, not inherited from the base class, propagates a state
  // value change from the system to the executive.
  // Passes value onto executive, which makes no guarantees about non-modification so can't be const reference
  void propagateValueChange (const PLEXIL::State& state,
                             const std::vector<PLEXIL::Value>& vals);

  std::set<PLEXIL::State> m_subscribedStates;
  bool m_ok_on_exit;
  bool m_flush_on_exit;
  bool m_flush_on_start;
};

extern "C" {
  void initCheckpointAdapter();
}

#endif
