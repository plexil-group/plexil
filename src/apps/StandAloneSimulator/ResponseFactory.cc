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

#include "ResponseFactory.hh"

#include "LineInStream.hh"

using namespace PLEXIL;

Value parseReturnValue(LineInStream &instream, ValueType returnType)
{
  std::string const &line = instream.getLineStream().str();

  switch (returnType) {
  case BOOLEAN_TYPE: {
    Boolean bv;
    parseValue(line, bv);
    return Value(bv);
  }

  case INTEGER_TYPE: {
    Integer iv;
    parseValue(line, iv);
    return Value(iv);
  }

  case REAL_TYPE: {
    Real rv;
    parseValue(line, rv);
    return Value(rv);
  }

  case STRING_TYPE: {
    String sv;
    parseValue(line, sv);
    return Value(sv);
  }

  case COMMAND_HANDLE_TYPE:
    return Value(parseCommandHandleValue(line));

  case DATE_TYPE:
  case DURATION_TYPE:
  case ARRAY_TYPE:
  case BOOLEAN_ARRAY_TYPE:
  case INTEGER_ARRAY_TYPE:
  case REAL_ARRAY_TYPE:
  case STRING_ARRAY_TYPE:

    std::cerr << "Error: file " << instream.getFileName()
              << ", line " << instream.getLineCount()
              << ": unimplemented return value type "
              << valueTypeName(returnType)
              << std::endl;
    return Value();

  default:
    std::cerr << "Error: file " << instream.getFileName()
              << ", line " << instream.getLineCount()
              << ": invalid return value type " << returnType
              << std::endl;
    return Value();
  }
}
