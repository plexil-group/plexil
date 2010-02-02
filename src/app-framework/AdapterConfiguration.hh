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
/*
 * AdapterConfiguration.hh
 *
 *  Created on: Jan 28, 2010
 *      Author: jhogins
 */

#ifndef ADAPTERCONFIGURATION_HH_
#define ADAPTERCONFIGURATION_HH_

#include "LabelStr.hh"
#include "InterfaceAdapter.hh"
#include "Id.hh"

#include <set>

namespace PLEXIL {

  // forward references
  class InterfaceManager;

  class InterfaceAdapter;
  typedef Id<InterfaceAdapter> InterfaceAdapterId;

  class AdapterConfiguration;
  typedef Id<AdapterConfiguration> AdapterConfigurationId;

  class AdapterConfiguration {
  public:

    /**
     * @brief Default constructor.
     */
    AdapterConfiguration(InterfaceManager* manager);

    /**
     * @brief Virtual destructor.
     */
    virtual ~AdapterConfiguration();

    /**
     * @brief Returns the InterfaceManager set in initilization
     * @return The InterfaceManager set in initilization.
     */
    InterfaceManager* getInterfaceManager();

    /**
     * @brief Get the ID of this instance.
     */
    AdapterConfigurationId getId() {
      return m_id;
    }

    //
    // API to interface adapters
    //

    /**
     * @brief Register the given interface adapter.
     * @param adapter The interface adapter to be registered.
     */

    virtual void defaultRegisterAdapter(InterfaceAdapterId adapter);

    /**
     * @brief Register the given interface adapter for this command.
     Returns true if successful.  Fails and returns false
     iff the command name already has an adapter registered
              or setting a command interface is not implemented.
     * @param commandName The command to map to this adapter.
     * @param intf The interface adapter to handle this command.
     */
    virtual bool registerCommandInterface(const LabelStr & commandName,
                  InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter for this function.
              Returns true if successful.  Fails and returns false
              iff the function name already has an adapter registered
              or setting a function interface is not implemented.
     * @param functionName The function to map to this adapter.
     * @param intf The interface adapter to handle this function.
     */
    virtual bool registerFunctionInterface(const LabelStr & functionName,
                   InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter for lookups to this state.
     Returns true if successful.  Fails and returns false
     if the state name already has an adapter registered
              or registering a lookup interface is not implemented.
     * @param stateName The name of the state to map to this adapter.
     * @param intf The interface adapter to handle this lookup.
     */
    virtual bool registerLookupInterface(const LabelStr & stateName,
                 InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter for planner updates.
              Returns true if successful.  Fails and returns false
              iff an adapter is already registered
              or setting the default planner update interface is not implemented.
     * @param intf The interface adapter to handle planner updates.
     */
    virtual bool registerPlannerUpdateInterface(InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter as the default for all lookups and commands
     which do not have a specific adapter.  Returns true if successful.
     Fails and returns false if there is already a default adapter registered
              or setting the default interface is not implemented.
     * @param intf The interface adapter to use as the default.
     */
    virtual bool setDefaultInterface(InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter as the default for lookups.
              This interface will be used for all lookups which do not have
          a specific adapter.
              Returns true if successful.
          Fails and returns false if there is already a default lookup adapter registered
              or setting the default lookup interface is not implemented.
     * @param intf The interface adapter to use as the default.
     * @return True if successful, false if there is already a default adapter registered.
     */
    virtual bool setDefaultLookupInterface(InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter as the default for commands.
              This interface will be used for all commands which do not have
          a specific adapter.
              Returns true if successful.
          Fails and returns false if there is already a default command adapter registered.
     * @param intf The interface adapter to use as the default.
     * @return True if successful, false if there is already a default adapter registered.
     */
    virtual bool setDefaultCommandInterface(InterfaceAdapterId intf);

    /**
     * @brief Retract registration of the previous interface adapter for this command.
     * @param commandName The command.
     */
    virtual void unregisterCommandInterface(const LabelStr & commandName);

    /**
     * @brief Retract registration of the previous interface adapter for this function.
     * @param functionName The function.
     */
    virtual void unregisterFunctionInterface(const LabelStr & functionName);

    /**
     * @brief Retract registration of the previous interface adapter for this state.
     * @param stateName The state name.
     */
    virtual void unregisterLookupInterface(const LabelStr & stateName);

    /**
     * @brief Retract registration of the previous interface adapter for planner updates.
     */
    virtual void unregisterPlannerUpdateInterface();

    /**
     * @brief Retract registration of the previous default interface adapter.
     */
    virtual void unsetDefaultInterface();

    /**
     * @brief Retract registration of the previous default interface adapter for commands.
     */
    virtual void unsetDefaultCommandInterface();

    /**
     * @brief Retract registration of the previous default interface adapter for lookups.
     */
    virtual void unsetDefaultLookupInterface();

    /**
     * @brief Return the interface adapter in effect for this command, whether
     specifically registered or default. May return NoId().
     * @param commandName The command.
     */
    virtual InterfaceAdapterId getCommandInterface(const LabelStr & commandName);

    /**
     * @brief Return the current default interface adapter for commands.
              May return NoId().
     */
    virtual InterfaceAdapterId getDefaultCommandInterface();

    /**
     * @brief Return the interface adapter in effect for this function, whether
     specifically registered or default. May return NoId().
     * @param functionName The function.
     */
    virtual InterfaceAdapterId getFunctionInterface(const LabelStr & functionName);

    /**
     * @brief Return the interface adapter in effect for lookups with this state name,
     whether specifically registered or default. May return NoId().
     * @param stateName The state.
     */
    virtual InterfaceAdapterId getLookupInterface(const LabelStr & stateName);

    /**
     * @brief Return the current default interface adapter for lookups.
              May return NoId().
     */
    virtual InterfaceAdapterId getDefaultLookupInterface();

    /**
     * @brief Return the interface adapter in effect for planner updates,
              whether specifically registered or default. May return NoId().
     */
    virtual InterfaceAdapterId getPlannerUpdateInterface();

    /**
     * @brief Return the current default interface adapter. May return NoId().
     */
    virtual InterfaceAdapterId getDefaultInterface();

    /**
     * @brief Returns true if the given adapter is a known interface in the system. False otherwise
     */
    virtual bool isKnown(InterfaceAdapterId intf);

    /**
     * @brief Clears the interface adapter registry.
     */
    virtual void clearAdapterRegistry();
  protected:
    /**
     * For implementors, returns the vector of interface adapters from the manager
     */
    std::set<InterfaceAdapterId>& getAdaptersFromManager();
    /**
     * @brief Deletes the given adapter from the interface manager
     * @return true if the given adapter existed and was deleted. False if not found
     */
    bool deleteAdapter(InterfaceAdapterId intf);
  private:
    InterfaceManager* m_manager;
    AdapterConfigurationId m_id;
  };
}

#endif /* ADAPTERCONFIGURATION_HH_ */
