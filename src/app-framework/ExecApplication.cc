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

#ifdef PLEXIL_WITH_THREADS
#include "ThreadSemaphore.hh"
#include <exception>
#include <mutex>
#include <thread>

using ThreadMutexGuard = std::lock_guard<std::mutex>;

#if defined(HAVE_PTHREAD_H)
#include <pthread.h>
#endif

#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h> // pid_t
#endif

#endif // PLEXIL_WITH_THREADS

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

namespace PLEXIL
{

#ifdef PLEXIL_WITH_THREADS
  //
  // Helpers to configure signal handling
  //

  //! Exception thrown when the exec thread fails to stop as requested
  class EmergencyBrake final : public std::exception
  {
  public:
    EmergencyBrake() noexcept = default;
    virtual ~EmergencyBrake() noexcept = default;

    virtual const char *what() const noexcept
    { return "PLEXIL Exec emergency stop"; }
  };

  //! @brief Handler for asynchronous kill of Exec thread
  //! @param signo The signal.
  static void emergencyStop(int signo) 
  {
    debugMsg("ExecApplication:stop", " Received signal " << signo);
    throw EmergencyBrake();
  }

  // Prevent the worker thread from seeing the signals listed below.
  // Applications are free to deal with them in other ways.
  static bool initializeWorkerSignalHandling()
  {
    static int signumsToIgnore[] =
      {
        SIGINT,   // user interrupt (i.e. control-C)
        SIGHUP,   // hangup
        SIGQUIT,  // quit
        SIGTERM,  // kill
        SIGALRM,  // timer interrupt, used by (e.g.) ItimerTimebase
        SIGUSR1,  // user defined
        0,
      };
    int errnum = 0;

    // Initialize the mask
    sigset_t workerSigset;
    errnum = sigemptyset(&workerSigset);
    if (errnum != 0) {
      debugMsg("initializeWorkerSignalHandling", " sigemptyset returned " << errnum);
      return false;
    }
    int *sigptr = signumsToIgnore;
    while (*sigptr) {
      int sig = *sigptr++;
      errnum = sigaddset(&workerSigset, sig);
      if (errnum != 0) {
        debugMsg("initializeWorkerSignalHandling",
                 " sigaddset returned " << errnum);
        return false;
      }
    }

    // Set the mask for this thread
    errnum = pthread_sigmask(SIG_BLOCK, &workerSigset, nullptr);
    if (errnum != 0) {
      debugMsg("initializeWorkerSignalHandling", " pthread_sigmask returned " << errnum);
      return false;
    }

    // Add a handler for SIGUSR2 for killing the thread
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = emergencyStop;
    errnum = sigaction(SIGUSR2, &sa, nullptr);
    if (errnum != 0) {
      debugMsg("initializeWorkerSignalHandling", " sigaction returned " << errnum);
      return errnum;
    }

    debugMsg("initializeWorkerSignalHandling", " complete");
    return true;
  }

  // Prevent the main thread from seeing the worker loop kill signal.
  bool initializeMainSignalHandling()
  {
    // Mask off SIGUSR2
    sigset_t mainSigset;
    int errnum = sigemptyset(&mainSigset);
    if (errnum != 0) {
      debugMsg("initializeMainSignalHandling", " sigemptyset returned " << errnum);
      return false;
    }
    errnum = sigaddset(&mainSigset, SIGUSR2);
    if (errnum != 0) {
      debugMsg("initializeMainSignalHandling", " sigaddset returned " << errnum);
      return false;
    }
    // Set the mask for this thread
    errnum = pthread_sigmask(SIG_BLOCK, &mainSigset, nullptr); // sigprocmask()?
    if (errnum != 0) {
      debugMsg("initializeMainSignalHandling", " pthread_sigmask returned " << errnum);
      return false;
    }

    debugMsg("initializeMainSignalHandling", " complete");
    return true;
  }
#endif // PLEXIL_WITH_THREADS

  //! @class ExecApplicationImpl
  //! Implements the ExecApplication API
  class ExecApplicationImpl final : public ExecApplication
  {
  private:
    //
    // Typedefs
    //
    using AdapterConfigurationPtr = std::unique_ptr<AdapterConfiguration>;
    using ExecListenerHubPtr  = std::unique_ptr<ExecListenerHub>;
    using InterfaceManagerPtr = std::unique_ptr<InterfaceManager>;
    using PlexilExecPtr = std::unique_ptr<PlexilExec>;

    //
    // Member variables
    //

#ifdef PLEXIL_WITH_THREADS
    //
    // Synchronization and mutual exclusion
    //

    //! Thread in which the Exec runs
    std::thread m_workerThread;

    //! Serialize access to exec state to guarantee in-order processing of events

    //! This mutex must be held by the working thread from the start
    //! of input queue processing until the PlexilExec has reached
    //! quiescence. It can be held briefly by other threads for
    //! queries about the exec state.
    std::mutex m_execMutex;

    // Semaphore for notifying the Exec of external events
    ThreadSemaphore m_sem;

    // Semaphore for notifyAndWaitForCompletion()
    ThreadSemaphore m_markSem;

    // Semaphore for notifying external threads that the application is shut down
    ThreadSemaphore m_shutdownSem;

    // Semaphore for waiting on all plans to finish
    ThreadSemaphore m_allFinishedSem;

    //! Last mark seen
    unsigned int m_lastMark;
#endif 

    //! Interfacing database and dispatcher
    AdapterConfigurationPtr m_configuration;

    //! Interface manager
    InterfaceManagerPtr m_manager;

    //! Exec
    PlexilExecPtr m_exec;

    //! Exec listener hub
    ExecListenerHubPtr m_listener;

    // Flag to determine whether exec should run conservatively
    bool m_runExecInBkgndOnly;

    // True if application initialized, false otherwise.
    bool m_initialized;

    // True if interfaces have been started, false otherwise.
    bool m_interfacesStarted;

    //! True if at least one plan has been loaded.
    bool m_planLoaded;

    // Flag for halting the Exec thread
    bool m_stop;

    // Flag for suspend/resume
    bool m_suspended;

  public:

    //! Constructor.
    ExecApplicationImpl()
      : ExecApplication(),
#ifdef PLEXIL_WITH_THREADS
        m_workerThread(),
        m_execMutex(),
        m_sem(),
        m_markSem(),
        m_shutdownSem(),
        m_allFinishedSem(),
        m_lastMark(0),
#endif
        m_configuration(makeAdapterConfiguration()),
        m_manager(new InterfaceManager(this, m_configuration.get())),
        m_exec(makePlexilExec()),
        m_listener(new ExecListenerHub()),
        m_runExecInBkgndOnly(true),
        m_initialized(false),
        m_interfacesStarted(false),
        m_planLoaded(false),
        m_stop(false),
        m_suspended(false)
    {
      // Set globals that other pieces rely on
      // Required by Exec core
      g_dispatcher = static_cast<Dispatcher *>(m_configuration.get());
      g_exec = m_exec.get();

      // Link the Exec to the AdapterConfiguration
      m_exec->setDispatcher(m_configuration.get());

      // Link the Exec to the listener hub
      m_exec->setExecListener(m_listener.get());
    }

    virtual ~ExecApplicationImpl()
    {
      // Reset global pointers to objects we own before they are deleted
      g_dispatcher = nullptr;
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

    virtual ExecListenerHub *listenerHub() override
    {
      return m_listener.get();
    }

    virtual PlexilExec *exec() override
    {
      return m_exec.get();
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

      if (m_initialized) {
        debugMsg("ExecApplication:initialize",
                 " application already initialized, ignoring");
        return true;
      }

      // Perform one-time initializations

      // Load debug configuration from XML
      // *** NYI ***

      // Construct interfaces
      if (!m_configuration->constructInterfaces(configXml, *m_manager, *m_listener)) {
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

      // Initialize exec listeners
      if (!m_listener->initialize()) {
        debugMsg("ExecApplication:initialize",
                 " initialization of Exec listeners failed");
        return false;
      }

      // Initialize interface manager
      if (!m_manager->initialize()) {
        debugMsg("ExecApplication:initialize",
                 " initialization of interface manager failed");
        return false;
      }

      // Set the application state and return
      m_initialized = true;
      return true;
    }

    //! Start all the interfaces prior to execution.
    //! @return true if successful, false otherwise.
    virtual bool startInterfaces() override
    {
      if (!m_initialized) {
        warn("Error: startInterfaces() called before initialize()");
        return false;
      }

      if (m_interfacesStarted) {
        debugMsg("ExecApplication:startInterfaces",
                 " interfacese already started, ignoring");
        return true;
      }

      // Start 'em up!
      if (!m_configuration->start()) {
        warn("ExecApplication: Error: failed to start interfaces");
        return false;
      }

      if (!m_listener->start()) {
        warn("ExecApplication: Error: failed to start Exec listeners");
        return false;
      }

      m_interfacesStarted = true;
      return true;
    }

    //! Step the Exec once.
    //! @return true if Exec needs to be stepped again, false otherwise.
    virtual bool step() override
    {
      assertTrue_2(m_interfacesStarted,
                   "ExecApplication: Fatal error: step() called before startInterfaces()");
#ifdef PLEXIL_WITH_THREADS
      unsigned int oldMark = m_lastMark;
#endif
      bool needsStep = false;
      bool allFinished = false;
      {
#ifdef PLEXIL_WITH_THREADS
        ThreadMutexGuard guard(m_execMutex);
#endif
        debugMsg("ExecApplication:step", " Processing queue");
        m_manager->processQueue();
        debugMsg("ExecApplication:step", " Stepping exec");
        m_exec->step(StateCache::queryTime());
        // Take care of any plans which have finished
        m_exec->deleteFinishedPlans();
        allFinished = m_exec->allPlansFinished();
        needsStep = m_exec->needsStep();
      }
#ifdef PLEXIL_WITH_THREADS
      if (m_planLoaded && allFinished) {
        debugMsg("ExecApplication:step", " All plans finished ");
        m_allFinishedSem.post();
      }
      if (m_lastMark > oldMark) {
        debugMsg("ExecApplication:step", " queue mark(s) processed");
        m_markSem.post();
      }
#endif
      return needsStep;
    }

    //! Run the exec until the queue is empty and the plan state is quiescent.
    //! @note Acquires m_execMutex and holds until done.  
    virtual void runExec() override
    {
      assertTrue_2(m_interfacesStarted,
                   "ExecApplication: Fatal error: step() called before startInterfaces()");

#ifdef PLEXIL_WITH_THREADS
      unsigned int oldMark = m_lastMark;
#endif
      bool allFinished = false;
      {
#ifdef PLEXIL_WITH_THREADS
        ThreadMutexGuard guard(m_execMutex);
#endif
        debugMsg("ExecApplication:runExec", " Processing queue");
        m_manager->processQueue();
        do {
          do {
            debugMsg("ExecApplication:runExec", " Stepping exec");
            m_exec->step(StateCache::queryTime());
          } while (m_exec->needsStep());
          debugMsg("ExecApplication:runExec", " Processing queue");
        } while (m_manager->processQueue());

        // Clean up
        m_exec->deleteFinishedPlans();
        allFinished = m_exec->allPlansFinished();
        debugMsg("ExecApplication:runExec", " Queue empty and exec quiescent");
      }
#ifdef PLEXIL_WITH_THREADS
      if (m_planLoaded && allFinished) {
        debugMsg("ExecApplication:runExec", " All plans finished ");
        m_allFinishedSem.post();
      }
      if (m_lastMark > oldMark) {
        debugMsg("ExecApplication:runExec", " queue mark(s) processed");
        m_markSem.post();
      }
#endif
    }

    //
    // Running in a threaded environment
    //

    //! Runs the initialized Exec.
    //! @return true if successful, false otherwise.
    virtual bool run() override
    {
#ifdef PLEXIL_WITH_THREADS
      assertTrue_2(m_interfacesStarted,
                   "ExecApplication: Fatal error: run() called before startInterfaces()");

      // Set up signal handling in main thread
      if (!initializeMainSignalHandling()) {
        warn("ExecApplication: failed to initialize main thread signal handling");
        return false;
      }

      // Clear suspended flag just in case
      m_suspended = false;

      // Start the event listener thread
      return spawnWorkerThread();
#else // !defined(PLEXIL_WITH_THREADS)
      warn("ExecApplication: Threads not enabled");
      return false;
#endif // PLEXIL_WITH_THREADS
    }

    //! Suspends the running Exec.
    //! @return true if successful, false otherwise.
    virtual bool suspend() override
    {
      if (!m_interfacesStarted)
        return false; // couldn't possibly be running
      if (m_suspended)
        return true;  // already suspended, ignore

      // Suspend the Exec 
      m_suspended = true;
      // *** NYI: wait here til current step completes ***
      return true;
    }

    //! Query whether the Exec has been suspended. 
    //! @return True if suspended, false otherwise.
    virtual bool isSuspended() const override
    {
      return m_suspended;
    }

    //! Resumes a suspended Exec.
    //! @return true if successful, false otherwise.
    virtual bool resume() override
    {
      // Can only resume if ready and suspended
      if (!m_interfacesStarted)
        return false; // couldn't possibly be running
      if (!m_suspended)
        return true;  // silently "fail"

      // Resume the Exec
      m_suspended = false;
      notifyExec();
      return true;
    }

    //! Stops the Exec and its interfaces.
    virtual void stop() override
    {
      if (!m_interfacesStarted)
        return; // nothing to stop

      // If sleeping, wake up
      if (m_suspended) {
        m_suspended = false;
        notifyExec();
      }

#ifdef PLEXIL_WITH_THREADS
      // Stop the Exec
      if (m_workerThread.joinable()) {
        debugMsg("ExecApplication:stop", " Halting top level thread");
        m_stop = true;
        notifyExec();
        int status = m_sem.post();
        if (status) {
          warn("ExecApplication: semaphore post failed, status = " << status);
          return;
        }
        sleep(1);

        if (m_stop) {
          // Exec thread failed to acknowledge stop - resort to stronger measures
          status = kill(getpid(), SIGUSR2);
          if (status) {
            warn("ExecApplication: kill failed, status = " << status);
            return; // not much else we can do
          }
          sleep(1);
        }

        m_workerThread.join();
        debugMsg("ExecApplication:stop", " Worker thread stopped");
      }
#endif // PLEXIL_WITH_THREADS

      // Stop interfaces
      m_configuration->stop();
      m_listener->stop();

      m_interfacesStarted = false;
      m_initialized = false;

#ifdef PLEXIL_WITH_THREADS
      // Tell anyone waiting that we're finished
      m_shutdownSem.post();
#endif // PLEXIL_WITH_THREADS
    }

    /**
     * @brief Whatever state the application may be in, bring it down in a controlled fashion.
     */
    virtual void terminate() override
    {
      std::cout << "Terminating PLEXIL Exec application" << std::endl;
      if (m_initialized && m_interfacesStarted) {
        if (m_suspended) {
          m_suspended = false;
          notifyExec();
        }
        stop();
      }
      std::cout << "PLEXIL Exec terminated" << std::endl;
    }

    //! Notify the Exec thread that it should check the queue and run
    //! one cycle.
    virtual void notifyExec() override
    {
#ifdef PLEXIL_WITH_THREADS
      // FIXME: Don't run if called from thread holding mutex - undefined behavior
      // How to fix? Call command handlers from outside the mutex hold.
      if (!m_runExecInBkgndOnly && m_execMutex.try_lock()) {
        // Exec is idle, so run it
        debugMsg("ExecApplication:notify", " exec was idle, stepping it");
        this->runExec();
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
#endif
    }

    //! Run the exec and wait until all events in the queue have been processed. 
    virtual void notifyAndWaitForCompletion() override
    {
#ifdef PLEXIL_WITH_THREADS
      debugMsg("ExecApplication:notifyAndWait", " received external event");
      unsigned int sequence = m_manager->markQueue();
      notifyExec();
      debugMsg("ExecApplication:notifyAndWait", " waiting for mark #" << sequence);
      do {
        m_markSem.wait();
        m_markSem.post(); // in case it's not our mark and we got there first
      } while (m_lastMark < sequence);
      debugMsg("ExecApplication:notifyAndWait", " proceeding");
#else // !defined(PLEXIL_WITH_THREADS)
      warn("notifyAndWaitForCompletion: Threads not enabled");
#endif // PLEXIL_WITH_THREADS
    }

    /**
     * @brief Suspend the current thread until the plan finishes executing.
     */
    virtual void waitForPlanFinished() override
    {
#ifdef PLEXIL_WITH_THREADS
      debugMsg("ExecApplication:waitForPlanFinished", " waiting");
      int waitStatus = m_allFinishedSem.wait();
      checkError(!waitStatus,
                 "waitForPlanFinished: semaphore wait returned error " << waitStatus);
      debugMsg("ExecApplication:waitForPlanFinished", " proceeding");
#else // !defined(PLEXIL_WITH_THREADS)
      warn("waitForPlanFinished: Threads not enabled");
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
      debugMsg("ExecApplication:waitForShutdown", " waiting");
      int waitStatus = m_shutdownSem.wait();
      checkError(!waitStatus,
                 "waitForShutdown: semaphore wait returned error " << waitStatus);
      debugMsg("ExecApplication:waitForShutdown", " proceeding");
      m_shutdownSem.post(); // pass it on to the next, if any
#else // !defined(PLEXIL_WITH_THREADS)
      warn("waitForShutdown: Threads not enabled");
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
      bool result = false;
      // Delegate to InterfaceManager
      try {
        result = m_manager->handleLoadLibrary(name);
      }
      catch (const ParserException& e) {
        warn("loadLibrary: Error:\n" << e.what());
        return false;
      }

      debugMsg("ExecApplication:loadLibrary",
               " Library " << name << (result ? " loaded." : " not found."));
      return result;
    }

    /**
     * @brief Add a plan as an XML document.
     * @return true if successful, false otherwise.
     */
    virtual bool addPlan(pugi::xml_document* planXml) override
    {
#ifdef PLEXIL_WITH_THREADS
      if (!m_workerThread.joinable()) {
        warn("Error: addPlan failed; worker thread not running");
        return false;
      }
#else
      if (!m_interfacesStarted) {
        warn("Error: addPlan failed; interfaces not started");
        return false;
      }
#endif

      // Delegate to InterfaceManager
      try {
        m_manager->handleAddPlan(planXml->document_element());
        debugMsg("ExecApplication:addPlan", " successful");
        m_planLoaded = true;
        return true;
      }
      catch (const ParserException& e) {
        warn("addPlan: Plan parser error: \n" << e.what());
        return false;
      }
    }

    //! Notify the application that a queue mark was processed.
    virtual void markProcessed(unsigned int sequence) override
    {
#ifdef PLEXIL_WITH_THREADS
      debugMsg("ExecApplication:markProcessed", " sequence #" << sequence);
      m_lastMark = sequence;
#endif
    }

  private:

    //
    // Implementation methods
    //

#ifdef PLEXIL_WITH_THREADS
    /**
     * @brief Spawns the worker thread which runs the exec's top level loop.
     * @return true if successful, false otherwise.
     */
    bool spawnWorkerThread()
    {
      debugMsg("ExecApplication:run", " Spawning top level thread");
      m_workerThread = std::thread([this]() -> void {this->worker();});
      debugMsg("ExecApplication:run", " Top level thread running");
      return m_workerThread.joinable();
    }

    //! The top level of the worker thread.
    void worker()
    {
      debugMsg("ExecApplication:worker", " started");

      // set up signal handling environment for this thread
      if (!initializeWorkerSignalHandling()) {
        warn("ExecApplication: Worker signal handling initialization failed.");
        return;
      }

      try {
        // must step exec once to initialize time and
        // give any preloaded plans a chance to start
        step();
        debugMsg("ExecApplication:worker", " Initial step complete");

        while (waitForExternalEvent()) {
          if (m_stop) {
            debugMsg("ExecApplication:worker", " Received stop request");
            m_stop = false; // acknowledge stop request
            break;
          }
          runExec();
        }
      } catch (EmergencyBrake const &b) {
        debugMsg("ExecApplication:worker", " exiting on signal");
      } catch (std::exception const &e) {
        debugMsg("ExecApplication:worker",
                 " exiting due to exception:\n " << e.what());
      }

      debugMsg("ExecApplication:worker", " finished.");
    }

    //! Suspends the calling thread until another thread has placed a
    //! call to notifyExec(). Can return immediately if the call to
    //! wait() returns an error.
    //! @return true if resumed normally, false if wait resulted in an error.
    //! @note Can wait here indefinitely while the application is suspended.
    //! @note Stop overrides suspend.
    bool waitForExternalEvent()
    {
      debugMsg("ExecApplication:wait", " waiting for external event");
      do {
        if (m_sem.wait())
          return false;
        if (!m_stop && m_suspended) {
          debugMsg("ExecApplication:wait",
                   " Application is suspended, ignoring external event");
        }
      } while (!m_stop && m_suspended);

      debugMsg("ExecApplication:wait", " processing external event");
      return true;
    }
#endif // PLEXIL_WITH_THREADS

  }; // class ExecApplicationImpl

  ExecApplication *makeExecApplication()
  {
    return new ExecApplicationImpl();
  }

}
