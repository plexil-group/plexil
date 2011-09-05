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

#include "ExecConnector.hh"
#include "generic_hash_map.hh"
#include "LabelStr.hh"
#include "PlexilPlan.hh"

#include <list>
#include <set>

namespace PLEXIL 
{
  // Forward references
  class ExternalInterface;
  typedef Id<ExternalInterface> ExternalInterfaceId;
  class ExecListenerBase;
  typedef Id<ExecListenerBase> ExecListenerBaseId;
  class ExecListenerHub;
  typedef Id<ExecListenerHub> ExecListenerHubId;

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
  class PlexilExec : public ExecConnector
  {
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
     * @brief Queries whether the named library node is loaded.
     * @param nodeName The name of the library node.
     * @return True if the node is already defined, false otherwise.
     */
    bool hasLibrary(const std::string& nodeName) const;

    /**
     * @brief Retrieves the named library node if it is present.
     * @param nodeName The name of the library node.
     * @return The library node, or noId() if not found.
     */
    const PlexilNodeId getLibrary(const std::string& nodeName) const;

    /**
     * @brief Add the plan under the node named by the parent.
     * @param plan The intermediate representation of the plan.
     * @param parent The name of the node under which to insert this plan.
	 * @return true if successful, false otherwise.
     * @note If the plan references any library nodes, they are linked in.
     * @note Currently parent is ignored.
     */
    bool addPlan(PlexilNodeId& plan, const LabelStr& parent = EMPTY_LABEL());

    /**
     * @brief Begins a single "macro step" i.e. the entire quiescence cycle.
     */
    void step();

	/**
	 * @brief Returns true if the Exec needs to be stepped.
	 */
	bool needsStep() const;

	/**
	 * @brief Set the ExecListenerHub instance.
	 */
	void setExecListenerHub(const ExecListenerHubId& hub)
	{
	  m_listener = hub;
	}

	/**
	 * @brief Get the ExecListenerHub instance.
	 */
	const ExecListenerHubId& getExecListenerHub() const
	{
	  return m_listener;
	}

    /**
     * @brief Adds an ExecListener for publication of node transition events.
	 * @note Convenience method for backward compatibility.
     */
    void addListener(const ExecListenerBaseId& listener);

    /**
     * @brief Removes an ExecListener.
	 * @note Convenience method for backward compatibility.
     */
    void removeListener(const ExecListenerBaseId& listener);

    /**
     * @brief accessor for the state cache.
     */
    StateCacheId& getStateCache() {return m_cache;}

    /**
     * @brief Queries whether all plans are finished.
     * @return true if all finished, false otherwise.
     */
    bool allPlansFinished() const;

	//
	// API to Node classes
	//

	/**
	 * @brief Schedule this assignment for execution.
	 */
	void enqueueAssignment(const AssignmentId& assign);

	/**
	 * @brief Schedule this command for execution.
	 */
	void enqueueCommand(const CommandId& cmd);

	/**
	 * @brief Schedule this update for execution.
	 */
	void enqueueUpdate(const UpdateId& update);

	/**
	 * @brief Needed for stupid unit test
	 */
	virtual void notifyExecuted(const NodeId& node) 
	{
	}

  protected:
    friend class RealExecConnector;

    /**
     * @brief Handle the fact that a node's conditions may have changed (it is eligible for state change).
     * @param node The node which is eligible for state change.
     */
    void notifyNodeConditionChanged(NodeId node);

    /**
     * @brief Handle the fact that a node's relevant conditions have changed (it is eligible for state change).
     * Adds assignment nodes that are eligible for execution to the resource conflict map.
     * @param node The node which is eligible for state change.
     */
    void handleConditionsChanged(const NodeId& node, NodeState newState);

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
    void addToResourceContention(const NodeId& node);

    /**
     * @brief Removes a node from consideration for resource contention.  This is usually because some condition has changed that makes the node no longer
     * eligible for execution.
     * @param node The assignment node.
     */
    void removeFromResourceContention(const NodeId& node);

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

	typedef std::map<unsigned int, NodeTransition> StateChangeQueue;
	typedef std::multiset<NodeId, NodeConflictComparator> VariableConflictSet;
	typedef std::map<VariableId, VariableConflictSet> VariableConflictMap;
    PlexilExecId m_id; /*<! The Id for this executive.*/
    unsigned int m_cycleNum, m_queuePos;
    StateCacheId m_cache;
    ExternalInterfaceId m_interface;
    std::list<NodeId> m_plan; /*<! The root of the plan.*/
    std::vector<NodeId> m_nodesToConsider; /*<! Nodes whose conditions have changed and may be eligible to transition. */
    StateChangeQueue m_stateChangeQueue; /*<! A list of nodes that are eligible for state transition.*/
    std::vector<AssignmentId> m_assignmentsToExecute;
    std::list<CommandId> m_commandsToExecute;
    std::list<UpdateId> m_updatesToExecute;
    VariableConflictMap m_resourceConflicts; /*<! A map from variables to sets of nodes which is used to resolve resource contention.
											   The nodes in the sets are assignment nodes which can assign values to the variable.
											   The sets are ordered by priority, but the order is dominated by EXECUTING nodes.
											   Essentially, at each quiescence cycle, the first node in each set that isn't already
											   in state EXECUTING gets added to the end of the queue. */
    ExecListenerHubId m_listener;
    std::map<std::string, PlexilNodeId> m_libraries;
  };
}

#endif
