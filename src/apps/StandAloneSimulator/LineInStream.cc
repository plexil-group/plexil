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

#include "LineInStream.hh"

#include "Debug.hh"

LineInStream::LineInStream()
  : m_filename(),
    m_filestream(),
    m_linestream(),
    m_linecount(0),
    m_linebuf(MAX_LINE_LENGTH, '\0')
{
}

bool LineInStream::open(std::string const &fname)
{
  close();
  m_linecount = 0;
  m_filestream.open(fname.c_str());
  if (m_filestream.fail()) {
    debugMsg("LineInStream:open", " for " << fname << " failed");
    return false;
  }
  debugMsg("LineInStream:open", ' ' << fname);
  m_filename = fname;
  return true;
}

void LineInStream::close()
{
  if (m_filestream.is_open()) {
    m_filestream.close();
    m_filename.clear();
  }
}

std::istream &LineInStream::getLine()
{
  if (!m_filestream.good() || m_filestream.eof()) {
    debugMsg("LineInStream:getLine", " at EOF or error");
    m_linebuf.clear();
  }

  bool ignoreLine = false;
  do {
    debugMsg("LineInStream:getLine", " not EOF");
    std::getline(m_filestream, m_linebuf);
    ++m_linecount;
    size_t firstNonWhitespace = m_linebuf.find_first_not_of(" \t\n\r");
    ignoreLine = (m_linebuf.size() == 0)
      || (std::string::npos == firstNonWhitespace)
      || !isalnum(m_linebuf[firstNonWhitespace]);
    condDebugMsg(ignoreLine,
                 "LineInStream:getLine",
                 " ignoring blank or comment line");
  }
  while (ignoreLine && m_filestream.good() && !m_filestream.eof());

  debugMsg("LineInStream:getLine",
           " line = \"" << m_linebuf << "\"");

  m_linestream.clear();
  m_linestream.str(m_linebuf);
  return m_linestream;
}

std::istringstream &LineInStream::getLineStream()
{
  return m_linestream;
}

std::string const &LineInStream::getFileName() const
{
  return m_filename;
}

unsigned int LineInStream::getLineCount() const
{
  return m_linecount;
}

bool LineInStream::good() const
{
  return m_filestream.good();
}

bool LineInStream::eof() const
{
  return m_filestream.eof();
}
