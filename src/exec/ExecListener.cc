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

#include "ExecListener.hh"
#include "ExecListenerFilter.hh"
#include "ExecListenerFilterFactory.hh"
#include "Debug.hh"
#include "Expression.hh"
#include "InterfaceSchema.hh"
#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

namespace PLEXIL
{

  /**
   * @brief Default constructor.
   */
  ExecListener::ExecListener()
    : ExecListenerBase(),
	  m_id(this, ExecListenerBase::getId()), 
      m_filter()
  {
  }

  /**
   * @brief Constructor from configuration XML.
   */
  ExecListener::ExecListener(const TiXmlElement* xml)
    : ExecListenerBase(xml),
	  m_id(this, ExecListenerBase::getId()), 
      m_filter()
  {
    if (xml != NULL) {
	  const TiXmlElement * filterSpec = xml->FirstChildElement(InterfaceSchema::FILTER_TAG());
	  if (filterSpec != NULL) {
		// Construct specified event filter
              
		const char* filterType = filterSpec->Attribute(InterfaceSchema::FILTER_TYPE_ATTR());
		assertTrueMsg(filterType != NULL,
					  "ExecListener constructor: invalid XML: <"
					  << InterfaceSchema::FILTER_TAG()
					  << "> element without a "
					  << InterfaceSchema::FILTER_TYPE_ATTR()
					  << " attribute");
		ExecListenerFilterId f = 
		  ExecListenerFilterFactory::createInstance(LabelStr(filterType),
													filterSpec);
		assertTrue(f.isId(),
				   "ExecListener constructor: failed to construct filter");
		m_filter = f;
	  }
	}
  }

  /**
   * @brief Destructor.
   */
  ExecListener::~ExecListener() 
  { 
    m_id.removeDerived(ExecListenerBase::getId()); 
  }

  /**
   * @brief Notify that nodes have changed state.
   * @param Vector of node state transition info.
   */
  void ExecListener::notifyOfTransitions(const std::vector<NodeTransition>& transitions) const
  {
	debugMsg("ExecListener:notifyOfTransitions", " reporting " << transitions.size() << " transitions");
	this->implementNotifyNodeTransitions(transitions);
  }

  /**
   * @brief Notify that a plan has been received by the Exec.
   * @param plan The intermediate representation of the plan.
   * @param parent The name of the parent node under which this plan will be inserted.
   */
  void
  ExecListener::notifyOfAddPlan(const PlexilNodeId& plan, 
                                const LabelStr& parent) const
  {
    if (m_filter.isNoId()
        || m_filter->reportAddPlan(plan, parent))
      this->implementNotifyAddPlan(plan, parent);
  }

  /**
   * @brief Notify that a library node has been received by the Exec.
   * @param libNode The intermediate representation of the plan.
   */
  void 
  ExecListener::notifyOfAddLibrary(const PlexilNodeId& libNode) const
  {
    if (m_filter.isNoId()
        || m_filter->reportAddLibrary(libNode))
      this->implementNotifyAddLibrary(libNode);
  }

  /**
   * @brief Notify that a variable assignment has been performed.
   * @param dest The Expression being assigned to.
   * @param destName A LabelStr that names the destination.
   * @param value The value (in internal Exec representation) being assigned.
   */
  void
  ExecListener::notifyOfAssignment(const ExpressionId & dest,
                                   const std::string& destName,
                                   const double& value) const
  {
    if (m_filter.isNoId()
        || m_filter->reportAssignment(dest, destName, value))
      this->implementNotifyAssignment(dest, destName, value);
  }

  /**
   * @brief Perform listener-specific initialization.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool ExecListener::initialize()
  {
    return true; 
  }

  /**
   * @brief Perform listener-specific startup.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool ExecListener::start() 
  { 
    return true; 
  }

  /**
   * @brief Perform listener-specific actions to stop.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool ExecListener::stop() 
  {
    return true; 
  }

  /**
   * @brief Perform listener-specific actions to reset to initialized state.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool ExecListener::reset() 
  {
    return true; 
  }

  /**
   * @brief Perform listener-specific actions to shut down.
   * @return true if successful, false otherwise.
   * @note Default method provided as a convenience for backward compatibility.
   */
  bool ExecListener::shutdown() 
  { 
    return true; 
  }

  /**
   * @brief Set the filter of this instance.
   * @param fltr Smart pointer to the filter.
   */
  void ExecListener::setFilter(ExecListenerFilterId fltr)
  {
    m_filter = fltr;
  }

  //
  // Default methods to be overridden by derived classes
  //


  /**
   * @brief Notify that nodes have changed state.
   * @param Vector of node state transition info.
   * @note Current states are accessible via the node.
   * @note This default method is a convenience for backward compatibility.
   */
  void ExecListener::implementNotifyNodeTransitions(const std::vector<NodeTransition>& transitions) const
  {
    debugMsg("ExecListener:implementNotifyNodeTransitions", " default method called");
    if (m_filter.isNoId()) {
	  for (std::vector<NodeTransition>::const_iterator it = transitions.begin();
		   it != transitions.end();
		   it++) 
		this->implementNotifyNodeTransition(it->oldState, it->node);
	}
	else {
	  for (std::vector<NodeTransition>::const_iterator it = transitions.begin();
		   it != transitions.end();
		   it++)
		if (m_filter->reportNodeTransition(it->oldState, it->node))
		  this->implementNotifyNodeTransition(it->oldState, it->node);
	}
  }

  /**
   * @brief Notify that a node has changed state.
   * @param prevState The old state.
   * @param node The node that has transitioned.
   * @note The current state is accessible via the node.
   * @note The default method does nothing.
   */
  void ExecListener::implementNotifyNodeTransition(NodeState /* prevState */,
						   const NodeId& /* node */) const
  {
    debugMsg("ExecListener:implementNotifyNodeTransition", " default method called");
  }

  /**
   * @brief Notify that a plan has been received by the Exec.
   * @param plan The intermediate representation of the plan.
   * @param parent The name of the parent node under which this plan will be inserted.
   * @note The default method does nothing.
   */
  void ExecListener::implementNotifyAddPlan(const PlexilNodeId& /* plan */, 
					    const LabelStr& /* parent */) const
  {
    debugMsg("ExecListener:implementNotifyAddPlan", " default method called");
  }

  /**
   * @brief Notify that a library node has been received by the Exec.
   * @param libNode The intermediate representation of the plan.
   * @note The default method does nothing.
   */
  void ExecListener::implementNotifyAddLibrary(const PlexilNodeId& /* libNode */) const
  {
    debugMsg("ExecListener:implementNotifyAddLibrary", " default method called");
  }

  /**
   * @brief Notify that a variable assignment has been performed.
   * @param dest The Expression being assigned to.
   * @param destName A string naming the destination.
   * @param value The value (in internal Exec representation) being assigned.
   */
  void ExecListener::implementNotifyAssignment(const ExpressionId & /* dest */,
					       const std::string& /* destName */,
					       const double& /* value */) const
  {
    debugMsg("ExecListener:implementNotifyAssignment", " default method called");
  }
}
