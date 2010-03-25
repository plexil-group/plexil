/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

#include "ExternalInterface.hh"
#include "PlexilExec.hh"

namespace PLEXIL {

  ExternalInterface::ExternalInterface() : m_id(this) 
  {
  }

  ExternalInterface::~ExternalInterface() 
  {
    m_id.remove();
  }

  void ExternalInterface::setExec(const PlexilExecId exec)
  {
    m_exec = exec;
    m_exec->setExternalInterface(m_id);
  }
  
  void ExternalInterface::registerChangeLookup(const LookupKey& source, const State& state, const StateKey& key, const std::vector<double>& tolerances, 
					       std::vector<double>& dest) {}
  void ExternalInterface::registerChangeLookup(const LookupKey& source, const StateKey& key, const std::vector<double>& tolerances) {}

  void ExternalInterface::registerFrequencyLookup(const LookupKey& source, const State& state, const StateKey& key, const double& lowFreq, const double& highFreq,
						  std::vector<double>& dest) {}
  void ExternalInterface::registerFrequencyLookup(const LookupKey& source, const StateKey& key, const double& lowFreq, const double& highFreq) {}

  void ExternalInterface::lookupNow(const State& state, const StateKey& key, std::vector<double>& dest) {}
  void ExternalInterface::lookupNow(const StateKey& key, std::vector<double>& dest) {}
  
  void ExternalInterface::unregisterChangeLookup(const LookupKey& dest) {}
  void ExternalInterface::unregisterFrequencyLookup(const LookupKey& dest) {}

  void ExternalInterface::batchActions(std::list<CommandId>& commands){}
  void ExternalInterface::batchActions(std::list<FunctionCallId>& functionCalls){}
  void ExternalInterface::updatePlanner(std::list<UpdateId>& updates){}
  
  void ExternalInterface::executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack){}

  void ExternalInterface::executeFunctionCalls(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack){}
  
  void ExternalInterface::invokeAbort(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack){}

  void ExternalInterface::addPlan(const TiXmlElement& plan, const LabelStr& parent)
    throw(ParserException)
  {}

  void ExternalInterface::addPlan(PlexilNode* plan, const LabelStr& parent) {}

  double ExternalInterface::currentTime() {return 0.0;}
}
