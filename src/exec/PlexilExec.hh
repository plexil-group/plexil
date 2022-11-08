// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_EXEC_HH
#define PLEXIL_EXEC_HH

#include <list>
#include <memory>

namespace PLEXIL 
{

  // Forward declarations
  class Assignment;
  class CommandImpl;
  class Dispatcher;
  class ExecListenerBase; 
  class Node;
  class ResourceArbiterInterface;
  class Update;

  using NodePtr = std::unique_ptr<Node>;

  //! \defgroup Exec-Core The PLEXIL Executive core

  //! \class PlexilExec
  //! \brief Stateless abstract base class defining the API of the %PLEXIL executive core.
  //! \ingroup Exec-Core

  /**
   * @brief The API of the core PLEXIL executive.
   */
  class PlexilExec
  {
  public:

    //! \brief Virtual destructor.
    virtual ~PlexilExec() = default;

    //
    // API to Node classes
    //

    //! \brief Consider a node for potential state transition.
    //! \param node Pointer to the node.
    //! \note Node's queue status must be QUEUE_NONE.
    virtual void addCandidateNode(Node *node) = 0;

    //! \brief Schedule this assignment for execution.
    //! \param assign Pointer to the Assignment.
    virtual void enqueueAssignment(Assignment *assign) = 0;

    //! \brief Schedule this assignment for retraction.
    //! \param assign Pointer to the Assignment.
    virtual void enqueueAssignmentForRetraction(Assignment *assign) = 0;

    //! \brief Schedule this command for execution.
    //! \param cmd Pointer to the CommandImpl.
    virtual void enqueueCommand(CommandImpl *cmd) = 0;

    //! \brief Schedule this command to be aborted.
    //! \param cmd Pointer to the CommandImpl.
    virtual void enqueueAbortCommand(CommandImpl *cmd) = 0;

    //! \brief Schedule this update for execution.
    //! \param update Pointer to the Update.
    virtual void enqueueUpdate(Update *update) = 0;

    //! \brief Mark node as finished and no longer eligible for execution.
    //! \param node Pointer to the Node.
    virtual void markRootNodeFinished(Node *node) = 0;

    //
    // API to application
    //

    //! \brief Set the dispatcher
    //! \param intf Pointer to the Dispatcher instance.
    virtual void setDispatcher(Dispatcher *intf) = 0;

    //! \brief Set the exec listener.
    //! \param listener Pointer to the listener instance.
    virtual void setExecListener(ExecListenerBase *listener) = 0;

    //! \brief Get the exec listener.
    //! \return Pointer to the listener instance.  May be null.
    virtual ExecListenerBase *getExecListener() = 0;

    //! \brief Get the command resource arbiter.
    //! \return Pointer to the arbiter instance.  May be null.
    virtual ResourceArbiterInterface *getArbiter() = 0;

    //! \brief Run a single "macro step" i.e. the entire quiescence cycle.
    //! \param startTime The time at which the step is run.  Used as the
    //!                  timestamp for node transitions in this step.
    virtual void step(double startTime) = 0;

    //! \brief Query whether the Exec needs to be stepped.
    //! \return True if the Exec needs to be stepped, false otherwise.
    virtual bool needsStep() const = 0;

    //! \brief Prepare the given plan for execution.
    //! \param root Pointer to the plan's root node.
    //! \return True if succesful, false otherwise.
    virtual bool addPlan(Node *root) = 0;

    //! \brief Delete any plans (root nodes) which have finished.
    virtual void deleteFinishedPlans() = 0;

    //! \brief Queries whether all plans are finished.
    //! \return true if at least one plan has been run and all have finished, false otherwise.
    virtual bool allPlansFinished() const = 0;

    //
    // Introspection
    //
    
    //! \brief Get the list of active plans.
    //! \return Const reference to the list of root nodes.
    virtual std::list<NodePtr> const &getPlans() const = 0;

  };

  //! \brief Global pointer to the active PlexilExec instance.
  //! \ingroup Exec-Core
  extern PlexilExec *g_exec;

  //! \brief Construct a PlexilExec instance.
  //! \return Pointer to the new PlexilExec instance.
  //! \ingroup Exec-Core
  extern PlexilExec *makePlexilExec();

}

#endif
