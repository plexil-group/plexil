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

#ifndef _H_InterfaceManager
#define _H_InterfaceManager

#include "ExecDefs.hh"
#include "Expression.hh"
#include "ExternalInterface.hh"
#include "AdapterExecInterface.hh"
#include "PlexilPlan.hh"

// STL
#include <queue>
#include <set>
#include <vector>

// Forward reference in global namespace
class TiXmlElement;

namespace PLEXIL 
{
  class AdapterConfiguration;

  // Forward references
  class ExecApplication;

  class InterfaceAdapter;
  typedef Id<InterfaceAdapter> InterfaceAdapterId;

  class AdapterConfiguration;
  typedef Id<AdapterConfiguration> AdapterConfigurationId;

  class InterfaceManager;
  typedef Id<InterfaceManager> InterfaceManagerId;

  class ResourceArbiterInterface;
  typedef Id<ResourceArbiterInterface> ResourceArbiterInterfaceId;

  /**
   * @brief A concrete derived class implementing the APIs of the
   *        ExternalInterface and AdapterExecInterface classes.
   * @details The InterfaceManager class is responsible for keeping track
   *          of all the external interfaces used by the PlexilExec.  It
   *          maintains a queue of messages for the Exec to process.  Interface
   *          instantiation, initialization, startup, stopping, shutdown, and
   *          deallocation are all handled by the InterfaceManager instance.
   * @note Supersedes the old ThreadedExternalInterface class.
   */
  class InterfaceManager :
    public ExternalInterface,
    public AdapterExecInterface
  {
  public:

    /**
     * @brief Constructor. A new DefaultAdapterConfiguration is used as the AdapterConfiguration.
     * @param app The ExecApplication instance to which this object belongs.
     */
    InterfaceManager(ExecApplication & app);

    /**
     * @brief Destructor.
     */
    virtual ~InterfaceManager();

    /**
     * @brief Get internal ID pointer.
     */
    inline const InterfaceManagerId& getInterfaceManagerId() const
    {
      return m_interfaceManagerId;
    }

    /**
     * @brief Get parent application.
     */
    inline ExecApplication& getApplication() const
    {
      return m_application;
    }

    /**
     * @brief Get adapterConfiguration.
     */
    inline AdapterConfigurationId getAdapterConfig() const
    {
      return m_adapterConfig;
    }

    //
    // API for all related objects
    //

    /**
     * @brief Associate an arbitrary object with a string.
     * @param name The string naming the property.
     * @param thing The property value as an untyped pointer.
     */
    virtual void setProperty(const std::string& name, void * thing);

    /**
     * @brief Fetch the named property.
     * @param name The string naming the property.
     * @return The property value as an untyped pointer.
     */
    virtual void* getProperty(const std::string& name);


    //
    // API for ExecApplication
    //

    /**
     * @brief Constructs interface adapters from the provided XML.
     * @param configXml The XML element used for interface configuration.
     */
    void constructInterfaces(const TiXmlElement * configXml);

    /**
     * @brief Add an externally constructed interface adapter.
     * @param adapter The adapter ID.
     */
    void addInterfaceAdapter(const InterfaceAdapterId& adapter);

    /**
     * @brief Add an externally constructed ExecListener.
     * @param listener The ExecListener ID.
     */
    void addExecListener(const ExecListenerId& listener);

    /**
     * @brief Performs basic initialization of the interface and all adapters.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize();

    /**
     * @brief Prepares the interface and adapters for execution.
     * @return true if successful, false otherwise.
     */
    virtual bool start();

    /**
     * @brief Halts all interfaces.
     * @return true if successful, false otherwise.
     */
    virtual bool stop();

    /**
     * @brief Resets the interface prior to restarting.
     * @return true if successful, false otherwise.
     */
    virtual bool reset();

    /**
     * @brief Shuts down the interface.
     * @return true if successful, false otherwise.
     */
    virtual bool shutdown();
    
    /**
     * @brief Updates the state cache from the items in the queue.
     * @return True if the exec needs to be stepped, false otherwise.
     * @note Should only be called with exec locked by the current thread.
     */
    bool processQueue();

    //
    // API for exec
    //
    
    /**
     * @brief Delete any entries in the queue.
     */
    void resetQueue();

    /**
     * @brief Register a change lookup on a new state, expecting values back.
     * @param source The unique key for this lookup.
     * @param state The state
     * @param key The key for the state to be used in future communications about the state.
     * @param tolerances The tolerances for the lookup.  May be used by the FL to reduce the number of updates sent to the exec.
     * @param dest The destination for the current values for the state.
     * @note dest is stack allocated, therefore pointers to it should not be stored!
     */
    void registerChangeLookup(const LookupKey& source,
			      const State& state,
			      const StateKey& key,
			      const std::vector<double>& tolerances, 
			      std::vector<double>& dest);

    /**
     * @brief Register a change lookup on an existing state.
     * @param source The unique key for this lookup.
     * @param key The key for the state.
     * @param tolerances The tolerances for the lookup.  May be used by the FL to reduce the number of updates sent to the exec.
     */
    void registerChangeLookup(const LookupKey& source,
			      const StateKey& key, 
			      const std::vector<double>& tolerances);

    /**
     * @brief Perform an immediate lookup on a new state.
     * @param state The state
     * @param key The key for the state to be used in future communications about the state.
     * @param dest The destination for the current values for the state.
     * @note dest is stack allocated, therefore pointers to it should not be stored!
     */
    void lookupNow(const State& state,
		   const StateKey& key,
		   std::vector<double>& dest);

    /**
     * @brief Perform an immediate lookup on an existing state.
     * @param key The key for the state.
     * @param dest The destination for the current values for the state.
     * @note dest is stack allocated, therefore pointers to it should not be stored!
     */
    void lookupNow(const StateKey& key, 
		   std::vector<double>& dest);

    /**
     * @brief Inform the FL that a lookup should no longer receive updates.
     */
    void unregisterChangeLookup(const LookupKey& dest);

    //this batches the set of actions from quiescence completion.  calls PlexilExecutive::step() at the end
    //assignments must be performed first.
    //though it only takes a list of commands as an argument at the moment, it will eventually take function calls and the like
    //so the name remains "batchActions"
    void batchActions(std::list<CommandId>& commands);
    void batchActions(std::list<FunctionCallId>& functionCalls);
    void updatePlanner(std::list<UpdateId>& updates);

    /**
     * @brief Abort the pending command with the supplied name and arguments.
     * @param cmdName The LabelString representing the command name.
     * @param cmdArgs The command arguments expressed as doubles.
     * @param cmdAck The acknowledgment of the pending command
     * @param abrtAck The expression in which to store an acknowledgment of command abort.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    void invokeAbort(const LabelStr& cmdName, const std::list<double>& cmdArgs, ExpressionId abrtAck, ExpressionId cmdAck);

    double currentTime();


    //
    // API to interface adapters
    //

    /**
     * @brief Register the given interface adapter based on its configuration XML.  
     * @param adapter The interface adapter to handle this command.
     */

    virtual void defaultRegisterAdapter(InterfaceAdapterId adapter);

    /**
     * @brief Register the given interface adapter for this command.  
     Returns true if successful.  Fails and returns false 
     iff the command name already has an adapter registered.
     * @param commandName The command to map to this adapter.
     * @param intf The interface adapter to handle this command.
     */
    bool registerCommandInterface(const LabelStr & commandName,
				  InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter for this function.  
              Returns true if successful.  Fails and returns false 
              iff the function name already has an adapter registered.
     * @param functionName The function to map to this adapter.
     * @param intf The interface adapter to handle this function.
     */
    bool registerFunctionInterface(const LabelStr & functionName,
				   InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter for lookups to this state.
     Returns true if successful.  Fails and returns false 
     if the state name already has an adapter registered.
     * @param stateName The name of the state to map to this adapter.
     * @param intf The interface adapter to handle this lookup.
     */
    bool registerLookupInterface(const LabelStr & stateName,
        const InterfaceAdapterId& intf);

    /**
     * @brief Register the given interface adapter for planner updates.
              Returns true if successful.  Fails and returns false 
              iff an adapter is already registered.
     * @param intf The interface adapter to handle planner updates.
     */
    bool registerPlannerUpdateInterface(InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter as the default for all lookups and commands
     which do not have a specific adapter.  Returns true if successful.
     Fails and returns false if there is already a default adapter registered.
     * @param intf The interface adapter to use as the default.
     */
    bool setDefaultInterface(InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter as the default for lookups.
              This interface will be used for all lookups which do not have
	      a specific adapter.  
              Returns true if successful.
	      Fails and returns false if there is already a default lookup adapter registered.
     * @param intf The interface adapter to use as the default.
     * @return True if successful, false if there is already a default adapter registered.
     */
    bool setDefaultLookupInterface(InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter as the default for commands.
              This interface will be used for all commands which do not have
	      a specific adapter.  
              Returns true if successful.
	      Fails and returns false if there is already a default command adapter registered.
     * @param intf The interface adapter to use as the default.
     * @return True if successful, false if there is already a default adapter registered.
     */
    bool setDefaultCommandInterface(InterfaceAdapterId intf);

    /**
     * @brief Retract registration of the previous interface adapter for this command.  
     * @param commandName The command.
     */
    void unregisterCommandInterface(const LabelStr & commandName);

    /**
     * @brief Retract registration of the previous interface adapter for this function.  
     * @param functionName The function.
     */
    void unregisterFunctionInterface(const LabelStr & functionName);

    /**
     * @brief Retract registration of the previous interface adapter for this state.
     * @param stateName The state name.
     */
    void unregisterLookupInterface(const LabelStr & stateName);

    /**
     * @brief Retract registration of the previous interface adapter for planner updates.
     */
    void unregisterPlannerUpdateInterface();

    /**
     * @brief Retract registration of the previous default interface adapter.
     */
    void unsetDefaultInterface();

    /**
     * @brief Retract registration of the previous default interface adapter for commands.
     */
    void unsetDefaultCommandInterface();

    /**
     * @brief Retract registration of the previous default interface adapter for lookups.
     */
    void unsetDefaultLookupInterface();

    /**
     * @brief Return the interface adapter in effect for this command, whether 
     specifically registered or default. May return NoId().
     * @param commandName The command.
     */
    InterfaceAdapterId getCommandInterface(const LabelStr & commandName);

    /**
     * @brief Return the current default interface adapter for commands.
              May return NoId().
     */
    InterfaceAdapterId getDefaultCommandInterface();

    /**
     * @brief Return the interface adapter in effect for this function, whether 
     specifically registered or default. May return NoId().
     * @param functionName The function.
     */
    InterfaceAdapterId getFunctionInterface(const LabelStr & functionName);

    /**
     * @brief Return the interface adapter in effect for lookups with this state name,
     whether specifically registered or default. May return NoId().
     * @param stateName The state.
     */
    InterfaceAdapterId getLookupInterface(const LabelStr & stateName);

    /**
     * @brief Return the current default interface adapter for lookups.
              May return NoId().
     */
    InterfaceAdapterId getDefaultLookupInterface();

    /**
     * @brief Return the interface adapter in effect for planner updates,
              whether specifically registered or default. May return NoId().
     */
    InterfaceAdapterId getPlannerUpdateInterface();

    /**
     * @brief Return the current default interface adapter. May return NoId().
     */
    InterfaceAdapterId getDefaultInterface();

    /**
     * @brief Register the given resource arbiter interface for all commands
     Returns true if successful.
     Fails and returns false if there is already an interface registered.
     * @param raIntf The resource arbiter interface to use.
     */
    bool setResourceArbiterInterface(ResourceArbiterInterfaceId raIntf);

    /**
     * @brief Retract registration of the previous resource arbiter interface.
     */
    void unsetResourceArbiterInterface();

    /**
     * @brief Return the current resource arbiter interface. May return NoId().
     */
    ResourceArbiterInterfaceId getResourceArbiterInterface() const {return m_raInterface;}

    /**
     * @brief Notify of the availability of new values for a lookup.
     * @param key The state key for the new values.
     * @param values The new values.
     */
    void handleValueChange(const StateKey& key, 
			   const std::vector<double>& values);

    /**
     * @brief Notify of the availability of (e.g.) a command return or acknowledgement.
     * @param exp The expression whose value is being returned.
     * @param value The new value of the expression.
     */
    void handleValueChange(const ExpressionId & exp,
			   double value);

    /**
     * @brief Tells the external interface to expect a return value from this command.
     Use handleValueChange() to actually return the value.
     * @param dest The expression whose value will be returned.
     * @param name The command whose value will be returned.
     * @param params The parameters associated with this command.
     */
    void registerCommandReturnValue(ExpressionId dest,
				    const LabelStr & name,
				    const std::list<double> & params);


    /**
     * @brief Tells the external interface to expect a return value from this function.
     Use handleValueChange() to actually return the value.
     * @param dest The expression whose value will be returned.
     * @param functionName The function whose value will be returned.
     * @param params The parameters associated with this function.
     */
    void registerFunctionReturnValue(ExpressionId dest,
				     const LabelStr & functionName,
				     const std::list<double> & params);

    /**
     * @brief Notify the external interface that this previously registered expression
     should not wait for a return value.
     * @param dest The expression whose value was to be returned.
     */
    void unregisterCommandReturnValue(ExpressionId dest);

    /**
     * @brief Notify the external interface that this previously registered expression
     should not wait for a return value.
     * @param dest The expression whose value was to be returned.
     */
    void unregisterFunctionReturnValue(ExpressionId dest);

    /**
     * @brief Notify the executive of a new plan.
     * @param planXml The TinyXML representation of the new plan.
     * @param parent The node which is the parent of the new node.
     * @note This is deprecated, use the PlexilNodeId variant instead.
     */
    void handleAddPlan(TiXmlElement * planXml,
		       const LabelStr& parent)
      throw(ParserException);

    /**
     * @brief Notify the executive of a new plan.
     * @param planStruct The PlexilNode representation of the new plan.
     * @param parent The node which is the parent of the new node.
     */
    void handleAddPlan(PlexilNodeId planStruct,
                       const LabelStr& parent);

    /**
     * @brief Notify the executive of a new library node.
     * @param planStruct The PlexilNode representation of the new library node.
     */
    void handleAddLibrary(PlexilNodeId planStruct);

    /**
     * @brief Notify the executive that it should run one cycle.  This should be sent after
     each batch of lookup and command return data.
    */
    void notifyOfExternalEvent();

    StateCacheId getStateCache() const;

    //
    // Utility accessors
    //

    // *** Is this actually needed?? ***
    inline PlexilExecId getExec() const
    { 
      return m_exec; 
    }

    /**
     * @brief Look up the unique key for a state.
     * @param state The state.
     * @param key The key associated with this state.
     * @return True if the key was found.
     */
    bool findStateKey(const State& state, StateKey& key);

    /**
     * @brief Get a unique key for a state, creating a new key for a new state.
     * @param state The state.
     * @param key The key.
     * @return True if a new key had to be generated.
     */
    bool keyForState(const State& state, StateKey& key);

    /**
     * @brief Get (a copy of) the State for this StateKey.
     * @param key The key to look up.
     * @param state The state associated with the key.
     * @return True if the key is found, false otherwise.
     */
    bool stateForKey(const StateKey& key, State& state) const;

    /**
     * @brief Clears the interface adapter registry.
     */
    void clearAdapterRegistry();


  protected:

    //
    // Internal functionality
    //

    // executes a command with the given arguments
    void executeCommand(const LabelStr& name,
			const std::list<double>& args,
			ExpressionId dest,
			ExpressionId ack);

    // rejects a command due to non-availability of resources
    void rejectCommand(const LabelStr& name,
			const std::list<double>& args,
			ExpressionId dest,
			ExpressionId ack);

    // executes a function call with the given arguments
    void executeFunctionCall(const LabelStr& name,
			     const std::list<double>& args,
			     ExpressionId dest,
			     ExpressionId ack);

    /**
     * @brief Removes the adapter and deletes it iff nothing refers to it.
     */
    void deleteIfUnknown(InterfaceAdapterId intf);

    friend class AdapterConfiguration;

  private:

    // Deliberately unimplemented
    InterfaceManager();
    InterfaceManager(const InterfaceManager &);
    InterfaceManager & operator=(const InterfaceManager &);

    /**
     * @brief update the resoruce arbiter interface that an ack or return value
     * has been received so that resources can be released.
     * @param ackOrDest The expression id for which a value has been posted.
     */
    void releaseResourcesAtCommandTermination(ExpressionId ackOrDest);

    /**
     * @brief Deletes the given adapter
     * @return true if the given adapter existed and was deleted. False if not found
     */
    bool deleteAdapter(InterfaceAdapterId intf);

    //
    // Internal types and classes
    //

    //
    // Value queue
    //

    /**
     * @brief Represents the type of a ValueQueue::QueueEntry instance.
     */
    enum QueueEntryType
      {
	queueEntry_EMPTY,
	queueEntry_MARK,
	queueEntry_LOOKUP_VALUES,
	queueEntry_RETURN_VALUE,
	queueEntry_PLAN,
	queueEntry_LIBRARY,
	queueEntry_ERROR
      };

    /**
     * @brief A private internal class where the InterfaceManager temporarily
     *        stores the results of asynchronous operations on the world outside
     *        the Exec.
     */
    class ValueQueue
    {
    public:
      ValueQueue();
      ~ValueQueue();

      // Inserts the new expression/value pair into the queue
      void enqueue(const ExpressionId & exp, double newValue);
      void enqueue(const StateKey & stateKey, const std::vector<double> & newValues);
      void enqueue(PlexilNodeId newPlan, const LabelStr & parent);
      void enqueue(PlexilNodeId newlibraryNode);

      /**
       * @brief Atomically check head of queue and dequeue if appropriate
       * @return Type of entry dequeued; queueEntry_EMPTY and queueEntry_MARK
       * indicate nothing of interest was dequeued
       */
      QueueEntryType dequeue(StateKey& stateKey, std::vector<double>& newStateValues,
			     ExpressionId& exp, double& newExpValue,
			     PlexilNodeId& plan, LabelStr& planParent);

      // returns true iff the queue is empty
      bool isEmpty() const;

      // inserts a marker expression into the queue
      void mark();

      // Remove queue head and ignore (presumably a mark)
      void pop();

    private:
      // deliberately unimplemented
      ValueQueue(const ValueQueue &);
      ValueQueue & operator=(const ValueQueue &);

      //
      // Internal methods
      //

      //
      // Member variables
      //

      DECLARE_STATIC_CLASS_CONST(State, 
				 NULL_STATE_KEY,
				 State(0.0, std::vector<double>()));

      /**
       * @brief Represents one entry in a ValueQueue.  
       *        A private class internal to ValueQueue.
       */
      class QueueEntry
      {
      public:
        /*
         * @brief Constructor for a QueueEntry representing a function or command return value.
         */
	QueueEntry(const ExpressionId & exp,
		   double val)
	  : expression(exp),
	    stateKey(),
	    values(1, val),
	    plan(),
	    parent(),
	    type(queueEntry_RETURN_VALUE)
	{
	}

        /*
         * @brief Constructor for a QueueEntry representing a single LookupOnChange return value.
         */
	QueueEntry(const StateKey & st,
		   double val)
	  : expression(),
	    stateKey(st),
	    values(1, val),
	    plan(),
	    parent(),
	    type(queueEntry_LOOKUP_VALUES)
	{
	}

        /*
         * @brief Constructor for a QueueEntry representing multiple return values
         *        for a LookupOnChange.
         */
	QueueEntry(const StateKey& st,
		   const std::vector<double> vals)
	  : expression(),
	    stateKey(st),
	    values(vals),
	    plan(),
	    parent(),
	    type(queueEntry_LOOKUP_VALUES)
	{
	}

        /*
         * @brief Constructor for a QueueEntry representing an external command
         *        to add a new plan or library node.
         */
	QueueEntry(PlexilNodeId newPlan, 
		   const LabelStr& parentNode,
                   const QueueEntryType typ)
	  : expression(),
	    stateKey(),
	    values(),
	    plan(newPlan),
	    parent(parentNode),
	    type(typ)
	{
          assertTrue((typ == queueEntry_PLAN) || (typ == queueEntry_LIBRARY),
                     "QueueEntry constructor: invalid entry type for plan or library");
	}

        /*
         * @brief Constructor for an empty QueueEntry of an arbitrary QueueEntryType.
         */
	QueueEntry(QueueEntryType typ)
	  : expression(),
	    stateKey(),
	    values(),
	    plan(),
	    parent(),
	    type(typ)
	{
	}

        /*
         * @brief Destructor.
         */
	~QueueEntry()
	{
	}
	
	ExpressionId expression;
	StateKey stateKey;
	std::vector<double> values;
	PlexilNodeId plan;
	LabelStr parent;
	QueueEntryType type;
      };

      /** The actual queue data structure. */
      std::queue<QueueEntry> m_queue;

      /** 
       * @brief Pointer to a mutex to prevent collisions between threads.
       * @note Implemented as a pointer so isEmpty() can be const.
       */
      ThreadMutex * m_mutex;

    };

    //
    // State cache
    //
    // The state cache is always updated by the exec thread.  Older 
    // implementations could have been updated by another thread, which
    // caused race conditions in the quiescence loop.
    //

    bool expressionToState(const ExpressionId & exp, State & key) const;
    bool updateStateListener(const ExpressionId & dest, double value);

    //
    // Private member variables
    //

    //* ID as an interface manager
    InterfaceManagerId m_interfaceManagerId;

    //* Parent object
    ExecApplication& m_application;
    //* Adapter Config
    AdapterConfigurationId m_adapterConfig;

    //* The queue
    ValueQueue m_valueQueue;

    //* Vector of all known ExecListener instances
    std::vector<ExecListenerId> m_listeners;

    //* Set of all known InterfaceAdapter instances
    std::set<InterfaceAdapterId> m_adapters;

    // Maps by lookup key
    typedef std::map<LookupKey, InterfaceAdapterId> LookupAdapterMap;
    LookupAdapterMap m_lookupAdapterMap;
    std::map<ExpressionId, CommandId> m_ackToCmdMap;
    std::map<ExpressionId, CommandId> m_destToCmdMap;

    // Properties
    typedef std::map<const std::string, void*> PropertyMap;
    PropertyMap m_propertyMap;

    //* The resource arbiter
    ResourceArbiterInterfaceId m_raInterface;

    //* Holds the most recent idea of the current time
    double m_currentTime;

  };

}

#endif // _H_InterfaceManager
