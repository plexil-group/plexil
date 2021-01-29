// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "PlexilSimResponseFactory.hh"

#include "Agenda.hh"
#include "CommandResponseManager.hh"
#include "GenericResponse.hh"
#include "LineInStream.hh"
#include "ResponseFactory.hh"
#include "ResponseMessage.hh"

#include "Debug.hh"
#include "timeval-utils.hh"

using PLEXIL::Value;
using PLEXIL::ValueType;

struct PlexilSimResponseFactory final : public ResponseFactory
{
public:
  bool parseTelemetryReturn(Agenda *agenda,
                            LineInStream &instream,
                            std::string const &name,
                            ValueType returnType)
  {
    debugMsg("SimulatorScriptReader:parseTelemetryReturn", ' ' << name);
    
    // Construct the ResponseMessage and add it to the agenda
    double delay;
    std::istringstream &linestream = instream.getLineStream();
    linestream >> delay;
    if (linestream.fail()) {
      std::cerr << "Error: file " << instream.getFileName()
                << ", line " << instream.getLineCount()
                << ": parse error in telemetry delay for "
                << name
                << std::endl;
      return false;
    }
    Value returnValue;
    // Return value is on next line
    instream.getLine();
    returnValue = parseReturnValue(instream, returnType);
    // Error handling??

    timeval timeDelay = doubleToTimeval(delay);
    debugMsg("SimulatorScriptReader:readScript",
             " Adding telemetry for " << name << " value " << returnValue
             << " at delay " << timeDelay.tv_sec << '.'
             << std::setw(6) << std::setfill('0') << timeDelay.tv_usec);

    agenda->scheduleResponse(timeDelay,
                             new ResponseMessage(name, returnValue, MSG_TELEMETRY));
    return true;
  }

  bool parseCommandReturn(CommandResponseManager *mgr,
                          LineInStream &instream,
                          std::string const &name,
                          ValueType returnType)
  {
    debugMsg("SimulatorScriptReader:parseCommandReturn", ' ' << name);
    
    // Construct the GenericResponse and add it to the manager map
    unsigned long commandIndex;
    unsigned int numOfResponses;
    double delay;
    std::istringstream &linestream = instream.getLineStream();
    linestream >> commandIndex;
    if (linestream.fail()) {
      std::cerr << "Error: file " << instream.getFileName()
                << ", line " << instream.getLineCount()
                << ": parse error in command index for "
                << name
                << std::endl;
      return false;
    }
    linestream >> numOfResponses;
    if (linestream.fail()) {
      std::cerr << "Error: file " << instream.getFileName()
                << ", line " << instream.getLineCount()
                << ": parse error in command number of responses for "
                << name
                << std::endl;
      return false;
    }
    linestream >> delay;
    if (linestream.fail()) {
      std::cerr << "Error: file " << instream.getFileName()
                << ", line " << instream.getLineCount()
                << ": parse error in command response delay for "
                << name
                << std::endl;
      return false;
    }

    Value returnValue;
    // Return value is on next line
    instream.getLine();
    if (instream.eof()) {
      std::cerr << "Error: file " << instream.getFileName()
                << ", line " << instream.getLineCount()
                << ": premature end of file reading return value for "
                << name
                << std::endl;
      return false;
    }

    // TODO: check IO errors
    returnValue = parseReturnValue(instream, returnType);
    // TODO: error handling

    timeval timeDelay = doubleToTimeval(delay);
    debugMsg("SimulatorScriptReader:readScript",
             " Adding command return for " << name
             << " index " << commandIndex
             << " at interval " << timeDelay.tv_sec << '.'
             << std::setw(6) << std::setfill('0') << timeDelay.tv_usec);

    GenericResponse *resp = new GenericResponse(name, returnValue, timeDelay, numOfResponses);
    mgr->addResponse(resp, commandIndex);
    return true;
  }
};

ResponseFactory *makePlexilSimResponseFactory()
{
  return new PlexilSimResponseFactory();
}
