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

#ifndef NO_DEBUG_MESSAGE_SUPPORT

#include "Error.hh"
#include "lifecycle-utils.h"

#include <cstring>
#include <fstream>

using std::string;

static std::ostream *debugStream = NULL;

/**
 * @brief List of pointers to all debug messages.
 */
static DebugMessage *allMsgs = NULL;

/**
 * @brief List of all enabled debug patterns.
 */
static DebugPattern *enabledPatterns = NULL;

static void purgePatternsAndMessages()
{
  while (enabledPatterns) {
    DebugPattern *pat = enabledPatterns;
    enabledPatterns = pat->m_next;
    delete pat;
  }
  while (allMsgs) {
    DebugMessage *msg = allMsgs;
    allMsgs = msg->next();
    delete msg;
  }
}

static void ensureDebugInited()
{
  static bool sl_inited = false;
  if (!sl_inited) {
    addFinalizer(&purgePatternsAndMessages);
    debugStream = &std::cout;
    allMsgs = NULL;
    enabledPatterns = NULL;
    sl_inited = true;
  }
}

bool DebugPattern::operator==(DebugPattern const &other) const
{
  return (!strcmp(m_file, other.m_file))
    && (!strcmp(m_pattern, other.m_pattern));
}

class DebugErr
{
public:
  DECLARE_ERROR(DebugMessageError);
  DECLARE_ERROR(DebugMemoryError);
  DECLARE_ERROR(DebugConfigError);
};

DebugMessage::DebugMessage(char const *file,
                           char const *marker,
                           bool enabled)
  : m_next(NULL),
    m_file(file), 
    m_marker(marker),
    m_enabled(enabled)
{
}

DebugMessage *DebugMessage::addMsg(char const *file,
                                   char const *marker) 
{
  ensureDebugInited();
  check_error_3(file && *file && marker && *marker,
                "debug messages must have non-empty file and marker",
                DebugErr::DebugMessageError());
  DebugMessage *msg = findMsg(file, marker);
  if (!msg) {
    msg = new DebugMessage(file, marker);
    check_error(msg, "no memory for new debug message",
                DebugErr::DebugMemoryError());
    msg->m_next = allMsgs;
    allMsgs = msg;
    DebugPattern const *pat = enabledPatterns;
    while (pat) {
      if (msg->matches(*pat)) {
        msg->enable();
        break;
      }
      pat = pat->m_next;
    }
  }
  return(msg);
}

void DebugMessage::enable()
{
  check_error_2(isGood(), 
                "cannot enable debug message(s) without a good debug stream");
  m_enabled = true;
}

/**
   @brief Assign a stream to which all debug messages will be sent.
   @param os
*/
void DebugMessage::setStream(std::ostream& os)
{
  debugStream = &os;
}

std::ostream& DebugMessage::getStream()
{
  return *debugStream;
}

bool DebugMessage::isGood() 
{
  if (!debugStream)
    return false;
  return debugStream->good();
}

/**
   @brief Print the data members of the debug message in a format
   that Emacs can use to display the corresponding source code.
   @param os
*/
void DebugMessage::print(std::ostream &os) const
{
  try {
    os.exceptions(std::ostream::badbit);
    os << m_file << ':' << m_marker << ' ';
  }
  catch(std::ios_base::failure& exc) {
    checkError(ALWAYS_FAIL, exc.what());
    throw;
  }
}

/**
   @brief Whether the given marker matches the "pattern".
   Exists solely to ensure the same method is always used to check
   for a match.
*/
inline static bool markerMatches(char const *marker,
                                 char const *pattern) 
{
  if (!pattern || !*pattern)
    return true;
  return (NULL != strstr(marker, pattern));
}

/**
   @brief Whether the message is matched by the pattern.
*/
bool DebugMessage::matches(DebugPattern const &pattern) const 
{
  return markerMatches(m_file, pattern.m_file)
    && markerMatches(m_marker, pattern.m_pattern);
}

DebugMessage *DebugMessage::findMsg(char const *file,
                                    char const *pattern)
{
  DebugPattern const dp(file, pattern);
  DebugMessage *next = allMsgs;
  while (next) {
    if (next->matches(dp)) 
      return next;
    next = next->m_next;
  }
  return NULL;
}

void DebugMessage::findMatchingMsgs(char const *file,
                                    char const *pattern,
                                    std::vector<DebugMessage*> &matches) 
{
  DebugPattern const dp(file, pattern);
  DebugMessage *next = allMsgs;
  while (next) {
    if (next->matches(dp))
      matches.push_back(next);
    next = next->m_next;
  }
}

void DebugMessage::enableAll() {
  allEnabled() = true;
  while (enabledPatterns) {
    DebugPattern *tmp = enabledPatterns;
    enabledPatterns = enabledPatterns->m_next;
    delete tmp;
  }
  DebugMessage *next = allMsgs;
  while (next) {
    next->enable();
    next = next->m_next;
  }
}

static char *copyString(char const *orig)
{
  char *result = NULL;
  if (orig && *orig) {
    size_t len = strlen(orig);
    result = new char[len + 1];
    strcpy(result, orig);
  }
  else {
    result = new char[1];
    *result = '\0';
  }
  return result;
}

void DebugMessage::enableMatchingMsgs(char const *file,
                                      char const *pattern)
{
  if ((!file || !*file) && (!pattern || !*pattern)) {
    enableAll();
    return;
  }
  
  DebugPattern* dp = 
    new DebugPattern(copyString(file), copyString(pattern), true);
  dp->m_next = enabledPatterns;
  enabledPatterns = dp;

  DebugMessage *next = allMsgs;
  while (next) {
    if (next->matches(*dp))
      next->enable();
    next = next->m_next;
  }
}

bool DebugMessage::readConfigFile(std::istream& is)
{
  static const string sl_whitespace(" \f\n\r\t\v");
  static const string sl_comment(";#/");

  ensureDebugInited();

  check_error(is.good(), "cannot read debug config from invalid/error'd stream",
              DebugErr::DebugConfigError());

  string input;
  while (is.good() && !is.eof()) {
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
    enableMatchingMsgs(content.c_str(), pattern.c_str());
  }

  check_error(is.eof(), "error while reading debug config file",
              DebugErr::DebugConfigError());
  return(is.eof());
}

#endif /* NO_DEBUG_MESSAGE_SUPPORT */
