/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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
#include "resource-tags.hh"

#include <queue>
#include <fstream>
#include <cmath>
#include <cctype>

namespace PLEXIL {

  //
  // String constants
  //
  const std::string RESOURCE_NAME_STR(RESOURCE_NAME_TAG);
  const std::string RESOURCE_PRIORITY_STR(RESOURCE_PRIORITY_TAG);
  const std::string RESOURCE_LOWER_BOUND_STR(RESOURCE_LOWER_BOUND_TAG);
  const std::string RESOURCE_UPPER_BOUND_STR(RESOURCE_UPPER_BOUND_TAG);
  const std::string RESOURCE_RELEASE_AT_TERMINATION_STR(RESOURCE_RELEASE_AT_TERMINATION_TAG);

  bool ResourceComparator::operator() (const ChildResourceNode& x, const ChildResourceNode& y) const
  {
    return (x.name < y.name ? true : false);
  }

  void ResourceArbiterInterface::arbitrateCommands(const std::list<CommandId>& cmds,
                                                   std::set<CommandId>& acceptCmds)
  {
    //1. Construct the sorted priority table for each resource

    preprocessCommandToArbitrate(cmds, acceptCmds);
    
    printResourceCommandMap();

    printSortedCommands();

    optimalResourceArbitration(acceptCmds);
    
    printAcceptedCommands(acceptCmds);

    // Also print all the locked resources.
    printLockedResources();
  }

  void ResourceArbiterInterface::preprocessCommandToArbitrate
  (const std::list<CommandId>& cmds, std::set<CommandId>& acceptCmds)
  {
    m_prioritySortedCommands.clear();
    m_resCmdMap.clear();

    //Loop through each command
    for (std::list<CommandId>::const_iterator it = cmds.begin(); it != cmds.end(); ++it) {
      CommandId cmd = *it;
      check_error(cmd.isValid());
      const ResourceValuesList& resList = cmd->getResourceValues();

      if (!resList.empty()) {
        // Sort commands by priority
            
        ResourceValues::const_iterator prioIt = resList.begin()->find(RESOURCE_PRIORITY_STR);
        assertTrueMsg(prioIt != resList.begin()->end(), "ResourcePriority not found");
        int priority = (int) prioIt->second.getIntValue();
        m_prioritySortedCommands.insert(std::make_pair(priority, cmd));

        std::set<ChildResourceNode, ResourceComparator> resourcesNeeded;
        // Expand all the resources in the hierarchy.
            
        for (ResourceValuesList::const_iterator resListIter = resList.begin();
             resListIter != resList.end();
             ++resListIter) {
          ResourceValues::const_iterator nameit = resListIter->find(RESOURCE_NAME_STR);
          assertTrueMsg(nameit != resListIter->end(), "ResourceName not found");
          std::string resName = nameit->second.getStringValue();
          // Flatten out the hierarchy into a vector ChildResourceNode with scaled weights

          std::vector<ChildResourceNode> flattenedRes;
          determineAllChildResources(*resListIter, flattenedRes);

          //loop through each hierarchy element in the flattened structure
          for(std::vector<ChildResourceNode>::const_iterator rIter = flattenedRes.begin();
              rIter != flattenedRes.end(); ++rIter)
            {
              if (resourcesNeeded.find(*rIter) != resourcesNeeded.end())
                {
                  // resource is already present. If current is explicitly specified in 
                  // the command then the value can be overwritten. The
                  // child resource scales will not be overwritten however.
                  // In fact this check will be valid only for the first element
                  // of the flattened vector. This means that if a resource value
                  // has to be overwritten it has to be explicitly scpecified in the 
                  // plan
                  if ((*rIter).name == resName)
                    resourcesNeeded.insert(*rIter);
                }
              else
                {
                  // resource not already present. Insert it in the map.
                  resourcesNeeded.insert(*rIter);
                }

            }
        }
        // Loop through the expanded set of resources per command
        for (std::set<ChildResourceNode, ResourceComparator>::const_iterator resNeededIter = resourcesNeeded.begin();
             resNeededIter != resourcesNeeded.end(); ++resNeededIter)
          {
            if (m_resCmdMap.find(resNeededIter->name) != m_resCmdMap.end())
              {
                // resource is already present in the map. just append the new command
                m_resCmdMap.find(resNeededIter->name)->second.insert(cmd);
              }
            else
              {
                // resource is not already in the map. Add it
                std::set<CommandId> setTmp;
                setTmp.insert(cmd);
                m_resCmdMap.insert(std::make_pair(resNeededIter->name, setTmp));
              }
          }
            
        m_cmdResMap[LabelStr(cmd->getName()).toString()] = resourcesNeeded;
      }
      else
        acceptCmds.insert(cmd);
    }
  }


  void ResourceArbiterInterface::releaseResourcesForCommand(const LabelStr& cmdName)
  {
    // loop through all the resources used by the command and remove each of them
    // from the locked list as well as the command list if there are releasable.

    std::map<std::string, std::set<ChildResourceNode, ResourceComparator> >::iterator resListIter;
    if ((resListIter = m_cmdResMap.find(cmdName.toString())) != m_cmdResMap.end())
      {
        for(std::set<ChildResourceNode, ResourceComparator>::const_iterator resIter = 
              resListIter->second.begin();
            resIter != resListIter->second.end(); ++resIter)
          {
            if (resIter->release) 
              m_lockedRes[resIter->name] -= resIter->weight;
            if (m_lockedRes[resIter->name] == 0)
              m_lockedRes.erase(resIter->name);
          }
        m_cmdResMap.erase(resListIter);
      }
    
    debugMsg("ResourceArbiterInterface:releaseResourcesForCommand", 
             "remaining locked resources after releasing for command: " << cmdName.toString());
    printLockedResources();
  }


  double ResourceArbiterInterface::resourceAmountNeededByCommand(const std::string& resName, 
                                                                 const std::string& cmdName)
  {
    const std::set<ChildResourceNode, ResourceComparator>& resList = 
      m_cmdResMap.find(cmdName)->second;
    double resNeeded=0.0;
    bool found = false;
    for(std::set<ChildResourceNode, ResourceComparator>::const_iterator resListIter = resList.begin();
        (resListIter != resList.end()) && !found; ++resListIter)
      {
        if (resListIter->name == resName)
          {
            found = true;
            resNeeded = resListIter->weight;
          }
      }
    return resNeeded;
  }

  void ResourceArbiterInterface::optimalResourceArbitration (std::set<CommandId>& acceptCmds)
  {
    // convert the priority sorted map to a vector for convenience

    std::vector<CommandId> sortedCmdVector;
    std::vector<CommandId> commandCombo;
    std::map<std::string, double> totalConsResMap;
    std::map<std::string, double> totalRenewResMap;

    for(std::multimap<int, CommandId>::const_iterator iter = m_prioritySortedCommands.begin();
        iter != m_prioritySortedCommands.end(); ++iter)
      sortedCmdVector.push_back(iter->second);

    // Initialize the total resource levels to the locked values
    for (std::map<std::string, std::set<CommandId> >::const_iterator iter = 
           m_resCmdMap.begin(); iter != m_resCmdMap.end(); ++iter)
      {
        std::string resName = iter->first;
        double resLocked = (m_lockedRes.find(resName) != m_lockedRes.end()) ?
          m_lockedRes.find(resName)->second : 0.0;
        totalConsResMap[resName] = totalRenewResMap[resName] = resLocked;
      }
    
    for (std::vector<CommandId>::const_iterator cmdIter1 = sortedCmdVector.begin();
         cmdIter1 != sortedCmdVector.end(); ++cmdIter1)
      {
        std::string cmd1Name = (*cmdIter1)->getName().getStringValue();

        std::map<std::string, double> localConsResMap = totalConsResMap;
        std::map<std::string, double> localRenewResMap = totalRenewResMap;

        bool invalid = false;

        for (std::map<std::string, std::set<CommandId> >::const_iterator iter = 
               m_resCmdMap.begin();
             (iter != m_resCmdMap.end()) && !invalid; ++iter)
          {
            std::string resName = iter->first;

            double resValue = resourceAmountNeededByCommand(resName, cmd1Name);

            if (resValue < 0.0)
              localRenewResMap[resName] += resValue;
            else
              localConsResMap[resName] += resValue;

            // Make sure that each of the individual resource usage does not exceed
            // the permitted maximum. This handles the worst case resource usage 
            // behavior of both types of resources.
            if (isResourceUsageOutsideLimits(localConsResMap[resName], resName) ||
                isResourceUsageOutsideLimits(localRenewResMap[resName], resName))
              {
                invalid = true;
              }
          }
        
        if (!invalid)
          {
            commandCombo.push_back(*cmdIter1);

            // Update the total resource levels to include the chosen command
            for (std::set<ChildResourceNode, ResourceComparator>::const_iterator resIter = 
                   m_cmdResMap[(*cmdIter1)->getName().getStringValue()].begin();
                 resIter != m_cmdResMap[(*cmdIter1)->getName().getStringValue()].end(); ++resIter)
              //            for (std::map<std::string, std::set<CommandId> >::const_iterator iter = 
              //                   m_resCmdMap.begin(); iter != m_resCmdMap.end(); ++iter)
              {
                //                std::string resName = iter->first;
                std::string resName = resIter->name;
                totalConsResMap[resName] += localConsResMap[resName];
                totalRenewResMap[resName] += localRenewResMap[resName];
              }
          }
      }

    // Process accepted command combination
    for(std::vector<CommandId>::const_iterator cIter = commandCombo.begin();
        cIter < commandCombo.end(); ++cIter)
      {
        acceptCmds.insert(*cIter);
        
        for (std::set<ChildResourceNode, ResourceComparator>::const_iterator resIter = 
               m_cmdResMap[(*cIter)->getName().getStringValue()].begin();
             resIter != m_cmdResMap[(*cIter)->getName().getStringValue()].end(); ++resIter)
          {
            // Check if the resource is already in the locked structure
            // If yes, just add to the existing usage contribution.
            // if no, add new entry.
            
            if (m_lockedRes.find(resIter->name) != m_lockedRes.end())
              {
                m_lockedRes[resIter->name] += resIter->weight;
              }
            else
              {
                m_lockedRes[resIter->name] = resIter->weight;
              }
          }
      }
  }

  bool ResourceArbiterInterface::isResourceUsageOutsideLimits(const double resNeeded, 
                                                              const std::string& resName) const
  {
    return ((resNeeded < maxRenewableResourceValue(resName)) ||
            (resNeeded > maxConsumableResourceValue(resName)));
  }

  void ResourceArbiterInterface::determineAllChildResources(const ResourceValues& res,
                                                            std::vector<ChildResourceNode>& flattenedRes)
  {
    ResourceValues::const_iterator nameit = res.find(RESOURCE_NAME_STR);
    assertTrueMsg(nameit != res.end(), "ResourceName not found");
    std::string resName = nameit->second.getStringValue();

    double scale = (res.find(RESOURCE_UPPER_BOUND_STR) != res.end()) ? 
      (double) (res.find(RESOURCE_UPPER_BOUND_STR)->second.getDoubleValue()) : 1.0;

    bool release = (res.find(RESOURCE_RELEASE_AT_TERMINATION_STR) != res.end()) 
      ? (bool) res.find(RESOURCE_RELEASE_AT_TERMINATION_STR)->second.getBoolValue() : true;
    // Push the root in to the flattened vector
    flattenedRes.push_back(ChildResourceNode(scale, resName, release));
    // Push all the children into a queue (uses a std::vector).

    if (m_resourceHierarchy.find(resName) != m_resourceHierarchy.end()) {
      const std::vector<ChildResourceNode>& children = 
        m_resourceHierarchy.find(resName)->second.children;
      std::queue<ChildResourceNode> q;
      for (std::vector<ChildResourceNode>::const_iterator cIter = children.begin();
           cIter != children.end();
           ++cIter) {
        q.push((*cIter));
      }
      while (!q.empty()) {
        const ChildResourceNode& child = q.front();
        q.pop();
        flattenedRes.push_back(ChildResourceNode(child.weight, child.name, release));
        const std::vector<ChildResourceNode>& children = 
          m_resourceHierarchy.find(child.name)->second.children;
        for (std::vector<ChildResourceNode>::const_iterator cIter = children.begin();
             cIter != children.end();
             ++cIter) {
          q.push((*cIter));
        }
      }
    }
  }

  bool ResourceArbiterInterface::readResourceHeirarchy(const std::string& fName)
  {
    std::string dataStr;
    std::string delimiter = " ";
    std::ifstream myFile;
    myFile.open(fName.c_str());
    if (!myFile.is_open())
      {
        debugMsg("ResourceArbiterInterface:readResourceHeirarchy", "The file: " 
                 << fName << " does not exist. No resources read.");
        return false;
      }
    while (!myFile.eof())
      {
        std::getline(myFile, dataStr);
        if (dataStr.substr(0,1) != "%" && ~isspace(dataStr.substr(0,1)[0]))
          {
            // first element which is the parent resource name
            std::string::size_type lastPos = dataStr.find_first_not_of(delimiter, 0);
            std::string::size_type pos = dataStr.find_first_of(delimiter, lastPos);
            if ((std::string::npos == pos) && (std::string::npos == lastPos))
              {
                std::cerr << "Error reading the first element of resource file: "
                          << dataStr << std::endl;
                myFile.close();
                return false;
              }
            std::string pName = dataStr.substr(lastPos, pos - lastPos);

            // second element which is the max renewable value. DOn't have this 
            // anymore. Fixed to 0.0. Can be converted to initial value.
            /*
              lastPos = dataStr.find_first_not_of(delimiter, pos);
              pos = dataStr.find_first_of(delimiter, lastPos);
              if ((std::string::npos == pos) && (std::string::npos == lastPos))
              {
              std::cerr << "Error reading the second element of resource file: "
              << dataStr << std::endl;
              myFile.close();
              return false;
              }
              std::istringstream maxRstr(dataStr.substr(lastPos, pos - lastPos));
              double maxRen;
              maxRstr >> maxRen;
            */
            double maxRen = 0.0;

            // third element which is the max consumable value
            lastPos = dataStr.find_first_not_of(delimiter, pos);
            pos = dataStr.find_first_of(delimiter, lastPos);
            if ((std::string::npos == pos) && (std::string::npos == lastPos))
              {
                std::cerr << "Error reading the third element of resource file: "
                          << dataStr << std::endl;
                myFile.close();
                return false;
              }
            std::istringstream maxCstr(dataStr.substr(lastPos, pos - lastPos));
            double maxCons;
            maxCstr >> maxCons;

            lastPos = dataStr.find_first_not_of(delimiter, pos);
            pos = dataStr.find_first_of(delimiter, lastPos);
            std::vector<ChildResourceNode> children;
            while ((std::string::npos != pos) || (std::string::npos != lastPos))
              {

                std::istringstream str(dataStr.substr(lastPos, pos - lastPos));
                double d;
                str >> d;

                lastPos = dataStr.find_first_not_of(delimiter, pos);
                pos = dataStr.find_first_of(delimiter, lastPos);

                std::string cName = dataStr.substr(lastPos, pos - lastPos);
                
                lastPos = dataStr.find_first_not_of(delimiter, pos);
                pos = dataStr.find_first_of(delimiter, lastPos);

                children.push_back(ChildResourceNode(d, cName));
              }

            m_resourceHierarchy[pName] = ResourceNode(maxCons, maxRen, children);
          }
      }
    return true;
  }

  double ResourceArbiterInterface::maxConsumableResourceValue(const std::string& resName) const
  {
    if (m_resourceFileRead && (m_resourceHierarchy.find(resName) != m_resourceHierarchy.end()))
      return m_resourceHierarchy.find(resName)->second.maxConsumableValue;
    return 1.0;
  }

  double ResourceArbiterInterface::maxRenewableResourceValue(const std::string& resName) const
  {
    if (m_resourceFileRead && (m_resourceHierarchy.find(resName) != m_resourceHierarchy.end()))
      return -1.0*m_resourceHierarchy.find(resName)->second.maxRenewableValue;
    return 0.0;
  }

  void ResourceArbiterInterface::printSortedCommands() const
  {
    for(std::multimap<int, CommandId>::const_iterator iter = m_prioritySortedCommands.begin();
        iter != m_prioritySortedCommands.end(); ++iter)
      {
        CommandId cmdId = iter->second;
        debugMsg("ResourceArbiterInterface:printSortedCommands", 
                 "CommandName: " << cmdId->getName().getStringValue() 
                 << " Priority: " << iter->first);
      }
  }

  void ResourceArbiterInterface::printResourceCommandMap() const
  {
    for (std::map<std::string, std::set<CommandId> >::const_iterator iter = 
           m_resCmdMap.begin();
         iter != m_resCmdMap.end(); ++iter)
      {
        debugMsg("ResourceArbiterInterface:printResourceCommandMap", 
                 "Resource name: " << iter->first);
        for (std::set<CommandId>::const_iterator iter2 = iter->second.begin(); 
             iter2 != iter->second.end(); ++iter2)
          {
            CommandId cmdId = *iter2;
            debugMsg("ResourceArbiterInterface:printResourceCommandMap", 
                     "cmds: " << cmdId->getName().getStringValue() << " uses the following resources");

            const ResourceValuesList& resList = cmdId->getResourceValues();
            for (ResourceValuesList::const_iterator resListIter = resList.begin();
                 resListIter != resList.end();
                 ++resListIter) {
              ResourceValues::const_iterator nameit = resListIter->find(RESOURCE_NAME_STR);
              assertTrueMsg(nameit != resListIter->end(), "ResourceName not found");
              std::string resName = nameit->second.getStringValue();
              double ubound = (resListIter->find(RESOURCE_UPPER_BOUND_STR) != resListIter->end()) ? 
                (double) (resListIter->find(RESOURCE_UPPER_BOUND_STR)->second.getDoubleValue()) : 1.0;
              double lbound = (resListIter->find(RESOURCE_LOWER_BOUND_STR) != resListIter->end()) ? 
                (double) (resListIter->find(RESOURCE_LOWER_BOUND_STR)->second.getDoubleValue()) : 1.0;
              ResourceValues::const_iterator prioIt = resListIter->find(RESOURCE_PRIORITY_STR);
              assertTrueMsg(prioIt != resListIter->end(), "ResourcePriority not found");
              int priority = static_cast<int>(prioIt->second.getIntValue());
              debugMsg("ResourceArbiterInterface:printResourceCommandMap", 
                       "<" << resName << "," << priority << "," << lbound
                       << "," << ubound << ">");
            }
          }
      }
  }

  void ResourceArbiterInterface::printLockedResources() const
  {
    for (std::map<std::string, double>::const_iterator it = m_lockedRes.begin(); 
         it != m_lockedRes.end(); ++it)
      {
        debugMsg("ResourceArbiterInterface:printLockedResources", it->first << ", " << it->second);
      }
  }

  void ResourceArbiterInterface::printAcceptedCommands(const std::set<CommandId>& acceptCmds)
  {
    // Print accepted commands and the resources they consume.
    
    
    for (std::set<CommandId>::const_iterator it = acceptCmds.begin(); 
         it != acceptCmds.end(); ++it)
      {
        CommandId cmd = *it;
        const std::string& name = cmd->getName().getStringValue();
        debugMsg("ResourceArbiterInterface:printAcceptedCommands", 
                 "Accepted command: " << name
                 << " uses resources:");
        for (std::set<ChildResourceNode, ResourceComparator>::const_iterator resIter = 
               m_cmdResMap[name].begin();
             resIter != m_cmdResMap[name].end(); ++resIter)
          debugMsg("ResourceArbiterInterface:printAcceptedCommands", resIter->name);
      }
  }

}
