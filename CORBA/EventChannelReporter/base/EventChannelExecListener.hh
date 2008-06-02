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

#ifndef EC_EXEC_LISTENER_H
#define EC_EXEC_LISTENER_H

#include "PlexilExec.hh" // for ExecListener
#include "orbsvcs/CosEventChannelAdminC.h" 
#include "orbsvcs/CosEventCommS.h" 

namespace PLEXIL
{
  // forward references
  class EventFormatter;
  typedef Id<EventFormatter> EventFormatterId;
  class EventFilter;
  typedef Id<EventFilter> EventFilterId;

  class BaseEventChannelExecListener : public ExecListener
  {
  public:
    virtual ~BaseEventChannelExecListener();

    virtual bool isConnected() const = 0;
    virtual bool connect(const std::string & eventChannelName) = 0;
    virtual bool disconnect() = 0;

    void setFormatter(EventFormatterId fmtr);
    void setFilter(EventFilterId fltr);

    void notifyOfTransition(const LabelStr& prevState, const NodeId& node) const;
    void notifyOfAddPlan(const PlexilNodeId& plan, const LabelStr& parent) const;

    // CosEventComm::PushSupplier API
    virtual void disconnect_push_supplier()
      throw (CORBA::SystemException)
      = 0;

  protected:
    // Only for use by derived classes
    BaseEventChannelExecListener();

    // required API for derived classes
    virtual void
    pushTransitionToChannel(const LabelStr& prevState, const NodeId& node) const = 0;
    virtual void
    pushAddPlanToChannel(const PlexilNodeId& plan, const LabelStr& parent) const = 0;

    EventFormatterId m_formatter;
    EventFilterId m_filter;

  private:
    // Deliberately unimplemented
    BaseEventChannelExecListener(const BaseEventChannelExecListener &);
    BaseEventChannelExecListener & operator=(const BaseEventChannelExecListener &);

  };

  class EventChannelExecListener : public BaseEventChannelExecListener,
				   public POA_CosEventComm::PushSupplier
  {
  public:
    EventChannelExecListener();
    virtual ~EventChannelExecListener();

    virtual bool isConnected() const;
    virtual bool connect(const std::string & eventChannelName);
    virtual bool disconnect();

    virtual void disconnect_push_supplier()
      throw (CORBA::SystemException);

  protected:
    virtual void
    pushTransitionToChannel(const LabelStr& prevState, const NodeId& node) const;
    virtual void
    pushAddPlanToChannel(const PlexilNodeId& plan, const LabelStr& parent) const;

  private:
    // Deliberately unimplemented
    EventChannelExecListener(const EventChannelExecListener &);
    EventChannelExecListener & operator=(const EventChannelExecListener &);

    CosEventChannelAdmin::EventChannel_var m_eventChannel;
    CosEventChannelAdmin::ProxyPushConsumer_var m_pushConsumer;

    bool m_isConnected;
  };

  typedef Id<EventChannelExecListener> EventChannelExecListenerId;

}

#endif // EC_EXEC_LISTENER_H
