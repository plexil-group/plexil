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

#ifndef _H_ResourceArbiterInterface
#define _H_ResourceArbiterInterface

// For int32_t
#include "plexil-config.h"
#ifdef HAVE_STDINT_H
#include <stdint.h>
#elif defined(__VXWORKS__)
#include <vxWorks.h>
#endif

#include <map>
#include <set>
#include <string>
#include <vector>

namespace PLEXIL 
{
  class Command;

  struct ChildResourceNode
  {
    ChildResourceNode(const double _weight,
                      const std::string& _name,
                      const bool _release = true)
      : weight(_weight),
        name(_name),
        release(_release)
    {}

    double weight;
    std::string name;
    bool release;
  };

  struct ResourceNode 
  {
    ResourceNode()
      : maxConsumableValue(0.0), maxRenewableValue(0.0)
    {}

    ResourceNode(const double _maxConsumableValue, 
                 const double _maxRenewableValue,
                 const std::vector<ChildResourceNode>& _children)
      : maxConsumableValue(_maxConsumableValue),
        maxRenewableValue(_maxRenewableValue),
        children(_children)
    {}

    double maxConsumableValue;
    double maxRenewableValue;
    std::vector<ChildResourceNode> children;
  };

  struct ResourceComparator
  {
    bool operator() (const ChildResourceNode& x, const ChildResourceNode& y)const;
  };

  class ResourceArbiterInterface
  {
  public:
    // Convenience typedef
    typedef std::set<Command *> CommandSet;
    
    ResourceArbiterInterface();
    ~ResourceArbiterInterface();

    void arbitrateCommands(std::vector<Command *> const &cmds,
                           CommandSet& acceptCmds);
    void releaseResourcesForCommand(const std::string& cmdName);

  private:
    // Type names
    typedef std::set<ChildResourceNode, ResourceComparator> ResourceMapEntry;
    typedef std::map<std::string, ResourceMapEntry> ResourceMap;
    typedef std::map<std::string, CommandSet> ResourceCommandMap;
    typedef std::multimap<int32_t, Command *> CommandPriorityMap;

    std::map<std::string, double> m_lockedRes;
    ResourceMap m_cmdResMap;
    std::map<std::string, ResourceNode> m_resourceHierarchy;
    CommandPriorityMap m_prioritySortedCommands;
    ResourceCommandMap m_resCmdMap;
    bool m_resourceFileRead;

    void preprocessCommandToArbitrate(const std::vector<Command *>& cmds,
                                      CommandSet& acceptCmds);
    double resourceAmountNeededByCommand(const std::string& resName, 
                                         const std::string& cmdName);
    void optimalResourceArbitration (CommandSet& acceptCmds);

    bool readResourceHierarchy(const std::string& fName);
    double maxConsumableResourceValue(const std::string& resName) const;
    double maxRenewableResourceValue(const std::string& resName) const;
    bool isResourceUsageOutsideLimits(const double resNeeded, 
                                      const std::string& resName) const;

    void printResourceCommandMap() const;
    void printSortedCommands() const;
    void printLockedResources() const;
    void printAcceptedCommands(const CommandSet& acceptCmds);

  };
  
}
#endif // _H_ResourceArbiterInterface
