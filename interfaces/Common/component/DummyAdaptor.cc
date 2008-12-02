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

#include "DummyAdaptor.hh"

#include "Debug.hh"
#include "CoreExpressions.hh"
#include "AdaptorExecInterface.hh"
#include "Node.hh"

namespace PLEXIL
{
  DummyAdaptor::DummyAdaptor()
    : InterfaceAdaptor()
  {
  }

  DummyAdaptor::~DummyAdaptor()
  {
  }

  void DummyAdaptor::registerChangeLookup(const LookupKey& uniqueId,
					 const StateKey& stateKey,
					 const std::vector<double>& tolerances)
  {
    debugMsg("ExternalInterface:dummy", " registerChangeLookup called");
  }

  void DummyAdaptor::unregisterChangeLookup(const LookupKey& uniqueId)
  {
    debugMsg("ExternalInterface:dummy", " unregisterChangeLookup called");
  }

  void DummyAdaptor::registerFrequencyLookup(const LookupKey& uniqueId,
					    const StateKey& stateKey,
					    double lowFrequency, 
					    double highFrequency)
  {
    debugMsg("ExternalInterface:dummy", " registerFrequencyLookup called");
  }

  void DummyAdaptor::unregisterFrequencyLookup(const LookupKey& uniqueId)
  {
    debugMsg("ExternalInterface:dummy", " unregisterFrequencyLookup called");
  }

  void DummyAdaptor::lookupNow(const StateKey& key,
			      std::vector<double>& dest)
  {
    debugMsg("ExternalInterface:dummy", " lookupNow called; returning UNKNOWN");
    dest[0] = Expression::UNKNOWN();
  }

  void DummyAdaptor::sendPlannerUpdate(const NodeId& node,
				      const std::map<double, double>& valuePairs,
				      ExpressionId ack)
  {
    debugMsg("ExternalInterface:dummy", " sendPlannerUpdate called");

	// acknowledge updates
    debugMsg("ExternalInterface:dummy",
	     " faking acknowledgment of update node '"
	     << node->getNodeId().toString()
	     << "'");
    AdaptorExecInterface::instance()->handleValueChange(ack,
							BooleanVariable::TRUE());
    AdaptorExecInterface::instance()->notifyOfExternalEvent();
  }

  void DummyAdaptor::executeCommand(const LabelStr& name,
				   const std::list<double>& args,
				   ExpressionId dest,
				   ExpressionId ack)
  {
    debugMsg("ExternalInterface:dummy", " executeCommand called");
    AdaptorExecInterface::instance()->handleValueChange(ack,
							CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
    AdaptorExecInterface::instance()->notifyOfExternalEvent();
  }

  void DummyAdaptor::executeFunctionCall(const LabelStr& name,
					 const std::list<double>& args,
					 ExpressionId dest,
					 ExpressionId ack)
  {
    debugMsg("ExternalInterface:dummy", " executeFunctionCall called");
    AdaptorExecInterface::instance()->handleValueChange(ack,
							BooleanVariable::TRUE());
    AdaptorExecInterface::instance()->notifyOfExternalEvent();
  }

  //abort the given command with the given arguments.  store the abort-complete into dest
  void DummyAdaptor::invokeAbort(const LabelStr& name, 
				 const std::list<double>& args, 
				 ExpressionId dest)
  {
    debugMsg("ExternalInterface:dummy", " invokeAbort called");
    AdaptorExecInterface::instance()->handleValueChange(dest,
							BooleanVariable::TRUE());
    AdaptorExecInterface::instance()->notifyOfExternalEvent();
  }

}
