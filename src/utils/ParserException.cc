/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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
#include <cstring>
#include <iostream>

namespace PLEXIL
{

  ParserException::ParserException()
    throw ()
    : std::exception(), 
      m_what("Unspecified parser exception"),
      m_offset(0)
  {
  }

  // Must copy the message as it may be stack or dynamically allocated.
  // *** N.B. This is an obvious source of memory leaks.
  ParserException::ParserException(const char * msg, const char * file, const int& offset)
    throw()
    : std::exception(), m_what(new char[strlen(msg) + 1]), m_file(file), m_offset(offset)
  {
    strcpy(const_cast<char*>(m_what), msg);
    Logging::handle_message(Logging::LOG_ERROR, file, offset, m_what);
  }

  ParserException::ParserException(const ParserException& other, const char * file, const int& offset)
    throw()
    : std::exception(other), m_what(other.m_what), m_file(file), m_offset(offset)
  {
    Logging::handle_message(Logging::LOG_ERROR, file, offset, m_what);
  }
  
  ParserException& ParserException::operator=(const ParserException& other)
    throw()
  {
    this->std::exception::operator=(other);
    m_what = other.m_what;
    m_offset = other.m_offset;
    return *this;
  }

  ParserException::~ParserException()
  throw()
  {
  }

  const char* ParserException::what() const
    throw()
  {
    return m_what;
  }

}
