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

#include "commandHandlerDefs.hh"
#include "AdapterConfiguration.hh"
#include "AdapterFactory.hh"
#include "AdapterExecInterface.hh"
#include "Command.hh"
#include "Debug.hh"
#include "InterfaceAdapter.hh"
#include "plan-utils.hh"

namespace PLEXIL
{

  //
  // Command implementation functions
  //

  static void utilityPrint1(Command *cmd, AdapterExecInterface *intf)
  {
    print(cmd->getArgValues());
    intf->handleCommandAck(cmd, COMMAND_SUCCESS);
    intf->notifyOfExternalEvent();
  }

  static void utilityPprint1(Command *cmd, AdapterExecInterface *intf)
  {
    pprint(cmd->getArgValues());
    intf->handleCommandAck(cmd, COMMAND_SUCCESS);
    intf->notifyOfExternalEvent();
  }    

  static void utilityPrintToString1(Command *cmd, AdapterExecInterface *intf)
  {
    intf->handleCommandReturn(cmd, printToString(cmd->getArgValues()));
    intf->handleCommandAck(cmd, COMMAND_SUCCESS);
    intf->notifyOfExternalEvent();
  }
  
  static void utilityPprintToString1(Command *cmd, AdapterExecInterface *intf)
  {
    intf->handleCommandReturn(cmd, pprintToString(cmd->getArgValues()));
    intf->handleCommandAck(cmd, COMMAND_SUCCESS);
    intf->notifyOfExternalEvent();
  }

  class UtilityAdapter : public InterfaceAdapter
  {
  public:

    UtilityAdapter(AdapterExecInterface &execInterface,
                   pugi::xml_node const configXml)
      : InterfaceAdapter(execInterface, configXml)
    {
      debugMsg("UtilityAdapter", " created.");
    }
    
    virtual bool initialize(AdapterConfiguration *config)
    {
      config->registerCommandHandler("print",
                                     (ExecuteCommandHandler) (&utilityPrint1));
      config->registerCommandHandler("pprint",
                                     (ExecuteCommandHandler) (&utilityPprint1));
      config->registerCommandHandler("printToString",
                                     (ExecuteCommandHandler) (&utilityPrintToString1));
      config->registerCommandHandler("pprintToString",
                                     (ExecuteCommandHandler) (&utilityPprintToString1));

      debugMsg("UtilityAdapter", " initialized.");
      return true;
    }

    // Adapter has no state, so just return true from each of these.
    
    virtual bool start()
    {
      return true;
    }

    virtual bool stop()
    {
      return true;
    }

    virtual bool reset()
    {
      return true;
    }

    virtual bool shutdown()
    {
      return true;
    }
  };

} // namespace PLEXIL

extern "C" {
  void initUtilityAdapter() {
    REGISTER_ADAPTER(PLEXIL::UtilityAdapter, "Utility");
  }
}
