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

//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software.

/**
   @file Error.cc
   @author Will Edgington

   @brief Numerous declarations related to error generation and handling.
*/

#ifndef _NO_ERROR_EXCEPTIONS_
/* Contains the rest of this file */

#include "Error.hh"
#include "Logging.hh"
#include <cassert>

std::ostream *Error::s_os = 0;
bool Error::s_throw = false;
bool Error::s_printErrors = true;
bool Error::s_printWarnings = true;

Error::Error(const std::string& condition, const std::string& file, const int& line)
  : m_condition(condition), m_file(file), m_type("Error"), m_line(line) {
  if (s_os == 0)
    s_os = &(std::cerr);
}

Error::Error(const std::string& condition, const std::string& msg,
             const std::string& file, const int& line)
  : m_condition(condition), m_msg(msg), m_file(file), m_type("Error"), m_line(line) {
  if (s_os == 0)
    s_os = &(std::cerr);
}

Error::Error(const std::string& condition, const Error& exception,
             const std::string& file, const int& line)
  : m_condition(condition), m_msg(exception.getMsg()), m_file(file), m_type("Error"), m_line(line) {
  if (s_os == 0)
    s_os = &(std::cerr);
}

Error::Error(const std::string& condition, const std::string& msg, const std::string& type,
             const std::string& file, const int& line)
  : m_condition(condition), m_msg(msg), m_file(file), m_type(type), m_line(line) {
  if (s_os == 0)
    s_os = &(std::cerr);
}

void Error::handleAssert() {
  Logging::handle_message(Logging::LOG_ERROR, m_file.c_str(), m_line, m_msg.c_str());
  if (throwEnabled())
    throw *this;
  assert(false); // Need the stack to work backwards and look at state in the debugger
}

void Error::setCause(const std::string& condition, const std::string& file, const int& line) {
  m_condition = condition;
  m_file = file;
  m_line = line;
  display();
}

void Error::display() {
  if (!printingErrors())
    return;
  std::cout.flush();
  std::cerr.flush();
  getStream() << '\n' << m_file << ':' << m_line << ": Error: " << m_condition << " is false";
  if (!m_msg.empty())
    getStream() << "\n\t" << m_msg;
  getStream() << std::endl;
}

void Error::printWarning(const std::string& msg,
                         const std::string& file,
                         const int& line) {
  if (!displayWarnings())
    return;
  Logging::handle_message(Logging::WARNING, file.c_str(), line, msg.c_str());
}

void Error::print(std::ostream& os) const {
  os << "Error(\"" << m_condition << "\", \"";
  if (!m_msg.empty())
    os << m_msg << "\", \"";
  os << m_file << "\", " << m_line << ")";
}

Error::~Error() {
}

#endif /* _NO_ERROR_EXCEPTIONS_ */
