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

/**
  @file Debug.hh
  @author Europa project

  @brief Numerous declarations related to debugging.
*/

#ifndef _H_Debug
#define _H_Debug

#include "plexil-config.h"

#ifdef NO_DEBUG_MESSAGE_SUPPORT

#define debugMsg(marker, data)
#define condDebugMsg(cond, marker, data)
#define debugStmt(marker, stmt)
#define condDebugStmt(cond, marker, stmt)
#define initDebug()
#define SHOW(thing)
#define MARK
#define readDebugConfigStream(instream) (true)
#define setDebugOutputStream(outstream)

#else

#include <iostream>
#include <vector>

/**
   @brief The SHOW() macro is intended as a convenience debugging tool
   inserted briefly and removed when certainty is restored.  It prints
   the name of the passed parameter and it's value, along with the file
   and line number where it occurs in the code.
 */

#define SHOW(thing) (std::cout << __FILE__ << "(" << __LINE__ << ") " << #thing << ": " << (thing) << std::endl << std::flush)

/**
   @brief The MARK macro is intended as a convenience debugging tool
   inserted briefly and removed when certainty is restored.  It prints
   the file and line number where it occurs in the code.
 */

#define MARK std::cout << __FILE__ << "(" << __LINE__ << ") MARK" << std::endl << std::flush

/**
  @brief Use the debugMsg() macro to create a debug message that
  will be printed when the code is executed if and only if this
  particular debug message has been 'enabled'.
  @param marker A string that "marks" the message to enable it by.
  @param data The data to be printed when the message is enabled.
  @brief The data argument can be any "chain" of C++ expressions to
  output to the debug stream returned by DebugMessage::getStream()
  when this debug message is enabled (via, e.g. DebugMessage::enable()
  or DebugMessage::enableAll()).
  @see condDebugMsg
  @see debugStmt
  @see condDebugStmt
  @see DebugMessage
*/
#define debugMsg(marker, data) condDebugMsg(true, marker, data)

/**
  @brief Create a conditional debug message, which will
  only be created or used when the given condition is true at run time.
  @param cond An additional condition to be checked before printing the message,
         which can be any C/C++ expression that could be used in an if statement.
  @param marker A string that "marks" the message to enable it by.
  @param data The data to be printed when the message is enabled.
  @see debugMsg
  @see condDebugMsg
  @see debugStmt
  @see condDebugStmt
  @see DebugMessage
*/
#define condDebugMsg(cond, marker, data) { \
  static DebugMessage *dmPtr = NULL; \
  if (!dmPtr) \
     dmPtr = DebugMessage::addMsg(__FILE__, marker); \
  if (dmPtr->isEnabled() && (cond)) { \
    DebugMessage::getStream() << "[" << marker << "]" << data << std::endl; \
  } \
}

/**
  @brief Add code to be executed only if the DebugMessage is enabled.
  @param marker A string that "marks" the message to enable it by.
  @param stmt The code to be executed when the message is enabled.
  @see debugMsg
  @see condDebugMsg
  @see condDebugStmt
  @see DebugMessage
*/
#define debugStmt(marker, stmt) condDebugStmt(true, marker, stmt)

/**
  @brief Add code to be executed only if the DebugMessage is enabled and
         the condition is true.
  @param cond An additional condition to be checked before printing the message,
         which can be any C/C++ expression that could be used in an if statement.
  @param marker A string that "marks" the message to enable it by.
  @param stmt The code to be executed when the message is enabled and the condition
         is true.
  @see debugMsg
  @see condDebugMsg
  @see debugStmt
  @see DebugMessage
*/
#define condDebugStmt(cond, marker, stmt) { \
  static DebugMessage *dmPtr = DebugMessage::addMsg(__FILE__, marker); \
  if (dmPtr->isEnabled() && (cond)) { \
    stmt ; \
  } \
}

/**
 * @brief Load the debug configuration from the given stream
 * @param instream The input stream.
 * @return True if successful, false if error.
 */
#define readDebugConfigStream(instream) (DebugMessage::readConfigFile(instream))

/**
 * @brief Direct debug output to the given stream.
 * @param outstream The output stream.
 */
#define setDebugOutputStream(outstream) { DebugMessage::setStream(outstream); }

/**
 * @class DebugPattern Debug.hh
 * @brief Used to store the "patterns" of presently enabled debug messages.
 * @see DebugMessage::enableMatchingMsgs
 */
class DebugPattern
{
public:

  /**
   * @brief Destructor.
   */
  inline ~DebugPattern() 
  {
    if (m_garbage) {
      delete m_file;
      delete m_pattern;
    }
  }

  /**
   * @brief Constructor with data.
   * @note Should be the only constructor called explicitly.
   */
  inline DebugPattern(char const *f, char const *m, bool garbage = false)
    : m_next(NULL), 
      m_file(f),
      m_pattern(m),
      m_garbage(garbage)
  {
  }

  DebugPattern *m_next;

  /**
   * @brief The source file(s) that match the pattern.
   */
  char const *m_file;

  /**
   * @brief The markers that match the pattern.
   * @note Markers refer to those of class DebugMessage.
   * @see class DebugMessage
   */
  char const *m_pattern;

  /**
   * @brief True if the file and pattern should be deleted, false otherwise.
   */
  bool m_garbage;

  bool operator==(DebugPattern const &other) const;

private:
  // Not implemented
  DebugPattern();
  DebugPattern(DebugPattern const &);
  DebugPattern &operator=(DebugPattern const &);
};

/**
  @class DebugMessage Debug.hh
  @brief Implements support for debugMsg() macro, which should be used
  to create all instances.
*/
class DebugMessage 
{

private:

  /**
   * @brief Are all debug messages enabled?
   * @note Individual ones could be even when this is false.
   */
  static bool& allEnabled() {
    static bool s_allEnabled = false;
    return(s_allEnabled);
  }

  /**
   * @brief Construct a DebugMessage.
   * @param file File containing the debug message instance.
   * @param line Line on which it is declared/created.
   * @param marker Name for the particular instance (not required to be unique within the process).
   * @param enabled Whether the instance is enabled at creation.
   * @note Only constructor that should be used.
   * @note Should only be called from static member functions.
   */
  DebugMessage(char const *file,
               char const *marker,
               bool enabled = DebugMessage::allEnabled());
      
  /**
    @brief Enable matching debug messages, including those created later.
    @param file
    @param marker
    @par Errors thrown:
    @li If a message would be enabled but no debug stream has been set.
    @see DebugMessage::setStream
  */
  static void enableMatchingMsgs(char const *file,
                                 char const *marker);

public:

  /**
   * @brief Destroy a DebugMessage.
   * @note Should only be called by purgePatternsAndMessages().
   */
  inline virtual ~DebugMessage() 
  {
  }

  /**
    @brief Create a new DebugMessage.  Should only be called from the
    debugMsg() macro and readConfigFile().
    @param file
    @param marker
    @par Errors thrown:
    @li If no debug stream has been assigned.
    @see DebugMessage::enable
    @see DebugMessage::setStream
  */
  static DebugMessage *addMsg(char const *file,
                              char const *marker);

  /**
    @brief Find any matching DebugMessage.
    @param file
    @param pattern
  */
  static DebugMessage *findMsg(char const *file,
                               char const *pattern);

  /**
    @brief Find all matching DebugMessages and appends them to matches parameter
    without emptying it first.
    @param file
    @param pattern
    @param matches
  */
  static void findMatchingMsgs(char const *file,
                               char const *pattern,
                               std::vector<DebugMessage*>& matches);

  /**
    @brief Enable all debug messages, including ones not yet created.
    @par Errors thrown:
    @li If no debug stream has been assigned.
    @see DebugMessage::setStream
  */
  static void enableAll();

  /**
    @brief Assign a stream to which all debug messages will be sent.
    @param os
   */
  static void setStream(std::ostream& os);

  /**
   * @brief Return the stream being used for debug messages.
   * @note Doesn't seem to have any external callers.
   */
  static std::ostream& getStream();

  /**
    @brief Read a list of debug message enablements from the
    stream argument.
    @param is
    @par Errors thrown:
    @li If the stream is not good.
    @li If setStream() has not been called
    and some existing debug messages should be enabled.
   */
  static bool readConfigFile(std::istream& is);

  /**
    @brief Return whether the debug message is currently enabled.
   */
  inline bool isEnabled() const {
    return(m_enabled);
  }

  /**
    @brief Enable the debug message.
    @par Errors thrown:
    @li If the stream has not been set.
   */
  void enable();

  /**
    @brief Disable the debug message.
   */
  inline void disable() {
    m_enabled = false;
  }

  /**
    @brief Print the data members of the debug message in a format
    that Emacs can use to display the corresponding source code.
    @param os
   */
  void print(std::ostream &os = getStream()) const;

  /**
     @brief Whether the message is matched by the pattern.
  */
  bool matches(const DebugPattern& pattern) const;

  static bool isGood();

  DebugMessage *nextMsg() { return m_next; }

private:

  /**
   * @brief Pointer to next (previous) message in list.
   */
  DebugMessage *m_next;

  /**
    @brief File given when this instance was created.
  */
  char const *m_file;

  /**
    @brief Marker given when this instance was created.
  */
  char const *m_marker;

  /**
    @brief Whether this instance is 'enabled' or not.
  */
  bool m_enabled;

  /**
    @brief Should not be used.
  */
  DebugMessage();

  /**
    @brief Should not be used.
  */
  DebugMessage(const DebugMessage&);

  /**
    @brief Should not be used.
  */
  DebugMessage& operator=(const DebugMessage&);

  /**
    @brief Should not be used.
  */
  bool operator==(const DebugMessage&) const;

};

inline std::ostream& operator<<(std::ostream& os, const DebugMessage& dm)
{
  dm.print(os);
  return(os);
}

#endif /* NO_DEBUG_MESSAGE_SUPPORT */

#endif /* _H_Debug */
