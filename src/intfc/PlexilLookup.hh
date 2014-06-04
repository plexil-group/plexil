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

#ifndef PLEXIL_LOOKUP_PROTO_HH
#define PLEXIL_LOOKUP_PROTO_HH

#include "PlexilExpr.hh"

namespace PLEXIL
{
  // Forward references
  class PlexilState;
  DECLARE_ID(PlexilState);

  class PlexilState {
  public:
    PlexilState();
    ~PlexilState();

    const PlexilStateId& getId() const;
    const std::vector<PlexilExprId>& args() const;
    const std::string& name() const;
    const PlexilExprId& nameExpr() const;
    int lineNo() const;
    int colNo() const;

    void addArg(const PlexilExprId& arg);
    void setName(const std::string& name);
    void setNameExpr(const PlexilExprId& nameExpr);
    void setLineNo(int n);
    void setColNo(int n);

  private:
    PlexilStateId m_id;
    PlexilExprId m_nameExpr;
    std::vector<PlexilExprId> m_args;
    int m_lineNo;
    int m_colNo;
  };

  class PlexilLookup : public PlexilExpr {
  public:
    PlexilLookup();
    virtual ~PlexilLookup();

    const PlexilStateId& state() const;
    void setState(const PlexilStateId& state);

  private:
    PlexilStateId m_state;
  };


  class PlexilLookupNow : public PlexilLookup
  {
  public:
    PlexilLookupNow();
  };

  class PlexilChangeLookup : public PlexilLookup {
  public:
    PlexilChangeLookup();
    ~PlexilChangeLookup();

    const std::vector<PlexilExprId>& tolerances() const;
    void addTolerance(const PlexilExprId& tolerance);

  private:
    std::vector<PlexilExprId> m_tolerances;
  };

} // namespace PLEXIL

#endif // PLEXIL_LOOKUP_PROTO_HH
