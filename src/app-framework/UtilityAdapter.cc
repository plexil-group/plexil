/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

#include <iostream>
#include "AdapterFactory.hh"
#include "AdapterExecInterface.hh"
#include "BooleanVariable.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "UtilityAdapter.hh"
#include "plan-utils.hh"

namespace PLEXIL {

UtilityAdapter::UtilityAdapter(AdapterExecInterface& execInterface,
							   const pugi::xml_node& configXml) :
    InterfaceAdapter(execInterface, configXml)
{
  debugMsg("UtilityAdapter", " created.");
}

bool UtilityAdapter::initialize()
{
  m_execInterface.registerCommandInterface(LabelStr("print"), getId());
  m_execInterface.registerCommandInterface(LabelStr("pprint"), getId());
  debugMsg("UtilityAdapter", " initialized.");
  return true;
}

bool UtilityAdapter::start()
{
  debugMsg("UtilityAdapter", " started.");
  return true;
}

bool UtilityAdapter::stop()
{
  debugMsg("UtilityAdapter", " stopped.");
  return true;
}

bool UtilityAdapter::reset()
{
  debugMsg("UtilityAdapter", " reset.");
  return true;
}

bool UtilityAdapter::shutdown()
{
  debugMsg("UtilityAdapter", " shut down.");
  return true;
}

void UtilityAdapter::executeCommand (const LabelStr& command_name,
                                     const std::list<double>& args,
                                     ExpressionId dest,
                                     ExpressionId ack) 
{
  const std::string& name = command_name.toString();
  debugMsg("UtilityAdapter", "Received executeCommand for " << name);  

  if (name == "print") print (args);
  else if (name == "pprint") pprint (args);
  else std::cerr <<
         "Error in Utility Adapter: invalid command (should never happen!): "
                 << name << std::endl;

  m_execInterface.handleValueChange
    (ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());

  // Technically, this may be unnecessary, but is the closest equivalent of a
  // "void" return value.
  if (dest != ExpressionId::noId()) {
    m_execInterface.handleValueChange (dest, UNKNOWN());
  }

  m_execInterface.notifyOfExternalEvent();
}

void UtilityAdapter::invokeAbort(const LabelStr& command_name, 
                                 const std::list<double>& args, 
                                 ExpressionId abort_ack,
                                 ExpressionId cmd_ack)
{
  const std::string& name = command_name.toString();
  if (name != "print" && name != "pprint") {
    std::cerr << "Error in Utility Adapter: aborting invalid command \"" 
              << name << "\" (should never happen!)" << std::endl;
  }
  m_execInterface.handleValueChange(abort_ack, BooleanVariable::TRUE_VALUE());
  m_execInterface.notifyOfExternalEvent();
}

extern "C" {
  void initUtilityAdapter() {
    REGISTER_ADAPTER(UtilityAdapter, "UtilityAdapter");
  }
}

} // namespace PLEXIL
