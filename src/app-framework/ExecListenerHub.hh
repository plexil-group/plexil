/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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

#include "PlexilListener.hh"
#include "Value.hh"

namespace PLEXIL
{
  class ExecListener;
  
  /**
   * @brief A central dispatcher for multiple exec listeners,
   *        and an adapter between the old ExecListener API and the current Exec API.
   */
  class ExecListenerHub : public PlexilListener
  {
  public:
    ExecListenerHub();
    virtual ~ExecListenerHub();

    //
    // Interface management API
    // 

    /**
     * @brief Adds an Exec listener for publication of plan events.
     */
    void addListener(ExecListener *listener);

    /**
     * @brief Removes an Exec listener.
     */
    void removeListener(ExecListener *listener);

    //
    // API to Exec
    //

    /**
     * @brief Notify that a plan has been received by the Exec.
     * @param plan The intermediate representation of the plan.
     */
    void notifyOfAddPlan(pugi::xml_node const plan);

    /**
     * @brief Notify that a library node has been received by the Exec.
     * @param libNode The intermediate representation of the plan.
     */
    void notifyOfAddLibrary(pugi::xml_node const libNode);

	/**
	 * @brief Notify that a node has changed state.
	 * @param node Pointer to the node.
     * @param oldState State being transitioned from.
     * @param newState State being transitioned to.
     * @note This is called synchronously from the inner loop of the Exec.
     *       Listeners should not do any I/O during this call.
	 */
    void notifyNodeTransition(Node *node,
                              NodeState oldState,
                              NodeState newState);

    /**
     * @brief Notify that a variable assignment has been performed.
     * @param dest The Expression being assigned to.
     * @param destName A string naming the destination.
     * @param value The value (in internal Exec representation) being assigned.
     * @note This is called synchronously from the inner loop of the Exec.
     *       Listeners should not do any I/O during this call.
     */
    void notifyOfAssignment(Expression const *dest,
                            std::string const &destName,
                            Value const &value);

    /**
     * @brief Notify that a step is complete and the listener
     *        may publish transitions and assignments.
     */
    void stepComplete(unsigned int cycleNum);

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

    // Internal data type
    struct AssignmentRecord {
      Value value;
      std::string destName;
      Expression const *dest;

      AssignmentRecord(Expression const *dst,
                       std::string const &name,
                       Value const &val)
        : value(val),
          destName(name),
          dest(dst)
      {}

      // use default destructor, copy constructor, assignment
    };

    // Deliberately unimplemented
    ExecListenerHub(const ExecListenerHub&);
    ExecListenerHub& operator=(const ExecListenerHub&);

    // Clients
    std::vector<ExecListener *> m_listeners;

    // Queues
    std::vector<NodeTransition> m_transitions;
    std::vector<AssignmentRecord> m_assignments;

  };

}

#endif // EXEC_LISTENER_HUB_HH
