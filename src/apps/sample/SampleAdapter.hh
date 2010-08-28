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

// This file defines an simple interface adapter for the example Plexil
// application in this directory.  See src/app-framework/InterfaceAdaptor.hh for
// brief documentation of the inherited class members.  See the implementation
// (.cc version) of this file for details on how this adapter works.

#ifndef _H__SampleAdapter
#define _H__SampleAdapter

#include "InterfaceAdapter.hh"
#include "types.hh"

class SampleAdapter : public PLEXIL::InterfaceAdapter
{
public:
  SampleAdapter (PLEXIL::AdapterExecInterface&, const TiXmlElement*&);

  bool initialize();
  bool start();
  bool stop();
  bool reset();
  bool shutdown();

  void executeCommand (const PLEXIL::LabelStr& name,
                       const std::list<double>& args,
                       PLEXIL::ExpressionId dest,
                       PLEXIL::ExpressionId ack);

  void lookupNow (const PLEXIL::StateKey&, std::vector<double>& dest);

  void registerChangeLookup(const PLEXIL::LookupKey& /* uniqueId */,
                            const PLEXIL::StateKey& stateKey,
                            const std::vector<double>& /* tolerances */);

  void unregisterChangeLookup(const PLEXIL::LookupKey& uniqueId);

  // The following member, not inherited from the base class, propagates a state
  // value change from the system to the executive.
  //
  void propagateValueChange (const PLEXIL::StateKey& key,
                             const std::vector<Any>& vals) const;
};

extern "C" {
  void initSampleAdapter();
}

#endif
