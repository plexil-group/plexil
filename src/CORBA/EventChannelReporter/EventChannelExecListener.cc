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
#include "EventFormatterSchema.hh"
#include "EventFormatterFactory.hh"
#include "EventFilter.hh"
#include "NameServiceHelper.hh"
#include "Debug.hh"
#include "CoreExpressions.hh"
#include "Node.hh"
#include <sstream>

namespace PLEXIL
{
  BaseEventChannelExecListener::BaseEventChannelExecListener(const TiXmlElement* xml,
                                                             InterfaceManagerBase & mgr)
    : ManagedExecListener(xml, mgr),
      m_formatter()
  {
    // Get formatter spec from XML
    if (this->getXml() == NULL)
      return;
    const TiXmlElement* formatterXml = 
      this->getXml()->FirstChildElement(EventFormatterSchema::EVENT_FORMATTER_TAG());
    if (formatterXml == NULL)
      return;
    m_formatter = EventFormatterFactory::createInstance(formatterXml,
                                                        this->getManager());
  }

  BaseEventChannelExecListener::~BaseEventChannelExecListener()
  {
    delete (EventFormatter*) m_formatter;
  }

  void BaseEventChannelExecListener::setFormatter(EventFormatterId fmtr)
  {
    m_formatter = fmtr;
  }

  EventChannelExecListener::EventChannelExecListener(const TiXmlElement* xml,
                                                     InterfaceManagerBase & mgr)
    : BaseEventChannelExecListener(xml, mgr),
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

  //
  // ManagedExecListener API
  //

  /**
   * @brief Perform listener-specific initialization.
   * @return true if successful, false otherwise.
   */
  bool EventChannelExecListener::initialize()
  {
    // Some checks before we get rolling
    if (this->getXml() == NULL)
      {
        // can't get channel name from nonexistent XML, so fail
        debugMsg("EventChannelExecListener:initialize",
                 " failed; no configuration XML provided");
        return false;
      }

    // Extract event channel name from XML
    const char* channelName =
      this->getXml()->Attribute(EventFormatterSchema::EVENT_CHANNEL_NAME_ATTRIBUTE());
    if (channelName == NULL)
      {
        // can't connect without a channel name, so fail
        debugMsg("EventChannelExecListener:initialize",
                 " failed; no " << EventFormatterSchema::EVENT_CHANNEL_NAME_ATTRIBUTE()
                 << " attribute provided");
        return false;
      }

    return true;
  }


  /**
   * @brief Perform listener-specific startup.
   * @return true if successful, false otherwise.
   */
  bool EventChannelExecListener::start()
  {
    if (isConnected())
      return true; // already done

    if (this->getXml() == NULL)
      {
        // can't get channel name from nonexistent XML, so fail
        debugMsg("EventChannelExecListener:start",
                 " failed; no configuration XML provided");
        return false;
      }

    // Extract event channel name from XML
    const char* channelName =
      this->getXml()->Attribute(EventFormatterSchema::EVENT_CHANNEL_NAME_ATTRIBUTE());
    if (channelName == NULL)
      {
        // can't connect without a channel name, so fail
        debugMsg("EventChannelExecListener:initialize",
                 " failed; no " << EventFormatterSchema::EVENT_CHANNEL_NAME_ATTRIBUTE()
                 << " attribute provided");
        return false;
      }
    
    return connect(std::string(channelName));
  }


  /**
   * @brief Perform listener-specific actions to stop.
   * @return true if successful, false otherwise.
   */
  bool EventChannelExecListener::stop()
  {
    debugMsg("EventChannelExecListener:stop", " disconnecting");
    return disconnect();
  }


  /**
   * @brief Perform listener-specific actions to reset to initialized state.
   * @return true if successful, false otherwise.
   */
  bool EventChannelExecListener::reset()
  {
    // Nothing to do here since we should be disconnected
    return true;
  }


  /**
   * @brief Perform listener-specific actions to shut down.
   * @return true if successful, false otherwise.
   */
  bool EventChannelExecListener::shutdown()
  {
    // Nothing to do here since we should be disconnected
    return true;
  }


  bool EventChannelExecListener::connect(const std::string & eventChannelNameString)
  {
    NameServiceHelper & helper = NameServiceHelper::getInstance();
    checkError(helper.isInitialized(),
               "EventChannelExecListener::connect: Name service is not initialized");

    CosNaming::Name eventChannelName =
      NameServiceHelper::parseName(eventChannelNameString);

    CORBA::Object_var ecAsObject =
      helper.queryNamingServiceForObject(eventChannelName);
    if (CORBA::is_nil(ecAsObject.in()))
      {
        std::cerr << "EventChannelExecListener::connect: naming service unable to find '"
                  << NameServiceHelper::nameToEscapedString(eventChannelName)
                  << "'" << std::endl;
        m_isConnected = false;
        return false;
      }

    try
      {
        m_eventChannel = CosEventChannelAdmin::EventChannel::_narrow(ecAsObject.in());
        debugMsg("EventChannelExecListener:connect",
                 " successfully narrowed reference to event channel");
      }
    catch (CORBA::Exception & e)
      {
        std::cerr << "EventChannelExecListener::connect: Unexpected CORBA exception "
                  << e
                  << " while narrowing to EventChannel" << std::endl;
        m_isConnected = false;
        return false;
      }

    if (CORBA::is_nil(m_eventChannel.in()))
      {
        std::cerr << "EventChannelExecListener::connect: object named '"
                  << NameServiceHelper::nameToEscapedString(eventChannelName)
                  << "' is not an event channel!" << std::endl;
        m_isConnected = false;
        return false;
      }
    debugMsg("EventChannelExecListener:connect",
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
        std::cerr << "EventChannelExecListener::connect: Unexpected CORBA exception "
                  << e << "\n while attempting to get push-consumer proxy from event channel"
                  << std::endl;
        m_isConnected = false;
        return false;
      }
    debugMsg("EventChannelExecListener:connect",
             " event channel " << m_eventChannel << " obtained push consumer");
    m_isConnected = true;
    return true;
  }

  bool EventChannelExecListener::disconnect()
  {
    if (this->isConnected()) {
      debugMsg("EventChannelExecListener:disconnect", " from event channel");
      try {
        m_pushConsumer->disconnect_push_consumer();
      }
      catch (CORBA::Exception &e) {
        debugMsg("EventChannelExecListener:disconnect",
                 " ignoring CORBA exception " << e << " while attempting to disconnect");
      }
      m_pushConsumer = CosEventChannelAdmin::ProxyPushConsumer::_nil();
      m_isConnected = false;
    }

    debugMsg("EventChannelExecListener:disconnect", " successful");
    return true;
  }

  // CosEventComm::PushSupplier API

  // The push consumer calls this to break the connection.
  void EventChannelExecListener::disconnect_push_supplier()
    throw (CORBA::SystemException)
  {
    debugMsg("EventChannelExecListener:disconnect_push_supplier",
             " disconnecting at event channel's request");
    if (!this->isConnected())
      return;

    // Since the consumer is telling us he's shutting down,
    // should be no need to call disconnect_push_consumer()
    m_pushConsumer = CosEventChannelAdmin::ProxyPushConsumer::_nil();
    m_isConnected = false;
    return;
  }


  /**
   * @brief Notify that a node has changed state.
   * @param prevState The old state.
   * @param node The node that has transitioned.
   * @note The current state is accessible via the node.
   */
  void
  EventChannelExecListener::implementNotifyNodeTransition(const LabelStr& prevState,
                                                          const NodeId& node) const
  {

    // It would help to know we actually have a formatter at this point...
    checkError(!m_formatter.isNoId(),
               "notifyOfTransition: m_formatter is null!");

    checkError(this->isConnected(),
               "BaseEventChannelExecListener::notifyOfTransition: not connected to event channel!");

    CORBA::Any_var pushAny = m_formatter->formatTransition(prevState, node);
    // *** breaks with strings!
    //     debugMsg("EventChannelExecListener:notifyOfTransition",
    //       " formatter returned object of type id "
    //       << pushAny->type()->id());

    try
      {
        m_pushConsumer->push(*pushAny);
        debugMsg("EventChannelExecListener:notifyOfTransition", " push successful");
      }
    catch (CORBA::Exception & e)
      {
        std::cerr << "EventChannelExecListener::implementNotifyNodeTransition: unexpected CORBA exception "
                  << e
                  << std::endl;
      }
  }

  /**
   * @brief Notify that a plan has been received by the Exec.
   * @param plan The intermediate representation of the plan.
   * @param parent The name of the parent node under which this plan will be inserted.
   * @note The default method does nothing.
   */
  void
  EventChannelExecListener::implementNotifyAddPlan(const PlexilNodeId& plan,
                                                   const LabelStr& parent) const
  {
    // It would help to know we actually have a formatter at this point...
    checkError(!m_formatter.isNoId(),
               "notifyOfAddPlan: m_formatter is null!");

    checkError(this->isConnected(),
               "EventChannelExecListener::notifyOfAddPlan: not connected to event channel!");

    CORBA::Any_var pushAny = m_formatter->formatPlan(plan, parent);
    // *** breaks with strings!
    //     debugMsg("EventChannelExecListener:notifyOfAddPlan",
    //       " formatter returned object of type id "
    //       << pushAny->type()->id());

    try
      {
        m_pushConsumer->push(*pushAny);
        debugMsg("EventChannelExecListener:notifyOfAddPlan", " push successful");
      }
    catch (CORBA::Exception & e)
      {
        std::cerr << "EventChannelExecListener::implementNotifyAddPlan: unexpected CORBA exception "
                  << e
                  << std::endl;
      }
  }

}
