/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#ifndef LISTENER_FILTERS_HH
#define LISTENER_FILTERS_HH

#include "ExecListenerFilter.hh"

namespace PLEXIL
{

  /**
   * @class NodeStateFilter
   * @brief Limits reporting of node state transitions to a subset of node states.
   */
  class NodeStateFilter : public ExecListenerFilter
  {
  public:
    NodeStateFilter(pugi::xml_node const xml);

    virtual ~NodeStateFilter();

    /**
     * @brief Determine whether this node transition event should be reported.
     * @param prevState The node's previous state.
     * @param node Smart pointer to the node that changed state.
     * @return true to notify on this event, false to ignore it.
     * @note The default method simply returns true.
     */
    bool reportNodeTransition(NodeState prevState, Node *node);

  private:

    //
    // Deliberately unimplemented
    //
    NodeStateFilter();
    NodeStateFilter(const NodeStateFilter&);
    NodeStateFilter& operator=(const NodeStateFilter&);

    bool m_stateEnabled[NODE_STATE_MAX];
  };

}

#endif // LISTENER_FILTERS_HH

