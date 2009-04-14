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

#include <vector>
#include "Id.hh"
#include <set>

// Forward reference in global namespace
class TiXmlElement;

namespace PLEXIL
{

  // forward references
  class PlexilExec;
  typedef Id<PlexilExec> PlexilExecId;
  class ExecListener;
  typedef Id<ExecListener> ExecListenerId;
  class ThreadedExternalInterface;
  typedef Id<ThreadedExternalInterface> ThreadedExternalInterfaceId;
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

    inline ExecApplicationId getId()
    {
      return m_id;
    }

    inline PlexilExecId getExec()
    {
      return m_exec;
    }

    inline ThreadedExternalInterfaceId getExternalInterface()
    {
      return m_interface;
    }

    inline ApplicationState getApplicationState()
    {
      return m_state;
    }

    /**
     * @brief Initialize all internal data structures and interfaces.
     * @return true if successful, false otherwise.
     * @note The caller must ensure that all adaptor and listener factories
     *       have been created and registered before this call.
     */
    virtual bool initialize(const TiXmlElement* configXml);

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
     * @brief Constructs interface adaptors from the provided XML.
     * @param configXml The XML element used for interface configuration.
     */
    void constructInterfaces(const TiXmlElement* configXml);

  protected:

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

    PlexilExecId m_exec;
    ThreadedExternalInterfaceId m_interface;
    std::set<InterfaceAdaptorId> m_adaptors;
    std::vector<ExecListenerId> m_listeners;

    ApplicationState m_state;

  };

}

#endif // EXEC_APPLICATION_H
