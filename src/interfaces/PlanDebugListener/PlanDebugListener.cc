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


#include "Debug.hh"
#include "Error.hh"
#include "ExecListener.hh"
#include "ExecListenerFactory.hh"
#include "NodeImpl.hh"
#include "NodeTransition.hh"

#include "pugixml.hpp"

#include <iomanip> // for setprecision

namespace PLEXIL
{

  //! @class PlanDebugListener
  //! Provides output from execution useful for debugging a PLEXIL plan.
  class PlanDebugListener final : public ExecListener
  {
  public:
    PlanDebugListener() = default;

    PlanDebugListener (pugi::xml_node const xml)
      : ExecListener(xml)
    {
    }

    virtual ~PlanDebugListener() = default;

    // For now, use the DebugMsg facilities (really intended for debugging the
    // *executive* and not plans) to display messages of interest.  Later, a more
    // structured approach including listener filters and a different user
    // interface may be in order.

    virtual void 
    implementNotifyNodeTransition(NodeTransition const &trans) const override
    {
      NodeImpl *node = dynamic_cast<NodeImpl *>(trans.node);
      assertTrueMsg(node,
                    "PlanDebugListener:implementNotifyNodeTransition: not a node");
      condDebugMsg((trans.newState == FINISHED_STATE),
                   "Node:clock",
                   " Node '" << node->getNodeId() <<
                   "' finished at " << std::fixed << std::setprecision(6) <<
                   node->getCurrentStateStartTime() << " (" <<
                   outcomeName(node->getOutcome()) << ")");
      condDebugMsg((trans.newState == EXECUTING_STATE),
                   "Node:clock",
                   " Node '" << node->getNodeId() <<
                   "' started at " << std::fixed << std::setprecision(6) <<
                   node->getCurrentStateStartTime());
    }
  };

  ExecListener *makePlanDebugListener()
  {
    return new PlanDebugListener();
  }

} // namespace PLEXIL
  
extern "C"
void initPlanDebugListener()
{
  REGISTER_EXEC_LISTENER(PLEXIL::PlanDebugListener, "PlanDebugListener");
}
