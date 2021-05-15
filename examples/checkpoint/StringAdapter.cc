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

#include "stringFunctions.hh"

#include "AdapterConfiguration.hh"
#include "AdapterFactory.hh"
#include "AdapterExecInterface.hh"
#include "Command.hh"
#include "Debug.hh"
#include "Expression.hh"
#include "InterfaceAdapter.hh"
#include "LookupReceiver.hh"
#include "Value.hh"

#include <algorithm> //transform
#include <iostream>
#include <sstream>

#include <cstdlib> // strtol
#include <cmath> // HUGE_VAL

using namespace PLEXIL;

using std::cerr;
using std::endl;
using std::string;
using std::map;
using std::vector;
using std::copy;

///////////////////////////// Conveniences //////////////////////////////////

#define LOOKUP_HANDLER_WRAPPER(STRING_FN)               \
  [](const State &state, LookupReceiver *rcvr) -> void  \
  { rcvr->update(STRING_FN(state.parameters())); }

#define EXECUTE_COMMAND_WRAPPER(STRING_FN)              \
  [](Command *cmd, AdapterExecInterface *intf)          \
  { Value result = STRING_FN(cmd->getArgValues());     \
    if (result.isKnown()) {                             \
      intf->handleCommandReturn(cmd, result);           \
      intf->handleCommandAck(cmd, COMMAND_SUCCESS);}    \
    else {                                              \
      intf->handleCommandAck(cmd, COMMAND_FAILED);}     \
  }

///////////////////////////// Member functions //////////////////////////////////

class StringAdapter : public PLEXIL::InterfaceAdapter
{
public:
  StringAdapter(AdapterExecInterface& execInterface,
                               AdapterConf* config) :
    InterfaceAdapter(execInterface, config)
  {
    debugMsg("StringAdapter", " created.");
  }

  bool initialize(AdapterConfiguration *config)
  {
    config->registerCommandHandlerFunction("ToString",
                                           EXECUTE_COMMAND_WRAPPER(toStringFunction));
    config->registerCommandHandlerFunction("StringToInteger",
                                           EXECUTE_COMMAND_WRAPPER(stringToIntegerFunction));
    config->registerCommandHandlerFunction("StringToReal",
                                           EXECUTE_COMMAND_WRAPPER(stringToRealFunction));
    config->registerCommandHandlerFunction("StringToBoolean",
                                           EXECUTE_COMMAND_WRAPPER(stringToBooleanFunction));
  
    config->registerCommandHandlerFunction("substr",
                                           EXECUTE_COMMAND_WRAPPER(substrFunction));
    config->registerCommandHandlerFunction("strlwr",
                                           EXECUTE_COMMAND_WRAPPER(strlwrFunction));
    config->registerCommandHandlerFunction("strupr",
                                           EXECUTE_COMMAND_WRAPPER(struprFunction));
    config->registerCommandHandlerFunction("strindex",
                                           EXECUTE_COMMAND_WRAPPER(strindexFunction));
    config->registerCommandHandlerFunction("find_first_of",
                                           EXECUTE_COMMAND_WRAPPER(find_first_of_Function));
    config->registerCommandHandlerFunction("find_last_of",
                                           EXECUTE_COMMAND_WRAPPER(find_last_of_Function));

    config->registerLookupHandlerFunction("ToString",
                                          LOOKUP_HANDLER_WRAPPER(toStringFunction));
    config->registerLookupHandlerFunction("StringToInteger",
                                          LOOKUP_HANDLER_WRAPPER(stringToIntegerFunction));
    config->registerLookupHandlerFunction("StringToReal",
                                          LOOKUP_HANDLER_WRAPPER(stringToRealFunction));
    config->registerLookupHandlerFunction("StringToBoolean",
                                          LOOKUP_HANDLER_WRAPPER(stringToBooleanFunction));
  
    config->registerLookupHandlerFunction("substr",
                                          LOOKUP_HANDLER_WRAPPER(substrFunction));
    config->registerLookupHandlerFunction("strlwr",
                                          LOOKUP_HANDLER_WRAPPER(strlwrFunction));
    config->registerLookupHandlerFunction("strupr",
                                          LOOKUP_HANDLER_WRAPPER(struprFunction));
    config->registerLookupHandlerFunction("strindex",
                                          LOOKUP_HANDLER_WRAPPER(strindexFunction));
    config->registerLookupHandlerFunction("find_first_of",
                                          LOOKUP_HANDLER_WRAPPER(find_first_of_Function));
    config->registerLookupHandlerFunction("find_last_of",
                                          LOOKUP_HANDLER_WRAPPER(find_last_of_Function));
  
    debugMsg("StringAdapter", " initialized.");
    return true;
  }

  bool start()
  {
    debugMsg("StringAdapter", " started.");
    return true;
  }

  void stop()
  {
    debugMsg("StringAdapter", " stopped.");
  }

private:

  // Disallow default constructor, copy, move, assign
  StringAdapter() = delete;
  StringAdapter(const StringAdapter&) = delete;
  StringAdapter(StringAdapter&&) = delete;
  StringAdapter & operator=(const StringAdapter&) = delete;
  StringAdapter & operator=(StringAdapter&&) = delete;
  
};
  
// Necessary boilerplate
extern "C" {
  void initStringAdapter() {
    REGISTER_ADAPTER(StringAdapter, "StringAdapter");
  }
}
