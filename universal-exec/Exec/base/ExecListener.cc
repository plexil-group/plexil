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

#include "ExecListener.hh"
#include "tinyxml.h"

namespace PLEXIL
{

  /**
   * @brief Default constructor.
   */
  ExecListener::ExecListener()
    : m_id(this), 
      m_xml(NULL),
      m_filter()
  {
  }

  /**
   * @brief Constructor from configuration XML.
   */
  ExecListener::ExecListener(const TiXmlElement* xml)
    : m_id(this),
      m_xml(xml),
      m_filter()
  {
  }

  /**
   * @brief Destructor.
   */
  ExecListener::~ExecListener() 
  { 
    m_id.remove(); 
  }

  /**
   * @brief Notify that a node has changed state.
   * @param prevState The old state.
   * @param node The node that has transitioned.
   * @note The current state is accessible via the node.
   */
  void 
  ExecListener::notifyOfTransition(const LabelStr& prevState, 
                                   const NodeId& node) const
  {
    if (m_filter.isNoId()
        || m_filter->reportNodeTransition(prevState, node))
      this->implementNotifyNodeTransition(prevState, node);
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
  // ExecListenerFilter methods
  //

  /**
   * @brief Constructor.
   */
  ExecListenerFilter::ExecListenerFilter(TiXmlElement* xml)
    : m_id(this),
      m_xml(xml)
  {
  }

  /**
   * @brief Destructor.
   */
  ExecListenerFilter::~ExecListenerFilter()
  {
    m_id.remove();
  }

  /**
   * @brief Determine whether this node transition event should be reported.
   * @param prevState The LabelStr naming the node's previous state.
   * @param node Smart pointer to the node that changed state.
   * @return true to notify on this event, false to ignore it.
   * @note The default method simply returns true.
   */
  bool 
  ExecListenerFilter::reportNodeTransition(const LabelStr& prevState, 
                                           const NodeId& node)
  {
    return true;
  }

  /**
   * @brief Determine whether this AddPlan event should be reported.
   * @param plan Smart pointer to the plan's intermediate representation.
   * @param parent The LabelStr naming the new plan's parent node.
   * @return true to notify on this event, false to ignore it.
   * @note The default method simply returns true.
   */
  bool
  ExecListenerFilter::reportAddPlan(const PlexilNodeId& plan,
                                    const LabelStr& parent)
  {
    return true;
  }

  /**
   * @brief Determine whether this AddLibraryNode event should be reported.
   * @param plan Smart pointer to the library's intermediate representation.
   * @return true to notify on this event, false to ignore it.
   * @note The default method simply returns true.
   */
  bool 
  ExecListenerFilter::reportAddLibrary(const PlexilNodeId& plan)
  {
    return true;
  }

}
