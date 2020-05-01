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

#include "plexil-config.h"

#include "Error.hh"
#include "Logging.hh"

#if defined(HAVE_CASSERT)
#include <cassert>
#elif defined(HAVE_ASSERT_H)
#include <assert.h>
#endif

namespace PLEXIL
{
  static std::ostream *Error_os = 0;      /**<The stream to write all error data to. */
  static bool Error_throw = false;        /**<Set to throw exception. */
  static bool Error_printErrors = true;   /**<Set to print errors when detetected */
  static bool Error_printWarnings = true; /**<Set to print warnings */

  Error::Error(const Error& err)
    : std::exception(err),
      m_condition(err.m_condition), m_msg(err.m_msg), m_file(err.m_file), m_line(err.m_line) 
  {
  }

  Error::Error(const std::string& msg)
    : std::exception(),
      m_msg(msg), m_line(0) 
  {
  }

  Error::Error(const std::string& condition, const std::string& file, const int& line)
    : std::exception(),
      m_condition(condition), m_file(file), m_line(line) {
    if (Error_os == 0)
      Error_os = &(std::cerr);
  }

  Error::Error(const std::string& condition, const std::string& msg,
               const std::string& file, const int& line)
    : std::exception(),
      m_condition(condition), m_msg(msg), m_file(file), m_line(line) {
    if (Error_os == 0)
      Error_os = &(std::cerr);
  }

  Error::Error(const std::string& condition, const Error& exception,
               const std::string& file, const int& line)
    : std::exception(),
      m_condition(condition), m_msg(exception.m_msg), m_file(file), m_line(line) {
    if (Error_os == 0)
      Error_os = &(std::cerr);
  }

  Error::~Error() PLEXIL_NOEXCEPT
  {
  }

  Error& Error::operator=(const Error& err) 
  {
    std::exception::operator=(err);
    m_condition = err.m_condition;
    m_msg = err.m_msg;
    m_file = err.m_file;
    m_line = err.m_line;
    return *this;
  }

  char const *Error::what() const PLEXIL_NOEXCEPT
  {
    return m_msg.c_str();
  }

  /**
     @brief Compare two Errors.
  */
  bool Error::operator==(const Error& err) const 
  {
    return m_condition == err.m_condition &&
           m_msg == err.m_msg &&
           m_file == err.m_file &&
           m_line == err.m_line;
  }

  /**
     @brief Return true iff (if and only if) the two Errors
     "match": are the same except for possibly the line numbers.
  */
  bool Error::matches(const Error& err) const {
    return m_condition == err.m_condition &&
           m_msg == err.m_msg &&
           m_file == err.m_file;
  }

  void Error::handleAssert() {
    Logging::handle_message(Logging::LOG_ERROR, m_file.c_str(), m_line, m_msg.c_str());
    if (throwEnabled())
      throw *this;
    assert(false); // Need the stack to work backwards and look at state in the debugger
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

  void Error::print(std::ostream& ostr) const {
    ostr << "Error(";
    if (!m_condition.empty())
      ostr << '"' << m_condition << "\", \"";
    if (!m_msg.empty())
      ostr << m_msg << "\", \"";
    ostr << m_file << "\", " << m_line << ")";
  }

  //
  // Static member functions
  // 

  /**
     @brief Return true if printing warnings and false if not.
  */
  bool Error::displayWarnings()
  {
    return Error_printWarnings;
  }

  /**
   * Indicate that warnings should be printed when detected.
   */
  void Error::doDisplayWarnings()
  {
    Error_printWarnings = true;
  }

  /**
   * Indicate that warnings should not be printed.
   */
  void Error::doNotDisplayWarnings()
  {
    Error_printWarnings = false;
  }

  /**
   * Indicate that errors should throw exceptions rather than
   * complaining and aborting.
   */
  void Error::doThrowExceptions()
  {
    Error_throw = true;
  }

  /**
   * Indicate that errors should complain and abort rather than throw
   * exceptions.
   */
  void Error::doNotThrowExceptions()
  {
    Error_throw = false;
  }


  /**
   * Are errors set to throw exceptions?
   * @return true if so; false if errors will complain and abort.
   */
  bool Error::throwEnabled()
  {
    return Error_throw;
  }


  /**
     @brief Return whether all error information should be printed when detected.
  */
  bool Error::printingErrors() {
    return(Error_printErrors);
  }

  /**
     @brief Indicate that error information should be printed at detection.
  */
  void Error::doDisplayErrors() {
    Error_printErrors = true;
  }

  /**
     @brief Indicate that nothing should be printed when an error is detected.
  */
  void Error::doNotDisplayErrors() {
    Error_printErrors = false;
  }

  /**
     @brief Return the output stream to which error information should be sent.
  */
  std::ostream& Error::getStream() {
    if (Error_os == 0)
      Error_os = &(std::cerr);
    return *Error_os;
  }

  /**
     @brief Indicate where output related to errors should be directed.
  */
  void Error::setStream(std::ostream& ostr) {
    Error_os = &ostr;
  }

  std::ostream& operator<<(std::ostream& ostr, const Error& err) {
    err.print(ostr);
    return ostr;
  }

} // namespace PLEXIL
