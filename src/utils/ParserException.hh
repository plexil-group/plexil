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

#ifndef PARSER_EXCEPTION_H
#define PARSER_EXCEPTION_H

#include "Error.hh" // PLEXIL_NOEXCEPT

#include <exception>
#include <sstream>

/**
 * @def reportParserException
 * @brief Unconditionally throw a ParserException with the given message
 * @param msg Anything suitable as the right-hand side of <<.
 */
#define reportParserException(msg) { \
    std::ostringstream whatstr; \
    whatstr << msg; \
    throw PLEXIL::ParserException(whatstr.str().c_str()); \
}

/**
 * @def checkParserException
 * @brief If the condition is false, throw a ParserException
 * @param cond The condition to test; if false, throw the exception
 * @param msg Anything suitable as the right-hand side of <<.
 */
#define checkParserException(cond, msg) { \
  if (!(cond)) \
    { \
      std::ostringstream whatstr; \
      whatstr << msg; \
      throw PLEXIL::ParserException(whatstr.str().c_str()); \
    } \
}

namespace PLEXIL
{

  struct ParserException : public std::exception
  {
    ParserException() PLEXIL_NOEXCEPT;
    ParserException(const char* msg) PLEXIL_NOEXCEPT;
    ParserException(const char* msg, const char* filename, int offset) PLEXIL_NOEXCEPT;
    ParserException(const char* msg, const char* filename, int line, int col) PLEXIL_NOEXCEPT;

    ParserException(ParserException const &) = default;
    ParserException(ParserException &&) PLEXIL_NOEXCEPT = default;

    ParserException& operator=(ParserException const &) = default;
    // g++ 4.8.x doesn't support PLEXIL_NOEXCEPT on default move assignment
    ParserException& operator=(ParserException &&) = default;

    virtual ~ParserException() PLEXIL_NOEXCEPT = default;

    virtual const char *what() const PLEXIL_NOEXCEPT override;

    std::string message;
    std::string file; /**<The source file in which the error was detected (__FILE__). */
    int line;         /**< Line number of the error */
	int column;       /**< The character offset of the error */
  };

}


#endif // PARSER_EXCEPTION_H
