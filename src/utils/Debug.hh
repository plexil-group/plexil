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

#include <iostream>

#ifdef NO_DEBUG_MESSAGE_SUPPORT

#define debugMsg(marker, data)
#define condDebugMsg(cond, marker, data)
#define debugStmt(marker, stmt)
#define condDebugStmt(cond, marker, stmt)
#define initDebug()
#define SHOW(thing)
#define MARK

// Dummies
inline bool setDebugOutputStream(std::ostream & /* os */)
{
  return true;
}

inline bool readDebugConfigStream(std::istream & /* is */)
{
  return true;
}

inline bool allDebugMessagesEnabled()
{
  return false;
}

inline void enableAllDebugMessages() 
{
}

inline void disableAllDebugMessages()
{
}

inline void enableMatchingDebugMessages(char const * /* file */,
                                        char const * /* marker */)
{
}

#else

/**
   @brief The SHOW() macro is intended as a convenience debugging tool
   inserted briefly and removed when certainty is restored.  It prints
   the name of the passed parameter and it's value, along with the file
   and line number where it occurs in the code.
 */

#define SHOW(thing) (std::cout << __FILE__ << "(" << __LINE__ << ") " << #thing << ": " << (thing) << std::endl)

/**
   @brief The MARK macro is intended as a convenience debugging tool
   inserted briefly and removed when certainty is restored.  It prints
   the file and line number where it occurs in the code.
 */

#define MARK (std::cout << __FILE__ << "(" << __LINE__ << ") MARK" << std::endl)

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
     dmPtr = addDebugMessage(__FILE__, marker); \
  if (dmPtr->enabled && (cond)) { \
    getDebugOutputStream() << "[" << marker << "]" << data << std::endl; \
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
  static DebugMessage *dmPtr = NULL; \
  if (!dmPtr) \
     dmPtr = addDebugMessage(__FILE__, marker); \
  if (dmPtr->enabled && (cond)) { \
    stmt ; \
  } \
}

//
// Public API
// 

// These are needed for initialization, so must be available
// whether or not debugging is defined to be on.

extern bool setDebugOutputStream(std::ostream &os);
extern bool readDebugConfigStream(std::istream &is);
extern bool allDebugMessagesEnabled();
extern void enableAllDebugMessages();
extern void disableAllDebugMessages();
extern void enableMatchingDebugMessages(char const *file,
                                        char const *marker);

// Forward references
struct DebugMessage;

//
// Internal to macros
//
extern std::ostream &getDebugOutputStream();

extern DebugMessage *addDebugMessage(char const *file,
                                     char const *marker);

/**
 * @struct DebugMessage
 * @brief Represents one debug marker in a source file.
 */

struct DebugMessage 
{
  /**
   * @brief Pointer to next (previous) message in list.
   */
  DebugMessage *next;

  /**
    @brief File given when this instance was created.
  */
  char const *file;

  /**
    @brief Marker given when this instance was created.
  */
  char const *marker;

  /**
    @brief Whether this instance is 'enabled' or not.
  */
  bool enabled;

  /**
   * @brief Construct a DebugMessage.
   * @param file File containing the debug message instance.
   * @param marker Name for the particular instance (not required to be unique within the process).
   * @note Only constructor that should be used.
   * @note Should only be called from static member functions.
   */
  DebugMessage(char const *f,
               char const *m);

  /**
   * @brief Destroy a DebugMessage.
   * @note Should only be called by purgePatternsAndMessages().
   */
  ~DebugMessage();

  /**
    @brief Print the data members of the debug message in a format
    that Emacs can use to display the corresponding source code.
    @param os
   */
  void print(std::ostream &os) const;

private:
  // Not implemented
  DebugMessage();
  DebugMessage(const DebugMessage&);
  DebugMessage& operator=(const DebugMessage&);
};

std::ostream &operator<<(std::ostream &os, const DebugMessage &dm);

#endif /* NO_DEBUG_MESSAGE_SUPPORT */

#endif /* _H_Debug */
