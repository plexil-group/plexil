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

#include "DebugMessage.hh"

#include "Error.hh"

#ifdef STDC_HEADERS
#include <cstring> // strstr()
#endif

#include <iostream>
#include <vector>

namespace PLEXIL
{

  // Forward declaration
  static bool matchesPatterns(char const *marker);

  //
  // DebugMessage
  //

  static DebugMessage *allDebugMessages = nullptr;

  DebugMessage::DebugMessage(char const *mrkr)
    : marker(mrkr),
      next(allDebugMessages),
      enabled(matchesPatterns(marker))
  {
    allDebugMessages = this;
  }

  std::ostream &operator<<(std::ostream &ostr, DebugMessage const &msg)
  {
    ostr << msg.marker << " ("
         << (msg.enabled ? "en" : "dis") << "abled)";
    return ostr;
  }

  //
  // Debug stream
  //

  /**
   * @brief The debug output stream.
   */
  static std::ostream *debugStream = nullptr;

  std::ostream &getDebugOutputStream()
  {
    assertTrue_2(debugStream != nullptr && debugStream->good(),
                 "Null or invalid debug output stream");
    return *debugStream;
  }

  bool setDebugOutputStream(std::ostream &ostr)
  {
    if (!ostr.good())
      return false;
    debugStream = &ostr;
    return true;
  }

  static void ensureDebugInited()
  {
    static bool debugInited = false;
    if (debugInited)
      return;

    // Default value for debug stream
    debugStream = &std::cout;
    debugInited = true;
  }

  //
  // Patterns
  //

  static std::vector<std::string> allDebugPatterns;

  /**
   *  @brief Whether the given marker string matches the pattern string.
   *  Exists solely to ensure the same method is always used to check
   *  for a match.
   */
  static bool markerMatches(char const *marker, std::string const &pattern)
  {
    return nullptr != strstr(marker, pattern.c_str());
  }

  static bool matchesPatterns(char const *marker)
  {
    for (std::string const &pat : allDebugPatterns)
      if (markerMatches(marker, pat))
        return true;
    return false;
  }

  void enableMatchingDebugMessages(std::string &&pattern)
  {
    // Enable any existing messages that match
    for (DebugMessage *m = allDebugMessages; m != nullptr; m = m->next)
      if (!m->enabled
          && markerMatches(m->marker, pattern))
        m->enabled = true;

    // Add pattern for messages added in the future
    allDebugPatterns.push_back(std::move(pattern));
  }

  bool readDebugConfigStream(std::istream& istr)
  {
    static const char *sl_whitespace = " \f\n\r\t\v";
    static const char *sl_comment = "#/";

    ensureDebugInited();

    assertTrue_2(istr.good(),
                 "Cannot read debug configuration from invalid/error'd stream");

    while (istr.good() && !istr.eof()) {
      std::string input;
      getline(istr, input);
      if (input.empty())
        continue;

      // Find leftmost non-blank character
      std::string::size_type left = input.find_first_not_of(sl_whitespace);
      if (left == std::string::npos)
        continue; // line is all whitespace

      // Find trailing comment, if any
      std::string::size_type comment = input.find_first_of(sl_comment, left);
      if (comment == left)
        continue; // line is a comment

      // Trim whitespace before comment
      if (comment != std::string::npos)
        comment--; // start search just before comment
      std::string::size_type right = input.find_last_not_of(sl_whitespace, comment);
      right++; // point just past last non-blank char

      // Trim leading colon for backwards compatibility
      if (input[left] == ':')
        ++left;
      enableMatchingDebugMessages(input.substr(left, right - left));
    }

    assertTrue_2(istr.eof(),
                 "I/O error while reading debug configuration file");
    return istr.eof();
  }

} // namespace PLEXIL
