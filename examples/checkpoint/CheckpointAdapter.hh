/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

// See src/app-framework/InterfaceAdaptor.hh for
// brief documentation of the inherited class members.  See the implementation
// (.cc version) of this file for details on how this adapter works.


#ifndef _H__CheckpointAdapter
#define _H__CheckpointAdapter

#include "Command.hh"
#include "CheckpointAdapter.hh"
#include "Value.hh"

using namespace PLEXIL;

class CheckpointAdapter : public InterfaceAdapter
{
public:
  static CheckpointAdapter* getInstance() {
    return m_adapter;
  }
  
  CheckpointAdapter (AdapterExecInterface&, const pugi::xml_node&);
  ~CheckpointAdapter();
  bool initialize();
  bool start();
  bool stop();
  bool reset();
  bool shutdown();

  virtual void lookupNow (State const &state, StateCacheEntry &cacheEntry);
  virtual void subscribe(const State& state);
  virtual void unsubscribe(const State& state);
  virtual void setThresholds(const State& state, double hi, double lo);
  virtual void setThresholds(const State& state, int32_t hi, int32_t lo);
  virtual void executeCommand(Command *cmd);

  // The following member, not inherited from the base class, propagates a state
  // value change from the system to the executive.
  //
  void propagateValueChange (const State& state,
                             const std::vector<Value>& vals) const;
  
  void propagate (const State& state, const std::vector<Value>& value);

private:
  bool isStateSubscribed(const State& state) const;

  static CheckpointAdapter * m_adapter;
  std::set<State> m_subscribedStates;
  InterfaceAdapter m_timeAdapter;

};

extern "C" {
  void initCheckpointAdapter();
}

#endif
