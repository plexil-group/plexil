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

#ifndef EVENT_FORMATTER_H
#define EVENT_FORMATTER_H

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include "tinyxml.h"

#include "Id.hh"
#include "Node.hh"

// ACE/TAO includes
#include "tao/Version.h"
#if (TAO_MAJOR_VERSION == 1)
#if (TAO_MINOR_VERSION == 4)
// TAO 1.4.x
#if (TAO_BETA_VERSION <= 7)
#include "tao/Any.h"
#else
#include "tao/AnyTypeCode/Any.h"
#endif
#elif (TAO_MINOR_VERSION >= 5)
// TAO 1.5.x
#include "tao/AnyTypeCode/Any.h"
#endif
#endif

namespace PLEXIL
{
  // forward references
  class EventFormatter;
  typedef Id<EventFormatter> EventFormatterId;

  // A virtual base class for defining event formatters
  class EventFormatter
  {
  public:
    DECLARE_STATIC_CLASS_CONST(char*,
			       EVENT_FORMATTER_TYPE,
			       "EventFormatterType");

    EventFormatter(const TiXmlElement* xml)
      : m_id(this),
	m_xml(xml)
    {
    }

    virtual ~EventFormatter()
    {
      m_id.remove();
    }

    virtual CORBA::Any_ptr formatTransition(NodeState prevState,
					    const NodeId& node) const = 0;

    virtual CORBA::Any_ptr formatPlan(const PlexilNodeId& plan,
				      const LabelStr& parent) const = 0;

    EventFormatterId getId()
    {
      return m_id;
    }

    const TiXmlElement* getXml() const
    {
      return m_xml;
    }

  private:
    // deliberately unimplemented
    EventFormatter();
    EventFormatter(const EventFormatter &);
    EventFormatter & operator=(const EventFormatter &);
    
    // private member variables
    EventFormatterId m_id;
    const TiXmlElement* m_xml;
  };

}

#endif // EVENT_FORMATTER_H
