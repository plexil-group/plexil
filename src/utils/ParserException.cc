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


#include "ParserException.hh"
#include "Logging.hh"

#include <iostream>

namespace PLEXIL
{

  ParserException::ParserException() PLEXIL_NOEXCEPT
    : std::exception(), 
      message("Unspecified parser exception"),
      file(),
      line(0),
      column(0)
  {
  }

  // Must copy the message as it may be stack or dynamically allocated.
  ParserException::ParserException(const char * msg) PLEXIL_NOEXCEPT
    : std::exception(),
      message(),
      file(),
      line(0),
      column(0)
  {
    if (msg)
      message = msg;
    else
      message = "Message not specified";
    Logging::handle_message(Logging::LOG_ERROR, message.c_str());
  }
  
  // Used to report (e.g.) pugixml errors.
  ParserException::ParserException(const char * msg, const char * fyle, int offset) PLEXIL_NOEXCEPT
    : std::exception(),
      message(),
      file(),
      line(0),
      column(offset)
  {
    if (msg)
      message = msg;
    else
      message = "Message not specified";
    if (fyle)
      file = fyle;
    Logging::handle_message(Logging::LOG_ERROR, fyle, offset, message.c_str());
  }
  
  // When we have complete information about the location.
  ParserException::ParserException(const char * msg, const char * fyle, int lyne, int col) PLEXIL_NOEXCEPT
    : std::exception(),
      message(),
      file(),
      line(lyne),
      column(col)
  {
    if (msg)
      message = msg;
    else
      message = "Message not specified";
    if (fyle)
      file = fyle;
    Logging::handle_message(Logging::LOG_ERROR, fyle, lyne, col, message.c_str());
  }

  const char* ParserException::what() const PLEXIL_NOEXCEPT
  {
    return message.c_str();
  }

}
