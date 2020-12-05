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

#include <string>
#include <vector>

namespace PLEXIL
{

  /**
   * @brief This class provides utilities related to the generic interface XML schema.
   */
  struct InterfaceSchema
  {
    //
    // XML tags
    //

    static constexpr char const *ADAPTER_TAG = "Adapter";
    static constexpr char const *COMMAND_HANDLER_TAG = "CommandHandler";
    static constexpr char const *COMMAND_NAMES_TAG = "CommandNames";
    static constexpr char const *DEFAULT_ADAPTER_TAG = "DefaultAdapter";
    static constexpr char const *DEFAULT_COMMAND_ADAPTER_TAG = "DefaultCommandAdapter";
    static constexpr char const *DEFAULT_LOOKUP_ADAPTER_TAG = "DefaultLookupAdapter";
    static constexpr char const *FILTER_TAG = "Filter";
    static constexpr char const *INTERFACES_TAG = "Interfaces";
    static constexpr char const *INTERFACE_LIBRARY_TAG = "InterfaceLibrary";
    static constexpr char const *LIBRARY_NODE_PATH_TAG = "LibraryNodePath";
    static constexpr char const *LISTENER_TAG = "Listener";
    static constexpr char const *LOOKUP_HANDLER_TAG = "LookupHandler";
    static constexpr char const *LOOKUP_NAMES_TAG = "LookupNames";
    static constexpr char const *PLAN_PATH_TAG = "PlanPath";
    static constexpr char const *PLANNER_UPDATE_TAG = "PlannerUpdate";
    static constexpr char const *PLANNER_UPDATE_HANDLER_TAG = "PlannerUpdateHandler";
    static constexpr char const *IP_ADDRESS_TAG = "IpAddress";
    static constexpr char const *PORT_NUMBER_TAG = "PortNumber";

    //
    // Attributes
    //

    static constexpr char const *ADAPTER_TYPE_ATTR = "AdapterType";
    static constexpr char const *DEFAULT_HANDLER_ATTR = "DefaultHandler";
    static constexpr char const *FILTER_TYPE_ATTR = "FilterType";
    static constexpr char const *HANDLER_TYPE_ATTR = "HandlerType";
    static constexpr char const *LIB_PATH_ATTR = "LibPath";
    static constexpr char const *LISTENER_TYPE_ATTR = "ListenerType";
    static constexpr char const *NAME_ATTR = "Name";

    /**
     * @brief Extract comma separated arguments from a character string.
     * @return pointer to vector of strings
     * @note Caller is responsible for disposing of the vector.
     */
    static std::vector<std::string> * parseCommaSeparatedArgs(const char * argString);

  };
}
