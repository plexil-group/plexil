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

//
// *** TO DO ***
//  - Deal with ExecListeners as we do adaptors
//  - Find right place to load initial libraries & plans
//

#include "ExecApplication.hh"

#include "AdaptorFactory.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExecListenerFactory.hh"
#include "Expressions.hh"
#include "InterfaceAdaptor.hh"
#include "InterfaceSchema.hh"
#include "PlexilExec.hh"
#include "StateManagerInit.hh"
#include "ThreadedExternalInterface.hh"
#include "tinyxml.h"

namespace PLEXIL
{
  ExecApplication::ExecApplication()
    : m_id(this),
      m_exec(),
      m_interface(),
      m_adaptors(),
      m_state(APP_UNINITED)
  {
  }

  ExecApplication::~ExecApplication()
  {
    // unregister and delete adaptors
    // *** NYI ***

    delete (ThreadedExternalInterface*) m_interface;
    delete (PlexilExec*) m_exec;
  }

  /**
   * @brief Initialize all internal data structures and interfaces.
   * @return true if successful, false otherwise.
   * @note The caller must ensure that all adaptor and listener factories
   *       have been created and registered before this call.
   */
  bool ExecApplication::initialize(const TiXmlElement* configXml)
  {
    if (m_state != APP_UNINITED)
      return false;

    // Perform one-time initializations

    // Load debug configuration from XML
    // *** NYI ***

    // Initialize Exec static data structures
    initializeExpressions();
    initializeStateManagers();

    // Construct interfaces
    constructInterfaces(configXml);

    // Initialize them
    for (std::set<InterfaceAdaptorId>::iterator it = m_adaptors.begin();
         it != m_adaptors.end();
         it++)
      {
        (*it)->initialize();
      }
    
    m_state = APP_INITED;
    return true;
  }

  /**
   * @brief Start all the interfaces prior to execution.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::startInterfaces()
  {
    if (m_state != APP_INITED)
      return false;

    // Start 'em up!
    for (std::set<InterfaceAdaptorId>::iterator it = m_adaptors.begin();
         it != m_adaptors.end();
         it++)
      {
        (*it)->start();
      }

    m_state = APP_INTERFACES_STARTED;
    return true;
  }

  /**
   * @brief Runs the initialized Exec.
   * @return true if successful, false otherwise.
   */

  bool ExecApplication::run()
  {
    if (m_state != APP_INTERFACES_STARTED)
      return false;

    // Start the Exec

    // Start the event listener thread
    m_interface->spawnExecThread();

    
    m_state = APP_RUNNING;
    return true;
  }

  /**
   * @brief Suspends the running Exec.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::suspend()
  {
    if (m_state != APP_RUNNING)
      return false;

    // Suspend the Exec
    // *** NYI ***

    // Suspend the interfaces
    // *** NYI ***
    
    m_state = APP_SUSPENDED;
    return true;
  }

  /**
   * @brief Resumes a suspended Exec.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::resume()
  {
    if (m_state != APP_SUSPENDED)
      return false;

    // Resume the interfaces
    // *** NYI ***

    // Resume the Exec
    // *** NYI ***
    
    m_state = APP_RUNNING;
    return true;
  }

  /**
   * @brief Stops the Exec.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::stop()
  {
    if (m_state != APP_RUNNING
        && m_state != APP_SUSPENDED)
      return false;

    // Stop the Exec
    // *** NYI ***

    // Stop interfaces
    for (std::set<InterfaceAdaptorId>::iterator it = m_adaptors.begin();
         it != m_adaptors.end();
         it++)
      {
        (*it)->stop();
      }
    
    m_state = APP_STOPPED;
    return true;
  }

   
  /**
   * @brief Resets a stopped Exec so that it can be run again.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::reset()
  {
    if (m_state != APP_STOPPED)
      return false;

    // Reset the Exec
    // *** NYI ***
    
    m_state = APP_INITED;
    return true;
  }


  /**
   * @brief Shuts down a stopped Exec.
   * @return true if successful, false otherwise.
   */
  bool ExecApplication::shutdown()
  {
    if (m_state != APP_STOPPED)
      return false;

    // Shut down the Exec
    // *** NYI ***

    // Shut down interfaces
    for (std::set<InterfaceAdaptorId>::iterator it = m_adaptors.begin();
         it != m_adaptors.end();
         it++)
      {
        (*it)->shutdown();
      }
    
    m_state = APP_SHUTDOWN;
    return true;
  }


  /**
   * @brief Constructs interface adaptors from the provided XML.
   * @param configXml The XML element used for interface configuration.
   */
  void ExecApplication::constructInterfaces(const TiXmlElement* configXml)
  {
    debugMsg("ExecApplication:constructInterfaces", " constructing interface adaptors");
    if (configXml != 0)
      {
        const char* elementType = configXml->Value();
        checkError(strcmp(elementType, INTERFACES_TAG) == 0,
                   "constructInterfaces: invalid configuration XML: \n"
                   << *configXml);
        // Walk the children of the configuration XML element
        // and register the adaptor according to the data found there
        TiXmlElement* element = configXml->FirstChildElement();
        while (element != 0)
          {
            const char* elementType = element->Value();
            if (strcmp(elementType, ADAPTOR_TAG) == 0)
              {
                // Get the kind of adaptor to make
                const char* adaptorType = element->Attribute(ADAPTOR_TYPE_ATTR);
                checkError(adaptorType != 0,
                           "constructInterfaces: no " << ADAPTOR_TYPE_ATTR
                           << " attribute for adaptor XML:\n"
                           << *element);
                
                // Construct the adaptor
                InterfaceAdaptorId adaptor = 
                  AdaptorFactory::createInstance(LabelStr(adaptorType),
                                                 element,
                                                 *((AdaptorExecInterface*) m_interface));
                checkError(adaptor.isNoId(),
                           "constructInterfaces: failed to construct adaptor of type "
                           << adaptorType);
                m_adaptors.insert(adaptor);
              }
            else if (strcmp(elementType, LISTENER_TAG) == 0)
              {
                // Get the kind of listener to make
                const char* listenerType = element->Attribute(LISTENER_TYPE_ATTR);
                checkError(listenerType != 0,
                           "constructInterfaces: no " << LISTENER_TYPE_ATTR
                           << " attribute for listener XML:\n"
                           << *element);

                // Construct an ExecListener instance and attach it to the Exec
                ExecListenerId listener = 
                  ExecListenerFactory::createInstance(LabelStr(listenerType),
                                                      element);
                checkError(listener.isNoId(),
                           "constructInterfaces: failed to construct listener of type "
                           << listenerType);
                m_exec->addListener(listener);
                m_listeners.push_back(listener);
              }
            else
              {
                checkError(ALWAYS_FAIL,
                           "constructInterfaces: unrecognized XML element \""
                           << elementType << "\"");
              }

            element = element->NextSiblingElement();
          }

      }
    debugMsg("ExecApplication:constructInterfaces", " done.");
  }

}
