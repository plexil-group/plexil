/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_EXTERNAL_INTERFACE_HH
#define PLEXIL_EXTERNAL_INTERFACE_HH

#include "CommandHandle.hh"
#include "Expression.hh"
#include "LinkedQueue.hh"
#include "State.hh"

namespace PLEXIL {
  // Forward declarations
  class Command;
  class CommandImpl;
  class LookupReceiver;
  struct Message;
  class ResourceArbiterInterface;
  class Update;

  /**
   * @class ExternalInterface
   * @brief Abstract base class for anything that interfaces the Exec to the outside world.
   */

  class ExternalInterface {
  public:

    virtual ~ExternalInterface();

    //
    // API for Lookup
    //

    /**
     * @brief Perform an immediate lookup on an existing state.
     * @param state The state.
     * @param receiver Callback object to receive the lookup result.
     * @note Value is returned via methods on the LookupReceiver callback.
     */
    virtual void lookupNow(State const &state, LookupReceiver *receiver) = 0;

    //!
    // @brief Advise the interface of the current thresholds to use when reporting this state.
    // @param state The state.
    // @param hi The upper threshold, at or above which to report changes.
    // @param lo The lower threshold, at or below which to report changes.
    //
    // @note This is a kludge which is only used for the 'time' state, to set
    //       wakeups in tickless systems.
    //
    virtual void setThresholds(const State& state, Real hi, Real lo) = 0;
    virtual void setThresholds(const State& state, Integer hi, Integer lo) = 0;

    //!
    // @brief Tell the interface that thresholds are no longer in effect
    //        for this state.
    // @param state The state.
    //
    virtual void clearThresholds(const State& state) = 0;

    //
    // API to CommandImpl
    //
    
    /**
     * @brief Release resources in use by the command.
     */
    void releaseResourcesForCommand(CommandImpl *cmd);

    //
    // API to Exec
    //

    // Made virtual for convenience of module tests

    //! If the command has no resource requirements,
    //! execute it immediately.
    //! Otherwise set it aside for resource arbitration.
    //! @param cmd The command.
    virtual void processCommand(CommandImpl *cmd);

    //! Arbitrate any commands with resource requirements,
    //! and dispose of them as appropriate.
    virtual void partitionResourceCommands();

    //! Delegate this command to be aborted.
    //! @param cmd The command.
    virtual void abortCommand(CommandImpl *cmd);

    //! Delegate this update for execution.
    //1 @param update The update.
    virtual void executeUpdate(Update *update) = 0;

    //
    // Interface from outside world to plan state
    //

    /**
     * @brief Return a value from a lookup.
     * @param state Const reference to the state.
     * @param value Const reference to the value.
     */
    void lookupReturn(State const &state, Value const &value);

    /**
     * @brief Return a value from a command
     * @param cmd Pointer to the Command.
     * @param value Const reference to the value.
     */
    void commandReturn(Command *cmd, Value const &value);

    /**
     * @brief Return a command handle value for a command.
     * @param cmd Pointer to the Command.
     * @param value The command handle value.
     */
    void commandHandleReturn(Command *cmd, CommandHandleValue val);

    /**
     * @brief Return an abort-acknowledge value for a command.
     * @param cmd Pointer to the Command.
     * @param ack The acknowledgement value.
     */
    void commandAbortAcknowledge(Command *cmd, bool ack);

    /**
     * @brief Return an update acknowledgment value
     * @param upd Update ID reference.
     * @param value The ack value.
     */
    void acknowledgeUpdate(Update *upd, bool val);

    //
    // Message API
    //

    //! Receive notification of a message becoming available.
    //! @param msg Const pointer to the new message.
    //! @note Populates the PeekAtMessage and PeekAtMessageSender states.
    void messageReceived(Message const *msg);

    //! Receive notification that the message queue is empty.
    //! @note Sets the PeekAtMessage and PeekAtMessageSender states to unknown.
    void messageQueueEmpty();

    //! Accept an incoming message and associate it with the handle.
    //! @param msg Pointer to the message.  StateCache takes ownership of the message.
    //! @param handle String used as a handle for the message.
    //! @note StateCache is responsible to populate the message lookups.
    void assignMessageHandle(Message *msg, std::string const &handle);

    //! Release the message handle, and clear the message data
    //! associated with that handle.
    //! @param handle The handle being released.
    void releaseMessageHandle(std::string const &handle);

    //
    // API to application
    //
    
    /**
     * @brief Read command resource hierarchy from the named file.
     * @param fname File name.
     * @return True if successful, false otherwise.
     */

    bool readResourceFile(std::string const &fname);

  protected:

    // Default constructor.
    ExternalInterface();

    //
    // Member functions that are expected to be implemented by derived classes
    //

    /**
     * @brief Report the failure in the appropriate way for the application.
     */
    virtual void reportCommandArbitrationFailure(Command *cmd) = 0;

    //! Delegate this command for execution.
    //! @param cmd The command.
    virtual void executeCommand(Command *cmd) = 0;

    //! Delegate this command to be aborted.
    //! @param cmd The command.
    virtual void invokeAbort(Command *cmd) = 0;

  private:

    // Copy, assign disallowed
    ExternalInterface(ExternalInterface const &) = delete;
    ExternalInterface(ExternalInterface &&) = delete;
    ExternalInterface &operator=(ExternalInterface const &) = delete;
    ExternalInterface &operator=(ExternalInterface &&) = delete;

    //! Pending commands to arbitrate
    LinkedQueue<CommandImpl> m_resourceCmds;
    //! The resource arbiter
    ResourceArbiterInterface *m_raInterface;
  };

  extern ExternalInterface *g_interface;
}

#endif
