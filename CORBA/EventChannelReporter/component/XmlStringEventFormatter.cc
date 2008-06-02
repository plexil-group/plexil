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


#include "XmlStringEventFormatter.hh"
#include "Node.hh"
#include "CoreExpressions.hh"
#include "ThreadedExternalInterface.hh"
#include "Debug.hh"
#include "PlexilXmlParser.hh"
#include "event-support.hh"

#include <iomanip>
#include <cmath>
#include <sstream>

#include "tinyxml.h"

namespace PLEXIL
{

  XmlStringEventFormatter::XmlStringEventFormatter()
    : EventFormatter()
  {
  }

  XmlStringEventFormatter::~XmlStringEventFormatter()
  {
  }

  CORBA::Any_ptr 
  XmlStringEventFormatter::formatTransition(const LabelStr& prevState,
					    const NodeId& node) const
  {
    CORBA::Any_ptr pushAny = new CORBA::Any();
    const std::string xmlString = transitionXmlString(prevState, node);
    debugMsg("ExecListener:pushTransitionToChannel",
	     " data being pushed to event channel: "
	     << xmlString.c_str());
    (*pushAny) <<= xmlString.c_str();
    return pushAny;
  }

  CORBA::Any_ptr 
  XmlStringEventFormatter::formatPlan(const PlexilNodeId& plan,
				      const LabelStr& parent) const
  {
    const std::string xmlString = planXmlString(plan, parent);
    debugMsg("ExecListener:pushAddPlanToChannel",
	     " data being pushed to event channel: "
	     << xmlString.c_str());
    CORBA::Any_ptr pushAny = new CORBA::Any();
    (*pushAny) <<= xmlString.c_str();
    return pushAny;
  }


  
  //
  // Formatting code begins here
  // Thanks to Mike Dalal for the roots of this code
  //

  const std::string binding_element (const std::map<double, double>& bindings)
  {
    //
    // Returns an XML element for a given set of variable bindings.
    //
    std::ostringstream s;
    for (std::map<double,double>::const_iterator i = bindings.begin();
         i != bindings.end();
         i++) {
      s << element ("binding",
                    element ("bindingName", (LabelStr ((*i).first)).toString()),
                    element ("bindingValue", to_string<double>((*i).second)));
    }
    return s.str();
  }

  // Static helper method
  const std::string 
  XmlStringEventFormatter::transitionXmlString(const LabelStr& prevState,
					       const NodeId& node)
  {
    // Output as integer for now
    std::string timestamp = 
      to_string<long>
      ((long) floor((node->findVariable (node->getState().toString() + ".START"))->getValue()));

    NodeId parent = node->getParent();
    const LabelStr& nodeState = node->getState();
    
    std::string body =
      element ("sender", "UnivExec") +
      element ("nodeId", node->getNodeId().toString()) +
			element ("parentId",
				 parent ? parent->getNodeId().toString() : "0") +
      element ("nodeType", node->getType().toString()) +
      element ("executionStatus", nodeState.toString()) +
      element ("primitive", ((node->getType() == Node::LIST()) ? "F" : "T"));

    bool nodeFinished = (StateVariable::FINISHED() == nodeState);
    bool nodeExecuting = (StateVariable::EXECUTING() == nodeState);
    if (nodeFinished)
      body = body + element ("resultStatus", node->getOutcome().toString());

    // Additional elements for Command nodes transitioned to executing
    if ((node->getType() == Node::COMMAND()) &&
	(nodeFinished || nodeExecuting))
      {
	CommandId cmd = node->getCommand();
	checkError(!cmd.isNoId(),
		   "transitionXmlString: command is null!");
	// format parameters
	std::string paramStr;
	for (std::list<double>::const_iterator it = cmd->getArgValues().begin();
	     it != cmd->getArgValues().end();
	     it++)
	  {
	    paramStr +=
	      element("commandArgument",
		      ThreadedExternalInterface::valueToString(*it));
	  }

	body = body + element("command",
			      element("commandName", cmd->getName().toString()),
			      paramStr);
      }

    // Additional elements for Update nodes
    if ((node->getType() == Node::UPDATE()) && 
	(nodeFinished || nodeExecuting))
      {
	const std::map<double, double>& bindings = node->getUpdate()->getPairs();
	body =
	  body +
	  element ("bindingDimension", to_string<int> (bindings.size())) +
	  element ("bindings", binding_element (bindings));
      }

    const std::string el =
      element ("Event",
	       element ("EventId", event_id()),
	       element ("TimeStamp", timestamp),
	       element ("body", body));

    return el;
  }

  // static helper method
  const std::string
  XmlStringEventFormatter::planXmlString(const PlexilNodeId& plan, const LabelStr& parent)
  {
    std::ostringstream planStream;
    {
      const TiXmlElement* xmlPlan = PlexilXmlParser::toXml(plan);
      planStream << *xmlPlan;
      delete xmlPlan;
    }
    const std::string el =
      element("AddPlan",
	      element("EventId", event_id()),
	      element("ParentNode", parent.toString()),
	      element("PlanBody", planStream.str()));
    return el;
  }

}
