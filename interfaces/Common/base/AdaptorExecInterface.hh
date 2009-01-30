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

#ifndef _H_AdaptorExecInterface
#define _H_AdaptorExecInterface

#include "ExecDefs.hh"
#include "LabelStr.hh"
#include "ParserException.hh"

// forward references w/o namespace
class TiXmlElement;

namespace PLEXIL
{
  // forward references
  class AdaptorExecInterface;
  typedef Id<AdaptorExecInterface> AdaptorExecInterfaceId;

  class InterfaceAdaptor;
  typedef Id<InterfaceAdaptor> InterfaceAdaptorId;

  class ResourceArbiterInterface;
  typedef Id<ResourceArbiterInterface> ResourceArbiterInterfaceId;

  class PlexilNode;
  typedef Id<PlexilNode> PlexilNodeId;

  /**
   * @brief An abstract base class representing the executive interface from the interface adaptor's point of view.
            Implements the Singleton design pattern.
   */

  class AdaptorExecInterface
  {
  public:

    // *** DEPRECATED ***
    static AdaptorExecInterfaceId instance();

    //
    // API to interface adaptors
    //

    /**
     * @brief Register the given interface adaptor for this command.  
              Returns true if successful.  Fails and returns false 
              iff the command name already has an adaptor registered.
     * @param commandName The command to map to this adaptor.
     * @param intf The interface adaptor to handle this command.
     * @return True if successful, false if there is already an adaptor registered for this command.
     */
    virtual bool registerCommandInterface(const LabelStr & commandName,
					  InterfaceAdaptorId intf) = 0;

    /**
     * @brief Register the given interface adaptor for this function.  
              Returns true if successful.  Fails and returns false 
              iff the function name already has an adaptor registered.
     * @param functionName The function to map to this adaptor.
     * @param intf The interface adaptor to handle this function.
     * @return True if successful, false if there is already an adaptor registered for this function.
     */
    virtual bool registerFunctionInterface(const LabelStr & functionName,
					   InterfaceAdaptorId intf) = 0;

    /**
     * @brief Register the given interface adaptor for lookups to this state.
              Returns true if successful.  Fails and returns false 
              iff the state name already has an adaptor registered.
     * @param stateName The name of the state to map to this adaptor.
     * @param intf The interface adaptor to handle this lookup.
     * @return True if successful, false if there is already an adaptor registered for this state name.
     */
    virtual bool registerLookupInterface(const LabelStr & stateName,
					 InterfaceAdaptorId intf) = 0;

    /**
     * @brief Register the given interface adaptor for planner updates.
              Returns true if successful.  Fails and returns false 
              iff an adaptor is already registered.
     * @param intf The interface adaptor to handle planner updates.
     * @return True if successful, false if there is already an adaptor registered for planner updates.
     */
    virtual bool registerPlannerUpdateInterface(InterfaceAdaptorId intf) = 0;

    /**
     * @brief Register the given interface adaptor as the default.
              This interface will be used for all lookups, commands, 
              and functions which do not have a specific adaptor.  
              Returns true if successful.
	      Fails and returns false if there is already a default adaptor registered.
     * @param intf The interface adaptor to use as the default.
     * @return True if successful, false if there is already a default adaptor registered.
     */
    virtual bool setDefaultInterface(InterfaceAdaptorId intf) = 0;

    /**
     * @brief Retract registration of the previous interface adaptor for this command.  
     * @param commandName The command.
     */
    virtual void unregisterCommandInterface(const LabelStr & commandName) = 0;

    /**
     * @brief Retract registration of the previous interface adaptor for this function.  
     * @param functionName The function.
     */
    virtual void unregisterFunctionInterface(const LabelStr & functionName) = 0;

    /**
     * @brief Retract registration of the previous interface adaptor for this state.
     * @param stateName The state name.
     */
    virtual void unregisterLookupInterface(const LabelStr & stateName) = 0;

    /**
     * @brief Retract registration of the previous interface adaptor for planner updates.
     */
    virtual void unregisterPlannerUpdateInterface() = 0;

    /**
     * @brief Retract registration of the previous default interface adaptor.
     */
    virtual void unsetDefaultInterface() = 0;

    /**
     * @brief Return the interface adaptor in effect for this command, whether 
     specifically registered or default. May return NoId().
     * @param commandName The command.
     */
    virtual InterfaceAdaptorId getCommandInterface(const LabelStr & commandName) = 0;

    /**
     * @brief Return the interface adaptor in effect for this function, whether 
     specifically registered or default. May return NoId().
     * @param functionName The function.
     */
    virtual InterfaceAdaptorId getFunctionInterface(const LabelStr & functionName) = 0;

    /**
     * @brief Return the interface adaptor in effect for lookups with this state name,
     whether specifically registered or default. May return NoId().
     * @param stateName The state.
     */
    virtual InterfaceAdaptorId getLookupInterface(const LabelStr & stateName) = 0;

    /**
     * @brief Return the current default interface adaptor. May return NoId().
     */
    virtual InterfaceAdaptorId getDefaultInterface() = 0;

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
     * @brief Notify of the availability of new values for a lookup.
     * @param key The state key for the new values.
     * @param values The new values.
     */
    virtual void handleValueChange(const StateKey& key,
				   const std::vector<double>& values) = 0;

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
     * @brief Tells the external interface to expect a return value from this function.
     Use handleValueChange() to actually return the value.
     * @param dest The expression whose value will be returned.
     * @param functionName The function whose value will be returned.
     * @param params The parameters associated with this function.
     */
    virtual void registerFunctionReturnValue(ExpressionId dest,
					     const LabelStr & functionName,
					     const std::list<double> & params) = 0;

    /**
     * @brief Notify the external interface that this previously registered expression
     should not wait for a return value.
     * @param dest The expression whose value was to be returned.
     */
    virtual void unregisterCommandReturnValue(ExpressionId dest) = 0;

    /**
     * @brief Notify the external interface that this previously registered expression
     should not wait for a return value.
     * @param dest The expression whose value was to be returned.
     */
    virtual void unregisterFunctionReturnValue(ExpressionId dest) = 0;

    /**
     * @brief Notify the executive of a new plan.
     * @param planXml The TinyXML representation of the new plan.
     * @param parent The node which is the parent of the new node.
     * @note This is deprecated, use the PlexilNodeId variant instead.
     */
    virtual void handleAddPlan(TiXmlElement * planXml,
			       const LabelStr& parent)
      throw(ParserException)
      = 0;

    /**
     * @brief Notify the executive of a new plan.
     * @param planStruct The PlexilNode representation of the new plan.
     * @param parent The node which is the parent of the new node.
     */
    virtual void handleAddPlan(PlexilNodeId planStruct,
			       const LabelStr& parent) = 0;

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
     * @brief Get the state cache for this instance of the interface.
     * @note This function is deprecated.  Use 
    */
    virtual StateCacheId getStateCache() const = 0;

    /**
     * @brief Get a unique key for a state, creating a new key for a new state.
     * @param state The state.
     * @param key The key.
     * @return True if a new key had to be generated.
     */
    virtual bool keyForState(const State& state, StateKey& key) = 0;

    /**
     * @brief Get (a copy of) the State for this StateKey.
     * @param key The key to look up.
     * @param state The state associated with the key.
     * @return True if the key is found, false otherwise.
     */
    virtual bool stateForKey(const StateKey& key, State& state) const = 0;

    //
    // Static utility functions
    //
    
    static std::string valueToString(double val);
    static double stringToValue(const char * rawValue);
    static std::string getText(const State& c);

  protected:

    /**
     * @brief Default constructor method.
     */
    AdaptorExecInterface();

    /**
     * @brief Destructor method.
     */
    virtual ~AdaptorExecInterface();

    AdaptorExecInterfaceId m_adaptorInterfaceId;

  private:

    static AdaptorExecInterfaceId s_instance;

  };

}

#endif // _H_AdaptorExecInterface
