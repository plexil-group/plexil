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

#include "PlexilLookup.hh"

namespace PLEXIL
{
  //
  // PlexilState
  //
  PlexilState::PlexilState() 
    : m_nameExpr(NULL),
      m_args(),
      m_lineNo(0),
      m_colNo(0) 
  {
  }

  PlexilState::~PlexilState()
  {
    for (std::vector<PlexilExpr *>::iterator it = m_args.begin();
         it != m_args.end();
         ++it)
      delete *it;
    m_args.clear();
    if (m_nameExpr)
      delete m_nameExpr;
  }

  const std::vector<PlexilExpr *>& PlexilState::args() const
  {
    return m_args;
  }
   
  const std::string& PlexilState::name() const
  {
    PlexilValue const *val = dynamic_cast<PlexilValue const *>(m_nameExpr);
    if (val)
      return val->value();
    else
      return m_nameExpr->name();
  }

  PlexilExpr const *PlexilState::nameExpr() const
  {
    return m_nameExpr;
  }
  
  int PlexilState::lineNo() const
  {
    return m_lineNo;
  }
  
  int PlexilState::colNo() const
  {
    return m_colNo;
  }

  void PlexilState::addArg(PlexilExpr *arg) 
  {
    m_args.push_back(arg);
  }

  void PlexilState::setName(const std::string& name)
  {
    m_nameExpr = new PlexilValue(STRING_TYPE, name);
  }

  void PlexilState::setNameExpr(PlexilExpr *nameExpr)
  {
    m_nameExpr = nameExpr;
  }

  void PlexilState::setLineNo(int n) 
  {
    m_lineNo = n;
  }

  void PlexilState::setColNo(int n)
  {
    m_colNo = n;
  }

  //
  // PlexilLookup
  //

  PlexilLookup::PlexilLookup()
    : PlexilExpr(),
      m_state(NULL)
  {
  }

  PlexilLookup::~PlexilLookup()
  {
    if (m_state)
      delete m_state;
  }

  PlexilState const *PlexilLookup::state() const
  {
    return m_state;
  }

  void PlexilLookup::setState(PlexilState *state)
  {
    m_state = state;
  }

  PlexilExpr const *PlexilLookup::tolerance() const
  {
    return NULL;
  }

  //
  // PlexilChangeLookup
  //
  PlexilChangeLookup::PlexilChangeLookup()
    : PlexilLookup(),
      m_tolerance(NULL)
  {
  }

  PlexilChangeLookup::~PlexilChangeLookup()
  {
    delete m_tolerance;
  }

  PlexilExpr const *PlexilChangeLookup::tolerance() const
  {
    return m_tolerance;
  }
  
  void PlexilChangeLookup::setTolerance(PlexilExpr *tolerance) 
  {
    m_tolerance = tolerance;
  }

} // namespace PLEXIL
