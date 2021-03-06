/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_EXEC_LISTENER_FILTER_HH
#define PLEXIL_EXEC_LISTENER_FILTER_HH

#include "NodeConstants.hh"
#include "pugixml.hpp"

namespace PLEXIL
{
  // Forward references
  class Expression;
  class Node;
  struct NodeTransition;
  class Value;

  //* Abstract base class for defining transition event filters
  class ExecListenerFilter
  {
  public:
    /**
     * @brief Constructor.
     */
    ExecListenerFilter();

    /**
     * @brief Constructor from configuration XML.
     */
    ExecListenerFilter(pugi::xml_node const xml);

    /**
     * @brief Destructor.
     */
    virtual ~ExecListenerFilter() = default;

    /**
     * @brief Parses configuration XML.
     * @return true if successful, false otherwise.
     * @note Default method simply returns true.
     */
    virtual bool initialize();

    /**
     * @brief Get the configuration XML of this instance.
     * @return A const reference to the XML element.
     */
    pugi::xml_node const getXml() const
    {
      return m_xml;
    }

    /**
     * @brief Determine whether this node transition event should be reported.
     * @param transition Const reference to a transition record.
     * @return true to notify on this event, false to ignore it.
     * @note The default method simply returns true.
     */
    virtual bool reportNodeTransition(NodeTransition const &/* transition */);

    /**
     * @brief Determine whether this AddPlan event should be reported.
     * @param plan XML representation of the plan.
     * @return true to notify on this event, false to ignore it.
     * @note The default method simply returns true.
     */
    virtual bool reportAddPlan(pugi::xml_node const plan);

    /**
     * @brief Determine whether this AddLibraryNode event should be reported.
     * @param plan XML representation of the plan.
     * @return true to notify on this event, false to ignore it.
     * @note The default method simply returns true.
     */
    virtual bool reportAddLibrary(pugi::xml_node const plan);

    /**
     * @brief Determine whether this variable assignment should be reported.
     * @param dest The Expression being assigned to.
     * @param destName A string naming the destination.
     * @param value The value (as a generic Value) being assigned.
     */
    virtual bool reportAssignment(Expression const *dest,
                                  std::string const &destName,
                                  Value const &value);

  private:
    //
    // Deliberately unimplemented
    //
    ExecListenerFilter(const ExecListenerFilter &);
    ExecListenerFilter& operator=(const ExecListenerFilter &);

    //
    // Member variables
    //

    /**
     * @brief The configuration XML used at construction time.
     */
    const pugi::xml_node m_xml;
  };

}

#endif // PLEXIL_EXEC_LISTENER_FILTER_HH
