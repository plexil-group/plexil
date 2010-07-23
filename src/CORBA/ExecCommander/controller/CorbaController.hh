/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

#ifndef CORBA_CONTROLLER_HH
#define CORBA_CONTROLLER_HH

#include "ExecCommanderS.h"
#include "ExecApplication.hh"
#include "ExecController.hh"

namespace PLEXIL {

  using gov::nasa::plexil::CommandStatus;
  using gov::nasa::plexil::ExecState;

  class CorbaController : public POA_gov::nasa::plexil::ExecCommander,
						  public ExecController
  {

  public:

	/**
	 * @brief The XML attribute for supplying a name to the CORBA name service.
	 */
	DECLARE_STATIC_CLASS_CONST(char*, CONTROLLER_NAME_ATTR, "ControllerName");

    /**
     * @brief Constructor from configuration XML.
     * @param app Reference to the parent ExecApplication object
     */
	CorbaController(ExecApplication& app, const TiXmlElement* configXml = NULL);

    /**
     * @brief Destructor.
     */
	virtual ~CorbaController();

	/**
     * @brief Prepare the controller for use.
     */
	bool initialize();

	/**
     * @brief Terminate operation
     */
	void controllerShutdown();

    //
    // ExecutionControl API
    //
	
	CommandStatus start();
	CommandStatus suspend();
	CommandStatus resume();
	CommandStatus stop();
	CommandStatus reset();
	CommandStatus shutdown();
	ExecState getExecState();

	//
	// PlanLoader API
	//

	CommandStatus loadPlan(const char* planXml);
	CommandStatus loadPlanFile(const char* filename);
	CommandStatus loadLibrary(const char* libraryXml);
	CommandStatus loadLibraryFile(const char* filename);

	/**
	 * @brief Register this object with the CORBA Naming Service.
	 * @param contactName The name to register.
	 * @return true if successful, false otherwise.
	 */
	bool registerWithNameService(const char* contactName);

	/**
	 * @brief Retract registration with the CORBA Naming Service.
	 * @param contactName The name that was registered.
	 * @return true if successful, false otherwise.
	 */
	bool unregisterWithNameService(const char* contactName);

  private:

	// Deliberately unimplemented
	CorbaController();
	CorbaController(const CorbaController&);
	CorbaController& operator=(const CorbaController&);

	// Utility
	static ExecState ExecStateFromAppState(ExecApplication::ApplicationState as);

	// Instance variables
	char* m_name;

  };

}


#endif // CORBA_CONTROLLER_HH
