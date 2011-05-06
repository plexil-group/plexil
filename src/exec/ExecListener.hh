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

#ifndef _H_ExecListener
#define _H_ExecListener

#include "ExecDefs.hh"
#include "LabelStr.hh"

namespace PLEXIL
{
  // Forward references
  class Node;
  typedef Id<Node> NodeId;
  class PlexilNode;
  typedef Id<PlexilNode> PlexilNodeId;
  class ExecListener;
  typedef Id<ExecListener> ExecListenerId;
  class ExecListenerFilter;
  typedef Id<ExecListenerFilter> ExecListenerFilterId;
  class Expression;
  typedef Id<Expression> ExpressionId;

  /**
   * @brief An abstract base class for notifying external agents about exec state changes.
   */
  class ExecListener 
  {
  public:

    /**
     * @brief Default constructor.
     */
    ExecListener();

    /**
     * @brief Destructor.
     */
    virtual ~ExecListener();

    //
    // API to Exec
    //

    /**
     * @brief Notify that a node has changed state.
     * @param prevState The old state.
     * @param node The node that has transitioned.
     * @note The current state is accessible via the node.
     */
    virtual void notifyOfTransition(NodeState prevState, 
                                    const NodeId& node) const;

    /**
     * @brief Notify that a plan has been received by the Exec.
     * @param plan The intermediate representation of the plan.
     * @param parent The name of the parent node under which this plan will be inserted.
     */
    virtual void notifyOfAddPlan(const PlexilNodeId& plan, 
                                 const LabelStr& parent) const;

    /**
     * @brief Notify that a library node has been received by the Exec.
     * @param libNode The intermediate representation of the plan.
     */
    virtual void notifyOfAddLibrary(const PlexilNodeId& libNode) const;

    //not sure if anybody wants this
    //virtual void notifyOfConditionChange(const NodeId& node,
    //                                     const LabelStr& condition,
    //                                     const bool value) const;

    /**
     * @brief Notify that a variable assignment has been performed.
     * @param dest The Expression being assigned to.
     * @param destName A string naming the destination.
     * @param value The value (in internal Exec representation) being assigned.
     */
    virtual void notifyOfAssignment(const ExpressionId & dest,
                                    const std::string& destName,
                                    const double& value) const;

    //
    // API to be implemented by derived classes
    //

    /**
     * @brief Perform listener-specific initialization.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    virtual bool initialize();

    /**
     * @brief Perform listener-specific startup.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    virtual bool start();

    /**
     * @brief Perform listener-specific actions to stop.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    virtual bool stop();

    /**
     * @brief Perform listener-specific actions to reset to initialized state.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    virtual bool reset();

    /**
     * @brief Perform listener-specific actions to shut down.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    virtual bool shutdown();

    /**
     * @brief Get the ID of this instance.
     * @return The ID.
     */
    inline const ExecListenerId& getId() const
    { 
      return m_id; 
    }

    /**
     * @brief Set the filter of this instance.
     * @param fltr Smart pointer to the filter.
     */
    void setFilter(ExecListenerFilterId fltr);

  protected:

    //
    // API to be implemented by subclasses
    //

    /**
     * @brief Notify that a node has changed state.
     * @param prevState The old state.
     * @param node The node that has transitioned.
     * @note The current state is accessible via the node.
     * @note The default method does nothing.
     */
    virtual void implementNotifyNodeTransition(NodeState /* prevState */,
                                               const NodeId& /* node */) const;

    /**
     * @brief Notify that a plan has been received by the Exec.
     * @param plan The intermediate representation of the plan.
     * @param parent The name of the parent node under which this plan will be inserted.
     * @note The default method does nothing.
     */
    virtual void implementNotifyAddPlan(const PlexilNodeId& /* plan */, 
                                        const LabelStr& /* parent */) const;

    /**
     * @brief Notify that a library node has been received by the Exec.
     * @param libNode The intermediate representation of the plan.
     * @note The default method does nothing.
     */
    virtual void implementNotifyAddLibrary(const PlexilNodeId& /* libNode */) const;

    /**
     * @brief Notify that a variable assignment has been performed.
     * @param dest The Expression being assigned to.
     * @param destName A string naming the destination.
     * @param value The value (in internal Exec representation) being assigned.
     */
    virtual void implementNotifyAssignment(const ExpressionId & /* dest */,
                                           const std::string& /* destName */,
                                           const double& /* value */) const;

  private:

    /**
     * @brief The ID of this instance.
     */
    ExecListenerId m_id;

    /**
     * @brief The ID of this instance's filter.
     */
    ExecListenerFilterId m_filter;
    
  };
}

#endif // _H_ExecListener
