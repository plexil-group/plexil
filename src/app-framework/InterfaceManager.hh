// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_INTERFACE_MANAGER_HH
#define PLEXIL_INTERFACE_MANAGER_HH

#include "AdapterExecInterface.hh"

#include <memory>

// Forward reference
namespace pugi
{
  class xml_node;
  class xml_document;
}

namespace PLEXIL 
{
  // Forward references
  class ExecApplication;

  class InterfaceAdapter;

  class AdapterConfiguration;

  class InputQueue;

  //! @class InterfaceManager
  //! A concrete derived class implementing the API of the
  //! AdapterExecInterface class.
  //! @details The InterfaceManager class is responsible for managing
  //!          input for the PlexilExec.  It maintains a queue of
  //!          messages for the Exec to process.
  class InterfaceManager :
    public AdapterExecInterface
  {
  public:

    //! Constructor.
    //! @param app The ExecApplication instance to which this object belongs.
    //! @param config The AdapterConfiguration instance which
    //!               delegates commands and lookups to the
    //!               appropriate adapters and handlers.
    InterfaceManager(ExecApplication *app,
                     AdapterConfiguration *config);

    //! Virtual destructor.
    virtual ~InterfaceManager() = default;

    //
    // API to ExecApplication
    //

    //! Initialize the interface manager.
    //! @return true if successful, false otherwise.
    virtual bool initialize();
    
    //! Read the input queue and give the received data to the Exec.
    //! @return True if the exec needs to be stepped as a result of
    //! queue processing, false otherwise.
    //! @note Should only be called with exec locked by the current thread.
    bool processQueue();

    //! Insert a mark in the value queue.
    //! @return The sequence number of the mark.
    unsigned int markQueue();

    //
    // API to interface handlers
    //

    //
    // Lookup API
    //

    //! Notify of the availability of a new value for a lookup.
    //! @param state The state for the new value.
    //! @param value The new value.
    virtual void handleValueChange(const State &state, const Value &value);
    virtual void handleValueChange(const State &state, Value &&value);
    virtual void handleValueChange(State &&state, const Value &value);
    virtual void handleValueChange(State &&state, Value &&value);

    //
    // Command API
    //
    // Note that these member functions take the abstract base class
    // Command rather than the full-featured CommandImpl.

    //! Notify of the availability of a return value for a command
    //! @param cmd The command.
    //! @param value The new value.
    virtual void handleCommandReturn(Command * cmd, const Value &value);
    virtual void handleCommandReturn(Command * cmd, Value &&value);

    //! Notify of the availability of a new command handle value.
    //! @param cmd The command.
    //! @param value The new value.
    virtual void handleCommandAck(Command * cmd, CommandHandleValue value);

    //! Notify of completion of a command abort.
    //! @param cmd The command.
    //! @param ack Whether or not the abort was successful. 
    virtual void handleCommandAbortAck(Command * cmd, bool ack);

    //! Notify of completion of an Update.
    //! @param upd The update.
    //! @param ack Whether or not the update was successful. 
    virtual void handleUpdateAck(Update * upd, bool ack);

    //
    // Message API
    //

    //! Notify the executive that a message has been received.
    //! @param message The message.
    virtual void notifyMessageReceived(Message *message);

    //! Notify the executive that the message queue is empty.
    virtual void notifyMessageQueueEmpty();

    //! Notify the executive that a message has been accepted.
    //! @param message The message
    //! @param handle The message handle.
    virtual void notifyMessageAccepted(Message *message, std::string const &handle);

    //! Notify the executive that a message handle has been released.
    //! @param handle The message handle.
    virtual void notifyMessageHandleReleased(std::string const &handle);

    //
    // Plan API
    //

    //! Notify the executive of a new plan.
    //! @param planXml The XML representation of the new plan.
    virtual void handleAddPlan(pugi::xml_node const planXml);

    //! Notify the executive of a new library node.
    //! @param planXml The XML document containing the new library node.
    //! @return True if successful, false otherwise.
    virtual bool handleAddLibrary(pugi::xml_document *planXml);

    //! Load the named library from the library path.
    //! @param libname Name of the library node.
    //! @return True if successful, false if not found.
    bool handleLoadLibrary(std::string const &libName);

    //! Determine whether the named library is loaded.
    //! @param libName Name of the library.
    //! @return True if the named library has been loaded, false otherwise.
    bool isLibraryLoaded(const std::string& libName) const;

    //
    // Notify API
    //

    //! Notify the executive that it should run one cycle.
    virtual void notifyOfExternalEvent();

#ifdef PLEXIL_WITH_THREADS
    //! Run the exec and block the calling thread until all events in
    //! the queue at the time of the call have been processed.
    virtual void notifyAndWaitForCompletion();
#endif

  private:

    // Deliberately unimplemented
    InterfaceManager() = delete;
    InterfaceManager(InterfaceManager const &) = delete;
    InterfaceManager(InterfaceManager &&) = delete;

    InterfaceManager &operator=(InterfaceManager const &) = delete;
    InterfaceManager &operator=(InterfaceManager &&) = delete;

    //
    // Private member variables
    //

    //! Parent object
    ExecApplication *m_application;

    //! Adapter registry
    AdapterConfiguration *m_configuration;

    //! The queue of input data for the Exec.
    std::unique_ptr<InputQueue> m_inputQueue;

    //! Index of last queue mark enqueued.
    unsigned int m_markCount;
  };

}

#endif // PLEXIL_INTERFACE_MANAGER_HH
