/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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

#ifndef EXEC_CONNECTOR_HH
#define EXEC_CONNECTOR_HH

#include "ExecDefs.hh"

namespace PLEXIL
{

  /**
   * @brief Class for managing the messages from nodes to the executive.  Primarily to facilitate testing.
   */
  class ExecConnector {
  public:
    ExecConnector() : m_id(this) {}
    virtual ~ExecConnector() {m_id.remove();}
    const ExecConnectorId& getId() const {return m_id;}
    virtual void notifyNodeConditionChanged(NodeId node) = 0;
    virtual void handleConditionsChanged(const NodeId& node, NodeState newState) = 0;

	/**
	 * @brief Schedule this assignment for execution.
	 */
	virtual void enqueueAssignment(const AssignmentId& assign) = 0;

	/**
	 * @brief Schedule this command for execution.
	 */
	virtual void enqueueCommand(const CommandId& cmd) = 0;

	/**
	 * @brief Schedule this update for execution.
	 */
	virtual void enqueueUpdate(const UpdateId& update) = 0;

	/**
	 * @brief Needed for stupid unit test
	 */
	virtual void notifyExecuted(const NodeId& node) = 0;

    virtual const StateCacheId& getStateCache() = 0;
    virtual const ExternalInterfaceId& getExternalInterface() = 0;
	virtual const ExecListenerHubId& getExecListenerHub() const = 0;

  private:
    ExecConnectorId m_id;
  };

}

#endif // EXEC_CONNECTOR_HH
