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

#include "ExecListener.hh" // for ExecListener
#include "orbsvcs/CosEventChannelAdminC.h" 
#include "orbsvcs/CosEventCommS.h" 

// Forward reference w/o namespace
class TiXmlElement;

namespace PLEXIL
{
  // forward references
  class EventFormatter;
  typedef Id<EventFormatter> EventFormatterId;

  class BaseEventChannelExecListener : public ExecListener
  {
  public:
    virtual ~BaseEventChannelExecListener();

    virtual bool isConnected() const = 0;
    virtual bool connect(const std::string & eventChannelName) = 0;
    virtual bool disconnect() = 0;

    void setFormatter(EventFormatterId fmtr);

    // CosEventComm::PushSupplier API
    virtual void disconnect_push_supplier()
      throw (CORBA::SystemException)
      = 0;

  protected:
    // Only for use by derived classes
    BaseEventChannelExecListener(const TiXmlElement* xml);

    EventFormatterId m_formatter;

  private:
    // Deliberately unimplemented
    BaseEventChannelExecListener(const BaseEventChannelExecListener &);
    BaseEventChannelExecListener & operator=(const BaseEventChannelExecListener &);

  };

  class EventChannelExecListener : public BaseEventChannelExecListener,
				   public POA_CosEventComm::PushSupplier
  {
  public:
    EventChannelExecListener(const TiXmlElement* xml);
    virtual ~EventChannelExecListener();

    virtual bool isConnected() const;
    virtual bool connect(const std::string & eventChannelName);
    virtual bool disconnect();

    virtual void disconnect_push_supplier()
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

    //
    // ExecListener API
    //


    /**
     * @brief Perform listener-specific initialization.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize();

    /**
     * @brief Perform listener-specific startup.
     * @return true if successful, false otherwise.
     */
    virtual bool start();

    /**
     * @brief Perform listener-specific actions to stop.
     * @return true if successful, false otherwise.
     */
    virtual bool stop();

    /**
     * @brief Perform listener-specific actions to reset to initialized state.
     * @return true if successful, false otherwise.
     */
    virtual bool reset();

    /**
     * @brief Perform listener-specific actions to shut down.
     * @return true if successful, false otherwise.
     */
    virtual bool shutdown();

  private:

    // Deliberately unimplemented
    EventChannelExecListener();
    EventChannelExecListener(const EventChannelExecListener &);
    EventChannelExecListener & operator=(const EventChannelExecListener &);

    CosEventChannelAdmin::EventChannel_var m_eventChannel;
    CosEventChannelAdmin::ProxyPushConsumer_var m_pushConsumer;

    bool m_isConnected;
  };

  typedef Id<EventChannelExecListener> EventChannelExecListenerId;

}

#endif // EC_EXEC_LISTENER_H
