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

#include <list>

namespace PLEXIL 
{
  // Forward references
  class Assignment;
  class ExecListenerBase;
  class Node;

  /**
   * @brief The API of the core PLEXIL executive.
   */
  class PlexilExec
  {
  public:
    /**
     * @brief Default constructor.
     */
    PlexilExec()
    {
    }

    /**
     * @brief Destructor.
     */
    virtual ~PlexilExec()
    {
    }

    //
    // API to ExternalInterface
    //

    /**
     * @brief Prepare the given plan for execution.
     * @param The plan's root node.
     * @return True if succesful, false otherwise.
     */
    virtual bool addPlan(Node *root) = 0;

    /**
     * @brief Begins a single "macro step" i.e. the entire quiescence cycle.
     */
    virtual void step(double startTime) = 0; // *** FIXME ***

    /**
     * @brief Returns true if the Exec needs to be stepped.
     */
    virtual bool needsStep() const = 0;

    /**
     * @brief Set the ExecListener instance.
     */
    virtual void setExecListener(ExecListenerBase *l) = 0;

    /**
     * @brief Get the ExecListener instance.
     * @return The ExecListener. May be NULL.
     */
    virtual ExecListenerBase *getExecListener() = 0;

    /**
     * @brief Queries whether all plans are finished.
     * @return true if all finished, false otherwise.
     */
    virtual bool allPlansFinished() const = 0;

    /**
     * @brief Deletes any finished root nodes.
     */
    virtual void deleteFinishedPlans() = 0;

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

    /**
     * @brief Get the list of active plans.
     */
    virtual std::list<Node *> const &getPlans() const = 0;

  private:
    // Not implemented
    PlexilExec(PlexilExec const &);
    PlexilExec &operator=(PlexilExec const &);
  };

  // Global pointer to the exec instance
  extern PlexilExec *g_exec;

  /**
   * @brief Construct a PlexilExec instance.
   * @return Pointer to the new PlexilExec instance.
   */
  extern PlexilExec *makePlexilExec();

}

#endif
