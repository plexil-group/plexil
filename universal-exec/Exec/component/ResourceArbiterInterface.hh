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

#ifndef _H_ResourceArbiterInterface
#define _H_ResourceArbiterInterface

#include "Id.hh"
#include "ExecDefs.hh"
#include "Node.hh"

namespace PLEXIL {

  struct ChildResourceNode
  {
    ChildResourceNode(const double _weight, const std::string& _name,
                      const bool _release=true)
      : weight(_weight), name(_name), release(_release){}
    double weight;
    std::string name;
    bool release;
  };

  struct ResourceNode 
  {
    ResourceNode() : maxConsumableValue(0.0), maxRenewableValue(0.0){}
    ResourceNode(const double _maxConsumableValue, 
                 const double _maxRenewableValue,
                 const std::vector<ChildResourceNode>& _children)
      : maxConsumableValue(_maxConsumableValue),
        maxRenewableValue(_maxRenewableValue), children(_children) {}
    double maxConsumableValue;
    double maxRenewableValue;
    std::vector<ChildResourceNode> children;
  };

  struct ResourceComparator
  {
    bool operator() (const ChildResourceNode& x, const ChildResourceNode& y)const;
  };

  class ResourceArbiterInterface;
  typedef Id<ResourceArbiterInterface> ResourceArbiterInterfaceId;

  class ResourceArbiterInterface
  {
  public:
    ResourceArbiterInterface(): m_resourceFileRead(false), m_id(this)
    {
      if (readResourceHeirarchy("resource.data"))
        m_resourceFileRead = true;
    }

    ~ResourceArbiterInterface(){m_id.remove();}

    void arbitrateCommands(const std::list<CommandId>& cmds,
                           std::set<CommandId>& acceptCmds);
    void releaseResourcesForCommand(const LabelStr& cmdName);
     
    ResourceArbiterInterfaceId getId()
    {
      return m_id;
    }

  private:
    bool m_resourceFileRead;
    std::map<std::string, double> m_lockedRes;
    std::map<std::string, std::set<ChildResourceNode, ResourceComparator> > m_cmdResMap;
    std::map<std::string, ResourceNode> m_resourceHierarchy;
    std::multimap<int, CommandId> m_prioritySortedCommands;
    std::map<std::string, std::set<CommandId> > m_resCmdMap;
    ResourceArbiterInterfaceId m_id;

    void preprocessCommandToArbitrate(const std::list<CommandId>& cmds,
                                      std::set<CommandId>& acceptCmds);
    double resourceAmountNeededByCommand(const std::string& resName, 
                                         const std::string& cmdName);
    void optimalResourceArbitration (std::set<CommandId>& acceptCmds);

    void determineAllChildResources
    (const std::map<std::string, double>& res, std::vector<ChildResourceNode>& flattenedRes);
    bool readResourceHeirarchy(const std::string& fName);
    double maxConsumableResourceValue(const std::string& resName) const;
    double maxRenewableResourceValue(const std::string& resName) const;
    bool isResourceUsageOutsideLimits(const double resNeeded, 
                                      const std::string& resName) const;

    void printResourceCommandMap() const;
    void printSortedCommands() const;
    void printLockedResources() const;
    void printAcceptedCommands(const std::set<CommandId>& acceptCmds);

  };
  
}
#endif // _H_ResourceArbiterInterface
