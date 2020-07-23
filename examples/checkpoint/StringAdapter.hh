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

// This file defines an Adapter which provides conversions between
// Strings and Integers,Reals, and Booleans

// It also provides the c string manipulation functions substr, strlen, strlwr, and strupr
// with the same behavior as in c


#ifndef _H__StringAdapter
#define _H__StringAdapter

#include "Command.hh"
#include "Value.hh"
#include "InterfaceAdapter.hh"

using namespace PLEXIL;

class StringAdapter : public InterfaceAdapter
{
public:
  static StringAdapter* getInstance() {
    return m_adapter;
  }
  
  StringAdapter (AdapterExecInterface&, const pugi::xml_node&);
  ~StringAdapter();
  
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
  
  static StringAdapter * m_adapter;
  std::set<State> m_subscribedStates;

};

std::string getChildWithAttribute(const pugi::xml_node& configXml,
	    const std::string& node_name,
	    const std::string& attribute_name);

extern "C" {
  void initCheckpointAdapter();
}

#endif
