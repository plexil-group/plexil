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

#include "ExecApplication.hh"

#include "AdapterConfiguration.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExecListenerHub.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceManager.hh"
#include "InputQueue.hh"
#include "PlexilExec.hh"
#include "PlexilSchema.hh"
#include "StateCache.hh"

#include "pugixml.hpp"

#if defined(HAVE_CSIGNAL)
#include <csignal>
#elif defined(HAVE_SIGNAL_H)
#include <signal.h>
#endif

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

#if defined(HAVE_UNISTD_H)
#include <unistd.h> // sleep()
#endif

#ifdef PLEXIL_WITH_THREADS
#include "ThreadSemaphore.hh"
#include <exception>
#include <mutex>
#include <thread>

using ThreadMutexGuard = std::lock_guard<std::mutex>;
using RTMutexGuard = std::lock_guard<std::recursive_mutex>;

#if defined(HAVE_PTHREAD_H)
#include <pthread.h>
#endif

#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h> // pid_t
#endif

#endif // PLEXIL_WITH_THREADS

#define EXEC_APPLICATION_MAX_N_SIGNALS 8

namespace PLEXIL
{
  
  //! @brief Return a human-readable name for the ApplicationState.
  //! @param state An ApplicationState.
  //! @return The name of the state as a C string.
  const char* getApplicationStateName(ApplicationState state)
  {
    switch (state) {
    case APP_UNINITED:
      return "APP_UNINITED";
      break;

    case APP_INITED:
      return "APP_INITED";
      break;

    case APP_READY:
      return "APP_READY";
      break;

    case APP_RUNNING:
      return "APP_RUNNING";
      break;

    case APP_STOPPED:
      return "APP_STOPPED";
      break;

    default:
      return "*** ILLEGAL APPLICATION STATE ***";
      break;
    }
  }

  class ExecApplicationImpl final : public ExecApplication
  {
  private:
    //
    // Typedefs
    //
    using AdapterConfigurationPtr = std::unique_ptr<AdapterConfiguration>;
    using InterfaceManagerPtr = std::unique_ptr<InterfaceManager>;

    //
    // Member variables
    //

#ifdef PLEXIL_WITH_THREADS
    //
    // Synchronization and mutual exclusion
    //

    // Thread in which the Exec runs
    std::thread m_execThread;

    // Serialize execution in exec to guarantee in-order processing of events
    std::recursive_mutex m_execMutex;

    // Mutex for application state
    std::mutex m_stateMutex;

    // Semaphore for notifying the Exec of external events
    ThreadSemaphore m_sem;

    // Semaphore for notifyAndWaitForCompletion()
    ThreadSemaphore m_markSem;

    // Semaphore for notifying external threads that the application is shut down
    ThreadSemaphore m_shutdownSem;

#endif 
    //! Interfacing database and dispatcher
    AdapterConfigurationPtr m_configuration;

    //! Interface manager
    InterfaceManagerPtr m_manager;

    //
    // Signal handling
    //
    sigset_t m_workerSigset;
    sigset_t m_restoreWorkerSigset;
    sigset_t m_mainSigset;
    sigset_t m_restoreMainSigset;
    struct sigaction m_restoreUSR2Handler;
    size_t m_nBlockedSignals;
    int m_blockedSignals[EXEC_APPLICATION_MAX_N_SIGNALS + 1];

    // Current state of the application
    ApplicationState m_state;

    // Flag to determine whether exec should run conservatively
    bool m_runExecInBkgndOnly;

    // Flag for halting the Exec thread
    bool m_stop;

    // Flag for suspend/resume
    bool m_suspended;

  public:

    //! Constructor.
    ExecApplicationImpl()
      : ExecApplication(),
#ifdef PLEXIL_WITH_THREADS
        m_execThread(),
        m_execMutex(),
        m_stateMutex(),
        m_sem(),
        m_markSem(),
        m_shutdownSem(),
#endif
        m_configuration(makeAdapterConfiguration()),
        m_manager(new InterfaceManager(this, m_configuration.get())),
        m_nBlockedSignals(0),
        m_state(APP_UNINITED),
        m_runExecInBkgndOnly(true),
        m_stop(false),
        m_suspended(false)
    {
      for (size_t i = 0; i <= EXEC_APPLICATION_MAX_N_SIGNALS; i++)
        m_blockedSignals[i] = 0;

      // Set globals that other pieces rely on
      // Required by Exec core
      g_interface = static_cast<ExternalInterface *>(m_manager.get());
      g_exec = makePlexilExec();

      // Link the Exec to the listener hub
      g_exec->setExecListener(m_configuration->getListenerHub());
    }

    virtual ~ExecApplicationImpl()
    {
      // Reset global pointers to objects we own before they are deleted
      g_interface = nullptr;

      delete g_exec;
      g_exec = nullptr;
    }

    //
    // Accessors
    //

    virtual AdapterConfiguration *configuration() override
    {
      return m_configuration.get();
    }

    virtual InterfaceManager *manager() override
    {
      return m_manager.get();
    }

    //
    // Application state
    //
    
    //! Get the application's current state.
    //! @return The application state.
    virtual ApplicationState getApplicationState() override
    {
#ifdef PLEXIL_WITH_THREADS
      ThreadMutexGuard guard(m_stateMutex);
#endif
      return m_state;
    }

    //
    // General configuration
    //

    //! Select whether the exec runs opportunistically or only in background thread.
    //! @param bkgndOnly True if background only, false if opportunistic.
    virtual void setRunExecInBkgndOnly(bool bkgndOnly) override
    { 
      m_runExecInBkgndOnly = bkgndOnly; 
    }

    //! Add the specified directory name to the end of the library node loading path.
    //! @param libdir The directory name.
    virtual void addLibraryPath(const std::string& libdir) override
    {
      m_configuration->addLibraryPath(libdir);
    }

    //! Add the specified directory names to the end of the library node loading path.
    //! @param libdirs The vector of directory names.
    virtual void addLibraryPath(const std::vector<std::string>& libdirs) override
    {
      m_configuration->addLibraryPath(libdirs);
    }

    //
    // Initialization and startup
    //

    //! Initialize all internal data structures and interfaces.
    //! @param configXml Configuration data to use; may be empty.
    //! @return true if successful, false otherwise.
    //! @note The caller must ensure that all adapter and listener
    //!       factories have been created and registered before this
    //!       call.
    virtual bool initialize(pugi::xml_node const configXml) override
    {
      condDebugMsg(configXml.empty(), "ExecApplication:initialize", " configuration is null");
      condDebugMsg(!configXml.empty(), "ExecApplication:initialize",
                   " configuration = " << configXml); // *** FIXME - PRINTS "1" ***

      if (m_state != APP_UNINITED) {
        debugMsg("ExecApplication:initialize",
                 " application already initialized");
        return false;
      }

      // Perform one-time initializations

      // Load debug configuration from XML
      // *** NYI ***

      // Construct interfaces
      if (!m_configuration->constructInterfaces(configXml, *m_manager)) {
        debugMsg("ExecApplication:initialize",
                 " construction of interfaces failed");
        return false;
      }

      // Initialize interfaces
      if (!m_configuration->initialize()) {
        debugMsg("ExecApplication:initialize",
                 " initialization of interfaces failed");
        return false;
      }

      // Initialize interface manager
      if (!m_manager->initialize()) {
        debugMsg("ExecApplication:initialize",
                 " initialization of interface manager failed");
        return false;
      }

      // Set the application state and return
      return setApplicationState(APP_INITED);
    }

    //! Start all the interfaces prior to execution.
    //! @return true if successful, false otherwise.
    virtual bool startInterfaces() override
    {
      if (m_state != APP_INITED)
        return false;

      // Start 'em up!
      if (!m_configuration->start()) {
        debugMsg("ExecApplication:startInterfaces",
                 " failed to start interfaces");
        return false;
      }
      
      return setApplicationState(APP_READY);
    }

    //! Step the Exec once.
    // @return true if successful, false otherwise.
    virtual bool step() override
    {
      if (m_state != APP_READY)
        return false;

      {
#ifdef PLEXIL_WITH_THREADS
        RTMutexGuard guard(m_execMutex);
#endif
        m_manager->processQueue();           // for effect
        double now = StateCache::queryTime(); // update time before attempting to step
        if (g_exec->needsStep()) {
          g_exec->step(now);
          debugMsg("ExecApplication:step", " complete");
        }
        else {
          debugMsg("ExecApplication:step", " no step required");
        }
      }

      return true;
    }

    //! Query whether the Exec has finished propagating state.
    //! @return True if quiescent, false otherwise.
    virtual bool isQuiescent() override
    {
      if (m_state != APP_READY)
        return true; // can't execute if not ready

      {
#ifdef PLEXIL_WITH_THREADS
        RTMutexGuard guard(m_execMutex);
#endif
        return !g_exec->needsStep();
      }
    }

    //! Step the Exec until the queue is empty.
    //! @return true if successful, false otherwise.
    virtual bool stepUntilQuiescent() override
    {
      if (m_state != APP_READY)
        return false;

      {
#ifdef PLEXIL_WITH_THREADS
        RTMutexGuard guard(m_execMutex);
#endif
        debugMsg("ExecApplication:stepUntilQuiescent", " Checking interface queue");
        m_manager->processQueue(); // for effect
        double now = StateCache::queryTime(); // update time before attempting to step
        while (g_exec->needsStep()) {
          debugMsg("ExecApplication:stepUntilQuiescent", " Stepping exec");
          g_exec->step(now);
          now = StateCache::queryTime(); // update time before attempting to step again
        }
        g_exec->deleteFinishedPlans();
      }
      debugMsg("ExecApplication:stepUntilQuiescent",
               " completed, queue empty and Exec quiescent.");

      return true;
    }

    //
    // Running in a threaded environment
    //

    //! Runs the initialized Exec.
    //! @return true if successful, false otherwise.
    virtual bool run() override
    {
#ifdef PLEXIL_WITH_THREADS
      if (m_state != APP_READY)
        return false;

      // Clear suspended flag just in case
      m_suspended = false;

      // Set up signal handling in main thread
      if (!initializeMainSignalHandling()) {
        warn("ExecApplication: failed to initialize main thread signal handling");
        return false;
      }

      // Start the event listener thread
      return spawnExecThread();
#else // !defined(PLEXIL_WITH_THREADS)
      warn("ExecApplication: Can't run background thread; threads not enabled in the build");
      return false;
#endif // PLEXIL_WITH_THREADS
    }

    //! Suspends the running Exec.
    //! @return true if successful, false otherwise.
    virtual bool suspend() override
    {
      if (m_state == APP_READY)
        return true; // already paused

      if (m_state != APP_RUNNING)
        return false;

      if (m_suspended)
        return true;

      // Suspend the Exec 
      m_suspended = true;
    
      // *** NYI: wait here til current step completes ***
      return setApplicationState(APP_READY);
    }

    //! Query whether the Exec has been suspended. 
    //! @return True if suspended, false otherwise.
    virtual bool isSuspended() const override
    {
      return m_state == APP_READY || m_suspended;
    }

    //! Resumes a suspended Exec.
    //! @return true if successful, false otherwise.
    virtual bool resume() override
    {
      // Can only resume if ready and suspended
      if (m_state != APP_READY)
        return false;
      if (!m_suspended)
        return true;

      // Resume the Exec
      m_suspended = false;
      notifyExec();
    
      return setApplicationState(APP_RUNNING);
    }

    //! Stops the Exec and its interfaces.
    virtual void stop() override
    {
      if (m_state != APP_RUNNING
          && m_state != APP_READY)
        return;

#ifdef PLEXIL_WITH_THREADS
      // Stop the Exec
      if (m_execThread.joinable()) {
        debugMsg("ExecApplication:stop", " Halting top level thread");
        m_stop = true;
        int status = m_sem.post();
        if (status) {
          warn("ExecApplication: semaphore post failed, status = " << status);
          return;
        }
        sleep(1);

        // FIXME
        if (m_stop) {
          // Exec thread failed to acknowledge stop - resort to stronger measures
          status = kill(getpid(), SIGUSR2);
          if (status) {
            warn("ExecApplication: kill failed, status = " << status);
            return; // not much else we can do
          }
          sleep(1);
        }

        m_execThread.join();
        debugMsg("ExecApplication:stop", " Top level thread stopped");

        if (!restoreMainSignalHandling()) {
          warn("ExecApplication: failed to restore signal handling for main thread");
          return;
        }
      }
#endif // PLEXIL_WITH_THREADS

      // Stop interfaces
      m_configuration->stop();
      setApplicationState(APP_STOPPED);
    }

    /**
     * @brief Whatever state the application may be in, bring it down in a controlled fashion.
     */
    virtual void terminate() override
    {
      std::cout << "Terminating PLEXIL Exec application" << std::endl;
      ApplicationState initState = getApplicationState();
      debugMsg("ExecApplication:terminate", " from state " << getApplicationStateName(initState));

      switch (initState) {
      case APP_UNINITED:
      case APP_STOPPED:
        // nothing to do
        break;

      case APP_INITED:
      case APP_READY:
        // Shut down interfaces
        m_configuration->stop();
        break;

      case APP_RUNNING:
        stop();
        break;
      }
      std::cout << "PLEXIL Exec terminated" << std::endl;
    }

    //! Notify the Exec thread that it should check the queue and run
    //! one cycle.
    virtual void notifyExec() override
    {
#ifdef PLEXIL_WITH_THREADS
      if (!m_runExecInBkgndOnly && m_execMutex.try_lock()) {
        // Exec is idle, so run it
        debugMsg("ExecApplication:notify", " exec was idle, stepping it");
        this->runExec(false);
        m_execMutex.unlock();
      }
      else {
        // Some thread currently owns the exec. Could be this thread.
        // runExec() could notice, or not.
        // Post to semaphore to ensure event is not lost.
        int status = m_sem.post();
        if (status) {
          warn("notifyExec: semaphore post failed, status = " << status);
        }
        else {
          debugMsg("ExecApplication:notify", " released semaphore");
        }
      }
#else
      // Don't do a thing - caller will tell us when to run
#endif
    }

    //! Run the exec and wait until all events in the queue have been processed. 
    virtual void notifyAndWaitForCompletion() override
    {
#ifdef PLEXIL_WITH_THREADS
      debugMsg("ExecApplication:notifyAndWait", " received external event");
      unsigned int sequence = m_manager->markQueue();
      notifyExec();
      while (m_manager->getLastMark() < sequence) {
        m_markSem.wait();
        m_markSem.post(); // in case it's not our mark and we got there first
      }
#else // !defined(PLEXIL_WITH_THREADS)
      warn("notifyAndWaitForCompletion: threads not enabled in build");
#endif // PLEXIL_WITH_THREADS
    }

    /**
     * @brief Suspend the current thread until the plan finishes executing.
     */
    virtual void waitForPlanFinished() override
    {
#ifdef PLEXIL_WITH_THREADS
      bool finished = false;
      while (!finished) {
        // sleep for a bit so as not to hog the CPU
        sleep(1);

        debugMsg("ExecApplication:waitForPlanFinished", " checking");
    
        // grab the exec and find out if it's finished yet
        RTMutexGuard guard(m_execMutex);
        finished = g_exec->allPlansFinished();
      }
      debugMsg("ExecApplication:waitForPlanFinished", " succeeded");
#else // !defined(PLEXIL_WITH_THREADS)
      warn("waitForPlanFinished: threads not enabled in build");
#endif // PLEXIL_WITH_THREADS
    }

    /**
     * @brief Suspend the current thread until the application reaches APP_STOPPED state.
     * @note May be called by multiple threads
     * @note Wait can be interrupted by signal handling; calling threads should block (e.g.) SIGALRM.
     */
    virtual void waitForShutdown() override
    {
#ifdef PLEXIL_WITH_THREADS
      int waitStatus = m_shutdownSem.wait();
      checkError(!waitStatus,
                 "waitForShutdown: semaphore wait got error " << waitStatus);
      m_shutdownSem.post(); // pass it on to the next, if any
#else // !defined(PLEXIL_WITH_THREADS)
      warn("waitForShutdown: threads not enabled in build");
#endif // PLEXIL_WITH_THREADS
    }

    //
    // Plans and libraries
    //

    /**
     * @brief Add a library as an XML document.
     * @return true if successful, false otherwise.
     */
    virtual bool addLibrary(pugi::xml_document* libraryXml) override
    {
      if (m_state != APP_RUNNING && m_state != APP_READY)
        return false;

      // Delegate to InterfaceManager
      if (m_manager->handleAddLibrary(libraryXml)) {
        debugMsg("ExecApplication:addLibrary", " Library added");
        return true;
      }
      else {
        debugMsg("ExecApplication:addLibrary", " failed");
        return true;
      }
    }

    /**
     * @brief Load the named library from the library path.
     * @param name The name of the library.
     * @return true if successful, false otherwise.
     */
    virtual bool loadLibrary(std::string const &name) override
    {
      if (m_state != APP_RUNNING && m_state != APP_READY)
        return false;

      bool result = false;

      // Delegate to InterfaceManager
      try {
        result = m_manager->handleLoadLibrary(name);
      }
      catch (const ParserException& e) {
        std::cerr << "loadLibrary: Error:\n" << e.what() << std::endl;
        return false;
      }

      if (result) {
        debugMsg("ExecApplication:loadLibrary", " Library " << name << " loaded");
      }
      else {
        debugMsg("ExecApplication:loadLibrary", " Library " << name << " not found");
      }
      return result;
    }

    /**
     * @brief Add a plan as an XML document.
     * @return true if successful, false otherwise.
     */
    virtual bool addPlan(pugi::xml_document* planXml) override
    {
      if (m_state != APP_RUNNING && m_state != APP_READY)
        return false;

      // Delegate to InterfaceManager
      try {
        m_manager->handleAddPlan(planXml->document_element());
        debugMsg("ExecApplication:addPlan", " successful");
        return true;
      }
      catch (const ParserException& e) {
        std::cerr << "addPlan: Plan parser error: \n" << e.what() << std::endl;
        return false;
      }
    }

    //! Notify the application that a queue mark was processed.
    virtual void markProcessed() override
    {
#ifdef PLEXIL_WITH_THREADS
      m_markSem.post();
#endif
    }

  private:

    //
    // Implementation methods
    //

#ifdef PLEXIL_WITH_THREADS
    /**
     * @brief Spawns a thread which runs the exec's top level loop.
     * @return true if successful, false otherwise.
     */
    bool spawnExecThread()
    {
      debugMsg("ExecApplication:run", " Spawning top level thread");
      m_execThread = std::thread([this]() -> void {this->runInternal();});
      debugMsg("ExecApplication:run", " Top level thread running");
      return setApplicationState(APP_RUNNING);
    }

    //
    // Exception used when the exec thread fails to stop as requested
    //

    class EmergencyBrake final : public std::exception
    {
    public:
      EmergencyBrake() noexcept = default;
      virtual ~EmergencyBrake() noexcept = default;

      virtual const char *what() const noexcept
      { return "PLEXIL Exec emergency stop"; }
    };

    //! The top level of the worker thread.
    void runInternal()
    {
      debugMsg("ExecApplication:runInternal", " Thread started");

      // set up signal handling environment for this thread
      if (!initializeWorkerSignalHandling()) {
        warn("ExecApplication: Worker signal handling initialization failed.");
        return;
      }

      try {
        // must step exec once to initialize time
        runExec(true);
        debugMsg("ExecApplication:runInternal", " Initial step complete");

        while (waitForExternalEvent()) {
          if (m_stop) {
            debugMsg("ExecApplication:runInternal", " Received stop request");
            m_stop = false; // acknowledge stop request
            break;
          }
          runExec(false);
        }
      } catch (EmergencyBrake const &b) {
        debugMsg("ExecApplication:runInternal", " exiting on signal");
      } catch (std::exception const &e) {
        debugMsg("ExecApplication:runInternal",
                 " exiting due to exception:\n " << e.what());
      }

      // restore old signal handlers for this thread
      // don't bother to check for errors
      restoreWorkerSignalHandling();

      debugMsg("ExecApplication:runInternal", " Ending the thread loop.");
    }
#endif // PLEXIL_WITH_THREADS

    /**
     * @brief Run the exec until the queue is empty.
     * @param stepFirst True if the exec should be stepped before checking the queue.
     * @note Acquires m_execMutex and holds until done.  
     */
    void runExec(bool stepFirst)
    {
#ifdef PLEXIL_WITH_THREADS
      RTMutexGuard guard(m_execMutex);
#endif
      if (stepFirst) {
        debugMsg("ExecApplication:runExec", " Stepping exec because stepFirst is set");
        g_exec->step(StateCache::queryTime());
      }
      if (m_suspended) {
        debugMsg("ExecApplication:runExec", " Suspended");
      }
      else {
        m_manager->processQueue(); // for effect
        do {
          double now = StateCache::queryTime(); // update time before attempting to step
          while (g_exec->needsStep()) {
            debugMsg("ExecApplication:runExec", " Stepping exec");
            g_exec->step(now);
            now = StateCache::queryTime(); // update time before stepping again
          }
        } while (m_manager->processQueue());
        debugMsg("ExecApplication:runExec", " Queue empty and exec quiescent");
      }

      // Clean up
      g_exec->deleteFinishedPlans();
    }

#ifdef PLEXIL_WITH_THREADS
    /**
     * @brief Suspends the calling thread until another thread has
     *         placed a call to notifyExec().  Can return
     *        immediately if the call to wait() returns an error.
     * @return true if resumed normally, false if wait resulted in an error.
     * @note Can wait here indefinitely while the application is suspended.
     */
    bool waitForExternalEvent()
    {
      if (m_nBlockedSignals == 0) {
        warn("ExecApplication: signal handling not initialized.");
        return false;
      }

      debugMsg("ExecApplication:wait", " waiting for external event");
      int status;
      do {
        status = m_sem.wait();
        if (status == 0) {
          condDebugMsg(!m_suspended, 
                       "ExecApplication:wait",
                       " acquired semaphore, processing external event");
          condDebugMsg(m_suspended, 
                       "ExecApplication:wait",
                       " Application is suspended, ignoring external event");
        }
      } 
      while (m_suspended);
      return (status == 0);
    }
#endif // PLEXIL_WITH_THREADS

    /**
     * @brief Transitions the application to the new state.
     * @return true if the new state is a legal transition from the current state, false if not.
     */ 
    bool setApplicationState(const ApplicationState& newState)
    {
      debugMsg("ExecApplication:setApplicationState",
               "(" << getApplicationStateName(newState)
               << ") from " << getApplicationStateName(m_state));

      assertTrueMsg(newState != APP_UNINITED,
                    "APP_UNINITED is an invalid state for setApplicationState");

      // variable binding context for guard -- DO NOT DELETE THESE BRACES!
      {
#ifdef PLEXIL_WITH_THREADS
        ThreadMutexGuard guard(m_stateMutex);
#endif
        switch (newState) {
        case APP_INITED:
          if (m_state != APP_UNINITED && m_state != APP_STOPPED) {
            debugMsg("ExecApplication:setApplicationState",
                     " Illegal application state transition to APP_INITED");
            return false;
          }
          m_state = newState;
          break;

        case APP_READY:
          if (m_state != APP_INITED && m_state != APP_RUNNING) {
            debugMsg("ExecApplication:setApplicationState",
                     " Illegal application state transition to APP_READY");
            return false;
          }
          m_state = newState;
          break;

        case APP_RUNNING:
          if (m_state != APP_READY) {
            debugMsg("ExecApplication:setApplicationState", 
                     " Illegal application state transition to APP_RUNNING");
            return false;
          }
          m_state = newState;
          break;

        case APP_STOPPED:
          if (m_state != APP_RUNNING && m_state != APP_READY) {
            debugMsg("ExecApplication:setApplicationState", 
                     " Illegal application state transition to APP_STOPPED");
            return false;
          }
          m_state = newState;
          break;

        default:
          debugMsg("ExecApplication:setApplicationState",
                   " Attempt to set state to illegal value " << newState);
          break;

        }
        // end variable binding context for guard -- DO NOT DELETE THESE BRACES!
      }

      if (newState == APP_STOPPED) {
#ifdef PLEXIL_WITH_THREADS
        // Notify any threads waiting for this state
        m_shutdownSem.post();
#endif
      }

      debugMsg("ExecApplication:setApplicationState",
               " to " << getApplicationStateName(newState) << " successful");
      return true;
    }

    //
    // Signal handling
    //

    /**
     * @brief Dummy signal handler function for signals we process.
     * @note This should never be called!
     */
    void dummySignalHandler(int /* signo */) {}

#ifdef PLEXIL_WITH_THREADS
    /**
     * @brief Handler for asynchronous kill of Exec thread
     * @param signo The signal.
     */
    static void emergencyStop(int signo) 
    {
      debugMsg("ExecApplication:stop", " Received signal " << signo);
      throw EmergencyBrake();
    }

    // Prevent the Exec run thread from seeing the signals listed below.
    // Applications are free to deal with them in other ways.

    bool initializeWorkerSignalHandling()
    {
      static int signumsToIgnore[EXEC_APPLICATION_MAX_N_SIGNALS] =
        {
         SIGINT,   // user interrupt (i.e. control-C)
         SIGHUP,   // hangup
         SIGQUIT,  // quit
         SIGTERM,  // kill
         SIGALRM,  // timer interrupt, used by (e.g.) ItimerTimebase
         SIGUSR1,  // user defined
         0,
         0
        };
      int errnum = 0;

      //
      // Generate the mask
      //
      errnum = sigemptyset(&m_workerSigset);
      if (errnum != 0) {
        debugMsg("ExecApplication:initializeWorkerSignalHandling", " sigemptyset returned " << errnum);
        return false;
      }
      for (m_nBlockedSignals = 0;
           m_nBlockedSignals < EXEC_APPLICATION_MAX_N_SIGNALS && signumsToIgnore[m_nBlockedSignals] != 0;
           m_nBlockedSignals++) {
        int sig = signumsToIgnore[m_nBlockedSignals];
        m_blockedSignals[m_nBlockedSignals] = sig; // save to restore it later
        errnum = sigaddset(&m_workerSigset, sig);
        if (errnum != 0) {
          debugMsg("ExecApplication:initializeWorkerSignalHandling", " sigaddset returned " << errnum);
          return false;
        }
      }
      // Set the mask for this thread
      errnum = pthread_sigmask(SIG_BLOCK, &m_workerSigset, &m_restoreWorkerSigset);
      if (errnum != 0) {
        debugMsg("ExecApplication:initializeWorkerSignalHandling", " pthread_sigmask returned " << errnum);
        return false;
      }

      // Add a handler for SIGUSR2 for killing the thread
      struct sigaction sa;
      sigemptyset(&sa.sa_mask); // *** is this enough?? ***
      sa.sa_flags = 0;
      sa.sa_handler = emergencyStop;

      errnum = sigaction(SIGUSR2, &sa, &m_restoreUSR2Handler);
      if (errnum != 0) {
        debugMsg("ExecApplication:initializeWorkerSignalHandling", " sigaction returned " << errnum);
        return errnum;
      }

      debugMsg("ExecApplication:initializeWorkerSignalHandling", " complete");
      return true;
    }

    bool restoreWorkerSignalHandling()
    {
      //
      // Restore old SIGUSR2 handler
      // 
      int errnum = sigaction(SIGUSR2, &m_restoreUSR2Handler, nullptr);
      if (errnum != 0) {
        debugMsg("ExecApplication:restoreWorkerSignalHandling", " sigaction returned " << errnum);
        return errnum;
      }

      //
      // Restore old mask
      //
      errnum = pthread_sigmask(SIG_SETMASK, &m_restoreWorkerSigset, nullptr);
      if (errnum != 0) { 
        debugMsg("ExecApplication:restoreWorkerSignalHandling", " failed; sigprocmask returned " << errnum);
        return false;
      }

      // flag as complete
      m_nBlockedSignals = 0;

      debugMsg("ExecApplication:restoreWorkerSignalHandling", " complete");
      return true;
    }

    // Prevent the main thread from seeing the worker loop kill signal.

    bool initializeMainSignalHandling()
    {
      //
      // Generate the mask
      //
      int errnum = sigemptyset(&m_mainSigset);
      if (errnum != 0) {
        debugMsg("ExecApplication:initializeMainSignalHandling", " sigemptyset returned " << errnum);
        return false;
      }
      errnum = sigaddset(&m_mainSigset, SIGUSR2);
      if (errnum != 0) {
        debugMsg("ExecApplication:initializeMainSignalHandling", " sigaddset returned " << errnum);
        return false;
      }

      // Set the mask for this thread
      errnum = pthread_sigmask(SIG_BLOCK, &m_mainSigset, &m_restoreMainSigset);
      if (errnum != 0) {
        debugMsg("ExecApplication:initializeMainSignalHandling", " pthread_sigmask returned " << errnum);
        return false;
      }

      debugMsg("ExecApplication:initializeMainSignalHandling", " complete");
      return true;
    }

    bool restoreMainSignalHandling()
    {
      //
      // Restore old mask
      //
      int errnum = pthread_sigmask(SIG_SETMASK, &m_restoreMainSigset, nullptr);
      if (errnum != 0) { 
        debugMsg("ExecApplication:restoreMainSignalHandling", " failed; pthread_sigmask returned " << errnum);
        return false;
      }

      debugMsg("ExecApplication:restoreMainSignalHandling", " complete");
      return true;
    }
#endif // PLEXIL_WITH_THREADS

    //
    // Static helper methods
    //
  };

  ExecApplication *makeExecApplication()
  {
    return new ExecApplicationImpl();
  }

}
