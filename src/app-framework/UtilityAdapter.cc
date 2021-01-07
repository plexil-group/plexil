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

#include "UtilityAdapter.hh"

#include "AdapterConfiguration.hh"
#include "AdapterFactory.hh"        // REGISTER_ADAPTER() macro
#include "AdapterExecInterface.hh"
#include "Command.hh"
#include "Configuration.hh"
#include "Debug.hh"
#include "InterfaceAdapter.hh"
#include "plan-utils.hh"

#include <iostream>

namespace PLEXIL {

  //
  // Handlers for UtilityAdapter commands
  //

  void executePrintCommand(Command *cmd, AdapterExecInterface *intf)
  {
      print(cmd->getArgValues());
      intf->handleCommandAck(cmd, COMMAND_SUCCESS);
  }

  void executePPrintCommand(Command *cmd, AdapterExecInterface *intf)
  {
    pprint(cmd->getArgValues());
    intf->handleCommandAck(cmd, COMMAND_SUCCESS);
  }

  void executePrintToStringCommand(Command *cmd, AdapterExecInterface *intf)
  {
      intf->handleCommandReturn(cmd, printToString(cmd->getArgValues()));
      intf->handleCommandAck(cmd, COMMAND_SUCCESS);
  }

  void executePPrintToStringCommand(Command *cmd, AdapterExecInterface *intf)
  {
    intf->handleCommandReturn(cmd, pprintToString(cmd->getArgValues()));
    intf->handleCommandAck(cmd, COMMAND_SUCCESS);
  }

  class UtilityAdapter final : public InterfaceAdapter
  {
  public:
    UtilityAdapter(AdapterExecInterface& execInterface,
                   AdapterConf *conf)
      : InterfaceAdapter(execInterface, conf)
    {
      debugMsg("UtilityAdapter", " created.");
    }

    virtual ~UtilityAdapter() = default;

    bool initialize(AdapterConfiguration *config)
    {
      config->registerCommandHandlerFunction("print",
                                             executePrintCommand);
      config->registerCommandHandlerFunction("pprint",
                                             executePPrintCommand);
      config->registerCommandHandlerFunction("printToString",
                                             executePrintToStringCommand);
      config->registerCommandHandlerFunction("pprintToString",
                                             executePPrintToStringCommand);
      debugMsg("UtilityAdapter", " initialized.");
      return true;
    }
  };

  extern "C"
  void initUtilityAdapter()
  {
    REGISTER_ADAPTER(UtilityAdapter, "UtilityAdapter");
  }

} // namespace PLEXIL
