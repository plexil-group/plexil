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

#ifndef _H_ThreadedExternalInterface
#define _H_ThreadedExternalInterface

#include "ExecDefs.hh"
#include "Expression.hh"
#include "ExternalInterface.hh"
#include "AdaptorExecInterface.hh"
#include "PlexilPlan.hh"
#include "RecursiveThreadMutex.hh"
#include <queue>
#include <pthread.h>
#include <semaphore.h>

namespace PLEXIL 
{

  // Forward references
  class InterfaceAdaptor;
  typedef Id<InterfaceAdaptor> InterfaceAdaptorId;

  class ThreadedExternalInterface;
  typedef Id<ThreadedExternalInterface> ThreadedExternalInterfaceId;

  class ResourceArbiterInterface;
  typedef Id<ResourceArbiterInterface> ResourceArbiterInterfaceId;

  class ThreadedExternalInterface :
    public ExternalInterface,
    public AdaptorExecInterface
  {
  public:
    /**
     * @brief Default constructor.
     */
    ThreadedExternalInterface();

    /**
     * @brief Destructor.
     */
    virtual ~ThreadedExternalInterface();

    /**
     * @brief Accessor to singleton.
     */
    static ThreadedExternalInterfaceId instance();

    /**
     * @brief Accessor to internal ID pointer.
     */
    ThreadedExternalInterfaceId getId();

    //
    // Top-level loop
    //

    /**
     * @brief Spawns a thread which runs the exec's top level loop, and waits for the thread to terminate.
     */
    void run();

    /**
     * @brief Spawns a thread which runs the exec's top level loop.
     */
    void spawnExecThread();

    /**
     * @brief Tells the exec's top level loop to exit.
     */
    void stop();

    //
    // API for exec
    //

    /**
     * @brief Suspends the calling thread until another thread has
              placed a call to notifyOfExternalEvent().  Can return
              immediately if there has been an external event since
              the previous call.
     * @return true if resumed normally, false if system error (e.g. termination).
     */
    bool waitForExternalEvent();
    
    /**
     * @brief Delete any entries in the queue.
     */
    void resetQueue();
    
    /**
     * @brief Updates the state cache from the items in the queue.
     * @return True if the exec needs to be stepped, false otherwise.
     */
    bool processQueue();

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
     * @brief Register a frequency lookup on a new state, expecting values back.
     * @param source The unique key for this lookup.
     * @param state The state
     * @param key The key for the state to be used in future communications about the state.
     * @param lowFreq The most time allowable between updates, or the exec will assume UNKNOWN.
     * @param highFreq The least time allowable between updates.
     * @param dest The destination for the current values for the state.
     * @note dest is stack allocated, therefore pointers to it should not be stored!
     */
    void registerFrequencyLookup(const LookupKey& source,
				 const State& state,
				 const StateKey& key,
				 const double& lowFreq,
				 const double& highFreq,
				 std::vector<double>& dest);

    /**
     * @brief Register a frequency lookup on an existing state.
     * @param source The unique key for this lookup.
     * @param key The key for the state.
     * @param lowFreq The most time allowable between updates, or the exec will assume UNKNOWN.
     * @param highFreq The least time allowable between updates.
     */
    void registerFrequencyLookup(const LookupKey& source,
				 const StateKey& key,
				 const double& lowFreq,
				 const double& highFreq);

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

    /**
     * @brief Inform the FL that a lookup should no longer receive updates.
     */ 
    void unregisterFrequencyLookup(const LookupKey& dest);

    //this batches the set of actions from quiescence completion.  calls PlexilExecutive::step() at the end
    //assignments must be performed first.
    //though it only takes a list of commands as an argument at the moment, it will eventually take function calls and the like
    //so the name remains "batchActions"
    void batchActions(std::list<CommandId>& commands);
    void batchActions(std::list<FunctionCallId>& functionCalls);
    void updatePlanner(std::list<UpdateId>& updates);

    //abort the given command with the given arguments.  store the abort-complete into dest
    void invokeAbort(const LabelStr& name,
		     const std::list<double>& args,
		     ExpressionId dest);

    double currentTime();


    //
    // API to interface adaptors
    //

    /**
     * @brief Register the given interface adaptor for this command.  
     Returns true if successful.  Fails and returns false 
     iff the command name already has an adaptor registered.
     * @param commandName The command to map to this adaptor.
     * @param intf The interface adaptor to handle this command.
     */
    bool registerCommandInterface(const LabelStr & commandName,
				  InterfaceAdaptorId intf);

    /**
     * @brief Register the given interface adaptor for this function.  
              Returns true if successful.  Fails and returns false 
              iff the function name already has an adaptor registered.
     * @param functionName The function to map to this adaptor.
     * @param intf The interface adaptor to handle this function.
     */
    bool registerFunctionInterface(const LabelStr & functionName,
				   InterfaceAdaptorId intf);

    /**
     * @brief Register the given interface adaptor for lookups to this state.
     Returns true if successful.  Fails and returns false 
     if the state name already has an adaptor registered.
     * @param stateName The name of the state to map to this adaptor.
     * @param intf The interface adaptor to handle this lookup.
     */
    bool registerLookupInterface(const LabelStr & stateName,
				 InterfaceAdaptorId intf);

    /**
     * @brief Register the given interface adaptor for planner updates.
              Returns true if successful.  Fails and returns false 
              iff an adaptor is already registered.
     * @param intf The interface adaptor to handle planner updates.
     */
    bool registerPlannerUpdateInterface(InterfaceAdaptorId intf);

    /**
     * @brief Register the given interface adaptor as the default for all lookups and commands
     which do not have a specific adaptor.  Returns true if successful.
     Fails and returns false if there is already a default adaptor registered.
     * @param intf The interface adaptor to use as the default.
     */
    bool setDefaultInterface(InterfaceAdaptorId intf);

    /**
     * @brief Retract registration of the previous interface adaptor for this command.  
     * @param commandName The command.
     */
    void unregisterCommandInterface(const LabelStr & commandName);

    /**
     * @brief Retract registration of the previous interface adaptor for this function.  
     * @param functionName The function.
     */
    void unregisterFunctionInterface(const LabelStr & functionName);

    /**
     * @brief Retract registration of the previous interface adaptor for this state.
     * @param stateName The state name.
     */
    void unregisterLookupInterface(const LabelStr & stateName);

    /**
     * @brief Retract registration of the previous interface adaptor for planner updates.
     */
    void unregisterPlannerUpdateInterface();

    /**
     * @brief Retract registration of the previous default interface adaptor.
     */
    void unsetDefaultInterface();

    /**
     * @brief Return the interface adaptor in effect for this command, whether 
     specifically registered or default. May return NoId().
     * @param commandName The command.
     */
    InterfaceAdaptorId getCommandInterface(const LabelStr & commandName);

    /**
     * @brief Return the interface adaptor in effect for this function, whether 
     specifically registered or default. May return NoId().
     * @param functionName The function.
     */
    InterfaceAdaptorId getFunctionInterface(const LabelStr & functionName);

    /**
     * @brief Return the interface adaptor in effect for lookups with this state name,
     whether specifically registered or default. May return NoId().
     * @param stateName The state.
     */
    InterfaceAdaptorId getLookupInterface(const LabelStr & stateName);

    /**
     * @brief Return the interface adaptor in effect for planner updates,
              whether specifically registered or default. May return NoId().
     */
    InterfaceAdaptorId getPlannerUpdateInterface();

    /**
     * @brief Return the current default interface adaptor. May return NoId().
     */
    InterfaceAdaptorId getDefaultInterface();

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
     * @param name The function whose value will be returned.
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

    inline PlexilExecId getExec() const
    { return m_exec; }

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

    static void * execTopLevel(void * this_as_void_ptr);
    void runInternal();

    std::vector<PlexilNodeId> m_libraries;

  private:

    // Deliberately unimplemented
    ThreadedExternalInterface(const ThreadedExternalInterface &);
    ThreadedExternalInterface & operator=(const ThreadedExternalInterface &);

    /**
     * @brief update the resoruce arbiter interface that an ack or return value
     * has been received so that resources can be released.
     * @param ackOrDest The expression id for which a value has been posted.
     */
    void releaseResourcesAtCommandTermination(ExpressionId ackOrDest);

    //
    // Internal types and classes
    //

    //
    // Value queue
    //
    // The value queue is where deferred data (i.e. LookupOnChange, 
    // LookupWithFrequency, command return values) are stored until the 
    // exec thread has a chance to look at them.
    //

    enum QueueEntryType
      {
	queueEntry_UNINIT,
	queueEntry_MARK,
	queueEntry_LOOKUP_VALUES,
	queueEntry_RETURN_VALUE,
	queueEntry_PLAN,
	queueEntry_LIBRARY,
	queueEntry_ERROR
      };

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

      // if the queue is empty, returns true
      // if the queue is non-empty, extracts the head into the reference variables
      //  and returns false
      // N.B. caller must invoke getEntryType() first to determine which variant to call!
      bool dequeue(ExpressionId & exp, double & newValue);
      bool dequeue(StateKey & state, std::vector<double> & newValues);
      bool dequeue(PlexilNodeId & newPlan, LabelStr & parent);
      bool dequeue(PlexilNodeId & newLibraryNode);

      // Remove queue head and ignore (presumably a mark)
      void pop();

      // returns entry type of queue head 
      QueueEntryType getEntryType() const;

      // returns true iff the queue is empty
      bool isEmpty() const;

      // inserts a marker expression into the queue
      void mark();

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

      class QueueEntry
      {
      public:
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

	QueueEntry(PlexilNodeId newPlan, 
		   const LabelStr& parent,
                   const QueueEntryType typ)
	  : expression(),
	    stateKey(),
	    values(),
	    plan(newPlan),
	    parent(),
	    type(typ)
	{
          checkError((typ == queueEntry_PLAN) || (typ == queueEntry_LIBRARY),
                     "QueueEntry constructor: invalid entry type for plan or library");
	}

	QueueEntry(QueueEntryType typ)
	  : expression(),
	    stateKey(),
	    values(),
	    plan(),
	    parent(),
	    type(typ)
	{
	}

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

      std::queue<QueueEntry> m_queue;

      // pointer so isEmpty() can be const
      RecursiveThreadMutex * m_mutex;

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
    // Private static methods
    //


    //
    // Private member variables
    //
    ThreadedExternalInterfaceId m_threadedInterfaceId;

    ValueQueue m_valueQueue;

    // Maps by lookup key
    typedef std::map<LookupKey, InterfaceAdaptorId> LookupAdaptorMap;
    LookupAdaptorMap m_lookupAdaptorMap;

    // Maps by command/function

    // Interface adaptor maps
    typedef std::map<double, InterfaceAdaptorId> InterfaceMap;
    InterfaceMap m_lookupMap;
    InterfaceMap m_commandMap;
    InterfaceMap m_functionMap;
    InterfaceAdaptorId m_plannerUpdateInterface;
    InterfaceAdaptorId m_defaultInterface;
    
    ResourceArbiterInterfaceId m_raInterface;
    std::map<ExpressionId, CommandId> m_ackToCmdMap;
    std::map<ExpressionId, CommandId> m_destToCmdMap;

    // The all-important "latest time" cache
    double m_currentTime;

    //
    // Synchronization and mutual exclusion
    //

    // Thread in which the Exec runs
    pthread_t m_execThread;

    // this is a pointer so lookups can be const
    RecursiveThreadMutex * m_adaptorMutex;

    // Semaphore for notifying the Exec of external events
    sem_t m_sem;

    static ThreadedExternalInterfaceId s_threadedInterfaceInstance;

  };

}

#endif // _H_ThreadedExternalInterface
