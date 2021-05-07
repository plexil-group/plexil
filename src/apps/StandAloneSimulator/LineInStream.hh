// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef SAS_LINE_IN_STREAM_HH
#define SAS_LINE_IN_STREAM_HH

#include <fstream>
#include <sstream>
#include <string>

#define MAX_LINE_LENGTH (1024)

// Helper class
class LineInStream {
public:
  LineInStream();
  ~LineInStream() = default;

  // (Re)Open the stream with a new file
  // Returns true on success
  bool open(std::string const &fname);

  void close();

  std::istream &getLine();

  std::istringstream &getLineStream();

  std::string const &getFileName() const;

  unsigned int getLineCount() const;

  bool good() const;

  bool eof() const;

private:
  std::string m_filename;
  std::ifstream m_filestream;
  std::istringstream m_linestream;
  unsigned int m_linecount;
  std::string m_linebuf;
};

#endif // SAS_LINE_IN_STREAM_HH
