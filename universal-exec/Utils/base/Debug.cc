/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

#include <fstream>
#include <algorithm>
#include <functional>

/**
 * @class DebugConfig
 * @brief Used to perform default allocation, based on 'Debug.cfg'.
 * @author Conor McGann
 * @see DebugMessage::addMsg
 */
class DebugConfig {
public:
  static void init() {
    static DebugConfig s_instance;
  }
private:
  DebugConfig() {
    DebugMessage::setStream(std::cout);
    //    std::ifstream config("Debug.cfg");
    //    if (config.good()) {
    //    DebugMessage::readConfigFile(config);
    //  }
  }
};

DebugMessage::DebugMessage(const std::string& file,
                           const int& line,
                           const std::string& marker,
                           const bool& enabled)
  : m_file(file), m_line(line), m_marker(marker),
    m_enabled(enabled) {
}

DebugMessage *DebugMessage::addMsg(const std::string &file, const int& line,
                                   const std::string &marker) {
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
      typedef std::list<DebugPattern>::iterator LDPI;
      LDPI iter = std::find_if(enabledPatterns().begin(),
                               enabledPatterns().end(),
                               PatternMatches<DebugPattern>(*msg));
      if (iter != enabledPatterns().end())
        msg->enable();
    }
  }
  return(msg);
}

DebugMessage *DebugMessage::findMsg(const std::string &file,
                                    const std::string &pattern) {
  typedef std::list<DebugMessage*>::const_iterator LDMPCI;
  LDMPCI iter = std::find_if(allMsgs().begin(),
                             allMsgs().end(),
                             MatchesPattern<DebugMessage*>(file, pattern));
  if (iter == allMsgs().end())
    return(0);
  return(*iter);
}

void DebugMessage::findMatchingMsgs(const std::string &file,
                                    const std::string &pattern,
                                    std::list<DebugMessage*> &matches) {
  std::for_each(allMsgs().begin(), allMsgs().end(), GetMatches(file, pattern, matches));
}

const std::list<DebugMessage*>& DebugMessage::getAllMsgs() {
  return(allMsgs());
}

void DebugMessage::enableAll() {
  allEnabled() = true;
  enabledPatterns().clear();
  std::for_each(allMsgs().begin(),
                allMsgs().end(),
                std::mem_fun(&DebugMessage::enable));
}

void DebugMessage::enableMatchingMsgs(const std::string& file,
                                      const std::string& pattern) {
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

void DebugMessage::disableMatchingMsgs(const std::string& file,
				       const std::string& pattern) {
  if(file.length() < 1 && pattern.length() < 1)
    return;

  DebugPattern dp(file, pattern);
  enabledPatterns().erase(std::find(enabledPatterns().begin(), enabledPatterns().end(), dp));
  std::for_each(allMsgs().begin(),
		allMsgs().end(),
		DisableMatches(dp));
}

bool DebugMessage::readConfigFile(std::istream& is) {
  check_error(is.good(), "cannot read debug config from invalid/error'd stream",
              DebugErr::DebugConfigError());
  std::string input;
  while (is.good() && !is.eof()) {
    getline(is, input);
    if (input.empty())
      continue;
    std::string::size_type i = 0;
    std::string::size_type len = input.length();
    while (i < len && isascii(input[i]) && isspace(input[i]))
      i++;
    if (input[i] == ';' || input[i] == '#' || input[i] == '/')
      continue; // input is a comment
    input = input.substr(i); // after white space
    i = input.find_first_of(";#/");
    input = input.substr(0, i); // chop off comment
    i = input.length();
    while (i > 0 && isascii(input[i - 1]) && isspace(input[i - 1]))
      --i;
    if (i <= 0)
      continue; // should be impossible
    input = input.substr(0, i);
    i = input.find(":");
    std::string pattern;
    if (i < input.length() && input[i] == ':') {
      pattern = input.substr(i + 1);
      input = input.substr(0, i);
    }
    enableMatchingMsgs(input, pattern);
  }
  check_error(is.eof(), "error while reading debug config file",
              DebugErr::DebugConfigError());
  return(is.eof());
}

#endif /* DEBUG_MESSAGE_SUPPORT */
