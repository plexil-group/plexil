/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_EXEC_HH
#define PLEXIL_EXEC_HH

#include "ExecConnector.hh"
#include "NodeTransition.hh"

#include <list>
#include <memory>

namespace PLEXIL 
{
  // Forward references
  class Expression;
  class ExecListenerBase;

  using NodePtr = std::unique_ptr<Node>;

  /**
   * @brief The API of the core PLEXIL executive.
   */
  class PlexilExec : public ExecConnector
  {
  public:
    /**
     * @brief Default constructor.
     */
    PlexilExec()
      : ExecConnector()
    {
    }

    /**
     * @brief Destructor.
     */
    virtual ~PlexilExec() = default;

    //
    // API to application
    //

    /**
     * @brief Set the ExecListener instance.
     */
    virtual void setExecListener(ExecListenerBase *l) = 0;

    /**
     * @brief Get the ExecListener instance.
     * @return The ExecListener. May be nullptr.
     */
    virtual ExecListenerBase *getExecListener() = 0;

    /**
     * @brief Get the list of active plans.
     */
    virtual std::list<NodePtr> const &getPlans() const = 0;

    /**
     * @brief Prepare the given plan for execution.
     * @param The plan's root node.
     * @return True if succesful, false otherwise.
     */
    virtual bool addPlan(Node *root) = 0;

    /**
     * @brief Queries whether all plans are finished.
     * @return true if all finished, false otherwise.
     */
    virtual bool allPlansFinished() const = 0;

    /**
     * @brief Deletes any finished root nodes.
     */
    virtual void deleteFinishedPlans() = 0;

    /**
     * @brief Returns true if the Exec needs to be stepped.
     */
    virtual bool needsStep() const = 0;

    /**
     * @brief Begins a single "macro step" i.e. the entire quiescence cycle.
     */
    virtual void step(double startTime) = 0; // *** FIXME: use real time type ***

    //
    // API to Node classes
    //

    /**
     * @brief Consider a node for potential state transition.
     * @param node Pointer to the node.
     * @note Node's queue status must be QUEUE_NONE.
     * @note Known callers are Node::notifyChanged(), PlexilExec::addPlan(), PlexilExec::getStateChangeNode().
     */
    virtual void addCandidateNode(Node *node) = 0;

    /**
     * @brief Schedule this assignment for execution.
     */
    virtual void enqueueAssignment(Assignment *assign) = 0;

    /**
     * @brief Schedule this assignment for retraction.
     */
    virtual void enqueueAssignmentForRetraction(Assignment *assign) = 0;

    /**
     * @brief Mark node as finished and no longer eligible for execution.
     */
    virtual void markRootNodeFinished(Node *node) = 0;

  private:

    // Not implemented
    PlexilExec(PlexilExec const &) = delete;
    PlexilExec(PlexilExec &&) = delete;
    PlexilExec &operator=(PlexilExec const &) = delete;
    PlexilExec &operator=(PlexilExec &&) = delete;
  };

  /**
   * @brief Construct a PlexilExec instance.
   * @return Pointer to the new PlexilExec instance.
   */
  extern PlexilExec *makePlexilExec();

}

#endif
