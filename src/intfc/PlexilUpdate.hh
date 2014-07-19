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

#ifndef PLEXIL_UPDATE_PROTO_HH
#define PLEXIL_UPDATE_PROTO_HH

#include "PlexilExpr.hh"

#include <string>
#include <vector>
#include <utility>

namespace PLEXIL
{
  class PlexilExpr;

  class PlexilUpdate {
  public:
    PlexilUpdate() : m_lineNo(0), m_colNo(0) {}
    ~PlexilUpdate()
    {
      for (std::vector<std::pair<std::string, PlexilExpr *> >::iterator it = m_map.begin();
           it != m_map.end();
           ++it)
        delete it->second;
      m_map.clear();
    }

    const std::vector<std::pair<std::string, PlexilExpr *> >& pairs() const {return m_map;}

    void addPair(const std::string& name, PlexilExpr *value)
    { m_map.push_back(std::make_pair(name, value));}
    int lineNo() const {return m_lineNo;}
    int colNo() const {return m_colNo;}
    void setLineNo(int n) {m_lineNo = n;}
    void setColNo(int n) {m_colNo = n;}
  private:
    std::vector<std::pair<std::string, PlexilExpr *> > m_map;
    int m_lineNo;
    int m_colNo;
  };

} // namespace PLEXIL

#endif // PLEXIL_UPDATE_PROTO_HH
