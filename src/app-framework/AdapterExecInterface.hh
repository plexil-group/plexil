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

#ifndef _H_AdapterExecInterface
#define _H_AdapterExecInterface

#include "ExecDefs.hh"
#include "LabelStr.hh"
#include "ParserException.hh"
#include "InterfaceManagerBase.hh"

// forward references w/o namespace
class TiXmlElement;

namespace PLEXIL
{
  // forward references
  class AdapterExecInterface;
  typedef Id<AdapterExecInterface> AdapterExecInterfaceId;

  class InterfaceAdapter;
  typedef Id<InterfaceAdapter> InterfaceAdapterId;

  class ResourceArbiterInterface;
  typedef Id<ResourceArbiterInterface> ResourceArbiterInterfaceId;

  class PlexilNode;
  typedef Id<PlexilNode> PlexilNodeId;

  /**
   * @brief An abstract base class representing the InterfaceManager API
   *        from the interface adapter's point of view.
   * @note This class exists so that InterfaceAdapter and its derived classes
   *       need not be aware of the implementation details of InterfaceManager.
   */

  class AdapterExecInterface : public InterfaceManagerBase
  {
  public:

    //
    // API to interface adapters
    //

    /**
     * @brief Register the given interface adapter based on its configuration XML.  
     * @param adapter The interface adapter.
     */

    virtual void defaultRegisterAdapter(InterfaceAdapterId adapter) = 0;

    /**
     * @brief Register the given interface adapter for this command.  
              Returns true if successful.  Fails and returns false 
              iff the command name already has an adapter registered.
     * @param commandName The command to map to this adapter.
     * @param intf The interface adapter to handle this command.
     * @return True if successful, false if there is already an adapter registered for this command.
     */
    virtual bool registerCommandInterface(const LabelStr & commandName,
					  InterfaceAdapterId intf) = 0;

    /**
     * @brief Register the given interface adapter for lookups to this state.
              Returns true if successful.  Fails and returns false 
              iff the state name already has an adapter registered.
     * @param stateName The name of the state to map to this adapter.
     * @param intf The interface adapter to handle this lookup.
     * @return True if successful, false if there is already an adapter registered for this state name.
     */
    virtual bool registerLookupInterface(const LabelStr & stateName,
        const InterfaceAdapterId& intf) = 0;

    /**
     * @brief Register the given interface adapter for planner updates.
              Returns true if successful.  Fails and returns false 
              iff an adapter is already registered.
     * @param intf The interface adapter to handle planner updates.
     * @return True if successful, false if there is already an adapter registered for planner updates.
     */
    virtual bool registerPlannerUpdateInterface(InterfaceAdapterId intf) = 0;

    /**
     * @brief Register the given interface adapter as the default.
              This interface will be used for all lookups, commands, and updates
              which do not have a specific adapter.  
              Returns true if successful.
	      Fails and returns false if there is already a default adapter registered.
     * @param intf The interface adapter to use as the default.
     * @return True if successful, false if there is already a default adapter registered.
     */
    virtual bool setDefaultInterface(InterfaceAdapterId intf) = 0;

    /**
     * @brief Register the given interface adapter as the default for commands.
              This interface will be used for all commands which do not have
	      a specific adapter.  
              Returns true if successful.
	      Fails and returns false if there is already a default command adapter registered.
     * @param intf The interface adapter to use as the default.
     * @return True if successful, false if there is already a default adapter registered.
     */
    virtual bool setDefaultCommandInterface(InterfaceAdapterId intf) = 0;

    /**
     * @brief Register the given interface adapter as the default for lookups.
              This interface will be used for all lookups which do not have
	      a specific adapter.  
              Returns true if successful.
	      Fails and returns false if there is already a default lookup adapter registered.
     * @param intf The interface adapter to use as the default.
     * @return True if successful, false if there is already a default adapter registered.
     */
    virtual bool setDefaultLookupInterface(InterfaceAdapterId intf) = 0;

    /**
     * @brief Retract registration of the previous interface adapter for this command.  
     * @param commandName The command.
     */
    virtual void unregisterCommandInterface(const LabelStr & commandName) = 0;

    /**
     * @brief Retract registration of the previous interface adapter for this state.
     * @param stateName The state name.
     */
    virtual void unregisterLookupInterface(const LabelStr & stateName) = 0;

    /**
     * @brief Retract registration of the previous interface adapter for planner updates.
     */
    virtual void unregisterPlannerUpdateInterface() = 0;

    /**
     * @brief Retract registration of the previous default interface adapter.
     */
    virtual void unsetDefaultInterface() = 0;

    /**
     * @brief Retract registration of the previous default interface adapter for commands.
     */
    virtual void unsetDefaultCommandInterface() = 0;

    /**
     * @brief Retract registration of the previous default interface adapter for lookups.
     */
    virtual void unsetDefaultLookupInterface() = 0;

    /**
     * @brief Return the interface adapter in effect for this command, whether 
     specifically registered or default. May return NoId().
     * @param commandName The command.
     */
    virtual InterfaceAdapterId getCommandInterface(const LabelStr & commandName) = 0;

    /**
     * @brief Return the current default interface adapter for commands.
              May return NoId().
     */
    virtual InterfaceAdapterId getDefaultCommandInterface() = 0;

    /**
     * @brief Return the interface adapter in effect for lookups with this state name,
     whether specifically registered or default. May return NoId().
     * @param stateName The state.
     */
    virtual InterfaceAdapterId getLookupInterface(const LabelStr & stateName) = 0;

    /**
     * @brief Return the current default interface adapter. May return NoId().
     */
    virtual InterfaceAdapterId getDefaultInterface() = 0;

    /**
     * @brief Return the current default interface adapter for lookups.
              May return NoId().
     */
    virtual InterfaceAdapterId getDefaultLookupInterface() = 0;

    /**
     * @brief Register the given resource arbiter interface for all commands
     Returns true if successful.
     Fails and returns false if there is already an interface registered.
     * @param raIntf The resource arbiter interface to use.
     */
    virtual bool setResourceArbiterInterface(ResourceArbiterInterfaceId raIntf) = 0;

    /**
     * @brief Retract registration of the previous resource arbiter interface.
     */
    virtual void unsetResourceArbiterInterface() = 0;

    /**
     * @brief Return the current resource arbiter interface. May return NoId().
     */
    virtual ResourceArbiterInterfaceId getResourceArbiterInterface() const = 0;

    /**
     * @brief Notify of the availability of a new value for a lookup.
     * @param state The state for the new value.
     * @param value The new value.
     */
    virtual void handleValueChange(const State& state, double value) = 0;

    /**
     * @brief Notify of the availability of (e.g.) a command return or acknowledgement.
     * @param exp The expression whose value is being returned.
     * @param value The new value of the expression.
     */
    virtual void handleValueChange(const ExpressionId & exp,
				   double value) = 0;

    /**
     * @brief Tells the external interface to expect a return value from this command.
     Use handleValueChange() to actually return the value.
     * @param dest The expression whose value will be returned.
     * @param commandName The command whose value will be returned.
     * @param params The parameters associated with this command.
     */
    virtual void registerCommandReturnValue(ExpressionId dest,
					    const LabelStr & commandName,
					    const std::list<double> & params) = 0;

    /**
     * @brief Notify the external interface that this previously registered expression
     should not wait for a return value.
     * @param dest The expression whose value was to be returned.
     */
    virtual void unregisterCommandReturnValue(ExpressionId dest) = 0;

    /**
     * @brief Notify the executive of a new plan.
     * @param planXml The TinyXML representation of the new plan.
     * @param parent The node which is the parent of the new node.
     * @return False if the plan references unloaded libraries, true otherwise.
     * @note This is deprecated, use the PlexilNodeId variant instead.
     */
    virtual bool handleAddPlan(TiXmlElement * planXml,
			       const LabelStr& parent)
      throw(ParserException)
      = 0;

    /**
     * @brief Notify the executive of a new plan.
     * @param planStruct The PlexilNode representation of the new plan.
     * @param parent The node which is the parent of the new node.
     * @return False if the plan references unloaded libraries, true otherwise.
     */
    virtual bool handleAddPlan(PlexilNodeId planStruct,
			       const LabelStr& parent) = 0;

	/**
	 * @brief Get the search path for library nodes.
	 * @return A reference to the library search path.
	 */
	virtual const std::vector<std::string>& getLibraryPath() const = 0;

	/**
	 * @brief Get the search path for plans.
	 * @return A reference to the plan search path.
	 */
	virtual const std::vector<std::string>& getPlanPath() const = 0;

	/**
	 * @brief Add the specified directory name to the end of the library node loading path.
	 * @param libdir The directory name.
	 */
	virtual void addLibraryPath(const std::string& libdir) = 0;

	/**
	 * @brief Add the specified directory names to the end of the library node loading path.
	 * @param libdirs The vector of directory names.
	 */
	virtual void addLibraryPath(const std::vector<std::string>& libdirs) = 0;

    /**
     * @brief Determine whether the named library is loaded.
     * @return True if loaded, false otherwise.
     */
    virtual bool isLibraryLoaded(const std::string& libName) const = 0;

	/**
	 * @brief Add the specified directory name to the end of the plan loading path.
	 * @param libdir The directory name.
	 */
	virtual void addPlanPath(const std::string& libdir) = 0;

	/**
	 * @brief Add the specified directory names to the end of the plan loading path.
	 * @param libdirs The vector of directory names.
	 */
	virtual void addPlanPath(const std::vector<std::string>& libdirs) = 0;

    /**
     * @brief Notify the executive of a new library node.
     * @param planStruct The PlexilNode representation of the new library node.
     */
    virtual void handleAddLibrary(PlexilNodeId planStruct) = 0;

    /**
     * @brief Notify the executive that it should run one cycle.  This should be sent after
     each batch of lookup and command return data.
    */
    virtual void notifyOfExternalEvent() = 0;

    /**
     * @brief Run the exec and wait until all events in the queue have been processed.
	 */
	virtual void notifyAndWaitForCompletion() = 0;

	/**
	 * @brief Get the Exec's idea of the current time.
	 * @return Seconds since the epoch as a double float.
	 */
    virtual double currentTime() = 0;

    /**
     * @brief Get the state cache for this instance of the interface.
	 */
    virtual StateCacheId getStateCache() const = 0;

    //
    // Static utility functions
    //
    
    static std::string getText(const State& c);

  protected:

    /**
     * @brief Default constructor method.
     */
    AdapterExecInterface();

    /**
     * @brief Destructor method.
     */
    virtual ~AdapterExecInterface();

  private:

    // Deliberately unimplemented
    AdapterExecInterface(const AdapterExecInterface&);
    AdapterExecInterface& operator=(const AdapterExecInterface&);

  };

}

#endif // _H_AdapterExecInterface
