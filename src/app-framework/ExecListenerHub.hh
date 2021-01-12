/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "ExecListener.hh" // leaving this out causes compiler errors
#include "ExecListenerBase.hh"
#include "Value.hh"

#include <memory>

namespace PLEXIL
{
  //! @class ExecListenerHub
  //! A central dispatcher for multiple exec listeners.
  class ExecListenerHub : public ExecListenerBase
  {
  public:
    ExecListenerHub();
    virtual ~ExecListenerHub() = default;

    //
    // ExecListenerBase API to PlexilExec
    //

	//! Notify that some set of nodes has changed state.
	//! @param transitions Const reference to vector of node transition records.
    virtual void
    notifyOfTransitions(std::vector<NodeTransition> const &transitions) override;

    //! Notify that a variable assignment has been performed.
    //! @param dest The Expression being assigned to.
    //! @param destName A string naming the destination.
    //! @param value The value being assigned.
    virtual void notifyOfAssignment(Expression const *dest,
                                    std::string const &destName,
                                    Value const &value) override;

    //! Notify that a step is complete and the listener may publish
    //! transitions and assignments.
    virtual void stepComplete(unsigned int cycleNum) override;

    //
    // API to ExecApplication
    //

    //! Notify that a plan has been received by the Exec.
    //! @param plan The XML representation of the plan.
    void notifyOfAddPlan(pugi::xml_node const plan);

    //! Notify that a library node has been received by the Exec.
    //! @param libNode The XML representation of the plan.
    void notifyOfAddLibrary(pugi::xml_node const libNode);

    //
    // Interface management API to AdapterConfiguration
    // 

    //! Adds an Exec listener for publication of plan events.
    //! @param Pointer to an ExecListener instance.
    //! @note The ExecListenerHub takes ownership of the listener
    //!       instance, and will delete it when the hub is deleted.
    void addListener(ExecListener *listener);

    //! Initialize all the listeners registered with addListener().
    //! @return true if successful, false otherwise.
    bool initialize();

    //! Start all the registered listeners.
    //! @return true if successful, false otherwise.
    bool start();

    //! Stop all the registered listeners.
    void stop();

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

    // Local typedefs
    using ExecListenerPtr = std::unique_ptr<ExecListener>;

    // Deliberately unimplemented
    ExecListenerHub(ExecListenerHub const &) = delete;
    ExecListenerHub(ExecListenerHub &&) = delete;
    ExecListenerHub &operator=(ExecListenerHub const &) = delete;
    ExecListenerHub &operator=(ExecListenerHub &&) = delete;

    // Clients
    std::vector<ExecListenerPtr> m_listeners;

    // Queues
    std::vector<NodeTransition> m_transitions;
    std::vector<AssignmentRecord> m_assignments;
  };

}

#endif // EXEC_LISTENER_HUB_HH
