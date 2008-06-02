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

#include "InterfaceAdaptor.hh"
#include "AdaptorExecInterface.hh"
#include "ExecDefs.hh"
#include "Expression.hh"
#include "LabelStr.hh"
#include "StateCache.hh"
#include "Debug.hh"
#include "Error.hh"

namespace PLEXIL
{
  InterfaceAdaptor::InterfaceAdaptor()
    : m_id(this)
  {
  }

  InterfaceAdaptor::~InterfaceAdaptor()
  {
    m_id.remove();
  }

  void InterfaceAdaptor::registerChangeLookup(const LookupKey& uniqueId,
					      const StateKey& stateKey,
					      const std::vector<double>& tolerances)
  {
    checkError(ALWAYS_FAIL,
	       "InterfaceAdaptor::registerChangeLookup: default method called!");
  }

  void InterfaceAdaptor::unregisterChangeLookup(const LookupKey& uniqueId)
  {
    checkError(ALWAYS_FAIL,
	       "InterfaceAdaptor::unregisterChangeLookup: default method called!");
  }

  void InterfaceAdaptor::registerFrequencyLookup(const LookupKey& uniqueId,
						 const StateKey& stateKey,
						 double lowFrequency, 
						 double highFrequency)
  {
    checkError(ALWAYS_FAIL,
	       "InterfaceAdaptor::registerFrequencyLookup: default method called!");
  }


  void InterfaceAdaptor::unregisterFrequencyLookup(const LookupKey& uniqueId)
  {
    checkError(ALWAYS_FAIL,
	       "InterfaceAdaptor::registerFrequencyLookup: default method called!");
  }

  void InterfaceAdaptor::lookupNow(const StateKey& key,
				   std::vector<double>& dest)
  {
    checkError(ALWAYS_FAIL,
	       "InterfaceAdaptor::lookupNow: default method called!");
  }

  void InterfaceAdaptor::sendPlannerUpdate(const NodeId& node,
					   const std::map<double, double>& valuePairs,
					   const ExpressionId ack)
  {
    checkError(ALWAYS_FAIL,
	       "InterfaceAdaptor::updatePlanner: default method called!");
  }


  // executes a command with the given arguments
  void InterfaceAdaptor::executeCommand(const LabelStr& name,
					const std::list<double>& args,
					ExpressionId dest,
					ExpressionId ack)
  {
    checkError(ALWAYS_FAIL,
	       "InterfaceAdaptor::executeCommand: default method called!");
  }


  // executes a function call with the given arguments
  void InterfaceAdaptor::executeFunctionCall(const LabelStr& name,
					     const std::list<double>& args,
					     ExpressionId dest,
					     ExpressionId ack)
  {
    checkError(ALWAYS_FAIL,
	       "InterfaceAdaptor::executeCommand: default method called!");
  }


  //abort the given command with the given arguments.  store the abort-complete into dest
  void InterfaceAdaptor::invokeAbort(const LabelStr& name, 
				     const std::list<double>& args, 
				     ExpressionId dest)
  {
    checkError(ALWAYS_FAIL,
	       "InterfaceAdaptor::invokeAbort: default method called!");
  }

  //
  // Methods to facilitate implementations
  //

  void
  InterfaceAdaptor::registerAsynchLookup(const LookupKey& uniqueId,
					 const StateKey& key)
  {
    std::map<StateKey, std::set<LookupKey> >::iterator it =
      m_asynchLookups.find(key);
    if (it == m_asynchLookups.end())
      {
	debugMsg("InterfaceAdaptor:registerAsynchLookup", " adding new state with key " << key);
	std::set<LookupKey> theSet;
	theSet.insert(uniqueId);
	m_asynchLookups.insert(std::pair<StateKey, std::set<LookupKey> >(key, theSet));
      }
    else
      {
	debugMsg("InterfaceAdaptor:registerAsynchLookup", " adding new destination for old state");
	(*it).second.insert(uniqueId);
      }
  }

  void
  InterfaceAdaptor::unregisterAsynchLookup(const LookupKey& uniqueId)
  {
    debugMsg("InterfaceAdaptor:unregisterAsynchLookup",
	     " for unique ID '" << uniqueId);
    std::map<StateKey, std::set<LookupKey> >::iterator tableIt =
      m_asynchLookups.begin();
    for (; tableIt != m_asynchLookups.end(); tableIt++)
      {
	std::set<LookupKey> & theSet = tableIt->second;
	std::set<LookupKey>::iterator setIt = theSet.find(uniqueId);
	if (setIt != theSet.end())
	  {
	    // Found it -- is it the only member?
	    if (theSet.size() == 1)

	      {
		// delete entry from table
		debugMsg("InterfaceAdaptor:unregisterAsynchLookup",
			 " deleting last lookup for state " << tableIt->first);
		m_asynchLookups.erase(tableIt);
	      }
	    else
	      {
		// delete unique ID from entry
		debugMsg("InterfaceAdaptor:unregisterAsynchLookup",
			 " deleting lookup for state with remaining lookups");
		theSet.erase(setIt);
	      }
	    return; // done in either case
	  }
      }
    // Warn, don't barf.
    debugMsg("InterfaceAdaptor:unregisterAsynchLookup",
	     " Unique ID '" << uniqueId << "' not found.");
  }
    
  std::map<StateKey, std::set<LookupKey> >::const_iterator 
  InterfaceAdaptor::getAsynchLookupsBegin()
  {
    return m_asynchLookups.begin();
  }
  
  std::map<StateKey, std::set<LookupKey> >::const_iterator
  InterfaceAdaptor::getAsynchLookupsEnd()
  {
    return m_asynchLookups.end();
  }

  bool 
  InterfaceAdaptor::getState(const StateKey& key, State& state)
  {
    return AdaptorExecInterface::instance()->getStateCache()->stateForKey(key, state);
  }

  bool
  InterfaceAdaptor::getStateKey(const State& state, StateKey& key)
  {
    return !AdaptorExecInterface::instance()->getStateCache()->keyForState(state, key);
  }

}
