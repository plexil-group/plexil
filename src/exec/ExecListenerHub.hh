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

#ifndef EXEC_LISTENER_HUB_HH
#define EXEC_LISTENER_HUB_HH

#include "Id.hh"
#include "ExecListenerBase.hh"

namespace PLEXIL
{

  class ExecListenerHub;
  typedef Id<ExecListenerHub> ExecListenerHubId;

  /**
   * @brief A central dispatcher for multiple exec listeners.
   */
  class ExecListenerHub
  {
  public:
	ExecListenerHub();
	virtual ~ExecListenerHub();

	const ExecListenerHubId& getId() const
	{
	  return m_id;
	}

	//
	// Interface management API
	// 

    /**
     * @brief Adds an Exec listener for publication of plan events.
     */
    void addListener(const ExecListenerBaseId& listener);

    /**
     * @brief Removes an Exec listener.
     */
    void removeListener(const ExecListenerBaseId& listener);

    //
    // API to Exec
    //

	/**
	 * @brief Notify that nodes have changed state.
	 * @param Vector of node state transition info.
	 * @note Current states are accessible via the node.
	 */
	void notifyOfTransitions(const std::vector<NodeTransition>& transitions) const;

    /**
     * @brief Notify that a plan has been received by the Exec.
     * @param plan The intermediate representation of the plan.
     * @param parent The name of the parent node under which this plan will be inserted.
     */
     void notifyOfAddPlan(const PlexilNodeId& plan, 
						  const LabelStr& parent) const;

    /**
     * @brief Notify that a library node has been received by the Exec.
     * @param libNode The intermediate representation of the plan.
     */
	void notifyOfAddLibrary(const PlexilNodeId& libNode) const;

    //not sure if anybody wants this
    //void notifyOfConditionChange(const NodeId& node,
    //                             const LabelStr& condition,
    //                             const bool value) const;

    /**
     * @brief Notify that a variable assignment has been performed.
     * @param dest The Expression being assigned to.
     * @param destName A string naming the destination.
     * @param value The value (in internal Exec representation) being assigned.
     */
	void notifyOfAssignment(const ExpressionId & dest,
							const std::string& destName,
							const double& value) const;

	//
	// API to InterfaceManager
	//

    /**
     * @brief Perform listener-specific initialization.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    bool initialize();

    /**
     * @brief Perform listener-specific startup.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    bool start();

    /**
     * @brief Perform listener-specific actions to stop.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    bool stop();

    /**
     * @brief Perform listener-specific actions to reset to initialized state.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    bool reset();

    /**
     * @brief Perform listener-specific actions to shut down.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    bool shutdown();

  private:
	// Deliberately unimplemented
	ExecListenerHub(const ExecListenerHub&);
	ExecListenerHub& operator=(const ExecListenerHub&);

	std::vector<ExecListenerBaseId> m_listeners;
	ExecListenerHubId m_id;
  };

}

#endif // EXEC_LISTENER_HUB_HH
