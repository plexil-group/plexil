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

#include "EventChannelExecListener.hh"
#include "EventFormatter.hh"
#include "EventFilter.hh"
#include "NameServiceHelper.hh"
#include "Debug.hh"
#include "CoreExpressions.hh"
#include "Node.hh"
#include <sstream>

namespace PLEXIL
{
  BaseEventChannelExecListener::BaseEventChannelExecListener()
    : ExecListener()
  {
  }

  BaseEventChannelExecListener::~BaseEventChannelExecListener()
  {
    delete (EventFormatter*) m_formatter;
    delete (EventFilter*) m_filter;
  }

  void BaseEventChannelExecListener::setFormatter(EventFormatterId fmtr)
  {
    m_formatter = fmtr;
  }

  void BaseEventChannelExecListener::setFilter(EventFilterId fltr)
  {
    m_filter = fltr;
  }

  void BaseEventChannelExecListener::notifyOfTransition(const LabelStr& prevState,
							const NodeId& node) const
  {
    debugMsg("ExecListener", 
	     " notifyOfTransition for node " << node->getNodeId().toString());

    if (m_filter.isNoId())
      {
	debugMsg("ExecListener",
		 " no event filter, notifying on all events");
      }
    else
      {
	// Ignore events the filter rejects
	if (!m_filter->reportEvent(prevState, node))
	  {
	    debugMsg("ExecListener:notifyOfTransition",
		     " event filter rejected event");
	    return;
	  }
      }

    // Push events that make it through the filter onto the channel.
    this->pushTransitionToChannel(prevState, node);
  }

  void BaseEventChannelExecListener::notifyOfAddPlan(const PlexilNodeId& plan,
						     const LabelStr& parent) const
  {
    debugMsg("ExecListener", 
	     " notifyOfAddPlan for parent " << parent.toString());

    if (m_filter.isNoId())
      {
	debugMsg("ExecListener",
		 " no event filter, notifying on all events");
      }
    else
      {
	// Ignore events the filter rejects
	if (!m_filter->reportAddPlan(plan, parent))
	  {
	    debugMsg("ExecListener:notifyOfAddPlan",
		     " event filter rejected event");
	    return;
	  }
      }

    // Push events that make it through the filter onto the channel.
    this->pushAddPlanToChannel(plan, parent);
  }

  EventChannelExecListener::EventChannelExecListener()
    : BaseEventChannelExecListener(),
      POA_CosEventComm::PushSupplier(),
      m_isConnected(false)
  {
  }

  EventChannelExecListener::~EventChannelExecListener()
  {
  }

  bool EventChannelExecListener::isConnected() const
  {
    return m_isConnected;
  }

  bool EventChannelExecListener::connect(const std::string & eventChannelNameString)
  {
    NameServiceHelper & helper = NameServiceHelper::getInstance();
    checkError(helper.isInitialized(),
	       "BaseEventChannelExecListener::connect: Name service is not initialized");

    CosNaming::Name eventChannelName =
      NameServiceHelper::parseName(eventChannelNameString);

    CORBA::Object_var ecAsObject =
      helper.queryNamingServiceForObject(eventChannelName);
    if (CORBA::is_nil(ecAsObject.in()))
      {
	std::cerr << "BaseEventChannelExecListener::connect: naming service unable to find '"
		  << NameServiceHelper::nameToEscapedString(eventChannelName)
		  << "'" << std::endl;
	m_isConnected = false;
	return false;
      }

    try
      {
	m_eventChannel =
	  CosEventChannelAdmin::EventChannel::_duplicate(CosEventChannelAdmin::EventChannel::_narrow(ecAsObject.in()));
	debugMsg("ExecListener",
		 " successfully narrowed reference to event channel");
      }
    catch (CORBA::Exception & e)
      {
	std::cerr << "BaseEventChannelExecListener::connect: Unexpected CORBA exception "
		  << e
		  << " while narrowing to EventChannel" << std::endl;
	m_isConnected = false;
	return false;
      }

    if (CORBA::is_nil(m_eventChannel.in()))
      {
	std::cerr << "BaseEventChannelExecListener::connect: object named '"
		  << NameServiceHelper::nameToEscapedString(eventChannelName)
		  << "' is not an event channel!" << std::endl;
	m_isConnected = false;
	return false;
      }
    debugMsg("ExecListener",
	     " event channel " << m_eventChannel << " found");

    // Now that we have an event channel, get the push-consumer proxy
    try
      {
	CosEventChannelAdmin::SupplierAdmin_var admin =
	  m_eventChannel->for_suppliers();
	m_pushConsumer = admin->obtain_push_consumer();
	// don't need push supplier (I think) -- only used
	// for notifying us when event channel is destroyed
	CosEventComm::PushSupplier_var supplier = this->_this();
	m_pushConsumer->connect_push_supplier(supplier.in());
      }
    catch (CORBA::Exception & e)
      {
	std::cerr << "BaseEventChannelExecListener::connect: Unexpected CORBA exception "
		  << e << "\n while attempting to get push-consumer proxy from event channel"
		  << std::endl;
	m_isConnected = false;
	return false;
      }
    debugMsg("ExecListener",
	     " event channel " << m_eventChannel << " obtained push consumer");
    m_isConnected = true;
    return true;
  }

  bool EventChannelExecListener::disconnect()
  {
    if (!this->isConnected())
      return true;

    m_pushConsumer->disconnect_push_consumer();
    m_pushConsumer = CosEventChannelAdmin::ProxyPushConsumer::_nil();
    m_isConnected = false;
    return true;
  }

  void EventChannelExecListener::disconnect_push_supplier()
    throw (CORBA::SystemException)
  {
    if (!this->isConnected())
      return;

    m_pushConsumer->disconnect_push_consumer();
    m_pushConsumer = CosEventChannelAdmin::ProxyPushConsumer::_nil();
    m_isConnected = false;
    return;
  }

  void
  EventChannelExecListener::pushTransitionToChannel(const LabelStr& prevState,
						    const NodeId& node) const
  {

    // It would help to know we actually have a formatter at this point...
    checkError(!m_formatter.isNoId(),
	       "notifyOfTransition: m_formatter is null!");

    checkError(this->isConnected(),
	       "BaseEventChannelExecListener::notifyOfTransition: not connected to event channel!");

    CORBA::Any_var pushAny = m_formatter->formatTransition(prevState, node);
    // *** breaks with strings!
//     debugMsg("ExecListener:pushTransitionToChannel",
// 	     " formatter returned object of type id "
// 	     << pushAny->type()->id());

    try
      {
	m_pushConsumer->push(*pushAny);
	debugMsg("ExecListener:pushTransitionToChannel", " push successful");
      }
    catch (CORBA::Exception & e)
      {
	std::cerr << "notifyOfTransition: unexpected CORBA exception " << e << std::endl;
      }
  }

  void
  EventChannelExecListener::pushAddPlanToChannel(const PlexilNodeId& plan,
						 const LabelStr& parent) const
  {
    // It would help to know we actually have a formatter at this point...
    checkError(!m_formatter.isNoId(),
	       "notifyOfAddPlan: m_formatter is null!");

    checkError(this->isConnected(),
	       "BaseEventChannelExecListener::notifyOfAddPlan: not connected to event channel!");

    CORBA::Any_var pushAny = m_formatter->formatPlan(plan, parent);
    // *** breaks with strings!
//     debugMsg("ExecListener:pushAddPlanToChannel",
// 	     " formatter returned object of type id "
// 	     << pushAny->type()->id());

    try
      {
	m_pushConsumer->push(*pushAny);
	debugMsg("ExecListener:pushAddPlanToChannel", " push successful");
      }
    catch (CORBA::Exception & e)
      {
	std::cerr << "notifyOfAddPlan: unexpected CORBA exception " << e << std::endl;
      }
  }

}
