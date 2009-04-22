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

#ifndef EXEC_APPLICATION_H
#define EXEC_APPLICATION_H

#include "Id.hh"
#include "InterfaceManager.hh"
#include "PlexilExec.hh"
#include "PlexilXmlParser.hh"
#include "RecursiveThreadMutex.hh"
#include "ThreadSemaphore.hh"

// STL
#include <set>
#include <vector>

#include <pthread.h>

// Forward references in global namespace
class TiXmlDocument;
class TiXmlElement;

namespace PLEXIL
{

  // forward references
  class InterfaceAdaptor;
  typedef Id<InterfaceAdaptor> InterfaceAdaptorId;

  class ExecApplication;
  typedef Id<ExecApplication> ExecApplicationId;

  /**
   * @brief Provides a "pre-packaged" application skeleton for Universal Exec users.
   */
  class ExecApplication
  {
  public:
    enum ApplicationState
      {
        APP_UNINITED,
        APP_INITED,
        APP_INTERFACES_STARTED,
        APP_RUNNING,
        APP_SUSPENDED,
        APP_STOPPED,
        APP_SHUTDOWN
      };

    /**
     * @brief Default constructor.
     */
    ExecApplication();

    /**
     * @brief Destructor.
     */
    virtual ~ExecApplication();

    inline const ExecApplicationId getId() const
    {
      return m_id;
    }

    inline const PlexilExecId getExec() const
    {
      return m_exec.getId();
    }

    inline InterfaceManagerId getInterfaceManager() const
    {
      return m_interface.getInterfaceManagerId();
    }

    inline const PlexilXmlParser& getParser() const
    {
      return m_parser;
    }

    inline const ApplicationState& getApplicationState() const
    {
      return m_state;
    }

    /**
     * @brief Initialize all internal data structures and interfaces.
     * @param configXml Configuration data to use.
     * @return true if successful, false otherwise.
     * @note The caller must ensure that all adaptor and listener factories
     *       have been created and registered before this call.
     */
    virtual bool initialize(const TiXmlElement * configXml);

    /**
     * @brief Start all the interfaces prior to execution.
     * @return true if successful, false otherwise.
     */
    virtual bool startInterfaces();

    /**
     * @brief Runs the initialized Exec.
     * @return true if successful, false otherwise.
     */
    virtual bool run();

    /**
     * @brief Suspends the running Exec.
     * @return true if successful, false otherwise.
     */
    virtual bool suspend();

    /**
     * @brief Resumes a suspended Exec.
     * @return true if successful, false otherwise.
     */
    virtual bool resume();

    /**
     * @brief Stops the Exec.
     * @return true if successful, false otherwise.
     */
    virtual bool stop();
   
    /**
     * @brief Resets a stopped Exec so that it can be run again.
     * @return true if successful, false otherwise.
     */
    virtual bool reset();

    /**
     * @brief Shuts down a stopped Exec.
     * @return true if successful, false otherwise.
     */
    virtual bool shutdown();

    /**
     * @brief Notify the executive that it should run one cycle.  This should be sent after
     each batch of lookup and command return data.
    */
    void notifyExec();

    /**
     * @brief Add a library as an XML document.
     * @return true if successful, false otherwise.
     */
    bool addLibrary(TiXmlDocument* libXml);

    /**
     * @brief Add a plan as an XML document.
     * @return true if successful, false otherwise.
     */
    bool addPlan(TiXmlDocument* planXml);

  protected:

    //
    // Exec top level
    //

    /**
     * @brief Select whether the exec runs opportunistically or only in background thread.
     * @param bkgndOnly True if background only, false if opportunistic.
     * @note Default is opportunistic.
     */
    void setRunExecInBkgndOnly(bool bkgndOnly);

    /**
     * @brief Start the exec thread
     */
    bool spawnExecThread();

    /**
     * @brief Exec top level loop for use with pthread_create
     * @param A pointer to the ExecApplication instance as a void *
     */
    static void * execTopLevel(void * this_as_void_ptr);

    /**
     * @brief Exec top level loop
     */
    void runInternal();

    /**
     * @brief Run the exec until the queue is empty.
     * @param stepFirst True if the exec should be stepped before checking the queue.
     * @note Acquires m_execMutex and holds until done.  
     * @note This should be the only method that acquires m_execMutex.
     */
    void runExec(bool stepFirst = false);

    /**
     * @brief Suspends the calling thread until another thread has
     *         placed a call to notifyOfExternalEvent().  Can return
     *	    immediately if the call to wait() returns an error.
     * @return true if resumed normally, false if wait resulted in an error.
     * @note ThreadSemaphore handles case of interrupted wait (errno == EINTR).
     */
    bool waitForExternalEvent();

    //
    // Common methods provided to subclasses
    //

    /**
     * @brief Transitions the application to the new state.
     * @return true if the new state is a legal transition from the current state, false if not.
     */ 
    bool setApplicationState(const ApplicationState& newState);

  private:

    //
    // Deliberately unimplemented
    //
    ExecApplication(const ExecApplication&);
    ExecApplication& operator=(const ExecApplication&);
    
    //
    // Member variables
    //
    ExecApplicationId m_id;

    PlexilExec m_exec;
    InterfaceManager m_interface;
    PlexilXmlParser m_parser;

    //
    // Synchronization and mutual exclusion
    //

    // Thread in which the Exec runs
    pthread_t m_execThread;

    // Serialize execution in exec to guarantee in-order processing of events
    RecursiveThreadMutex m_execMutex;

    // Semaphore for notifying the Exec of external events
    ThreadSemaphore m_sem;

    // Current state of the application
    ApplicationState m_state;

    // Flag to determine whether exec should run conservatively
    bool m_runExecInBkgndOnly;

  };

}

#endif // EXEC_APPLICATION_H
