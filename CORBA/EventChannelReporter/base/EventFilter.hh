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

#ifndef EVENT_FILTER_H
#define EVENT_FILTER_H

#include "Id.hh"

namespace PLEXIL
{
  // fwd declarations
  class EventFilter;
  typedef Id<EventFilter> EventFilterId;

  class Node;
  typedef Id<Node> NodeId;

  class PlexilNode;
  typedef Id<PlexilNode> PlexilNodeId;

  class LabelStr;

  // Abstract base class for defining transition event filters
  class EventFilter
  {
  public:
    EventFilter()
      : m_id(this)
    {
    }

    virtual ~EventFilter()
    {
      m_id.remove();
    }

    virtual bool reportEvent(const LabelStr& prevState, const NodeId& node) const = 0;

    virtual bool reportAddPlan(const PlexilNodeId& plan,
			       const LabelStr& parent) const = 0;

    EventFilterId getId()
    {
      return m_id;
    }

  private:
    // deliberately unimplemented
    EventFilter(const EventFilter &);
    EventFilter & operator=(const EventFilter &);

    EventFilterId m_id;

  };

}

#endif // EVENT_FILTER_H
