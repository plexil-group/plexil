/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#ifndef _H_ExternalInterface
#define _H_ExternalInterface

#include "Expression.hh"
#include "Id.hh"
#include "State.hh"

#include <list>

namespace PLEXIL {
  // Forward declarations
  class Command;
  DECLARE_ID(Command);

  class StateCacheEntry;

  class Update;
  DECLARE_ID(Update);

  class ExternalInterface;
  DECLARE_ID(ExternalInterface);

  /**
   * @class ExternalInterface
   * @brief Abstract base class for anything that interfaces the Exec to the outside world.
   */

  class ExternalInterface {
  public:

    virtual ~ExternalInterface();

    /**
     * @brief Return the ID of this instance.
     */
    ExternalInterfaceId const &getId();

    //
    // API to Lookup and StateCacheEntry
    //

    //
    // The cycle counter is used by the Lookup interface to check whether a value is stale.
    // It is incremented by the PlexilExec.
    //

    /**
     * @brief Return the number of "macro steps" since this instance was constructed.
     * @return The macro step count.
     */
    unsigned int getCycleCount() const;

    /**
     * @brief Perform an immediate lookup on an existing state.
     * @param state The state.
     * @param cacheEntry The entry in the state cache.
     * @note Value is returned via callback on StateCacheEntry.
     */
    virtual void lookupNow(State const &state, StateCacheEntry &cacheEntry) = 0;

    /**
     * @brief Inform the interface that it should report changes in value of this state.
     * @param state The state.
     */
    virtual void subscribe(const State& state) = 0;

    /**
     * @brief Inform the interface that a lookup should no longer receive updates.
     */
    virtual void unsubscribe(const State& state) = 0;

    /**
     * @brief Advise the interface of the current thresholds to use when reporting this state.
     * @param state The state.
     * @param hi The upper threshold, at or above which to report changes.
     * @param lo The lower threshold, at or below which to report changes.
     */
    virtual void setThresholds(const State& state, double hi, double lo) = 0;
    virtual void setThresholds(const State& state, int32_t hi, int32_t lo) = 0;

    //
    // API to Node classes
    //

    /**
     * @brief Schedule this command for execution.
     */
    void enqueueCommand(CommandId const &cmd);

    /**
     * @brief Abort the pending command.
     */
    void abortCommand(CommandId const &cmd);

    /**
     * @brief Schedule this update for execution.
     */
    void enqueueUpdate(const UpdateId& update);

    //
    // API to Exec
    //

    /**
     * @brief Send all pending commands and updates to the external system(s).
     */
    void executeOutboundQueue();

    /**
     * @brief See if the command and update queues are empty.
     * @return True if both empty, false otherwise.
     */
    bool outboundQueueEmpty() const;

    // Returns the current time.
    // FIXME - use real time type
    virtual double currentTime() = 0;

    /**
     * @brief Increment the macro step count and return the new value.
     * @return The updated macro step count.
     */
    unsigned int incrementCycleCount();

  protected:

    // Default constructor.
    ExternalInterface();

    //
    // Member functions that are expected to be implemented by derived classes
    //

    /**
     * @brief Schedule this command for execution.
     */
    virtual void executeCommand(CommandId const &cmd) = 0;

    /**
     * @brief Abort the pending command.
     * @param cmd The command.
     */
    virtual void invokeAbort(CommandId const &cmd) = 0;

    /**
     * @brief Schedule this update for execution.
     */
    virtual void executeUpdate(const UpdateId& update) = 0;

  private:

    // Copy, assign disallowed
    ExternalInterface(ExternalInterface const &);
    ExternalInterface &operator=(ExternalInterface const &);

    ExternalInterfaceId m_id;
    std::vector<CommandId> m_commandsToExecute;
    std::vector<UpdateId> m_updatesToExecute;
    unsigned int m_cycleCount;
  };

  extern ExternalInterfaceId g_interface;
}

#endif
