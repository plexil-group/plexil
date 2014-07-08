/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

// #include "ExecDefs.hh"
#include "Id.hh"
#include "NodeConstants.hh"

namespace PLEXIL
{
  // Forward references
  class Assignment;
  DECLARE_ID(Assignment);

  class ExecConnector; // defined below
  DECLARE_ID(ExecConnector);

  class ExecListenerHub;
  DECLARE_ID(ExecListenerHub);

  class ExternalInterface;
  DECLARE_ID(ExternalInterface);

  class Node;
  DECLARE_ID(Node);

  class PlexilNode;
  DECLARE_ID(PlexilNode);

  /**
   * @brief Abstract class representing the key API of the PlexilExec. Facilitates testing.
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
     * @brief Schedule this assignment for execution.
     */
    virtual void enqueueAssignmentForRetraction(const AssignmentId& assign) = 0;

    /**
     * @brief Mark node as finished and no longer eligible for execution.
     */
    virtual void markRootNodeFinished(const NodeId& node) = 0;

    // Needed by TestExternalInterface

    /**
     * @brief Add the plan under the node named by the parent.
     * @param plan The intermediate representation of the plan.
     * @note If the plan references any library nodes, they are linked in.
     */
    void addPlan(PlexilNodeId const &plan);

    /**
     * @brief Process all the entries in the input queue.
     * @return True if the Exec needs to be stepped afterward.
     */
    bool processQueue();

    /**
     * @brief Begins a single "macro step" i.e. the entire quiescence cycle.
     */
    virtual void step(double startTime) = 0; // *** FIXME: use real time type ***

    /**
     * @brief Returns true if the Exec needs to be stepped.
     */
    virtual bool needsStep() const = 0;

    virtual const ExecListenerHubId& getExecListenerHub() const = 0;

  private:
    ExecConnectorId m_id;
  };

  // Global pointer to the exec instance
  extern ExecConnectorId g_exec;

}

#endif // EXEC_CONNECTOR_HH
