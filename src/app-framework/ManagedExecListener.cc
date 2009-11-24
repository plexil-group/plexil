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

#include "ManagedExecListener.hh"

#include "ExecListenerFactory.hh"
#include "InterfaceSchema.hh"
#include "tinyxml.h"

namespace PLEXIL
{
  //
  // ManagedExecListenerFilter
  //

  /**
   * @brief Constructor from configuration XML.
   */
  ManagedExecListener::ManagedExecListener(const TiXmlElement* xml)
    : ExecListener(),
      m_xml(xml)
  {
    if (xml != NULL)
      {
        const TiXmlElement * filterSpec = xml->FirstChildElement(InterfaceSchema::FILTER_TAG());
        if (filterSpec != NULL)
          {
            // Construct specified event filter
              
            const char* filterType = filterSpec->Attribute(InterfaceSchema::FILTER_TYPE_ATTR());
            assertTrueMsg(filterType != NULL,
                          "ManagedExecListener constructor: invalid XML: <"
                          << InterfaceSchema::FILTER_TAG()
                          << "> element without a "
                          << InterfaceSchema::FILTER_TYPE_ATTR()
                          << " attribute");
            ExecListenerFilterId f = 
              ExecListenerFilterFactory::createInstance(LabelStr(filterType),
                                                        filterSpec);
            assertTrue(f.isId(),
                       "ManagedExecListener constructor: failed to construct filter");
            this->setFilter(f);
          }
      }
  }

  /**
   * @brief Destructor.
   */
  ManagedExecListener::~ManagedExecListener()
  {
  }

  //
  // ManagedExecListenerFilter
  //

  /**
   * @brief Constructor from configuration XML.
   */
  ManagedExecListenerFilter::ManagedExecListenerFilter(TiXmlElement* xml)
    : ExecListenerFilter(),
      m_xml(xml)
  {
  }

  /**
   * @brief Destructor from configuration XML.
   */
  ManagedExecListenerFilter::~ManagedExecListenerFilter()
  {
  }

}
