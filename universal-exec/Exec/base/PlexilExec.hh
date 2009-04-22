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

#ifndef _H_PlexilExec
#define _H_PlexilExec

#include "ExecDefs.hh"
#include "PlexilPlan.hh"
#include "LabelStr.hh"

#include <list>
#include <set>

namespace PLEXIL 
{
  // Forward references
  class ExternalInterface;
  typedef Id<ExternalInterface> ExternalInterfaceId;

  // *** ExecListener now has its own file ***

  /**
   * @brief Class for managing the messages from nodes to the executive.  Primarily to facilitate testing.
   */
  class ExecConnector {
  public:
    ExecConnector() : m_id(this) {}
    virtual ~ExecConnector() {m_id.remove();}
    const ExecConnectorId& getId() const {return m_id;}
    virtual void handleConditionsChanged(const NodeId& node) = 0;
    virtual void handleNeedsExecution(const NodeId& node) = 0;
    //virtual const ExpressionId& findVariable(const LabelStr& name) = 0;
    virtual const StateCacheId& getStateCache() = 0;
    virtual const ExternalInterfaceId& getExternalInterface() = 0;
  protected:
  private:
    ExecConnectorId m_id;
  };

  /**
   * @brief Comparator for ordering nodes that are in conflict.  Higher priority wins, but nodes already EXECUTING dominate.
   */
  struct NodeConflictComparator {
    bool operator() (NodeId x, NodeId y) const;
  };

  /**
   * @brief The core PLEXIL executive.  Instantiate it with the XML representation for a plan, instantiate
   * an external interface, and it should start doing things the moment an event comes in.
   */
  class PlexilExec {
  public:
    /**
     * @brief Constructor.  Instantiates the entire plan from parsed XML.
     * @param plan The intermediate representation of the plan.
     */
    PlexilExec(PlexilNodeId& plan);

    /**
     * @brief Default constructor.
     */
    PlexilExec();

    /**
     * @brief Destructor.  Kills the plan dead.
     */
    ~PlexilExec();

    const PlexilExecId& getId() const {return m_id;}

    /**
     * @brief Set the ExternalInterface instance used by this Exec.
     * @param id The Id of the ExternalInterface instance.
     */
    void setExternalInterface(ExternalInterfaceId& id);

    /**
     * @brief Return the ExternalInterface instance used by this Exec.
     */
    inline ExternalInterfaceId& getExternalInterface()
    {
      return m_interface;
    }

    /**
     * @brief Add a library node.
     * @param libNode The intermediate representation of the library node.
     */
    void addLibraryNode(const PlexilNodeId& libNode);

    /**
     * @brief Add the plan under the node named by the parent.
     * @param plan The intermediate representation of the plan.
     * @param parent The name of the node under which to insert this plan.
     * @note If the plan references any library nodes, they are linked in.
     * @note Currently parent is ignored.
     */
    void addPlan(PlexilNodeId& plan, const LabelStr& parent = EMPTY_LABEL());

    /**
     * @brief Begins a single "macro step" i.e. the entire quiescence cycle.
     */
    void step();

    /**
     * @brief Adds an ExecListener for publication of node transition events.
     */
    void addListener(const ExecListenerId& listener);

    /**
     * @brief Removes an ExecListener.
     */
    void removeListener(const ExecListenerId& listener);

    /**
     * @brief accessor for the state cache.
     */
    StateCacheId& getStateCache() {return m_cache;}

  protected:
    friend class RealExecConnector;
    /**
     * @brief Handle the fact that a node's relevant conditions have changed (it is eligible for state change).
     * Adds assignment nodes that are eligible for execution to the resource conflict map.
     * @param node The node which is eligible for state change.
     */
    void handleConditionsChanged(const NodeId node);

    /**
     * @brief Handle the fact that a node has transitioned to EXECUTING.  Adds
     * whatever the node is supposed to do (assign, execute a command) to a list to be batched to the external interface.
     * @param node The node which has transitioned to state EXECUTING.
     */
    void handleNeedsExecution(const NodeId node);

    //const ExpressionId& findVariable(const LabelStr& name);


  private:

    /**
     * @brief Resolve conflicts among potentially executing assignment variables (in the future, this will also handle
     * command conflicts). 
     * @note The current implementation is wrong.  Assignment nodes that are eligible for execution should
     * not be added to the state change queue, and when a macro step begins, the system should take the first non-executing
     * node from each conflict list (the lists are ordered by priority) and add it to the end of the queue.
     */
    void resolveResourceConflicts();

    /**
     * @brief Adds a node to consideration for resource contention.  The node must be an assignment node and it must be eligible to transition to EXECUTING.
     * @param node The assignment node.
     */
    void addToResourceContention(const NodeId node);

    /**
     * @brief Removes a node from consideration for resource contention.  This is usually because some condition has changed that makes the node no longer
     * eligible for execution.
     * @param node The assignment node.
     */
    void removeFromResourceContention(const NodeId node);

    /**
     * @brief Gets a stringified version of the current state change queue.
     */

    std::string stateChangeQueueStr();

    /**
     * @brief Batch-perform internal assignments queued up from a quiescence step.
     */
    void performAssignments();

    /**
     * @brief True if the node is in the state change queue.
     */
    int inQueue(const NodeId node) const;

    /**
     * @brief The loop that performs quiescence.  This function is recursive.
     * @param depth The current depth of quiescence (used mostly for output).
     * @param stop The position in the state change queue at which this loop should terminate.
     * @param time
     */
    void quiescenceLoop(const int depth, const unsigned int stop, const double time);

    inline void publishTransition(const LabelStr& oldState, const NodeId& node);
    void publishAddPlan(const PlexilNodeId& plan, const LabelStr& parent);
    void publishAddLibrary(const PlexilNodeId& libNode);
    void publishAssignment(const ExpressionId & dest,
                           const std::string& destName,
                           const double& value);

    PlexilExecId m_id; /*<! The Id for this executive.*/
    unsigned int m_cycleNum, m_queuePos;
    ExecConnectorId m_connector;
    StateCacheId m_cache;
    ExternalInterfaceId m_interface;
    std::list<NodeId> m_plan; /*<! The root of the plan.*/
    //std::list<NodeId> m_stateChangeQueue; /*<! A list of nodes that are eligible for state transition.*/
    std::map<unsigned int, NodeId> m_stateChangeQueue;
    std::list<AssignmentId> m_assignmentsToExecute;
    std::list<CommandId> m_commandsToExecute;
    std::list<UpdateId> m_updatesToExecute;
    std::list<FunctionCallId> m_functionCallsToExecute;
    std::map<ExpressionId, std::multiset<NodeId, NodeConflictComparator> > m_resourceConflicts; /*<! A map from variables to sets of nodes which is used to resolve resource contention.
                                                                                                  The nodes in the sets are assignment nodes which can assign values to the variable.
                                                                                                  The sets are ordered by priority, but the order is dominated by EXECUTING nodes.
                                                                                                  Essentially, at each quiescence cycle, the first node in each set that isn't already
                                                                                                  in state EXECUTING gets added to the end of the queue. */
    std::list<ExecListenerId> m_listeners;
    std::vector<PlexilNodeId> m_libraries;
  };
}

#endif
