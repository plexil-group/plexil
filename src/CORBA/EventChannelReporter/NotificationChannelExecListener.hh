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

#ifndef NC_EXEC_LISTENER_H
#define NC_EXEC_LISTENER_H

#include "EventChannelExecListener.hh"
#include "orbsvcs/CosNotifyChannelAdminC.h" 
#include "orbsvcs/CosNotifyCommS.h" 

namespace PLEXIL
{
  // forward references
  class StructuredEventFormatter;
  typedef Id<StructuredEventFormatter> StructuredEventFormatterId;

  class NotificationChannelExecListener : public POA_CosNotifyComm::StructuredPushSupplier,
					  public BaseEventChannelExecListener
  {
  public:
    NotificationChannelExecListener(const TiXmlElement* xml,
				    InterfaceManagerBase & mgr);
    virtual ~NotificationChannelExecListener();

    virtual bool isConnected() const;
    virtual bool connect(const std::string & notifyChannelName);
    virtual bool disconnect();
    virtual void setFormatter(StructuredEventFormatterId fmtr);

    // CosEventComm::PushSupplier API
    void disconnect_push_supplier()
      throw (CORBA::SystemException);

    // CosNotifyComm::StructuredPushSupplier API
    void disconnect_structured_push_supplier()
      throw (CORBA::SystemException);

    // CosNotifyComm::NotifySubscribe API
    void subscription_change(const CosNotification::EventTypeSeq &,
			     const CosNotification::EventTypeSeq &)
      throw (CORBA::SystemException);

  protected:

    //
    // ExecListener API
    //

    /**
     * @brief Notify that a node has changed state.
     * @param prevState The old state.
     * @param node The node that has transitioned.
     * @note The current state is accessible via the node.
     */
    virtual void
    implementNotifyNodeTransition(NodeState prevState, const NodeId& node) const;
    
    /**
     * @brief Notify that a plan has been received by the Exec.
     * @param plan The intermediate representation of the plan.
     * @param parent The name of the parent node under which this plan will be inserted.
     */
    virtual void
    implementNotifyAddPlan(const PlexilNodeId& plan, const LabelStr& parent) const;

  private:
    // Deliberately unimplemented
    NotificationChannelExecListener();
    NotificationChannelExecListener(const NotificationChannelExecListener &);
    NotificationChannelExecListener & operator=(const NotificationChannelExecListener &);

    CosNotifyChannelAdmin::EventChannel_var m_notifyChannel;
    CosNotifyChannelAdmin::AdminID m_adminId;
    CosNotifyChannelAdmin::ProxyID m_pushConsumerId;
    CosNotifyChannelAdmin::ProxyPushConsumer_var m_pushConsumer;
    CosNotifyChannelAdmin::StructuredProxyPushConsumer_var m_structuredPushConsumer;

    StructuredEventFormatterId m_structuredFormatter;

    bool m_isConnectedToNotifyChannel;

  };

}

#endif // NC_EXEC_LISTENER_H
