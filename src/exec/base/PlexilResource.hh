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

#ifndef _H_PlexilResource
#define _H_PlexilResource

#include "Id.hh"

namespace PLEXIL {

  const std::string RESOURCE_TAG("Resource");
  const std::string RESOURCELIST_TAG("ResourceList");
  const std::string RESOURCENAME_TAG("ResourceName");
  const std::string RESOURCEPRIORITY_TAG("ResourcePriority");
  const std::string RESOURCELOWERBOUND_TAG("ResourceLowerBound");
  const std::string RESOURCEUPPERBOUND_TAG("ResourceUpperBound");
  const std::string RESOURCERELEASEATTERMINATION_TAG("ResourceReleaseAtTermination");

  class PlexilResource;
  class PlexilExpr;
  
  DECLARE_ID(PlexilResource);
  DECLARE_ID(PlexilExpr);

  class PlexilResource
  {
  public:
    PlexilResource() : m_id(this) {}
    ~PlexilResource() {m_id.remove();}
    const PlexilResourceId& getId() const {return m_id;}
    void addResource(const std::string& tag, const PlexilExprId& resource)
    {
      m_resource[tag] = resource;
    }
    
    const std::map<std::string, PlexilExprId>& getResourceMap() const
    {
      return m_resource;
    }
  private:
    PlexilResourceId m_id;
    std::map<std::string, PlexilExprId> m_resource;
  };
}

#endif // _H_PlexilResource
