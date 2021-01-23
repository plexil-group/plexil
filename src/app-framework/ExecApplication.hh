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

#ifndef EXEC_APPLICATION_H
#define EXEC_APPLICATION_H

#include "plexil-config.h"

#include <set>
#include <string>
#include <vector>

// Forward references
namespace pugi
{
  class xml_document;
  class xml_node;
}

namespace PLEXIL
{

  // forward references
  class AdapterConfiguration;
  class InterfaceAdapter;
  class InterfaceManager;

  //! Enumeration representing the states of the ExecApplication.
  enum ApplicationState
    {
     APP_UNINITED,
     APP_INITED,
     APP_READY,
     APP_RUNNING,
     APP_STOPPED
    };

  /**
   * @brief Return a human-readable name for the ApplicationState.
   * @param state An ApplicationState.
   * @return The name of the state as a C string.
   */
  const char* getApplicationStateName(ApplicationState state);

  //! @class ExecApplication
  //! Provides the skeleton of a complete PLEXIL Executive application.
  class ExecApplication
  {
  public:

    //! @brief Virtual destructor.
    virtual ~ExecApplication() = default;

    //
    // Application state
    //

    //! Get the current state of the application.
    //! @return The application state.
    virtual ApplicationState getApplicationState() = 0;

    //
    // General configuration
    //

    //! Select whether the exec runs opportunistically or only in background thread.
    //! @param bkgndOnly True if background only, false if opportunistic.
    //! @note Default is background only.
    virtual void setRunExecInBkgndOnly(bool bkgndOnly) = 0;

    //! Add the specified directory name to the end of the library node loading path.
    //! @param libdir The directory name.
    virtual void addLibraryPath(const std::string& libdir) = 0;

    //! Add the specified directory names to the end of the library node loading path.
    //! @param libdirs The vector of directory names.
    virtual void addLibraryPath(const std::vector<std::string>& libdirs) = 0;

    //
    // Initialization and startup
    //

    //! initialize all internal data structures and interfaces.
    //! @param configXml XML element of configuration data to use. May
    //!                  be empty.
    //! @return true if successful, false otherwise.
    //! @note The caller must ensure that all adapter and listener
    //!       factories have been created and registered before this
    //!       call.
    virtual bool initialize(pugi::xml_node const configXml) = 0;

    //! Start all the interfaces prior to execution.
    //! @return true if successful, false otherwise.
    //! @note If successful, the application will have transitioned to
    //!       the APP_READY state.
    virtual bool startInterfaces() = 0;

    //
    // Stepping the Exec under program control
    //

    //! Step the Exec once.
    //! @return true if successful, false otherwise.
    //! @note Can only be called in APP_READY state.
    //! @note Can be called when application is suspended.
    virtual bool step() = 0; 

    //! Ask the Exec whether all plan state is stable.
    //! @return true if stable, false if more transitions are possible.
    //! @note Meant to be called after step(), to see if more work needs doing.
    virtual bool isQuiescent() = 0;

    //! Step the Exec until the plan state is quiescent and the queue
    //! is empty.
    //! @return true if successful, false otherwise.
    //! @note Can only be called in APP_READY state.
    //! @note Can be called when application is suspended.
    virtual bool stepUntilQuiescent() = 0;

    //
    // Member functions for use in a threaded environment
    //

    //! Runs the initialized Exec in its own thread.
    //! @return true if successful, false otherwise.
    //! @note Application must be in APP_READY state.
    //! @note This is a no-op if PLEXIL is built without threads.
    virtual bool run() = 0;

    //! Suspends the running Exec.
    //! @return true if successful, false otherwise.
    virtual bool suspend() = 0;

    //! Query whether the Exec has been suspended. 
    //! @return True if suspended, false otherwise.
    virtual bool isSuspended() const = 0;

    //! Resumes a suspended Exec.
    //! @return true if successful, false otherwise.
    //! @note If called when not suspended, does nothing.
    virtual bool resume() = 0;

    //! Stops the Exec and its interfaces.
    virtual void stop() = 0;

    //! Whatever state the application may be in, bring it down in a
    //! controlled fashion.
    virtual void terminate() = 0;

    //!
    //! Notification and waiting
    //!

    //! Notify the exec thread that it should check the queue and run
    //! one cycle.
    //! @note This should be called after each batch of lookup and
    //!       command return data is enqueued.
    virtual void notifyExec() = 0;

    //! Notify the executive and wait for all queue entries up to the
    //! time of the call to be processed.
    virtual void notifyAndWaitForCompletion() = 0;

    //! Suspend the calling thread until the plan being executed
    //! finishes.
    virtual void waitForPlanFinished() = 0;

    //! Suspend the current thread until the application reaches
    //! APP_STOPPED state.
    //! @note Wait can be interrupted by signal handling; calling
    //! threads should block (e.g.) SIGALRM.
    virtual void waitForShutdown() = 0;

    //
    // Plans and libraries
    //

    //! Add a library from an XML document.
    //! @return true if successful, false otherwise.
    //! @note The application (actually the InterfaceManager)
    //        is responsible for deleting the document.
    virtual bool addLibrary(pugi::xml_document* libXml) = 0;

    //! Load the named library from a file in the library path.
    //! @param name The name of the library.
    //! @return true if successful, false otherwise.
    virtual bool loadLibrary(std::string const &name) = 0;

    //! Add a PLEXIL plan from an XML document.
    //! @return true if successful, false otherwise.
    //! @note The application (actually the InterfaceManager)
    //        is responsible for deleting the document.
    //! @note May only be called if application is in APP_READY or
    //!       APP_RUNNING state.
    virtual bool addPlan(pugi::xml_document* planXml) = 0;

    //!
    //! Communications with the InterfaceManager
    //!
    
    //! Notify the application that a queue mark was processed.
    //! @note Used to implement notifyAndWaitForCompletion().
    virtual void markProcessed() = 0;

    //
    // Accessors to application objects
    //

    virtual AdapterConfiguration *configuration() = 0;
    virtual InterfaceManager *manager() = 0;

  protected:

    //! Default constructor; only available to implementation classes.
    ExecApplication() = default;

  private:

    //
    // Deliberately unimplemented
    //
    ExecApplication(const ExecApplication &) = delete;
    ExecApplication(ExecApplication &&) = delete;
    ExecApplication &operator=(const ExecApplication&) = delete;
    ExecApplication &operator=(ExecApplication &&) = delete;
  };

  // Factory function
  ExecApplication *makeExecApplication();

}

#endif // EXEC_APPLICATION_H
