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

#include <cctype>
#include <cmath>
#include <cstdlib> // strtod()
#include <fstream>
#include <map>
#include <set>

namespace PLEXIL
{
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

  struct ResourceComparator
  {
    bool operator() (const ChildResourceNode& x, const ChildResourceNode& y) const
    {
      return x.name < y.name;
    }
  };

  // Type names
  typedef std::set<ChildResourceNode, ResourceComparator> ResourceMapEntry;
  typedef std::map<Command *, ResourceMapEntry> ResourceMap;
  typedef std::map<std::string, ResourceNode> ResourceHierarchyMap;
  typedef std::multimap<int32_t, Command *> CommandPriorityMap;
    
  static void partitionCommands(std::vector<Command *> const &cmds,
                                std::vector<Command *> &acceptCmds,
                                std::vector<Command *> &sortedCommands)
  {
    CommandPriorityMap sortedCommandMap;

    for (std::vector<Command *>::const_iterator it = cmds.begin(); it != cmds.end(); ++it) {
      Command *cmd = *it;
      assertTrue_1(cmd);
      const ResourceValueList& resList = cmd->getResourceValues();

      if (resList.empty()) {
        debugMsg("ResourceArbiterInterface:partitionCommands",
                 " accepting " << cmd->getName() << " with no resource requests");
        acceptCmds.push_back(cmd); // no arbitration required
      }
      else
        sortedCommandMap.insert(std::make_pair(resList.front().priority, cmd));
    }

    // Flatten map
    for (CommandPriorityMap::const_iterator iter = sortedCommandMap.begin();
         iter != sortedCommandMap.end();
         ++iter)
      sortedCommands.push_back(iter->second);
  }

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
                                         ResourceMapEntry &resourcesNeeded)
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
    std::map<std::string, double> m_lockedRes;
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

      std::vector<Command *> sortedCommands;

      partitionCommands(cmds, acceptCmds, sortedCommands);
      preprocessCommandsToArbitrate(sortedCommands);
    
      debugStmt("ResourceArbiterInterface:printSortedCommands",
                printSortedCommands(sortedCommands));

      optimalResourceArbitration(acceptCmds, sortedCommands);
    
      debugStmt("ResourceArbiterInterface:printAcceptedCommands",
                printAcceptedCommands(acceptCmds));
      // Also print all the locked resources. 
      debugStmt("ResourceArbiterInterface:printLockedResources",
                printLockedResources());
    }

    virtual void releaseResourcesForCommand(Command *cmd)
    {
      // loop through all the resources used by the command and remove each of them
      // from the locked list as well as the command list if there are releasable.
      ResourceMap::iterator resListIter = m_cmdResMap.find(cmd);
      if (resListIter != m_cmdResMap.end()) {
        for (ResourceMapEntry::const_iterator resIter = resListIter->second.begin();
             resIter != resListIter->second.end();
             ++resIter) {
          if (resIter->release) 
            m_lockedRes[resIter->name] -= resIter->weight;
          if (m_lockedRes[resIter->name] == 0)
            m_lockedRes.erase(resIter->name);
        }
        m_cmdResMap.erase(resListIter);
      }
    
      debugMsg("ResourceArbiterInterface:releaseResourcesForCommand", 
               "remaining locked resources after releasing for command " << cmd->getName());
      printLockedResources();
    }

  private:

    void preprocessCommandsToArbitrate(std::vector<Command *> const &cmds)
    {
      for (std::vector<Command *>::const_iterator it = cmds.begin(); it != cmds.end(); ++it)
        preprocessCommand(*it);
    }

    void preprocessCommand(Command *cmd)
    {
      const ResourceValueList& resList = cmd->getResourceValues();
      ResourceMapEntry resourcesNeeded;

      // Expand all the resources in the hierarchy.
      for (ResourceValueList::const_iterator resListIter = resList.begin();
           resListIter != resList.end();
           ++resListIter)
        determineAllChildResources(*resListIter, m_resourceHierarchy, resourcesNeeded);

      m_cmdResMap[cmd] = resourcesNeeded;
    }

    void optimalResourceArbitration(std::vector<Command *> &acceptCmds,
                                    std::vector<Command *> &sortedCommands)
    {
      // convert the priority sorted map to a vector for convenience
      std::vector<Command *> commandCombo;
      std::map<std::string, double> totalConsResMap;
      std::map<std::string, double> totalRenewResMap;

      // Initialize the total resource levels to the locked values
      for (ResourceMap::const_iterator cmdIt = m_cmdResMap.begin();
           cmdIt != m_cmdResMap.end();
           ++cmdIt) {
        for (ResourceMapEntry::const_iterator rit = cmdIt->second.begin();
             rit != cmdIt->second.end();
             ++rit) {
          std::string const &resName = rit->name;
          if (totalConsResMap.find(resName) != totalConsResMap.end())
            continue; // ignore duplicates
          double resLocked = (m_lockedRes.find(resName) != m_lockedRes.end()) ?
            m_lockedRes.find(resName)->second : 0.0;
          totalConsResMap[resName] = totalRenewResMap[resName] = resLocked;
        }
      }
    
      for (std::vector<Command *>::const_iterator cmdIter1 = sortedCommands.begin();
           cmdIter1 != sortedCommands.end(); ++cmdIter1) {
        debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                 " considering " << (*cmdIter1)->getName());
        
        std::map<std::string, double> localConsResMap = totalConsResMap;
        std::map<std::string, double> localRenewResMap = totalRenewResMap;

        bool invalid = false;

        ResourceMapEntry requests = m_cmdResMap[*cmdIter1];

        for (ResourceMapEntry::const_iterator iter = requests.begin();
             (iter != requests.end()) && !invalid;
             ++iter) {

          std::string const &resName = iter->name;
          double resValue = iter->weight;
          debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                   " " << (*cmdIter1)->getName() << " requires " << resValue << " of " << resName);

          if (resValue < 0.0)
            localRenewResMap[resName] += resValue;
          else
            localConsResMap[resName] += resValue;

          // Make sure that each of the individual resource usage does not exceed
          // the permitted maximum. This handles the worst case resource usage 
          // behavior of both types of resources.
          if (isResourceUsageOutsideLimits(localConsResMap[resName], resName) ||
              isResourceUsageOutsideLimits(localRenewResMap[resName], resName)) {
            invalid = true;
            debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                     " rejecting " << (*cmdIter1)->getName()
                     << " because usage of " << resName << " exceeds limits");
          }
        }
        
        if (invalid) {
          debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                   " rejected " << (*cmdIter1)->getName());
        }
        else {
          commandCombo.push_back(*cmdIter1);
          debugMsg("ResourceArbiterInterface:optimalResourceArbitration",
                   " accepting " << (*cmdIter1)->getName());

          // Update the total resource levels to include the chosen command
          ResourceMapEntry const & entry = m_cmdResMap[*cmdIter1];
          for (ResourceMapEntry::const_iterator resIter = entry.begin();
               resIter != entry.end(); ++resIter) {
            std::string const &resName = resIter->name;
            totalConsResMap[resName] += localConsResMap[resName];
            totalRenewResMap[resName] += localRenewResMap[resName];
          }
        }
      }

      // Process accepted command combination
      for (std::vector<Command *>::const_iterator cIter = commandCombo.begin();
           cIter < commandCombo.end(); ++cIter) {
        acceptCmds.push_back(*cIter);
        for (ResourceMapEntry::const_iterator resIter = 
               m_cmdResMap[*cIter].begin();
             resIter != m_cmdResMap[*cIter].end(); ++resIter) {

          // Check if the resource is already in the locked structure
          // If yes, just add to the existing usage contribution.
          // if no, add new entry.
          if (m_lockedRes.find(resIter->name) != m_lockedRes.end()) {
            m_lockedRes[resIter->name] += resIter->weight;
          }
          else {
            m_lockedRes[resIter->name] = resIter->weight;
          }
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

    void printSortedCommands(std::vector<Command *> &sortedCommands) const
    {
      for (std::vector<Command *>::const_iterator iter = sortedCommands.begin();
           iter != sortedCommands.end();
           ++iter) {
        Command const *cmd = *iter;
        double priority = cmd->getResourceValues().front().priority;
        debugMsg("ResourceArbiterInterface:printSortedCommands", 
                 "CommandName: " << cmd->getName()
                 << " Priority: " << priority);
      }
    }

    void printLockedResources() const
    {
      for (std::map<std::string, double>::const_iterator it = m_lockedRes.begin(); 
           it != m_lockedRes.end(); ++it)
        debugMsg("ResourceArbiterInterface:printLockedResources", it->first << ", " << it->second);
    }

    void printAcceptedCommands(const std::vector<Command *>& acceptCmds)
    {
      // Print accepted commands and the resources they consume.
      for (std::vector<Command *>::const_iterator it = acceptCmds.begin(); 
           it != acceptCmds.end();
           ++it) {
        Command *cmd = *it;
        debugMsg("ResourceArbiterInterface:printAcceptedCommands", 
                 "Accepted command: " << cmd->getName()
                 << " uses resources:");
        ResourceMapEntry const &res = m_cmdResMap[cmd];
        for (ResourceMapEntry::const_iterator resIter = res.begin();
             resIter != res.end();
             ++resIter)
          debugMsg("ResourceArbiterInterface:printAcceptedCommands", resIter->name);
      }
    }

  };

  ResourceArbiterInterface *makeResourceArbiter()
  {
    return new ResourceArbiterImpl();
  }

}
