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

#include "RoboSimResponseFactory.hh"

#include "Agenda.hh"
#include "CommandResponseManager.hh"
#include "GenericResponse.hh"
#include "LineInStream.hh"
#include "parseType.hh"
#include "simdefs.hh"

#include "ArrayImpl.hh"
#include "Debug.hh"

#include <iomanip>

using namespace PLEXIL;

// Local helper function
static Value returnValueFromVector(std::vector<Real> const &values)
{
  switch (values.size()) {
  case 0:
    return Value();

  case 1:
    return Value(values.front());

  default:
    return Value(RealArray(values));
  }
}

struct RoboSimResponseFactory final : public ResponseFactory
{
public:
  RoboSimResponseFactory() = default;
  virtual ~RoboSimResponseFactory() = default;

  //! Parse and schedule one telemetry response.
  //! @param agenda The Agenda in which to schedule the response.
  //! @param instream The input stream.
  //! @param name The name of the telemetry item.
  //! @param returnType The expected type of the response.
  //! @return true if successfully parsed, false if not.
  virtual bool parseTelemetryReturn(Agenda *agenda,
                                    LineInStream &instream,
                                    std::string const &name,
                                    ValueType returnType)
  {
    timeval timeDelay;
    if (!parseTelemetryHeader(instream, timeDelay))
      return false;

    // Return values are on next line
    std::vector<Real> values;
    instream.getLine();
    std::istringstream &lineStream = instream.getLineStream();
    if (name == "RobotState") {
      const size_t NUMBER_OF_STATE_READINGS = 3;
      for (size_t i = 0; i < NUMBER_OF_STATE_READINGS; ++i) {
        Real eLevel;
        if (parseType<Real>(lineStream, eLevel))
          values.push_back(eLevel);
        else
          break;
      }
      if (values.size() != NUMBER_OF_STATE_READINGS) {
        std::cerr << "File " << instream.getFileName()
                  << ", line " << instream.getLineCount()
                  << ": unable to parse return value for \""
                  << name << "\"" << std::endl;
        return false;
      }
    }
    else {
      // Default case
      while (!lineStream.eof()) {
        Real retVal;
        if (parseType<Real>(lineStream, retVal)) {
          values.push_back(retVal);
        }
        else if (!lineStream.eof()) {
          std::cerr << "File " << instream.getFileName()
                    << ", line " << instream.getLineCount()
                    << ": unrecognized return value format for \""
                    << name << "\"" << std::endl;
          return false;
        }
      }
    }

    Value v = returnValueFromVector(values);
    debugMsg("RoboSimResponseFactory:parseTelemetryReturn",
             " Adding telemetry for " << name << " value " << v
             << " at delay " << timeDelay.tv_sec << '.'
             << std::setw(6) << std::setfill('0') << timeDelay.tv_usec);
    agenda->scheduleResponse(timeDelay,
                             new ResponseMessage(name, v, MSG_TELEMETRY));
    return true;
  }

  //! Parse and schedule one command response.
  //! @param mgr The CommandResponseManager for this command name.
  //! @param instream The input stream.
  //! @param name The command name.
  //! @param returnType The expected type of the response.
  //! @return true if successfully parsed, false if not.
  virtual bool parseCommandReturn(CommandResponseManager *mgr,
                                  LineInStream &instream,
                                  std::string const &name,
                                  ValueType returnType)
  {
    debugMsg("RoboSimResponseFactory:parseCommandReturn", ' ' << name);
    std::istream &lineStream = instream.getLineStream();

    unsigned long commandIndex;
    unsigned int numOfResponses;
    timeval timeDelay;

    if (!parseCommandResponseHeader(instream, commandIndex, numOfResponses, timeDelay))
      return false;

    // Return values are on next line
    instream.getLine();
    if (instream.eof()) {
      std::cerr << "Error: file " << instream.getFileName()
                << ", line " << instream.getLineCount()
                << ": premature end of file reading return value for "
                << name
                << std::endl;
      return false;
    }

    std::vector<Real> values;
    if (name == "QueryEnergySensor") {
      const size_t NUMBER_OF_ENERGY_LEVEL_READINGS = 5;
      for (size_t i = 0; i < NUMBER_OF_ENERGY_LEVEL_READINGS; ++i) {
        Real eLevel;
        if (parseType<Real>(lineStream, eLevel))
          values.push_back(eLevel);
        else
          break;
      }
      if (values.size() != NUMBER_OF_ENERGY_LEVEL_READINGS) {
        std::cerr << "File " << instream.getFileName()
                  << ", line " << instream.getLineCount()
                  << ": unable to parse return value for \""
                  << name << "\"" << std::endl;
        return false;
      }
    }
    else if ((name == "MoveUp")
             || (name == "MoveRight")
             || (name == "MoveDown")
             || (name == "MoveLeft")) {
      Integer returnValue;
      if (parseType<Integer>(lineStream, returnValue)) {
        values.push_back((Real) returnValue);
      }
      else {
        std::cerr << "File " << instream.getFileName()
                  << ", line " << instream.getLineCount()
                  << ": unable to parse return value for \""
                  << name << "\"" << std::endl;
        return false;
      }
    }
    else {
      // Default case
      while (!lineStream.eof()) {
        Real retVal;
        if (parseType<Real>(lineStream, retVal)) {
          values.push_back(retVal);
        }
        else if (!lineStream.eof()) {
          std::cerr << "File " << instream.getFileName()
                    << ", line " << instream.getLineCount()
                    << ": unrecognized return value format for \""
                    << name << "\"" << std::endl;
          return false;
        }
      }
    }

    Value v = returnValueFromVector(values);
    debugMsg("PlexilSimResponseFactory:parseCommandReturn",
             " Adding command return for " << name
             << " index " << commandIndex
             << " value " << v
             << " at interval " << timeDelay.tv_sec << '.'
             << std::setw(6) << std::setfill('0') << timeDelay.tv_usec);

    mgr->addResponse(new GenericResponse(name, v, timeDelay, numOfResponses),
                     commandIndex);
    return true;
  }
};

ResponseFactory *makeRoboSimResponseFactory()
{
  return new RoboSimResponseFactory();
}
