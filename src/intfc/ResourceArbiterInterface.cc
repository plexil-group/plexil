/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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
#include <fstream>
#include <map>
#include <set>

#include <cctype>
#include <cstdlib> // strtod()
#include <cstring> // strspn(), strcspn() et al

namespace PLEXIL
{

  //! \struct ChildResourceNode
  //! \brief Represents both a child resource in the resource
  //!        hierarchy, and a resource request by a command.
  //! \see ResourceNode
  struct ChildResourceNode final
  {

    //! \brief Constructor.
    //! \param _weight The weight of this resource.
    //! \param _name The name of this resource.
    //! \param _release Whether this resource is released when the command using it completes.
    ChildResourceNode(const double _weight,
                      const std::string &_name,
                      const bool _release = true)
      : name(_name),
        weight(_weight),
        release(_release)
    {
    }

    //
    // The constructors and assignment operators below should only be
    // used by container templates.
    //

    //! \brief Default constructor.
    ChildResourceNode()
      : name(),
        weight(0.0),
        release(false)
    {
    }

    //! \brief Copy constructor.
    ChildResourceNode(ChildResourceNode const &) = default;

    //! \brief Move constructor.
    ChildResourceNode(ChildResourceNode &&) = default;

    //! \brief Copy assignment operator.
    ChildResourceNode &operator=(ChildResourceNode const &) = default;

    //! \brief Move assignment operator.
    ChildResourceNode &operator=(ChildResourceNode &&) = default;

    //! \brief Destructor.
    ~ChildResourceNode() = default;

    std::string name;  //!< The name of this resource.
    double weight;     //!< The weight of this resource.
    bool release;      //!< Whether the resource is released when a command completes.
  };

  //! \brief Compare two ChildResourceNode instances by their names.
  //! \param x Const reference to a ChildResourceNode.
  //! \param y Const reference to another ChildResourceNode.
  //! \return true if x.name < y.name, false otherwise.
  inline bool operator<(ChildResourceNode const &x, ChildResourceNode const &y)
  {
    return x.name < y.name;
  }

  //! \typedef ResourceSet
  //! \brief A sorted set of ChildResourceNodes.
  //! \see ResourceMap
  using ResourceSet = std::set<ChildResourceNode>;

  //! \typedef ResourceMap
  //! \brief A map, keyed by command, to a set of resources required by that command.
  //! \see ResourceSet
  using ResourceMap = std::map<CommandImpl *, ResourceSet>;

  //! \typedef ResourceMapEntry
  //! \brief The type of an entry in a ResourceMap.
  //! \see ResourceMap
  using ResourceMapEntry = ResourceMap::value_type;

  //! \struct ResourceNode
  //! \brief Represents a resource, optionally with children.
  //!        Its name is stored in the ResourceHierarchyMap.
  //! \see ChildResourceNode
  struct ResourceNode final
  {
    ResourceNode(const double _maxConsumableValue)
      : children(),
        maxConsumableValue(_maxConsumableValue)
    {
    }

    //
    // The constructors and assignment operators below should only be
    // used by container templates.
    //

    //! \brief Default constructor.
    ResourceNode()
      : children(),
        maxConsumableValue(0.0)
    {
    }

    //! \brief Copy constructor.
    ResourceNode(ResourceNode const &) = default;

    //! \brief Move constructor.
    ResourceNode(ResourceNode &&) = default;

    //! \brief Copy assignment operator.
    ResourceNode &operator=(ResourceNode const &) = default;

    //! \brief Move assignment operator.
    ResourceNode &operator=(ResourceNode &&) = default;

    //! \brief Destructor.
    ~ResourceNode() = default;

    std::vector<ChildResourceNode> children; //!< The children of this resource.
    double maxConsumableValue;               //!< The available amount of this resource.
  };

  //! \typedef ResourceHierarchyMap
  //! \brief A map, keyed by resource name, of ResourceNode instances.
  //! \see Resource Node
  using ResourceHierarchyMap = std::map<std::string, ResourceNode>;

  //! \struct CommandPriorityEntry
  //! \brief Associates a command to be executed, its priority, and
  //!        the resources it requires.
  struct CommandPriorityEntry final
  {

    //! \brief Constructor.
    //! \param prio The command priority.
    //! \param cmd Pointer to the actual command instance.
    CommandPriorityEntry(int32_t prio, CommandImpl *cmd)
      : resources(),
        command(cmd),
        priority(prio)
    {
    }

    //! \brief Copy constructor.
    CommandPriorityEntry(CommandPriorityEntry const &) = default;

    //! \brief Move constructor.
    CommandPriorityEntry(CommandPriorityEntry &&) = default;

    //! \brief Copy assignment operator.
    CommandPriorityEntry &operator=(CommandPriorityEntry const &) = default;

    //! \brief Move assignment operator.
    CommandPriorityEntry &operator=(CommandPriorityEntry &&) = default;

    ResourceSet resources;  //!< The resources requested by this command.
    CommandImpl *command;   //!< Pointer to the command instance.
    int32_t priority;       //!< The priority of the command.
  };

  //! \brief Overloaded less-than operator for CommandPriorityEntry instances.
  //! \param x Const reference to a CommandPriorityEntry instance.
  //! \param y Const reference to another CommandPriorityEntry instance.
  //! \return true if x.priority < y.priority, false otherwise.
  bool operator<(CommandPriorityEntry const &x, CommandPriorityEntry const &y)
  {
    return x.priority < y.priority;
  }

  //! \typedef CommandPriorityList
  //! \brief A container of CommandPriorityEntry instances.
  using CommandPriorityList = std::vector<CommandPriorityEntry>;

  //! \struct ResourceEstimate
  //! \brief Represents the renewable and consumable usage of a
  //         resource.  Used internally by
  //         ResourceArbiterImpl::optimalResourceAllocation().
  struct ResourceEstimate final
  {
    //! \brief Constructor.
    //! \param initial The available amount of a resource at the start
    //!                of arbitration.
    ResourceEstimate(double initial)
      : renewable(initial),
        consumable(initial)
    {
    }

    //! \brief Default constructor.
    ResourceEstimate()
      : renewable(0.0),
        consumable(0.0)
    {
    }

    //! \brief Copy constuctor.
    ResourceEstimate(ResourceEstimate const &) = default;

    //! \brief Move constructor.
    ResourceEstimate(ResourceEstimate &&) = default;

    //! \brief Copy assignment operator.
    ResourceEstimate &operator=(ResourceEstimate const &) = default;

    //! \brief Move assignment operator.
    ResourceEstimate &operator=(ResourceEstimate &&) = default;

    //! \brief Destructor.
    ~ResourceEstimate() = default;
  
    double renewable;
    double consumable;
  };

  //! \typedef EstimateMap
  //! \brief A map, keyed by resource name, of estimated resource usage.
  using EstimateMap = std::map<std::string, ResourceEstimate>;

  //! \brief Recursively collect the descendants of a resource and their weights.
  //! \param[in] The map of all known resources.
  //! \param[in] resName The name of a resource.
  //! \param[in] release Whether the resource will be released upon command completion.
  //! \param[out] flattenedRes The resulting list of the resource and its descendants.
  static void determineChildResources(ResourceHierarchyMap const &resourceHierarchy,
                                      std::string const &resName,
                                      bool release,
                                      std::vector<ChildResourceNode> &flattenedRes)
  {
    debugMsg("ResourceArbiter:determineChildResources", ' ' << resName);
    ResourceHierarchyMap::const_iterator it = resourceHierarchy.find(resName);
    if (it == resourceHierarchy.end())
      return;

    const std::vector<ChildResourceNode> &children = it->second.children;
    if (children.empty())
      return;
    
    // Recurse over children
    for (ChildResourceNode const &child : children) {
      flattenedRes.emplace_back(ChildResourceNode(child.weight, child.name, release));
      determineChildResources(resourceHierarchy, child.name, release, flattenedRes);
    }
  }

  //! \brief ???
  //! \param[in] resourceHierarchy The map of all known resources.
  //! \param[in] request Const reference to a ResourceValue
  //!                    representing one resource requirement from a
  //!                    command.
  //! \param[in,out] resourcesNeeded The complete list of resources
  //!                                needed by the command.
  static void determineAllChildResources(ResourceHierarchyMap const &resourceHierarchy,
                                         ResourceValue const &request,
                                         ResourceSet &resourcesNeeded)
  {
    debugMsg("ResourceArbiter:determineAllChildResources", ' ' << request.name);
    std::string const &requestName = request.name;
    bool release = request.releaseAtTermination;

    // Collect the descendants of the requested resource
    std::vector<ChildResourceNode> flattenedRes;
    flattenedRes.emplace_back(ChildResourceNode(request.upperBound, requestName, release));
    determineChildResources(resourceHierarchy, requestName, release, flattenedRes);

    // loop through each hierarchy element in the flattened structure
    for (ChildResourceNode const &fres : flattenedRes) {
      if (resourcesNeeded.find(fres) == resourcesNeeded.end())
        resourcesNeeded.insert(fres);
      else if (fres.name == requestName)
        // If current is explicitly specified in 
        // the command then the value can be overwritten. The
        // child resource scales will not be overwritten however.
        // In fact this check will be valid only for the first element
        // of the flattened vector. This means that if a resource value
        // has to be overwritten it has to be explicitly scpecified in the 
        // plan
        resourcesNeeded.insert(fres);
    }
  }

  //! \brief Partition a list of commands into commands with and
  //!        without resource requirements, determine the total
  //!        requirements of each command, and sort the commands with
  //!        resource requirements by their priority.
  //! \param[in] resourceHierarchy The map of all known resources.
  //! \param[in] cmds A list of commands to be partitioned.  The list
  //!                 is consumed by this function and left empty upon
  //!                 return.
  //! \param[out] acceptCmds The list of commands which do not have resource requests.
  //! \param[out] sortedCommands Commands with resource requests, sorted by priority.
  static void partitionCommands(ResourceHierarchyMap const &resourceHierarchy,
                                LinkedQueue<CommandImpl> &cmds,
                                LinkedQueue<CommandImpl> &acceptCmds,
                                CommandPriorityList &sortedCommands)
  {
    while (!cmds.empty()) {
      CommandImpl *cmd = cmds.front();
      cmds.pop();
      const ResourceValueList &resList = cmd->getResourceValues();
      if (resList.empty()) {
        debugMsg("ResourceArbiter:partitionCommands",
                 " accepting command \"" << cmd->getName() << "\" with no resource requests");
        acceptCmds.push(cmd);
      }
      else {
        // Add the command to the list of commands in contention
        sortedCommands.emplace_back(CommandPriorityEntry(resList.front().priority, cmd));

        // Determine the total resource requirements of the command
        ResourceSet &resources = sortedCommands.back().resources; // initially empty
        for (ResourceValue const &request : resList)
          determineAllChildResources(resourceHierarchy, request, resources);
      }
    }

    // Sort the list of commands with resource requirements by priority
    if (sortedCommands.size() > 1)
      std::stable_sort(sortedCommands.begin(),
                       sortedCommands.end());
  }


  class ResourceArbiterImpl : public ResourceArbiterInterface
  {
  private:

    //! \brief The map of all known resources and their initial values.
    ResourceHierarchyMap m_resourceHierarchy;

    //! \brief The map of the resources currently allocated.
    std::map<std::string, double> m_allocated;

    //! \brief All currently executing commands with resource requirements.
    ResourceMap m_cmdResMap;
    
  public:

    //! \brief Default constructor.
    ResourceArbiterImpl() = default;

    //! \brief Virtual destructor.
    virtual ~ResourceArbiterImpl() = default;

    //! \brief Read in a resource hierarchy file.
    //! \param[in] fName The file name as a const reference to string.
    //! \return true if successful, false if not.
    virtual bool readResourceHierarchyFile(const std::string& fName)
    {
      std::ifstream myFile(fName);
      if (!myFile.is_open() || !myFile.good()) {
        debugMsg("ResourceArbiter:readResourceHierarchyFile",
                 " Unable to open file " << fName << ". No resources read.");
        return false;
      }
      bool result = readResourceHierarchy(myFile);
      myFile.close();
      condDebugMsg(result,
                   "ResourceArbiter:readResourceHierarchyFile",
                   " successfully read " << fName);
      return result;
    }

    //! \brief Read the resource hierarchy from an input stream.
    //! \param[in] s The stream.
    //! \return true if successful, false if not.
    virtual bool readResourceHierarchy(std::istream &s)
    {
      static char const *WHITESPACE = " \t\n\r\v\f";

      m_resourceHierarchy.clear();
      std::string dataStr;
      while (!s.eof()) {
        std::getline(s, dataStr); // clears dataStr
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

        // We have enough information to construct the ResourceNode
        std::pair<ResourceHierarchyMap::iterator, bool> emplaceResult =
          m_resourceHierarchy.emplace(pName, ResourceNode(maxCons));
        if (!emplaceResult.second) {
          std::cerr << "Error: resource " << pName << " defined twice" << std::endl;
          return false;
        }

        debugMsg("ResourceArbiter:readResourceHierarchy",
                 " got resource name " << pName << ", value " << maxCons);

        // Skip whitespace
        endptr += strspn(endptr, WHITESPACE);
        len -= (endptr - data);
        data = endptr;

        // Get children vector from the ResourceNode we emplaced above
        std::vector<ChildResourceNode> &children = emplaceResult.first->second.children;
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

          debugMsg("ResourceArbiter:readResourceHierarchy",
                   "  got dependent resource value " << d << ", name " << cName);
          
          children.emplace_back(ChildResourceNode(d, cName));

          len -= ws;
          data += ws;

          // Skip whitespace
          offset = strspn(data, WHITESPACE);
          len -= offset;
          data += offset;
        }
      }
      return true;
    }
    
    //! \brief Partition a list of commands into accepted and rejected
    //!        requests by resources requested and priority.
    //! \param cmds Reference to a LinkedQueue which is consumed by the function.
    //! \param acceptCmds Reference to a LinkedQueue provided by the caller to receive accepted commands.
    //! \param rejectCmds Reference to a LinkedQueue provided by the caller to receive rejected commands.
    virtual void arbitrateCommands(LinkedQueue<CommandImpl> &cmds,
                                   LinkedQueue<CommandImpl> &acceptCmds,
                                   LinkedQueue<CommandImpl> &rejectCmds)
    {
      debugMsg("ResourceArbiter:arbitrateCommands",
               " processing " << cmds.size() << " commands");

      // Do initial partitioning of commands without resource requirements,
      // and sorting of the commands with requirements by their priority
      CommandPriorityList sortedCommands;
      partitionCommands(m_resourceHierarchy, cmds, acceptCmds, sortedCommands); // consumes cmds

      debugStmt("ResourceArbiter:printSortedCommands",
                printSortedCommands(sortedCommands));

      optimalResourceArbitration(sortedCommands, acceptCmds, rejectCmds);
    
      debugStmt("ResourceArbiter:printAcceptedCommands",
                printAcceptedCommands(acceptCmds));
      // Also print all the locked resources. 
      debugStmt("ResourceArbiter:printAllocatedResources",
                printAllocatedResources());
    }

    //! \brief Release the resources reserved by the given command, if any.
    //! \param[in] cmd Pointer to the command.
    virtual void releaseResourcesForCommand(CommandImpl *cmd)
    {
      // Review all resources used by the command and remove
      // releaseable reservations from the allocated list.
      ResourceMap::iterator resListIter = m_cmdResMap.find(cmd);
      if (resListIter == m_cmdResMap.end())
        return;

      for (ChildResourceNode const &res : resListIter->second) {
        if (res.release) {
          m_allocated[res.name] -= res.weight;
          if (m_allocated[res.name] == 0.0)
            m_allocated.erase(res.name);
        }
      }
      m_cmdResMap.erase(resListIter);
    
      condDebugMsg(m_allocated.empty(),
                   "ResourceArbiter:releaseResourcesForCommand", 
                   " released command " << cmd->getName()
                   << ", no resources currently allocated");
      condDebugMsg(!m_allocated.empty(),
                   "ResourceArbiter:releaseResourcesForCommand", 
                   " released command " << cmd->getName()
                   << ", remaining resource allocations:");
      condDebugStmt(!m_allocated.empty(),
                   "ResourceArbiter:releaseResourcesForCommand", 
                    printAllocatedResources();
                    );
    }

  private:

    //! \brief Evaluates resource requests and determines which
    //!        commands may be executed based on their resource
    //!        requirements and the current resource levels.
    //!        Appends to acceptCmds and rejectCmds.
    //! \param[in] sortedCommands Commands and their resource
    //!                            requirements, sorted by priority.
    //! \param[in,out] acceptCmds List of commands which can be
    //!                           executed.  Will be appended to by
    //!                           this function.
    //! \param[out] rejectCmds List of commands which cannot be
    //!                        executed due to resource limitations.
    //!                        Will be appended to by this function.
    void optimalResourceArbitration(CommandPriorityList const &sortedCommands,
                                    LinkedQueue<CommandImpl> &acceptCmds,
                                    LinkedQueue<CommandImpl> &rejectCmds)
    {
      EstimateMap estimates;

      // Prepare estimate map and ensure entries in allocated map based on requests
      for (CommandPriorityEntry const &entry : sortedCommands) {
        ResourceSet const &requests = entry.resources;
        for (ChildResourceNode const &res : requests) {
          std::string const &resName = res.name;
          double value = 0.0;
          if (m_allocated.find(resName) == m_allocated.end())
            m_allocated[resName] = 0.0; // ensure entry in allocation map
          else
            value = m_allocated[resName];
          estimates[resName] = ResourceEstimate(value);
        }
      }

      for (CommandPriorityEntry const &entry : sortedCommands) {
        EstimateMap savedEstimates = estimates;
        CommandImpl *cmd = entry.command;
        ResourceSet const &requests = entry.resources;
        bool invalid = false;
        
        debugMsg("ResourceArbiter:optimalResourceArbitration",
                 " considering \"" << cmd->getName() << '"');

        for (ChildResourceNode const &res : requests) {
          ResourceEstimate &est = estimates[res.name];

          debugMsg("ResourceArbiter:optimalResourceArbitration",
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
            debugMsg("ResourceArbiter:optimalResourceArbitration",
                     " rejecting " << cmd->getName()
                     << " because renewable usage of " << res.name << " exceeds limits");
            break; // from inner loop
          }
          if (est.consumable < 0.0 || est.consumable > resMax) {
            invalid = true;
            debugMsg("ResourceArbiter:optimalResourceArbitration",
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
          debugMsg("ResourceArbiter:optimalResourceArbitration",
                   " accepting " << cmd->getName());

          acceptCmds.push(cmd);
          m_cmdResMap[cmd] = requests;

          // Update the allocated resource map to include the chosen command
          for (ChildResourceNode const &res : requests)
            m_allocated[res.name] += res.weight;
        }
      }
    }

    double maxConsumableResourceValue(const std::string& resName) const
    {
      if (m_resourceHierarchy.find(resName) != m_resourceHierarchy.end())
        return m_resourceHierarchy.find(resName)->second.maxConsumableValue;
      return 1.0;
    }

    void printSortedCommands(CommandPriorityList &sortedCommands) const
    {
      for (CommandPriorityEntry const &cmd : sortedCommands) {
        debugMsg("ResourceArbiter:printSortedCommands", 
                 " command \"" << cmd.command->getName()
                 << "\", priority " << cmd.priority);
      }
    }

    void printAllocatedResources() const
    {
      for (std::pair<const std::string, double> const &pr : m_allocated) {
        debugMsg("ResourceArbiter:printAllocatedResources", ' ' << pr.first << " = " << pr.second);
      }
    }

    void printAcceptedCommands(LinkedQueue<CommandImpl> const &acceptCmds)
    {
      // Print accepted commands and the resources they consume.
      CommandImpl *cmd = acceptCmds.front();
      while (cmd) {
        ResourceMap::const_iterator resMapIt = m_cmdResMap.find(cmd);
        if (resMapIt != m_cmdResMap.end()) {
          debugMsg("ResourceArbiter:printAcceptedCommands",
                   " Accepted command \"" << cmd->getName()
                   << "\" uses resources:");
          for (ChildResourceNode const &res : resMapIt->second) {
            debugMsg("ResourceArbiter:printAcceptedCommands", "  " << res.name);
          }
        }
        else {
          debugMsg("ResourceArbiter:printAcceptedCommands",
                   " Accepted command \"" << cmd->getName() << '"');
        }
        cmd = cmd->next();
      }
    }

  };

  ResourceArbiterInterface *makeResourceArbiter()
  {
    return new ResourceArbiterImpl();
  }

}
