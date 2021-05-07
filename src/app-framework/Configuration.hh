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

#ifndef PLEXIL_CONFIGURATION_HH
#define PLEXIL_CONFIGURATION_HH

#include "pugixml.hpp"

#include <string>
#include <vector>

//
// Helpers for parsing interface configuration data
//

namespace PLEXIL
{

  struct AdapterConf {
    AdapterConf()
      : xml(), typeName(), commandNames(), lookupNames(),
        defaultCommandAdapter(false),
        defaultLookupAdapter(false),
        plannerUpdateAdapter(false)
    {}

    pugi::xml_node xml;
    std::string typeName;

    std::vector<std::string> commandNames;
    std::vector<std::string> lookupNames;
    
    bool defaultCommandAdapter;
    bool defaultLookupAdapter;
    bool plannerUpdateAdapter;
  };

  struct CommandHandlerConf {
    CommandHandlerConf()
      : xml(), typeName(), commandNames(),
        defaultHandler(false)
    {}

    pugi::xml_node xml;
    std::string typeName;

    std::vector<std::string> commandNames;
    bool defaultHandler;
  };

  struct LookupHandlerConf {
    LookupHandlerConf()
      : xml(), typeName(), lookupNames(),
        defaultHandler(false)
    {}

    pugi::xml_node xml;
    std::string typeName;

    std::vector<std::string> lookupNames;
    bool defaultHandler;
  };

  struct ListenerConf {
    ListenerConf() = default;

    pugi::xml_node xml;
    std::string typeName;
  };

  //! Parse the configuration data for one adapter.
  //! @param configXml An XML element specifying the adapter's
  //!                  configuration.
  //! @return A newly constructed AdapterConf instance; null
  //!         if parsing encountered an error.
 AdapterConf *parseAdapterConfiguration(pugi::xml_node const configXml);

  //! Parse the configuration data for one command handler.
  //! @param configXml An XML element specifying the handler's
  //!                  configuration.
  //! @return A newly constructed CommandHandlerConf
  //!         instance; null if parsing encountered an error.
 CommandHandlerConf *parseCommandHandlerConfiguration(pugi::xml_node const configXml);

  //! Parse the configuration data for one lookup handler.
  //! @param configXml An XML element specifying the handler's
  //!                  configuration.
  //! @return A newly constructed LookupHandlerConf
  //!         instance; null if parsing encountered an error.
 LookupHandlerConf *parseLookupHandlerConfiguration(pugi::xml_node const configXml);

  //! Parse the configuration data for one Exec listener.
  //! @param configXml An XML element specifying the handler's
  //!                  configuration.
  //! @return A newly constructed ListenerConf instance; null if
  //!         parsing encountered an error.
  ListenerConf *parseListenerConfiguration(pugi::xml_node const configXml);

} // namespace PLEXIL

#endif // PLEXIL_CONFIGURATION_HH
