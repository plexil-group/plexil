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

#ifndef SAS_RESPONSE_FACTORY_HH
#define SAS_RESPONSE_FACTORY_HH

#include "Value.hh"

// Forward references
class Agenda;
class CommandResponseManager;
class LineInStream;

//! @struct ResponseFactory
//! Abstract base class for simulator script response parsers.
struct ResponseFactory
{
public:
  virtual ~ResponseFactory() = default;

  //! Parse and schedule one telemetry response.
  //! @param agenda The Agenda in which to schedule the response.
  //! @param instream The input stream.
  //! @param name The name of the telemetry item.
  //! @param returnType The expected type of the response.
  //! @return true if successfully parsed, false if not.
  virtual bool parseTelemetryReturn(Agenda *agenda,
                                    LineInStream &instream,
                                    std::string const &name,
                                    PLEXIL::ValueType returnType) = 0;

  //! Parse and schedule one command response.
  //! @param mgr The CommandResponseManager for this command name.
  //! @param instream The input stream.
  //! @param name The command name.
  //! @param returnType The expected type of the response.
  //! @return true if successfully parsed, false if not.
  virtual bool parseCommandReturn(CommandResponseManager *mgr,
                                  LineInStream &instream,
                                  std::string const &name,
                                  PLEXIL::ValueType returnType) = 0;

};

// Utility needed by implementors of the above
PLEXIL::Value parseReturnValue(LineInStream &instream,
                               PLEXIL::ValueType returnType);

#endif // SAS_RESPONSE_FACTORY_HH
