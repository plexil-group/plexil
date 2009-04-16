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

#include "CommonDefs.hh"
#include <string>
#include <vector>

namespace PLEXIL
{

  /**
   * @brief This class provides utilities related to the generic interface XML schema.
   */

  class InterfaceSchema
  {
  public:

    //
    // XML tags
    //

    DECLARE_STATIC_CLASS_CONST(char*, ADAPTOR_TAG, "Adaptor");
    DECLARE_STATIC_CLASS_CONST(char*, COMMAND_NAMES_TAG, "CommandNames");
    DECLARE_STATIC_CLASS_CONST(char*, DEFAULT_ADAPTOR_TAG, "DefaultAdaptor");
    DECLARE_STATIC_CLASS_CONST(char*, FUNCTION_NAMES_TAG, "FunctionNames");
    DECLARE_STATIC_CLASS_CONST(char*, INTERFACES_TAG, "Interfaces");
    DECLARE_STATIC_CLASS_CONST(char*, LISTENER_TAG, "Listener");
    DECLARE_STATIC_CLASS_CONST(char*, LOOKUP_NAMES_TAG, "LookupNames");
    DECLARE_STATIC_CLASS_CONST(char*, PLANNER_UPDATE_TAG, "PlannerUpdate");

    //
    // Attributes
    //

    DECLARE_STATIC_CLASS_CONST(char*, ADAPTOR_TYPE_ATTR, "AdaptorType");
    DECLARE_STATIC_CLASS_CONST(char*, LISTENER_TYPE_ATTR, "ListenerType");


    /**
     * Extract comma separated arguments from a character string.
     * @return pointer to vector of strings
     * @note Caller is responsible for disposing of the vector.
     */

    static std::vector<std::string> * parseCommaSeparatedArgs(const char * argString);

  };
}
