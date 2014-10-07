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

// !! Two doxygen comments for each #define depending on another #define is
// !!   a very bad idea.  There's no way to keep them in sync and no point
// !!   in doing so if doxygen is not using the C/C++ preprocessor.
// !! --wedgingt 2004 Oct 5

/**
   @file Error.hh
   @author Will Edgington

   @brief Numerous declarations related to error generation and handling.

   @note These macros should be used rather than direct calls to throw
   unless an existing Error is being "re-thrown" from a catch block.

   @note This is presently only the "interface for programmers";
   nothing has been tested.

   @note Think of the first section, #ifdef PLEXIL_FAST, as the
   "prototypes" and documentation as well as the "production" variant.
*/

#ifndef _H_Error
#define _H_Error

/* $Id: Error.hh,v 1.1 2006/08/14 22:05:45 miatauro Exp $ */

#include <iostream>
#include <string>
#include <sstream>

// Helper macro for telling compiler that error handler won't return
#ifdef __GNUC__
// This version for g++ and clang++
#define PLEXIL_NORETURN __attribute__((__noreturn__))
#else
// TODO: Support other compilers here
// fallback (no-op)
#define PLEXIL_NORETURN
#endif


/**
 * @def ALWAYS_FAIL
 * False.
 * @note Used as argument to assert() and similar functions to
 * make clear that the assertion will fail, throw an error, etc.
 */
#define ALWAYS_FAIL (false)

/**
 * @def DECLARE_ERROR
 * Declare an error as a function that returns a string naming itself.
 * @param error The error to declare.
 */
#define DECLARE_ERROR(error) \
  static const std::string& error() { \
    static const std::string sl_lblStr(#error); \
    return(sl_lblStr); \
  }

//
// assertTrue & friends
//

/**
 * @def assertTrue_1
 * @brief Test a condition and create an error if false.
 * @param cond Expression that yields a true/false result.
 */
#define assertTrue_1(cond) { \
  if (!(cond)) { \
    Error(#cond, __FILE__, __LINE__).handleAssert(); \
  } \
}

/**
 * @def assertTrue_2
 * @brief Test a condition and create an error if false.
 * @param cond Expression that yields a true/false result.
 * @param msg A string or Error instance.
 */
#define assertTrue_2(cond, msg) { \
  if (!(cond)) { \
    Error(#cond, msg, __FILE__, __LINE__).handleAssert(); \
  } \
}

/**
 * @def assertTrue_3
 * @brief Test a condition and create an error if false.
 * @param cond Expression that yields a true/false result.
 * @param msg1 A string.
 * @param msg2 A string.
 */
#define assertTrue_3(cond, msg1, msg2) {                \
  if (!(cond)) { \
    Error(#cond, msg1, msg2, __FILE__, __LINE__).handleAssert();     \
  } \
}

#define FOURTH_ARG(arg1, arg2, arg3, arg4, ...) arg4

/**
 * @def assertTrue
 * @brief Test a condition and create an error if false.
 * @deprecated Not fully portable C++; use assertTrue_1, _2, _3 instead.
 */
#define assertTrue(...) \
 FOURTH_ARG(__VA_ARGS__, \
            assertTrue_3(__VA_ARGS__), \
            assertTrue_2(__VA_ARGS__), \
            assertTrue_1(__VA_ARGS__))

/**
 * @def assertTrueMsg
 * @brief Test a condition and create an error if false.
 * @param cond Expression that yields a true/false result.
 * @param msg Anything suitable as the right-hand side of <<.
 */
#define assertTrueMsg(cond, msg) { \
  if (!(cond)) { \
    std::ostringstream sstr; \
    sstr << msg; \
    Error(#cond, sstr.str(), __FILE__, __LINE__).handleAssert(); \
  } \
}

#ifdef PLEXIL_FAST

/**
 * @def check_error
 * Check whether an error has occurred and create an error instance if so.
 * @param cond The condition to verify; if false, an error has occurred.
 * @param optarg Other values to pass to the class Error constructor when creating the error instance.
 * @note When PLEXIL_FAST is defined, these are ignored.
 */
#define check_error(...)
#define checkError(cond, msg)

/**
 * @def warn
 * Print a warning if such is enabled.
 * @param msg The information to print.
 * @note When PLEXIL_FAST is defined, these are ignored
 */
#define warn(msg)

#else

//
// check_error implementation
//

/**
 * @def check_error_1
 * @brief If the condition is false, throw an exception.
 * @param cond The condition to test.
 */
#define check_error_1(cond) { \
  if (!(cond)) { \
    Error(#cond, __FILE__, __LINE__).handleAssert(); \
  } \
}

/**
 * @def check_error_2
 * @brief If the condition is false, throw an exception.
 * @param cond The condition to test.
 * @param msg A string or Error instance.
 */
#define check_error_2(cond, msg) { \
  if (!(cond)) { \
    Error(#cond, msg, __FILE__, __LINE__).handleAssert(); \
  } \
}

/**
 * @def check_error_3
 * @brief If the condition is false, throw an exception.
 * @param cond The condition to test.
 * @param msg1 A string.
 * @param msg2 A string.
 */
#define check_error_3(cond, msg1, msg2) { \
  if (!(cond)) { \
    Error(#cond, msg1, msg2, __FILE__, __LINE__).handleAssert(); \
  } \
}


/**
 * @def check_error
 * @brief If the condition is false, throw an exceptino.
 * @param cond The condition to test.
 * @deprecated Not fully portable C++; use check_error_1, _2, _3 instead.
 * @see assertTrue, ALWAYS_FAIL
*/

#define check_error(...) \
 FOURTH_ARG(__VA_ARGS__, \
            check_error_3(__VA_ARGS__), \
            check_error_2(__VA_ARGS__), \
            check_error_1(__VA_ARGS__))

#define checkError(cond, msg) { \
  if (!(cond)) { \
    std::ostringstream sstr; \
    sstr << msg; \
    Error(#cond, sstr.str(), __FILE__, __LINE__).handleAssert(); \
  } \
}

#define warn(msg) {                                             \
      std::ostringstream sstr;                                   \
      sstr << msg;                                              \
      Error::printWarning(sstr.str(), __FILE__, __LINE__);      \
}

#endif /* PLEXIL_FAST */

/**
   @class Error
   @brief Used whenever a C++ exception is thrown.
*/
class Error {
public:

  /**
     @brief Build an Error object from the usual information: the false/failed
     condition and the source file and line where it was tested/detected.
  */
  Error(const std::string& condition, const std::string& file, const int& line);

  /**
     @brief Build an Error object from the information given, including an extra message.
  */
  Error(const std::string& condition, const std::string& msg, const std::string& file, const int& line);

  /**
     @brief Build an Error object from the information given, including another Error object's info.
  */
  Error(const std::string& condition, const Error& exception, const std::string& file, const int& line);

  /**
     @brief Build an Error object from the information given, including an extra message and type.
  */
  Error(const std::string& condition, const std::string& msg, const std::string& type,
        const std::string &file, const int& line);

  /**
     @brief Build an Error object from only a message.

     @note Should only be used when setCause() will be called before
     the Error is thrown.

     @note Never prints anything, unlike the other constructors.
     @see Error::setCause()
  */
  inline Error(const std::string& msg)
    : m_msg(msg), m_type("Error"), m_line(0) {
  }

  /**
     @brief Copy constructor.
  */
  inline Error(const Error& err)
  : m_condition(err.m_condition), m_msg(err.m_msg), m_file(err.m_file), m_line(err.m_line) {
  }

  /**
     @brief Assignment operator.
  */
  Error& operator=(const Error& err) {
    m_condition = err.m_condition;
    m_msg = err.m_msg;
    m_file = err.m_file;
    m_type = err.m_type;
    m_line = err.m_line;
    return(*this);
  }

  /**
     @brief Modify the Error's information as requested.

     @note Appears to not work reliably with g++ v2.95.2 or v3.2.2 just
     before throwing the same Error.
  */
  void setCause(const std::string& condition, const std::string& file, const int& line);

  /**
     @brief Modify the Error's additional message as requested.
  */
  inline void setMsg(const std::string& msg) {
    m_msg = msg;
  }

  /**
     @brief Get the Error's message.
  */
  inline const std::string& getMsg() const { return m_msg; }

  /**
     @brief Set the Error's type.
  */
  inline void setType(const std::string& type) {
    m_type = type;
  }

  /**
     @brief Get the Error's type.
  */
  inline const std::string& getType() const {
    return(m_type);
  }

  /**
     @brief Set the Error's file.
  */
  inline void setFile(const std::string& file) {
    m_file = file;
  }

  /**
     @brief Get the Error's file.
  */
  inline const std::string& getFile() const {
    return(m_file);
  }


  /**
     @brief Get the Error's line.
  */
  inline const int& getLine() const { return m_line; }

  /**
     @brief Return whether all error information should be printed when detected.
  */
  inline static bool& printingErrors() {
    return(s_printErrors);
  }

  /**
     @brief Indicate that error information should be printed at detection.
  */
  inline static void doDisplayErrors() {
    s_printErrors = true;
  }

  /**
     @brief Indicate that nothing should be printed when an error is detected.
  */
  inline static void doNotDisplayErrors() {
    s_printErrors = false;
  }

  /**
     @brief Return the output stream to which error information should be sent.
  */
  inline static std::ostream& getStream() {
    if (s_os == 0)
      s_os = &(std::cerr);
    return(*s_os);
  }

  /**
     @brief Indicate where output related to errors should be directed.
  */
  inline static void setStream(std::ostream& os) {
    s_os = &os;
  }

  /**
   * Display in "error format" (for Emacs, e.g.) on the stream (getStream()).
   */
  void display();

  /**
   * Print as if calling an Error constructor.
   */
  void print(std::ostream& os = std::cerr) const;

  /**
     @brief Compare two Errors.
  */
  inline bool operator==(const Error& err) const {
    return(m_condition == err.m_condition &&
           m_msg == err.m_msg &&
           m_file == err.m_file &&
           m_line == err.m_line);
  }

  /**
     @brief Return true iff (if and only if) the two Errors
     "match": are the same except for possibly the line numbers.
  */
  inline bool matches(const Error& err) const {
    return(m_condition == err.m_condition &&
           m_msg == err.m_msg &&
           m_file == err.m_file);
  }

  /**
   * The Error destructor.
   * @note Should only be used implicitly.
   */
  virtual ~Error();

  /**
     @brief Print a warning in the same format used by Error::display()
     unless printing warnings has been disabled.
  */
  static void printWarning(const std::string& msg,
                           const std::string& file,
                           const int& line);

  /**
     @brief Return true if printing warnings and false if not.
  */
  inline static bool& displayWarnings() {
    return(s_printWarnings);
  }

  /**
   * Indicate that warnings should be printed when detected.
   */
  inline static void doDisplayWarnings() {
    s_printWarnings = true;
  }

  /**
   * Indicate that warnings should not be printed.
   */
  inline static void doNotDisplayWarnings() {
    s_printWarnings = false;
  }

  /**
   * Indicate that errors should throw exceptions rather than
   * complaining and aborting.
   */
  inline static void doThrowExceptions() {
    s_throw = true;
  }

  /**
   * Indicate that errors should complain and abort rather than throw
   * exceptions.
   */
  inline static void doNotThrowExceptions() {
    s_throw = false;
  }

  /**
   * Are errors set to throw exceptions?
   * @return true if so; false if errors will complain and abort.
   */
  inline static bool throwEnabled() {
    return(s_throw);
  }

  DECLARE_ERROR(GeneralMemoryError);
  DECLARE_ERROR(GeneralUnknownError);

  /**
   * Actually throw the exception or complain and abort.
   * @note Which is done depends on throwEnabled().
   * @see throwEnabled
   */
  void handleAssert() PLEXIL_NORETURN;

private:

  std::string m_condition; /**<The condition that, being false, implies the error has occurred. */
  std::string m_msg; /**<Additional info about the error. */
  std::string m_file; /**<The source file in which the error was detected (__FILE__). */
  std::string m_type; /**<The type of the error as. */
  int m_line; /**<The source line on which the error detected (__LINE__). */
  static bool s_throw; /**<Set to throw exception. */
  static bool s_printErrors; /**<Set to print errors when detetected */
  static bool s_printWarnings; /**<Set to print warnings */
  static std::ostream *s_os; /**<The stream to write all error data to. */
  Error(); /**<The zero argument constructor should not be used. */
};

inline std::ostream& operator<<(std::ostream& os, const Error& err) {
  err.print(os);
  return(os);
}

#endif /* _H_Error */
