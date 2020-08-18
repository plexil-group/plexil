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

// This interface adapter provides the following useful utilities for PLEXIL plans:
//   Command: print(exp1 exp2 ...)  - prints arguments to standard output
//   Command: pprint(exp1 exp2 ...) - "pretty print", as above but separates 
//                                    items with whitespace and adds newline
// This adapter is accessed by including the following entry in your interface
// configuration file:    <Adapter AdapterType="Utility"/>

#ifndef PLEXIL_UTILITY_ADAPTER_HH
#define PLEXIL_UTILITY_ADAPTER_HH

#include "InterfaceAdapter.hh"
#include "AdapterConfiguration.hh"

namespace PLEXIL {

class UtilityAdapter : public InterfaceAdapter
{
  typedef void (InterfaceAdapter::*LookupNowHandler)(const State &, StateCacheEntry&);
  typedef void (InterfaceAdapter::*SetThresholdsDoubleHandler)(const State &, double, double);
  typedef void (InterfaceAdapter::*SetThresholdsIntHandler)(const State &, int, int);
  typedef void (InterfaceAdapter::*SubscribeHandler)(const State &);
  typedef void (InterfaceAdapter::*UnsubscribeHandler)(const State &);
  
  class UtilityLookupHandler : public AbstractLookupHandler {
    InterfaceAdapter &m_context;
    LookupNowHandler m_lookupNowHandler;
    SetThresholdsDoubleHandler m_setThresholdsDoubleHandler;
    SetThresholdsIntHandler m_setThresholdsIntHandler;
    SubscribeHandler m_subscribeHandler;
    UnsubscribeHandler m_unsubscribeHandler;
  public:
    UtilityLookupHandler(InterfaceAdapter &ctx, LookupNowHandler ln, SetThresholdsDoubleHandler setTD = nullptr, 
        SetThresholdsIntHandler setTI = nullptr, SubscribeHandler sub = nullptr,
        UnsubscribeHandler unsub = nullptr) : m_context(ctx),
        m_lookupNowHandler(ln), m_setThresholdsDoubleHandler(setTD),
        m_setThresholdsIntHandler(setTI), m_subscribeHandler(sub), m_unsubscribeHandler(unsub) {}
    virtual void lookupNow(const State &state, StateCacheEntry &cacheEntry) {
      (m_context.*m_lookupNowHandler)(state, cacheEntry);
    }
    void setThresholds(const State &state, double hi, double lo) {
      if(m_setThresholdsDoubleHandler)
        (m_context.*m_setThresholdsDoubleHandler)(state, hi, lo);
    }
    void setThresholds(const State &state, int32_t hi, int32_t lo) {
      if(m_setThresholdsIntHandler)
        (m_context.*m_setThresholdsIntHandler)(state, hi, lo);
    }
    void subscribe(const State &state) {
      if(m_subscribeHandler)
        (m_context.*m_subscribeHandler)(state);
    }
    void unsubscribe(const State &state) {
      if(m_unsubscribeHandler)
        (m_context.*m_unsubscribeHandler)(state);
    }
  };

  typedef void (InterfaceAdapter::*ExecuteCommandHandler)(Command *);
  typedef void (InterfaceAdapter::*AbortCommandHandler)(Command *);

  class UtilityCommandHandler : public AbstractCommandHandler {
    InterfaceAdapter &m_context;
    ExecuteCommandHandler m_executeCommandHandler;
    AbortCommandHandler m_abortCommandHandler;
  public:
    UtilityCommandHandler(InterfaceAdapter &ctx, ExecuteCommandHandler exec, AbortCommandHandler abort = nullptr) :
      m_context(ctx), m_executeCommandHandler(exec), m_abortCommandHandler(abort) {}
    virtual void executeCommand(Command *cmd) {
      (m_context.*m_executeCommandHandler)(cmd);
    }
    void abortCommand(Command *cmd) {
      if(m_abortCommandHandler)
        (m_context.*m_abortCommandHandler)(cmd);
    }
  };
public:
  UtilityAdapter (AdapterExecInterface&, pugi::xml_node const);

  virtual bool initialize();
  virtual bool start();
  virtual bool stop();
  virtual bool reset();
  virtual bool shutdown();

  void print1(Command *cmd);

  void pprint1(Command *cmd);

  void printToString1(Command *cmd);

  void pprintToString1(Command *cmd);

  void abortCommand(Command *cmd);

};

extern "C" {
  void initUtilityAdapter();
}

} // namespace PLEXIL

#endif
