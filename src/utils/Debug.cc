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

#include "lifecycle-utils.h"

#include <fstream>
#include <algorithm>
#include <functional>

using std::string;

/**
 * @class DebugConfig
 * @brief Used to perform default allocation, based on 'Debug.cfg'.
 * @author Conor McGann
 * @see DebugMessage::addMsg
 */
class DebugConfig {
public:
  static void init() 
  {
    static bool sl_inited = false; 
    if (!sl_inited) {
      addFinalizer(&purge);
      s_instance = new DebugConfig();
      sl_inited = true;
    }
  }

private:
  DebugConfig() {
    DebugMessage::setStream(std::cout);
  }

  static void purge() {
    DebugConfig* inst = s_instance;
    s_instance = NULL;
    delete inst;
  }

  static DebugConfig* s_instance;
};

DebugConfig* DebugConfig::s_instance = NULL;

DebugMessage::DebugMessage(const string& file,
                           const int& line,
                           const string& marker,
                           const bool& enabled)
  : m_file(file), 
    m_marker(marker),
    m_line(line),
    m_enabled(enabled)
{
}

DebugMessage *DebugMessage::addMsg(const string &file, const int& line,
                                   const string &marker) {
  DebugConfig::init();
  check_error(line > 0, "debug messages must have positive line numbers",
              DebugErr::DebugMessageError());
  check_error(!file.empty() && !marker.empty(), "debug messages must have non-empty file and marker",
              DebugErr::DebugMessageError());
  DebugMessage *msg = findMsg(file, marker);
  if (msg == 0) {
    msg = new DebugMessage(file, line, marker);
    check_error(msg != 0, "no memory for new debug message",
                DebugErr::DebugMemoryError());
    allMsgs().push_back(msg);
    if (!msg->isEnabled()) {
      typedef std::vector<DebugPattern>::iterator VDPI;
      VDPI iter = std::find_if(enabledPatterns().begin(),
                               enabledPatterns().end(),
                               PatternMatches<DebugPattern>(*msg));
      if (iter != enabledPatterns().end())
        msg->enable();
    }
  }
  return(msg);
}

DebugMessage *DebugMessage::findMsg(const string &file,
                                    const string &pattern) {
  typedef std::vector<DebugMessage*>::const_iterator VDMPCI;
  VDMPCI iter = std::find_if(allMsgs().begin(),
                             allMsgs().end(),
                             MatchesPattern<DebugMessage*>(file, pattern));
  if (iter == allMsgs().end())
    return(0);
  return(*iter);
}

void DebugMessage::findMatchingMsgs(const string &file,
                                    const string &pattern,
                                    std::vector<DebugMessage*> &matches) {
  std::for_each(allMsgs().begin(), allMsgs().end(), GetMatches(file, pattern, matches));
}

const std::vector<DebugMessage*>& DebugMessage::getAllMsgs() {
  return(allMsgs());
}

/**
 *  @brief List of pointers to all debug messages.
 */
std::vector<DebugMessage*>& DebugMessage::allMsgs() {
  static std::vector<DebugMessage*> sl_msgs;
  static bool sl_inited = false;
  if (!sl_inited) {
    addFinalizer(&purge);
    sl_inited = true;
  }
  return sl_msgs;
}

void DebugMessage::enableAll() {
  allEnabled() = true;
  enabledPatterns().clear();
  std::for_each(allMsgs().begin(),
                allMsgs().end(),
                std::mem_fun(&DebugMessage::enable));
}

void DebugMessage::enableMatchingMsgs(const string& file,
                                      const string& pattern) {
  if (file.length() < 1 && pattern.length() < 1) {
    enableAll();
    return;
  }
  DebugPattern dp(file, pattern);
  enabledPatterns().push_back(dp);
  std::for_each(allMsgs().begin(),
                allMsgs().end(),
                EnableMatches(dp));
}

void DebugMessage::disableMatchingMsgs(const string& file,
                       const string& pattern) {
  if(file.length() < 1 && pattern.length() < 1)
    return;

  DebugPattern dp(file, pattern);
  enabledPatterns().erase(std::find(enabledPatterns().begin(), enabledPatterns().end(), dp));
  std::for_each(allMsgs().begin(),
        allMsgs().end(),
        DisableMatches(dp));
}

bool DebugMessage::readConfigFile(std::istream& is)
{
  static const string sl_whitespace(" \f\n\r\t\v");
  static const string sl_comment(";#/");

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
    enableMatchingMsgs(content, pattern);
  }

  check_error(is.eof(), "error while reading debug config file",
              DebugErr::DebugConfigError());
  return(is.eof());
}

void DebugMessage::purge()
{
  enabledPatterns().clear();
  for (std::vector<DebugMessage*>::const_iterator it = allMsgs().begin();
       it != allMsgs().end();
       ++it)
    delete *it;
  allMsgs().clear();
}

#endif /* NO_DEBUG_MESSAGE_SUPPORT */
