/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "ResourceArbiterInterface.hh"

#include "CommandImpl.hh"
#include "Debug.hh"
#include "LinkedQueue.hh"

#include <algorithm> // std::stable_sort()
#include <cctype>
#include <map>
#include <set>

#if defined(HAVE_CSTDLIB)
#include <cstdlib> // strtod()
#elif defined(HAVE_STDLIB_H)
#include <stdlib.h> // strtod()
#endif

#if defined(HAVE_CSTRING)
#include <cstring> // strspn(), strcspn() et al
#elif defined(HAVE_STRING_H)
#include <string.h> // strspn(), strcspn() et al
#endif

namespace PLEXIL
{
  template <typename T> struct NameComparator
  {
    bool operator() (T const &x, T const &y) const
    {
      return x.name < y.name;
    }
  };

  struct ChildResourceNode final
  {
    ChildResourceNode(const double _weight,
                      const std::string& _name,
                      const bool _release = true)
      : name(_name),
        weight(_weight),
        release(_release)
    {}

    // The rest should only be used by container templates.
    ChildResourceNode()
      : name(),
        weight(0.0),
        release(false)
    {
    }

    ChildResourceNode(ChildResourceNode const &) = default;
    ChildResourceNode(ChildResourceNode &&) = default;
    ChildResourceNode &operator=(ChildResourceNode const &) = default;
    ChildResourceNode &operator=(ChildResourceNode &&) = default;

    ~ChildResourceNode() = default;

    std::string name;
    double weight;
    bool release;
  };

  struct ResourceNode final
  {
    ResourceNode(const double _maxConsumableValue, 
                 const std::vector<ChildResourceNode>& _children)
      : children(_children),
      maxConsumableValue(_maxConsumableValue)
      {
      }

    ResourceNode(const double _maxConsumableValue, 
                 std::vector<ChildResourceNode> &&_children)
      : children(std::move(_children)),
      maxConsumableValue(_maxConsumableValue)
      {
      }

    // These should only be used by container templates
    ResourceNode()
      : maxConsumableValue(0.0)
      {}

    ResourceNode(ResourceNode const &) = default;
    ResourceNode(ResourceNode &&) = default;
    ResourceNode &operator=(ResourceNode const &) = default;
    ResourceNode &operator=(ResourceNode &&) = default;

    ~ResourceNode() = default;

    std::vector<ChildResourceNode> children;
    double maxConsumableValue;
  };

  typedef std::set<ChildResourceNode, NameComparator<ChildResourceNode> > ResourceSet;

  struct CommandPriorityEntry
  {
    CommandPriorityEntry(int32_t prio, CommandImpl *cmd)
      : resources(),
        command(cmd),
        priority(prio)
    {
    }

    CommandPriorityEntry(CommandPriorityEntry const &) = default;
    CommandPriorityEntry(CommandPriorityEntry &&) = default;
    CommandPriorityEntry &operator=(CommandPriorityEntry const &) = default;
    CommandPriorityEntry &operator=(CommandPriorityEntry &&) = default;

    ResourceSet resources;
    CommandImpl *command;
    int32_t priority;
  };

  // Used internally by optimalResourceAllocation() method.
  struct ResourceEstimate
  {
    ResourceEstimate()
      : renewable(0.0),
        consumable(0.0)
    {
    }

    ResourceEstimate(double initial)
      : renewable(initial),
        consumable(initial)
    {
    }
  
    double renewable;
    double consumable;
  };

  typedef std::map<std::string, ResourceEstimate> EstimateMap;

  // Type names
  typedef std::pair<CommandImpl *, ResourceSet> ResourceMapEntry;
  typedef std::map<CommandImpl *, ResourceSet> ResourceMap;
  typedef std::map<std::string, ResourceNode> ResourceHierarchyMap;
  typedef std::vector<CommandPriorityEntry> CommandPriorityList;


  struct CommandPriorityComparator
  {
    bool operator() (CommandPriorityEntry const &x, CommandPriorityEntry const &y) const
    {
      return x.priority < y.priority;
    }
  };

  // Flatten resource request into a preorder list (vector)
  static void determineChildResources(std::string const &resName,
                                      bool release,
                                      ResourceHierarchyMap const &resourceHierarchy,
                                      std::vector<ChildResourceNode>& flattenedRes)
  {
    debugMsg("ResourceArbiterInterface:determineChildResources", ' ' << resName);
    ResourceHierarchyMap::const_iterator it = resourceHierarchy.find(resName);
    if (it == resourceHierarchy.end())
      return;

    const std::vector<ChildResourceNode>& children = it->second.children;
    if (children.empty())
      return;
    
    // Recurse over children
    for (std::vector<ChildResourceNode>::const_iterator cIter = children.begin();
         cIter != children.end();
         ++cIter) {
      flattenedRes.emplace_back(ChildResourceNode(cIter->weight, cIter->name, release));
      determineChildResources(cIter->name, release, resourceHierarchy, flattenedRes);
    }
  }

  static void determineAllChildResources(ResourceValue const &res,
                                         ResourceHierarchyMap const &resourceHierarchy,
                                         ResourceSet &resourcesNeeded)
  {
    debugMsg("ResourceArbiterInterface:determineAllChildResources", ' ' << res.name);

    std::vector<ChildResourceNode> flattenedRes;
    std::string const &resName = res.name;
    bool release = res.releaseAtTermination;
    flattenedRes.emplace_back(ChildResourceNode(res.upperBound, resName, release));
    determineChildResources(resName, release, resourceHierarchy, flattenedRes);

    //loop through each hierarchy element in the flattened structure
    for (std::vector<ChildResourceNode>::const_iterator rIter = flattenedRes.begin();
         rIter != flattenedRes.end();
         ++rIter) {
      if (resourcesNeeded.find(*rIter) == resourcesNeeded.end())
        resourcesNeeded.insert(*rIter);
      else if (rIter->name == resName)
        // If current is explicitly specified in 
        // the command then the value can be overwritten. The
        // child resource scales will not be overwritten however.
        // In fact this check will be valid only for the first element
        // of the flattened vector. This means that if a resource value
        // has to be overwritten it has to be explicitly scpecified in the 
        // plan
        resourcesNeeded.insert(*rIter);
    }
  }

  class ResourceArbiterImpl : public ResourceArbiterInterface
  {
  private:
    // Persistent state across calls to arbitrateCommands(),
    // releaseResourcesForCommand()
    std::map<std::string, double> m_allocated;
    ResourceMap m_cmdResMap;

    // Set at initialization
    ResourceHierarchyMap m_resourceHierarchy;
    
  public:
    ResourceArbiterImpl() = default;
    virtual ~ResourceArbiterImpl() = default;

    virtual bool readResourceHierarchyFile(const std::string& fName)
    {
      std::ifstream myFile(fName);
      if (!myFile.is_open() || !myFile.good()) {
        debugMsg("ResourceArbiterInterface:readResourceHierarchyFile",
                 " Unable to open file " << fName << ". No resources read.");
        return false;
      }
      bool result = readResourceHierarchy(myFile);
      myFile.close();
      condDebugMsg(result,
                   "ResourceArbiterInterface:readResourceHierarchyFile",
                   " successfully read " << fName);
      return result;
    }
      
    virtual bool readResourceHierarchy(std::ifstream &s)
    {
      static char const *WHITESPACE = " \t\n\r\v\f";

      m_resourceHierarchy.clear();
      while (!s.eof()) {
        std::string dataStr;
        std::getline(s, dataStr);
        if (dataStr.empty())
          continue;

        char const *data = dataStr.c_str();
        size_t len = dataStr.size();

        // Skip leading whitespace
        size_t offset = strspn(data, WHITESPACE);
        if (offset == len)
          continue;
        data += offset;
        len -= offset;
        
        if (*data == '%')
          continue; // is a comment

        // first element which is the parent resource name
        size_t ws = strcspn(data, WHITESPACE);
        std::string const pName(data, ws);
        
        data += ws;
        len -= ws;

        char * endptr = nullptr;
        double maxCons = strtod(data, &endptr); // skips whitespace
        if (!maxCons && endptr == data) {
          std::cerr << "Error reading second element (consumable amount) of resource file: \n"
                    << dataStr << std::endl;
          return false;
        }

        debugMsg("ResourceArbiterInterface:readResourceHierarchy",
                 " got resource name " << pName << ", value " << maxCons);

        // Skip whitespace
        endptr += strspn(endptr, WHITESPACE);
        len -= (endptr - data);
        data = endptr;

        std::vector<ChildResourceNode> children;
        while (len && *data) {
          // Read dependent resource weight - name pairs
          double d = strtod(data, &endptr);
          if (!d && endptr == data) {
            std::cerr << "Error reading child resource weight of resource file: \n"
                      << dataStr << std::endl;
            return false;
          }

          // Skip whitespace
          endptr += strspn(endptr, WHITESPACE);
          len -= (endptr - data);
          data = endptr;

          if (!len || !*data) {
            std::cerr << "Error reading child resource name of resource file: \n"
                      << dataStr << std::endl;
            return false;
          }

          // get child resource name
          ws = strcspn(data, WHITESPACE);
          std::string const cName(data, ws);

          debugMsg("ResourceArbiterInterface:readResourceHierarchy",
                   " got dependent resource value " << d << ", name " << cName);
          
          children.emplace_back(ChildResourceNode(d, cName));

          len -= ws;
          data += ws;

          // Skip whitespace
          offset = strspn(data, WHITESPACE);
          len -= offset;
          data += offset;
        }

        m_resourceHierarchy[pName] = ResourceNode(maxCons, children);

      }
      return true;
    }
    
    virtual void arbitrateCommands(LinkedQueue<CommandImpl> &cmds,
                                   LinkedQueue<CommandImpl> &acceptCmds,
                                   LinkedQueue<CommandImpl> &rejectCmds)
    {
      debugMsg("ResourceArbiterInterface:arbitrateCommands",
               " processing " << cmds.size() << " commands");

      CommandPriorityList sortedCommands;
      partitionCommands(cmds, acceptCmds, sortedCommands); // consumes cmds

      debugStmt("ResourceArbiterInterface:printSortedCommands",
                printSortedCommands(sortedCommands));

      optimalResourceArbitration(acceptCmds, rejectCmds, sortedCommands);
    
      debugStmt("ResourceArbiterInterface:printAcceptedCommands",
                printAcceptedCommands(acceptCmds));
      // Also print all the locked resources. 
      debugStmt("ResourceArbiterInterface:printAllocatedResources",
                printAllocatedResources());
    }

    virtual void releaseResourcesForCommand(CommandImpl *cmd)
    {
      // loop through all the resources used by the command and remove each of them
      // from the locked list as well as the command list if there are releasable.
      ResourceMap::iterator resListIter = m_cmdResMap.find(cmd);
      if (resListIter != m_cmdResMap.end()) {
        for (ResourceSet::const_iterator resIter = resListIter->second.begin();
             resIter != resListIter->second.end();
             ++resIter) {
          if (resIter->release) 
            m_allocated[resIter->name] -= resIter->weight;
          if (m_allocated[resIter->name] == 0)
            m_allocated.erase(resIter->name);
        }
        m_cmdResMap.erase(resListIter);
      }
    
      debugMsg("ResourceArbiterInterface:releaseResourcesForCommand", 
               "remaining locked resources after releasing for command " << cmd->getName());
      printAllocatedResources();
    }

  private:
    
    // Consumes cmds
    void partitionCommands(LinkedQueue<CommandImpl> &cmds,
                           LinkedQueue<CommandImpl> &acceptCmds,
                           CommandPriorityList &sortedCommands)
    {
      while (CommandImpl *cmd = cmds.front()) {
        cmds.pop();
        const ResourceValueList& resList = cmd->getResourceValues();
        if (resList.empty()) {
          debugMsg("ResourceArbiterInterface:partitionCommands",
                   " accepting " << cmd->getName() << " with no resource requests");
          acceptCmds.push(cmd);
        }
        else {
          sortedCommands.emplace_back(CommandPriorityEntry(resList.front().priority, cmd));
        }

        ResourceSet &resources = sortedCommands.back().resources;
        for (ResourceValueList::const_iterator resListIter = resList.begin();
             resListIter != resList.end();
             ++resListIter)
          determineAllChildResources(*resListIter, m_resourceHierarchy, resources);
      }

      // Sort the resulting list by priority
      if (sortedCommands.size() > 1)
        std::stable_sort(sortedCommands.begin(),
                         sortedCommands.end(),
                         CommandPriorityComparator());
    }

    // Populates acceptCmds, rejectCmds
    void optimalResourceArbitration(LinkedQueue<CommandImpl> &acceptCmds,
                                    LinkedQueue<CommandImpl> &rejectCmds,
                                    CommandPriorityList const &sortedCommands)
    {
      EstimateMap estimates;

      // Prepare estimate map and ensure entries in allocated map based on requests
      for (CommandPriorityEntry const entry : sortedCommands) {
        ResourceSet const &requests = entry.resources;
        for (ChildResourceNode res : requests) {
          std::string const &resName = res.name;
          double value = 0.0;
          if (m_allocated.find(resName) == m_allocated.end())
            m_allocated[resName] = 0.0; // ensure entry in allocation map
          else
            value = m_allocated[resName];
          estimates[resName] = ResourceEstimate(value);
        }
      }

      for (CommandPriorityEntry const entry : sortedCommands) {
        EstimateMap savedEstimates = estimates;
        CommandImpl *cmd = entry.command;
        ResourceSet const &requests = entry.resources;
        bool invalid = false;
        
        debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                 " considering " << cmd->getName());

        for (ChildResourceNode res : requests) {
          ResourceEstimate &est = estimates[res.name];

          debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                   "  " << cmd->getName() << " requires " << res.weight << " of " << res.name);

          if (res.weight < 0.0)
            est.renewable += res.weight;
          else
            est.consumable += res.weight;

          // Make sure that each of the individual resource usage does not exceed
          // the permitted maximum. This handles the worst case resource usage 
          // behavior of both types of resources.
          double resMax = maxConsumableResourceValue(res.name);
          if (est.renewable < 0.0 || est.renewable > resMax) {
            invalid = true;
            debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                     " rejecting " << cmd->getName()
                     << " because renewable usage of " << res.name << " exceeds limits");
            break; // from inner loop
          }
          else if (est.consumable < 0 || est.consumable > resMax) {
            invalid = true;
            debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                     " rejecting " << cmd->getName()
                     << " because consumable usage of " << res.name << " exceeds limits");
            break; // from inner loop
          }
        }
        
        if (invalid) {
          // Back out effects of rejected command
          estimates = savedEstimates;
          rejectCmds.push(cmd);
        }
        else {
          debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                   " accepting " << cmd->getName());

          acceptCmds.push(cmd);
          m_cmdResMap[cmd] = requests;

          // Update the allocated resource map to include the chosen command
          for (ChildResourceNode res : requests)
            m_allocated[res.name] += res.weight;
        }
      }
    }

    bool isResourceUsageOutsideLimits(const double resNeeded, 
                                      const std::string& resName) const
    {
      return resNeeded < 0.0 || resNeeded > maxConsumableResourceValue(resName);
    }

    double maxConsumableResourceValue(const std::string& resName) const
    {
      if (m_resourceHierarchy.find(resName) != m_resourceHierarchy.end())
        return m_resourceHierarchy.find(resName)->second.maxConsumableValue;
      return 1.0;
    }

    void printSortedCommands(CommandPriorityList &sortedCommands) const
    {
      for (CommandPriorityList::const_iterator iter = sortedCommands.begin();
           iter != sortedCommands.end();
           ++iter)
        debugMsg("ResourceArbiterInterface:printSortedCommands", 
                 "CommandName: " << iter->command->getName()
                 << " Priority: " << iter->priority);
    }

    void printAllocatedResources() const
    {
      for (std::map<std::string, double>::const_iterator it = m_allocated.begin(); 
           it != m_allocated.end(); ++it)
        debugMsg("ResourceArbiterInterface:printAllocatedResources", ' ' << it->first << " = " << it->second);
    }

    void printAcceptedCommands(LinkedQueue<CommandImpl> const &acceptCmds)
    {
      // Print accepted commands and the resources they consume.
      CommandImpl *cmd = acceptCmds.front();
      while (cmd) {
        debugMsg("ResourceArbiterInterface:printAcceptedCommands", 
                 " Accepted command: " << cmd->getName()
                 << " uses resources:");
        ResourceSet const &res = m_cmdResMap[cmd];
        for (ResourceSet::const_iterator resIter = res.begin();
             resIter != res.end();
             ++resIter)
          debugMsg("ResourceArbiterInterface:printAcceptedCommands", "  " << resIter->name);
        cmd = cmd->next();
      }
    }

  };

  ResourceArbiterInterface *makeResourceArbiter()
  {
    return new ResourceArbiterImpl();
  }

}
