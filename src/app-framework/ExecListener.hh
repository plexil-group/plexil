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

#ifndef PLEXIL_EXEC_LISTENER_HH
#define PLEXIL_EXEC_LISTENER_HH

#include "ExecListenerFilter.hh"
#include "NodeTransition.hh"

#include "pugixml.hpp"

#include <memory>
#include <vector>

namespace PLEXIL
{
  using ExecListenerFilterPtr = std::unique_ptr<ExecListenerFilter>;

  class Expression;
  class Node;
  class Value;

  //! @class ExecListener
  //! A base class for notifying external agents about exec state changes.
  //! Base class methods do nothing, so implementors can only override
  //! the ones they care about.
  //! @note Provides event filtering hooks.
  class ExecListener
  {
  public:

    //! Default constructor.
    ExecListener() = default;

    //! Constructor from configuration XML
    //! @param xml Pointer to the (shared) configuration XML describing this listener.
    //! @note This is the signature of the constructor called by the
    //!       ExecListenerFactory.
    ExecListener(pugi::xml_node const xml);

    //! Virtual destructor.
    virtual ~ExecListener() = default;

    //
    // API to Exec
    // See ExecListenerBase.hh
    //

    //! Notify that one or more nodes have changed state.
    //! @param Vector of node state transition info.
    void notifyOfTransitions(std::vector<NodeTransition> const &transitions) const;

    //! Notify that a variable assignment has been performed.
    //! @param dest The Expression being assigned to.
    //! @param destName A string naming the destination expression.
    //! @param value The value (in internal Exec representation) being assigned.
    void notifyOfAssignment(Expression const *dest,
                            std::string const &destName,
                            Value const &value) const;

    //
    // API to application
    //

    //! Notify that a new plan has been received by the Exec.
    //! @param plan The XML representation of the plan.
    void notifyOfAddPlan(pugi::xml_node const plan) const;

    //! Notify that a new library node has been received by the Exec.
    // @param libNode The XML representation of the plan.
    void notifyOfAddLibrary(pugi::xml_node const libNode) const;

    //
    // Lifecycle API which can be overridden by derived classes
    //

    //! Perform listener-specific initialization.
    //! @return true if successful, false otherwise.
    //! @note Default method simply returns true.
    virtual bool initialize();

    //! Perform listener-specific startup.
    //! @return true if successful, false otherwise.
    //! @note Default method simply returns true.
    virtual bool start();

    //! Perform listener-specific actions to stop.
    //! @note Default method does nothing.
    virtual void stop();

    //
    // Public configuration API
    //

    //! Set the filter of this instance.
    //! @param fltr Pointer to the filter.
    void setFilter(ExecListenerFilter *fltr);

  protected:

    //
    // API which may be overridden by derived classes
    //

    //! Notify that one or mode nodes have changed state.
    //! @param Vector of node state transition info.
    //! @note Current node states are accessible via the nodes.

    //! @note ExecListener provides a default method which calls
    //!      implementNotifyNodeTransition() (below). Derived classes
    //!      may implement their own methods as an optimization.
    virtual void
    implementNotifyNodeTransitions(std::vector<NodeTransition> const & /* transitions */) const;

    //
    // API to be implemented by derived classes
    //

    //! Notify that a node has changed state.
    //! @param Const reference to one NodeTransition record.
    //! @note The default method does nothing.
    //! @note Derived classes may either implement this method, or
    //!       implementNotifyNodeTransition() (above), as appropriate
    //!       to the application.
    virtual void
    implementNotifyNodeTransition(NodeTransition const & /* transition */) const;

    //! Notify that a plan has been received by the Exec.
    //! @param plan The XML representation of the plan.
    //! @note The default method does nothing.
    virtual void implementNotifyAddPlan(pugi::xml_node const /* plan */) const;

    //! Notify that a library node has been received by the Exec.
    //! @param libNode The XML representation of the plan.
    //! @note The default method does nothing.
    virtual void implementNotifyAddLibrary(pugi::xml_node const /* libNode */) const;

    //! Notify that a variable assignment has been performed.
    //! @param dest The Expression being assigned to.
    //! @param destName A string naming the destination expression.
    //! @param value The value (in internal Exec representation) being assigned.
    //! @note The default method does nothing.
    virtual void implementNotifyAssignment(Expression const * /* dest */,
                                           std::string const & /* destName */,
                                           Value const & /* value */) const;


    //
    // Shared API made available to derived classes
    //

    //! Construct the ExecListenerFilter specified by this listener's
    //! configuration XML.
    //! @return True if successful, false otherwise.
    //! @note The default method uses the ExecListenerFilterFactory
    //!       facility. Most applications should find that sufficient.
    virtual bool constructFilter();

    //! Access this listener's configuration XML.
    //! @return Const reference to the XML.
    pugi::xml_node const &getXml() const
    {
      return m_xml;
    }

    //
    // Shared member variables
    //

    //! This instance's filter.
    //! @see ExecListenerFilter
    //! @note Shared in case a derived class implements its own
    //!       constructFilter() method, or its own filters.
    ExecListenerFilterPtr m_filter;

  private:

    // Not implemented
    ExecListener(ExecListener const &) = delete;
    ExecListener(ExecListener &&) = delete;
    ExecListener &operator=(ExecListener const &) = delete;
    ExecListener &operator=(ExecListener &&) = delete;

    //
    // Private member variables
    //

    //! Configuration XML.
    pugi::xml_node const m_xml;
  };
}

#endif // PLEXIL_EXEC_LISTENER_HH
