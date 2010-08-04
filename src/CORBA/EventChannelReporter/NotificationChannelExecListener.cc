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

#include "NotificationChannelExecListener.hh"
#include "StructuredEventFormatter.hh"
#include "EventFormatterSchema.hh"
#include "EventFormatterFactory.hh"
#include "NameServiceHelper.hh"
#include "Debug.hh"
#include "CoreExpressions.hh"
#include "Node.hh"
#include "PlexilXmlParser.hh"

#include <sstream>

namespace PLEXIL
{
  NotificationChannelExecListener::NotificationChannelExecListener(const TiXmlElement* xml,
								   InterfaceManagerBase & mgr)
    : POA_CosNotifyComm::StructuredPushSupplier(),
      BaseEventChannelExecListener(xml, mgr),
      m_isConnectedToNotifyChannel(false)
  {
    // Get structured formatter spec from XML
    if (this->getXml() == NULL)
      return;
    const TiXmlElement* formatterXml = 
      this->getXml()->FirstChildElement(EventFormatterSchema::STRUCTURED_FORMATTER_TAG());
    if (formatterXml == NULL)
      return;
    m_structuredFormatter =
      StructuredEventFormatterFactory::createInstance(formatterXml,
						      this->getManager());
  }

  NotificationChannelExecListener::~NotificationChannelExecListener()
  {
    if (isConnected())
      disconnect();
  }

  void NotificationChannelExecListener::setFormatter(StructuredEventFormatterId fmtr)
  {
    m_structuredFormatter = fmtr;
  }

  bool NotificationChannelExecListener::isConnected() const
  {
    return m_isConnectedToNotifyChannel;
  }

  bool NotificationChannelExecListener::connect(const std::string & notifyChannelNameString)
  {
    checkError(!m_formatter.isNoId() || !m_structuredFormatter.isNoId(),
	       "NotificationChannelExecListener::connect: Must have a formatter to connect!");

    NameServiceHelper & helper = NameServiceHelper::getInstance();
    checkError(helper.isInitialized(),
	       "NotificationChannelExecListener::connect: Name service is not initialized");

    CosNaming::Name notifyChannelName =
      NameServiceHelper::parseName(notifyChannelNameString);

    CORBA::Object_var ecAsObject =
      helper.queryNamingServiceForObject(notifyChannelName);
    if (CORBA::is_nil(ecAsObject.in()))
      {
	std::cerr << "NotificationChannelExecListener::connect: naming service unable to find '"
		  << NameServiceHelper::nameToEscapedString(notifyChannelName)
		  << "'" << std::endl;
	m_isConnectedToNotifyChannel = false;
	return false;
      }

    try
      {
	m_notifyChannel =
	  CosNotifyChannelAdmin::EventChannel::_duplicate(CosNotifyChannelAdmin::EventChannel::_narrow(ecAsObject.in()));
	debugMsg("NotificationChannelExecListener:connect",
		 " successfully narrowed reference to notification service event channel");
      }
    catch (CORBA::Exception & e)
      {
	std::cerr << "NotificationChannelExecListener::connect: Unexpected CORBA exception "
		  << e
		  << " while narrowing to notification EventChannel" << std::endl;
	m_isConnectedToNotifyChannel = false;
	return false;
      }

    if (CORBA::is_nil(m_notifyChannel.in()))
      {
	std::cerr << "NotificationChannelExecListener::connect: object named '"
		  << NameServiceHelper::nameToEscapedString(notifyChannelName)
		  << "' is not a notification service event channel!" << std::endl;
	m_isConnectedToNotifyChannel = false;
	return false;
      }
    debugMsg("NotificationChannelExecListener:connect",
	     " notification service event channel " << m_notifyChannel << " found");

    // Now that we have an event channel, get the push-consumer proxy
    CosNotifyChannelAdmin::ProxyConsumer_var proxy;
    try
      {
	CosNotifyChannelAdmin::SupplierAdmin_var admin =
	  m_notifyChannel->new_for_suppliers(CosNotifyChannelAdmin::AND_OP,
					     m_adminId);

	// Choose appropriate proxy type depending on whether we have
	// a structured formatter or not
	CosNotifyChannelAdmin::ClientType clientType;
	if (m_structuredFormatter.isNoId())
	  {
	    clientType = CosNotifyChannelAdmin::ANY_EVENT;
	    debugMsg("NotificationChannelExecListener:connect", " requesting ANY_EVENT proxy push consumer");
	  }
	else
	  {
	   clientType = CosNotifyChannelAdmin::STRUCTURED_EVENT;
	    debugMsg("NotificationChannelExecListener:connect", " requesting STRUCTURED_EVENT proxy push consumer");
	  }

	proxy =
	  admin->obtain_notification_push_consumer(clientType,
						   m_pushConsumerId);
      }
    catch (CORBA::Exception & e)
      {
	std::cerr << "NotificationChannelExecListener::connect: Unexpected CORBA exception "
		  << e << "\n while attempting to get structured push-consumer proxy from notification channel"
		  << std::endl;
	m_isConnectedToNotifyChannel = false;
	return false;
      }

    // Narrow to appropriate type
    try
      {
        if (m_structuredFormatter.isNoId())
	  {
	    // No structured formatter, push anys
	    m_pushConsumer = 
	      CosNotifyChannelAdmin::ProxyPushConsumer::_duplicate(CosNotifyChannelAdmin::ProxyPushConsumer::_narrow(proxy.in()));
	    debugMsg("NotificationChannelExecListener:connect",
		     " successfully narrowed reference to (any) proxy push consumer");
	  }
	else 
	  {
	    // Structured formatter, push structured events
	    m_structuredPushConsumer = 
	      CosNotifyChannelAdmin::StructuredProxyPushConsumer::_duplicate(CosNotifyChannelAdmin::StructuredProxyPushConsumer::_narrow(proxy.in()));
	    debugMsg("NotificationChannelExecListener:connect",
		     " successfully narrowed reference to structured proxy push consumer");
	  }
      }
    catch (CORBA::Exception & e)
      {
	std::cerr << "NotificationChannelExecListener::connect: Unexpected CORBA exception "
		  << e
		  << " while narrowing proxy push consumer" << std::endl;
	m_isConnectedToNotifyChannel = false;
	return false;
      }

    // connect to it
    try
      {
	// push supplier (I think) only used
	// for notifying us when event channel is destroyed
        if (m_structuredFormatter.isNoId())
	  {
	    checkError(ALWAYS_FAIL, "Push-any supplier not yet implemented!");
	    //m_pushConsumer->connect_any_push_supplier(this->CosEventComm::PushSupplier::_this());
	    debugMsg("NotificationChannelExecListener:connect",
		     " connected to UNstructured proxy push consumer");
	  }
	else
	  {
	    m_structuredPushConsumer->connect_structured_push_supplier(this->_this());
	    debugMsg("NotificationChannelExecListener:connecct",
		     " connected to structured proxy push consumer");
	  }
      }
    catch (CosEventChannelAdmin::AlreadyConnected & e)
      {
	// can ignore this error (?)
	debugMsg("NotificationChannelExecListener:connect", " Warning: Already connected to push-consumer proxy");
	m_isConnectedToNotifyChannel = true;
	return true;
      }
    catch (CORBA::Exception & e)
      {
	std::cerr << "NotificationChannelExecListener::connect: Unexpected CORBA exception "
		  << e << "\n while attempting to connect to push-consumer proxy"
		  << std::endl;
	m_isConnectedToNotifyChannel = false;
	return false;
      }
    m_isConnectedToNotifyChannel = true;
    return true;
  }

  bool NotificationChannelExecListener::disconnect()
  {
    if (this->isConnected()) {
	  if (m_pushConsumer != CosNotifyChannelAdmin::ProxyPushConsumer::_nil()) {
		try {
		  m_pushConsumer->disconnect_push_consumer();
		}
		catch (CORBA::Exception &e) {
		  debugMsg("NotificationChannelExecListener:disconnect",
				   " ignoring CORBA exception " << e << " while attempting to disconnect");
		}
		m_pushConsumer = CosNotifyChannelAdmin::ProxyPushConsumer::_nil();
	  }
	  if (m_structuredPushConsumer != CosNotifyChannelAdmin::StructuredProxyPushConsumer::_nil()) {
		try {
		  m_structuredPushConsumer->disconnect_structured_push_consumer();
		}
		catch (CORBA::Exception &e) {
		  debugMsg("NotificationChannelExecListener:disconnect",
				   " ignoring CORBA exception " << e << " while attempting to disconnect");
		}
		m_structuredPushConsumer = CosNotifyChannelAdmin::StructuredProxyPushConsumer::_nil();
	  }
	  m_isConnectedToNotifyChannel = false;
	}
	debugMsg("NotificationChannelExecListener:disconnect", " successful");
    return true;
  }

  void NotificationChannelExecListener::disconnect_push_supplier()
    throw (CORBA::SystemException)
  {
	debugMsg("NotificationChannelExecListener:disconnect_push_supplier",
			 " disconnecting at notification channel's request");
    if (!this->isConnected())
      return;

	m_pushConsumer = CosNotifyChannelAdmin::ProxyPushConsumer::_nil();
    m_isConnectedToNotifyChannel = false;
    return;
  }

  void NotificationChannelExecListener::disconnect_structured_push_supplier()
    throw (CORBA::SystemException)
  {
    if (!this->isConnected())
      return;

	m_structuredPushConsumer = CosNotifyChannelAdmin::StructuredProxyPushConsumer::_nil();
    m_isConnectedToNotifyChannel = false;
    return;
  }

  void NotificationChannelExecListener::subscription_change(const CosNotification::EventTypeSeq &,
							    const CosNotification::EventTypeSeq &)
      throw (CORBA::SystemException)
  {
    // Not yet implemented
  }

  void
  NotificationChannelExecListener::implementNotifyNodeTransition(const LabelStr& prevState,
								 const NodeId& node) const
  {
    checkError(this->isConnected(),
	       "NotificationChannelExecListener::notifyOfTransition: not connected to event channel!");

    if (m_structuredFormatter.isNoId()
	|| m_structuredPushConsumer == CosNotifyChannelAdmin::StructuredProxyPushConsumer::_nil())
      {
	// Push as ANY
	// It would help to know we actually have a connection at this point...
	checkError(m_pushConsumer != CosNotifyChannelAdmin::ProxyPushConsumer::_nil(),
		   "notifyOfTransition: no proxy push consumer!");

	CORBA::Any_var pushAny = m_formatter->formatTransition(prevState, node);
	debugMsg("ExecListener:notifyOfTransition",
		 " formatter returned object of type id "
		 << pushAny->type()->id());

	try
	  {
	    m_pushConsumer->push(*pushAny);
	    debugMsg("ExecListener:notifyOfTransition", " push any successful");
	  }
	catch (CORBA::Exception & e)
	  {
	    std::cerr << "notifyOfTransition: unexpected CORBA exception " << e
		      << "\n while pushing node transition to channel" << std::endl;
	  }
      }
    else
      {
	CosNotification::StructuredEvent_var event =
	  m_structuredFormatter->formatTransitionStructured(prevState, node);
	try
	  {
	    m_structuredPushConsumer->push_structured_event(*event);
	    debugMsg("ExecListener:notifyOfTransition", " push structured successful");
	  }
	catch (CORBA::Exception & e)
	  {
	    std::cerr << "NotificationChannelExecListener::implementNotifyNodeTransition: unexpected CORBA exception "
		      << e
		      << "\n while pushing node transition to channel"
		      << std::endl;
	  }
      }
  }

  void
  NotificationChannelExecListener::implementNotifyAddPlan(const PlexilNodeId& plan,
							  const LabelStr& parent) const
  {
    checkError(this->isConnected(),
	       "NotificationChannelExecListener::implementNotifyAddPlan: not connected to event channel!");
    if (m_structuredFormatter.isNoId()
	|| m_structuredPushConsumer == CosNotifyChannelAdmin::StructuredProxyPushConsumer::_nil())
      {
	// Push as ANY
	// It would help to know we actually have a connection at this point...
	checkError(m_pushConsumer != CosNotifyChannelAdmin::ProxyPushConsumer::_nil(),
		   "NotificationChannelExecListener::implementNotifyAddPlan: no proxy push consumer!");

	CORBA::Any_var pushAny = m_formatter->formatPlan(plan, parent);
	debugMsg("ExecListener:notifyOfAddPlan",
		 " formatter returned object of type id "
		 << pushAny->type()->id());

	try
	  {
	    m_pushConsumer->push(*pushAny);
	    debugMsg("ExecListener:notifyOfAddPlan", " push any successful");
	  }
	catch (CORBA::Exception & e)
	  {
	    std::cerr << "NotificationChannelExecListener::implementNotifyAddPlan: unexpected CORBA exception "
		      << e
		      << "\n while pushing AddPlan event to channel"
		      << std::endl;
	  }
      }
    else
      {
	CosNotification::StructuredEvent_var event =
	  m_structuredFormatter->formatPlanStructured(plan, parent);
	try
	  {
	    m_structuredPushConsumer->push_structured_event(*event);
	    debugMsg("ExecListener:notifyOfAddPlan", " push structured successful");
	  }
	catch (CORBA::Exception & e)
	  {
	    std::cerr << "NotificationChannelExecListener::implementNotifyAddPlan: unexpected CORBA exception "
		      << e
		      << "\n while pushing AddPlan event to channel" << std::endl;
	  }
      }
  }

}
