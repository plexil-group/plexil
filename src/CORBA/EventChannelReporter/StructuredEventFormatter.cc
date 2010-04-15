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

#include "StructuredEventFormatter.hh"
#include "InterfaceManagerBase.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "PlexilXmlParser.hh"

#include <cmath> // for floor()

namespace PLEXIL
{

  StructuredEventFormatter::StructuredEventFormatter(const TiXmlElement* xml, InterfaceManagerBase& mgr)
    : EventFormatter(xml, mgr),
      m_eventDomainName("_no_domain_name_")
  {
  }

  StructuredEventFormatter::StructuredEventFormatter(const TiXmlElement* xml, 
						     InterfaceManagerBase& mgr, 
						     const std::string & domainName)
    : EventFormatter(xml, mgr),
      m_eventDomainName(domainName)
  {
  }

  StructuredEventFormatter::~StructuredEventFormatter()
  {
  }

  // Private helper fn
  const int eventId()
  {
    static int id = 0;
    return ++id;
  }

  CosNotification::StructuredEvent *
  StructuredEventFormatter::formatTransitionStructured(const LabelStr& prevState,
						       const NodeId& node) const
  {
    CosNotification::StructuredEvent * pushEvent
      = new CosNotification::StructuredEvent();

    // event header
    pushEvent->header.fixed_header.event_type.domain_name = m_eventDomainName.c_str();
    pushEvent->header.fixed_header.event_type.type_name = "ue.nodeStatus";

    pushEvent->header.variable_header.length(2);
    pushEvent->header.variable_header[0].name = "eventId";
    pushEvent->header.variable_header[0].value <<= eventId();
    pushEvent->header.variable_header[1].name = "timestamp";
    // format as integer for now
    pushEvent->header.variable_header[1].value <<= 
      (CORBA::Long) floor(node->findVariable(node->getState().toString() + ".START")->getValue());

    const LabelStr& nodeState = node->getState();
    bool nodeFinished = (StateVariable::FINISHED() == nodeState);
    bool nodeExecuting = (StateVariable::EXECUTING() == nodeState);

    // Estimate buffer space needed
    int dataLength = 5;
    // outcome if node finished
    if (nodeFinished)
      dataLength++;
    // command if appropriate
    if ((node->getType() == Node::COMMAND()) &&
	(nodeFinished || nodeExecuting))
      dataLength += 2 + node->getCommand()->getArgValues().size();
    // planner update if appropriate
    if ((node->getType() == Node::UPDATE()) && 
	(nodeFinished || nodeExecuting))
      dataLength += 1 + 2 * node->getUpdate()->getPairs().size();

    debugMsg("ExecListener:formatTransition",
	     " expecting " << dataLength << " name/value pairs");

    int i = 0;
    NodeId parent = node->getParent();
      
    pushEvent->filterable_data.length(dataLength);
    pushEvent->filterable_data[i].name = "nodeId";
    pushEvent->filterable_data[i++].value <<= node->getNodeId().toString().c_str();
    pushEvent->filterable_data[i].name = "parentId";
    pushEvent->filterable_data[i++].value <<= 
      (parent.isNoId() ? "0" : parent->getNodeId().toString().c_str());
    pushEvent->filterable_data[i].name = "nodeType";
    pushEvent->filterable_data[i++].value <<= node->getType().toString().c_str();
    pushEvent->filterable_data[i].name = "executionStatus";
    pushEvent->filterable_data[i++].value <<= nodeState.toString().c_str();
    pushEvent->filterable_data[i].name = "primitive";
    pushEvent->filterable_data[i++].value <<= 
      (node->getType() == Node::LIST()) ? "F" : "T";

    if (nodeFinished)
      {
	pushEvent->filterable_data[i].name = "resultStatus";
	pushEvent->filterable_data[i++].value <<= node->getOutcome().toString().c_str();
      }

    // Additional elements for Command nodes transitioned to executing
    if ((node->getType() == Node::COMMAND()) &&
	(nodeFinished || nodeExecuting))
      {
	CommandId cmd = node->getCommand();
	checkError(!cmd.isNoId(),
		   "formatTransition: command is null!");

	pushEvent->filterable_data[i].name = "commandName";
	pushEvent->filterable_data[i++].value <<= cmd->getName().toString().c_str();

	pushEvent->filterable_data[i].name = "commandArgumentCount";
	pushEvent->filterable_data[i++].value <<= (CORBA::ULong) cmd->getArgValues().size();

	// format parameters
	std::string paramStr;
	for (std::list<double>::const_iterator it = cmd->getArgValues().begin();
	     it != cmd->getArgValues().end();
	     it++)
	  {
	    pushEvent->filterable_data[i].name = "commandArgument";
	    pushEvent->filterable_data[i++].value <<=
	      InterfaceManagerBase::valueToString(*it).c_str();
	  }
      }

    // Additional elements for Update nodes
    if ((node->getType() == Node::UPDATE()) && 
	(nodeFinished || nodeExecuting))
      {
	const std::map<double, double>& bindings = node->getUpdate()->getPairs();
	pushEvent->filterable_data[i].name = "bindingDimension";
	pushEvent->filterable_data[i++].value <<= (CORBA::ULong) bindings.size();
	for (std::map<double,double>::const_iterator it = bindings.begin();
	     it != bindings.end();
	     it++) 
	  {
	    pushEvent->filterable_data[i].name = "bindingName";
	    pushEvent->filterable_data[i++].value <<=
	      (LabelStr ((*it).first)).toString().c_str();
	    pushEvent->filterable_data[i].name = "bindingValue";
	    pushEvent->filterable_data[i++].value <<=
	      InterfaceManagerBase::valueToString((*it).second).c_str();
	  }
      }

    debugMsg("ExecListener:formatTransition",
	     " actually formatted " << i << " name/value pairs");
    return pushEvent;
  }

  CORBA::Any_ptr 
  StructuredEventFormatter::formatTransition(const LabelStr& prevState,
					     const NodeId& node) const
  {
    CORBA::Any_ptr pushAny = new CORBA::Any();
    CosNotification::StructuredEvent * pushEvent =
      formatTransitionStructured(prevState, node);
    (*pushAny) <<= *pushEvent;
    return pushAny;
  }

  

  CosNotification::StructuredEvent *
  StructuredEventFormatter::formatPlanStructured(const PlexilNodeId& plan,
						 const LabelStr& parent) const
  {
    CosNotification::StructuredEvent * pushEvent =
      new CosNotification::StructuredEvent();

    // event header
    pushEvent->header.fixed_header.event_type.domain_name = m_eventDomainName.c_str();
    pushEvent->header.fixed_header.event_type.type_name = "ue.addPlan";

    pushEvent->header.variable_header.length(1);
    pushEvent->header.variable_header[0].name = "eventId";
    pushEvent->header.variable_header[0].value <<= eventId();

    // format here
    pushEvent->filterable_data.length(2);
    pushEvent->filterable_data[0].name = "parentNodeId";
    pushEvent->filterable_data[0].value <<= parent.toString().c_str();
    pushEvent->filterable_data[1].name = "planBody";
    std::ostringstream planStream;
    {
      const TiXmlElement* xmlPlan = PlexilXmlParser::toXml(plan);
      planStream << xmlPlan;
      delete xmlPlan;
    }
    pushEvent->filterable_data[1].value <<= planStream.str().c_str();

    return pushEvent;
  }


  CORBA::Any_ptr 
  StructuredEventFormatter::formatPlan(const PlexilNodeId& plan,
				       const LabelStr& parent) const
  {
    CORBA::Any_ptr pushAny = new CORBA::Any();
    CosNotification::StructuredEvent * pushEvent =
      formatPlanStructured(plan, parent);
    (*pushAny) <<= *pushEvent;
    return pushAny;
  }

}
