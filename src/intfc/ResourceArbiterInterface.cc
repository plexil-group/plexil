/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

#include "Command.hh"
#include "Debug.hh"
#include "Error.hh"

#include <algorithm> // std::stable_sort()
#include <cctype>
#include <cmath>
#include <cstdlib> // strtod()
#include <cstring> // strspn(), strcspn() et al
#include <fstream>
#include <map>
#include <set>

namespace PLEXIL
{
  template <typename T> struct NameComparator
  {
    bool operator() (T const &x, T const &y) const
    {
      return x.name < y.name;
    }
  };

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
      : maxConsumableValue(0.0)
    {}

    ResourceNode(const double _maxConsumableValue, 
                 const std::vector<ChildResourceNode>& _children)
      : maxConsumableValue(_maxConsumableValue),
        children(_children)
    {}

    double maxConsumableValue;
    std::vector<ChildResourceNode> children;
  };

  typedef std::set<ChildResourceNode, NameComparator<ChildResourceNode> > ResourceSet;

  struct CommandPriorityEntry
  {
    CommandPriorityEntry(int32_t prio, Command *cmd)
      : priority(prio),
        command(cmd),
        resources()
    {
    }

    int32_t priority;
    Command *command;
    ResourceSet resources;
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
  typedef std::pair<Command *, ResourceSet> ResourceMapEntry;
  typedef std::map<Command *, ResourceSet> ResourceMap;
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
      flattenedRes.push_back(ChildResourceNode(cIter->weight, cIter->name, release));
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
    flattenedRes.push_back(ChildResourceNode(res.upperBound, resName, release));
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
    bool m_resourceFileRead;
    
  public:
    ResourceArbiterImpl()
      : m_resourceFileRead(false)
    {
      // TODO: Move this call out to application
      readResourceHierarchy("resource.data");
    }

    virtual ~ResourceArbiterImpl()
    {
    }

    virtual bool readResourceHierarchy(const std::string& fName)
    {
      m_resourceFileRead = false;
      std::ifstream myFile;
      myFile.open(fName.c_str());
      if (!myFile.is_open()) {
        debugMsg("ResourceArbiterInterface:readResourceHierarchy", "The file: " 
                 << fName << " does not exist. No resources read.");
        return false;
      }

      static char const *WHITESPACE = " \t\n\r\v\f";
      while (!myFile.eof()) {
        std::string dataStr;
        std::getline(myFile, dataStr);
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

        char * endptr = NULL;
        double maxCons = strtod(data, &endptr); // skips whitespace
        if (!maxCons && endptr == data) {
          std::cerr << "Error reading second element (consumable amount) of resource file: \n"
                    << dataStr << std::endl;
          myFile.close();
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
            myFile.close();
            return false;
          }

          // Skip whitespace
          endptr += strspn(endptr, WHITESPACE);
          len -= (endptr - data);
          data = endptr;

          if (!len || !*data) {
            std::cerr << "Error reading child resource name of resource file: \n"
                      << dataStr << std::endl;
            myFile.close();
            return false;
          }

          // get child resource name
          ws = strcspn(data, WHITESPACE);
          std::string const cName(data, ws);

          debugMsg("ResourceArbiterInterface:readResourceHierarchy",
                   " got dependent resource value " << d << ", name " << cName);
          
          children.push_back(ChildResourceNode(d, cName));

          len -= ws;
          data += ws;

          // Skip whitespace
          offset = strspn(data, WHITESPACE);
          len -= offset;
          data += offset;
        }

        m_resourceHierarchy[pName] = ResourceNode(maxCons, children);

      }
      debugMsg("ResourceArbiterInterface:readResourceHierarchy",
               " successfully read " << fName);
      m_resourceFileRead = true;
      return true;
    }
    
    virtual void arbitrateCommands(const std::vector<Command *>& cmds,
                                   std::vector<Command *>& acceptCmds)
    {
      debugMsg("ResourceArbiterInterface:arbitrateCommands",
               " processing " << cmds.size() << " commands");

      CommandPriorityList sortedCommands;
      partitionCommands(cmds, acceptCmds, sortedCommands);

      if (!sortedCommands.empty()) {
        debugStmt("ResourceArbiterInterface:printSortedCommands",
                  printSortedCommands(sortedCommands));

        optimalResourceArbitration(acceptCmds, sortedCommands);
      }
    
      debugStmt("ResourceArbiterInterface:printAcceptedCommands",
                printAcceptedCommands(acceptCmds));
      // Also print all the locked resources. 
      debugStmt("ResourceArbiterInterface:printAllocatedResources",
                printAllocatedResources());
    }

    virtual void releaseResourcesForCommand(Command *cmd)
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
    
    void partitionCommands(std::vector<Command *> const &cmds,
                           std::vector<Command *> &acceptCmds,
                           CommandPriorityList &sortedCommands)
    {
      for (std::vector<Command *>::const_iterator it = cmds.begin(); it != cmds.end(); ++it) {
        Command *cmd = *it;
        assertTrue_1(cmd);
        const ResourceValueList& resList = cmd->getResourceValues();

        if (resList.empty()) {
          debugMsg("ResourceArbiterInterface:partitionCommands",
                   " accepting " << cmd->getName() << " with no resource requests");
          acceptCmds.push_back(cmd); // no arbitration required
        }
        else {
          sortedCommands.push_back(CommandPriorityEntry(resList.front().priority,
                                                        cmd));

          ResourceSet &resources = sortedCommands.back().resources;
          for (ResourceValueList::const_iterator resListIter = resList.begin();
               resListIter != resList.end();
               ++resListIter)
            determineAllChildResources(*resListIter, m_resourceHierarchy, resources);
        }
      }

      // Sort the resulting list by priority
      if (sortedCommands.size() > 1)
        std::stable_sort(sortedCommands.begin(),
                         sortedCommands.end(),
                         CommandPriorityComparator());
    }

    void optimalResourceArbitration(std::vector<Command *> &acceptCmds,
                                    CommandPriorityList const &sortedCommands)
    {
      EstimateMap estimates;

      // Prepare estimate map and ensure entries in allocated map based on requests
      for (CommandPriorityList::const_iterator cmdIt = sortedCommands.begin();
           cmdIt != sortedCommands.end();
           ++cmdIt) {
        ResourceSet const &resources = cmdIt->resources;
        for (ResourceSet::const_iterator rit = resources.begin();
             rit != resources.end();
             ++rit) {
          std::string const &resName = rit->name;
          double value = 0.0;
          if (m_allocated.find(resName) == m_allocated.end())
            m_allocated[resName] = 0.0; // ensure entry in allocation map
          else
            value = m_allocated[resName];
          estimates[resName] = ResourceEstimate(value);
        }
      }

      for (CommandPriorityList::const_iterator cmdIt = sortedCommands.begin();
           cmdIt != sortedCommands.end();
           ++cmdIt) {
        EstimateMap savedEstimates = estimates;
        Command *cmd = cmdIt->command;
        ResourceSet const &requests = cmdIt->resources;
        bool invalid = false;
        
        debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                 " considering " << cmd->getName());

        for (ResourceSet::const_iterator iter = requests.begin();
             (iter != requests.end()) && !invalid;
             ++iter) {
          std::string const &resName = iter->name;
          double resValue = iter->weight;
          ResourceEstimate &est = estimates[resName];

          debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                   "  " << cmd->getName() << " requires " << resValue << " of " << resName);

          if (resValue < 0.0)
            est.renewable += resValue;
          else
            est.consumable += resValue;

          // Make sure that each of the individual resource usage does not exceed
          // the permitted maximum. This handles the worst case resource usage 
          // behavior of both types of resources.
          double resMax = maxConsumableResourceValue(resName);
          if (est.renewable < 0.0 || est.renewable > resMax) {
            invalid = true;
            debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                     " rejecting " << cmd->getName()
                     << " because renewable usage of " << resName << " exceeds limits");
          }
          else if (est.consumable < 0 || est.consumable > resMax) {
            invalid = true;
            debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                     " rejecting " << cmd->getName()
                     << " because consumable usage of " << resName << " exceeds limits");
          }
        }
        
        if (invalid) {
          // Back out effects of rejected command
          estimates = savedEstimates;
        }
        else {
          debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                   " accepting " << cmd->getName());

          acceptCmds.push_back(cmd);
          m_cmdResMap[cmd] = requests;

          // Update the allocated resource map to include the chosen command
          for (ResourceSet::const_iterator resIter = requests.begin();
               resIter != requests.end(); ++resIter)
            m_allocated[resIter->name] += resIter->weight;
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
      if (m_resourceFileRead && (m_resourceHierarchy.find(resName) != m_resourceHierarchy.end()))
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

    void printAcceptedCommands(const std::vector<Command *>& acceptCmds)
    {
      // Print accepted commands and the resources they consume.
      for (std::vector<Command *>::const_iterator it = acceptCmds.begin(); 
           it != acceptCmds.end();
           ++it) {
        Command *cmd = *it;
        debugMsg("ResourceArbiterInterface:printAcceptedCommands", 
                 " Accepted command: " << cmd->getName()
                 << " uses resources:");
        ResourceSet const &res = m_cmdResMap[cmd];
        for (ResourceSet::const_iterator resIter = res.begin();
             resIter != res.end();
             ++resIter)
          debugMsg("ResourceArbiterInterface:printAcceptedCommands", "  " << resIter->name);
      }
    }

  };

  ResourceArbiterInterface *makeResourceArbiter()
  {
    return new ResourceArbiterImpl();
  }

}
