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
  @file debug.cc
  @brief Define and implement variables and functions related to
  debugging and profiling.
*/

#include "Debug.hh"

#ifdef NO_DEBUG_MESSAGE_SUPPORT

#else // !NO_DEBUG_MESSAGE_SUPPORT

#include "Error.hh"
#include "lifecycle-utils.h"

#include <fstream>

#include <cstdlib> // for free()
#include <cstring>

using std::string;

//
// Forward references
//
struct DebugPattern;

//
// Implementation variables
//

/**
 * @brief The debug output stream.
 */
static std::ostream *debugStream = NULL;

/**
 * @brief Linked list of debug messages.
 */
static DebugMessage *allMsgs = NULL;

/**
 * @brief Linked list of all enabled debug patterns.
 */
static DebugPattern *enabledPatterns = NULL;

/**
 * @brief All-messages-enabled flag
 */
static bool allEnabled = false;

/**
 * @brief One-time-initialization flag.
 */
static bool debugInited = false;

//
// Errors specific to debug message facility
//

class DebugErr
{
public:
  DECLARE_ERROR(DebugConfigError);
  DECLARE_ERROR(DebugInternalError);
  DECLARE_ERROR(DebugMessageError);
  DECLARE_ERROR(DebugMemoryError);
};

//
// Structure definitions
//

/**
 * @struct DebugPattern
 * @brief Used to store the "patterns" of presently enabled debug messages.
 * @see enableMatchingDebugMsgs
 */
struct DebugPattern
{
  /**
   * @brief The source file substring the DebugMessage must match.
   */
  char *file;

  /**
   * @brief The marker substring the DebugMessage must match.
   * @note Markers refer to those of class DebugMessage.
   * @see class DebugMessage
   */
  char *pattern;

  /**
   * @brief Link to next DebugPattern in the list.
   */
  DebugPattern *next;

  /**
   * @brief Constructor with data.
   */
  DebugPattern(char const *f, char const *m, bool garbage = false)
    : file(strdup(f)),
      pattern(strdup(m)),
      next(NULL)
  {
    assertTrue_3(file && pattern,
                 "DebugPattern constructor: not enough memory to copy argument strings",
                 DebugErr::DebugMemoryError());
  }

  /**
   * @brief Destructor.
   */
  ~DebugPattern() 
  {
    free(file);
    free(pattern);
  }

private:

  // Not implemented
  DebugPattern();
  DebugPattern(DebugPattern const &);
  DebugPattern &operator=(DebugPattern const &);
};

// Take advantage of the fact that the DebugMessage constructor will usually be called 
// with literal strings, so we don't have to manage the storage.

DebugMessage::DebugMessage(char const *f,
                           char const *m)
  : next(NULL),
    file(f), 
    marker(m),
    enabled(false)
{
}

DebugMessage::~DebugMessage() 
{
}

void DebugMessage::print(std::ostream &os) const
{
  try {
    os.exceptions(std::ostream::badbit);
    os << file << ':' << marker << ' ';
  }
  catch (std::ios_base::failure& exc) {
    check_error_2(ALWAYS_FAIL, exc.what());
    throw;
  }
}

//
// Utility functions
//

static void deleteAllDebugPatterns()
{
  DebugPattern *nextPat = enabledPatterns;
  enabledPatterns = NULL;
  while (nextPat) {
    DebugPattern *pat = nextPat;
    nextPat = pat->next;
    delete pat;
  }
}

static void debugCleanup()
{
  deleteAllDebugPatterns();
  DebugMessage *nextMessage = allMsgs;
  allMsgs = NULL;
  while (nextMessage) {
    DebugMessage *msg = nextMessage;
    nextMessage = msg->next;
    delete msg;
  }
}

static void ensureDebugInited()
{
  if (debugInited)
    return;
  addFinalizer(&debugCleanup);
  debugStream = &std::cout;
  allMsgs = NULL;
  enabledPatterns = NULL;
  allEnabled = false;
  debugInited = true;
}

/**
 *  @brief Whether the given marker string matches the pattern string.
 *  Exists solely to ensure the same method is always used to check
 *  for a match.
 */
static bool markerMatches(char const *marker, char const *pattern) 
{
  assertTrue_3(marker, "markerMatches: Null marker", DebugErr::DebugInternalError());
  if (!*marker)
    return true;
  assertTrue_3(pattern, "markerMatches: Null pattern", DebugErr::DebugInternalError());
  if (!*pattern)
    return true;
  char const *result = strstr(marker, pattern);
  return result != NULL;
}

/**
 * @brief Find the existing DebugMessage with exactly the given file and marker.
 * @return Pointer to the message, or NULL if not found.
 */
static DebugMessage *findDebugMessage(char const *file, char const *marker)
{
  for (DebugMessage *m = allMsgs; m; m = m->next)
    if (!strcmp(m->file, file) && !strcmp(m->marker, marker))
      return m;
  return NULL;
}

//
// Advertised public API
//

bool setDebugOutputStream(std::ostream &os)
{
  if (!os.good())
    return false;
  debugStream = &os;
  return true;
}

bool readDebugConfigStream(std::istream& is)
{
  static const char *sl_whitespace = " \f\n\r\t\v";
  static const char *sl_comment = ";#/";

  ensureDebugInited();

  assertTrue_3(is.good(),
               "Cannot read debug configuration from invalid/error'd stream",
               DebugErr::DebugConfigError());

  while (is.good() && !is.eof()) {
    string input;
    getline(is, input);
    if (input.empty())
      continue;

    // Find leftmost non-blank character
    string::size_type left = input.find_first_not_of(sl_whitespace);
    if (left == string::npos)
      continue; // line is all whitespace

    // Find trailing comment, if any
    string::size_type comment = input.find_first_of(sl_comment, left);
    if (comment == left)
      continue; // line is a comment

    // Trim whitespace before comment
    if (comment != string::npos)
      comment--; // start search just before comment
    string::size_type right = input.find_last_not_of(sl_whitespace, comment);
    right++; // point just past last non-blank char

    string::size_type colon = input.find(":", left);
    string content;
    string pattern;
    if (colon == string::npos || colon > right) {
      content = input.substr(left, right - left);
    }
    else {
      content = input.substr(left, colon - left);
      pattern = input.substr(colon + 1, right - colon - 1);
    }
    enableMatchingDebugMessages(content.c_str(), pattern.c_str());
  }

  assertTrue_3(is.eof(),
               "I/O error while reading debug configuration file",
               DebugErr::DebugConfigError());
  return is.eof();
}

bool allDebugMessagesEnabled()
{
  return allEnabled;
}

void enableAllDebugMessages()
{
  allEnabled = true;
  for (DebugMessage *m = allMsgs; m; m = m->next)
    m->enabled = true;
  deleteAllDebugPatterns();
}

void disableAllDebugMessages()
{
  allEnabled = false;
  for (DebugMessage *m = allMsgs; m; m = m->next)
    m->enabled = false;
  deleteAllDebugPatterns();
}

void enableMatchingDebugMessages(char const *file,
                                 char const *pattern)
{
  assertTrue_3(file, "enableMatchingDebugMessages: Null file string", DebugErr::DebugInternalError());
  assertTrue_3(pattern, "enableMatchingDebugMessages: Null marker string", DebugErr::DebugInternalError());
  if (allEnabled)
    return; // nothing to do

  // Add pattern for messages added in the future
  DebugPattern *p = new DebugPattern(file, pattern);
  p->next = enabledPatterns;
  enabledPatterns = p;

  for (DebugMessage *m = allMsgs; m; m = m->next) {
    if (!m->enabled
        && markerMatches(m->file, file)
        && markerMatches(m->marker, pattern))
      m->enabled = true;
  }
}

//
// Macro internals
//

std::ostream &getDebugOutputStream()
{
  assertTrue_3(debugStream && debugStream->good(),
               "Null or invalid debug output stream",
               DebugErr::DebugInternalError());
  return *debugStream;
}

DebugMessage *addDebugMessage(char const *file, char const *marker)
{
  assertTrue_3(file && *file,
               "addDebugMessage: Null or empty file string",
               DebugErr::DebugInternalError());
  assertTrue_3(marker && *marker,
               "addDebugMessage: Null or empty marker string",
               DebugErr::DebugInternalError());

  ensureDebugInited();
  DebugMessage *result = findDebugMessage(file, marker);
  if (result)
    return result;
  // Add new and enable if appropriate
  result = new DebugMessage(file, marker);
  if (allEnabled) {
    result->enabled = true; 
  }
  else {
    // Enable if it matches an existing pattern
    for (DebugPattern *p = enabledPatterns; p; p = p->next) {
      if (markerMatches(file, p->file) && markerMatches(marker, p->pattern)) {
        result->enabled = true;
        break;
      }
    }
  }
  result->next = allMsgs;
  allMsgs = result;
  return result;
}

std::ostream& operator<<(std::ostream &os, const DebugMessage &dm)
{
  dm.print(os);
  return os;
}

#endif /* NO_DEBUG_MESSAGE_SUPPORT */
