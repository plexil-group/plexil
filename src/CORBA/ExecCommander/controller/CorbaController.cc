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

#include "CorbaController.hh"

#include "ControllerFactory.hh"

#include "CorbaHelper.hh"
#include "NameServiceHelper.hh"

#include "Debug.hh"
#include "pugixml.hpp"

#include <cstring>
#include <cstdlib>

namespace PLEXIL {

  using namespace gov::nasa::plexil;

  CorbaController::CorbaController(ExecApplication& app, const pugi::xml_node& configXml)
	: POA_gov::nasa::plexil::ExecCommander(),
	  ExecController(app, configXml),
	  m_name(NULL)
  {
  }

  CorbaController::~CorbaController() {
	// shut it down if not already down
	controllerShutdown();
  }

	/**
     * @brief Prepare the controller for use.
     */
  bool CorbaController::initialize() {
	bool result = true;
	// register self with name service if name provided
	if (getXml()) {
	  const char* myName = getXml().attribute(CONTROLLER_NAME_ATTR()).value();
	  if (myName) {
		result = registerWithNameService(myName);
	  }
	}
	return result;
  }

  /**
   * @brief Terminate operation
   */
  void CorbaController::controllerShutdown() {
	// unregister from name service if necessary
	if (m_name != NULL) {
	  unregisterWithNameService(m_name);
	}
	free(m_name);
  }

  //
  // ExecutionControl API
  //
	
  CommandStatus CorbaController::start()
    throw (CORBA::SystemException) {
	if (getApplication().getApplicationState() != ExecApplication::APP_INITED)
	  return WRONG_STATE;
	return (getApplication().run() ? OK : FAILED);
  }

  CommandStatus CorbaController::suspend() 
    throw (CORBA::SystemException) {
	if (getApplication().getApplicationState() != ExecApplication::APP_RUNNING)
	  return WRONG_STATE;
	return (getApplication().suspend() ? OK : FAILED);
  }

  CommandStatus CorbaController::resume() 
    throw (CORBA::SystemException) {
	if (getApplication().getApplicationState() != ExecApplication::APP_READY)
	  return WRONG_STATE;
	return (getApplication().resume() ? OK : FAILED);
  }

  CommandStatus CorbaController::stop() 
    throw (CORBA::SystemException){
	ExecApplication::ApplicationState s = getApplication().getApplicationState();
	if (s != ExecApplication::APP_RUNNING && s != ExecApplication::APP_READY)
	  return WRONG_STATE;
	return (getApplication().stop() ? OK : FAILED);
  }

  CommandStatus CorbaController::reset() 
   throw (CORBA::SystemException) {
	if (getApplication().getApplicationState() != ExecApplication::APP_STOPPED)
	  return WRONG_STATE;
	return (getApplication().reset() ? OK : FAILED);
  }

  CommandStatus CorbaController::shutdown() 
    throw (CORBA::SystemException) {
	if (getApplication().getApplicationState() != ExecApplication::APP_STOPPED)
	  return WRONG_STATE;
	return (getApplication().shutdown() ? OK : FAILED);
  }

  ExecState CorbaController::getExecState()
    throw (CORBA::SystemException) {
	return ExecStateFromAppState(getApplication().getApplicationState());
  }

  //
  // PlanLoader API
  //

  CommandStatus CorbaController::loadPlan(const char* planXml)
   throw (CORBA::SystemException) {
	if (getApplication().getApplicationState() != ExecApplication::APP_RUNNING)
	  return WRONG_STATE;

	// parse XML
	pugi::xml_document xdoc;
	pugi::xml_parse_result result = xdoc.load(planXml);
	if (result.status != pugi::status_ok)
	  return PLAN_PARSE_ERROR;

	// pass it to exec
	return (getApplication().addPlan(&xdoc) ? OK : FAILED);
  }

  CommandStatus CorbaController::loadPlanFile(const char* filename)
    throw (CORBA::SystemException) {
	if (getApplication().getApplicationState() != ExecApplication::APP_RUNNING)
	  return WRONG_STATE;

	// parse XML
	pugi::xml_document xdoc;
	pugi::xml_parse_result result = xdoc.load_file(filename);
	if (result.status != pugi::status_ok) {
	  if (result.status == pugi::status_file_not_found
		  || result.status == pugi::status_io_error)
		return IO_ERROR;
	  else
		return PLAN_PARSE_ERROR;
	}

	// pass it to exec
	return (getApplication().addPlan(&xdoc) ? OK : FAILED);
  }

  CommandStatus CorbaController::loadLibrary(const char* libraryXml)
   throw (CORBA::SystemException) {
	if (getApplication().getApplicationState() != ExecApplication::APP_RUNNING)
	  return WRONG_STATE;

	// parse XML
	pugi::xml_document xdoc;
	pugi::xml_parse_result result = xdoc.load(libraryXml);
	if (result.status != pugi::status_ok)
	  return PLAN_PARSE_ERROR;

	// pass it to exec
	return (getApplication().addLibrary(&xdoc) ? OK : FAILED);
  }

  CommandStatus CorbaController::loadLibraryFile(const char* filename)
    throw (CORBA::SystemException) {
	if (getApplication().getApplicationState() != ExecApplication::APP_RUNNING)
	  return WRONG_STATE;

	// parse XML
	pugi::xml_document xdoc;
	pugi::xml_parse_result result = xdoc.load_file(filename);
	if (result.status != pugi::status_ok) {
	  if (result.status == pugi::status_file_not_found
		  || result.status == pugi::status_io_error)
		return IO_ERROR;
	  else
		return PLAN_PARSE_ERROR;
	}

	// pass it to exec
	return (getApplication().addLibrary(&xdoc) ? OK : FAILED);
  }

  /**
   * @brief Register this object with the CORBA Naming Service.
   * @param contactName The name to register.
   * @return true if successful, false otherwise.
   */
  bool CorbaController::registerWithNameService(const char* contactName) {
    CosNaming::Name myName = NameServiceHelper::parseName(contactName);
    debugMsg("CorbaController:register",
	     " registering Ariel interface as '"
	     << NameServiceHelper::nameToEscapedString(myName) << "'");
    NameServiceHelper & helper = NameServiceHelper::getInstance();
    bool result = helper.nameServiceBind(myName, _this());
	if (result) {
	  m_name = static_cast<char*>(malloc(strlen(contactName) + 1));
	  strcpy(m_name, contactName);
	}
	return result;
  }

  /**
   * @brief Retract registration with the CORBA Naming Service.
   * @param contactName The name that was registered.
   * @return true if successful, false otherwise.
   */
  bool CorbaController::unregisterWithNameService(const char* contactName) {
    CosNaming::Name myName = NameServiceHelper::parseName(contactName);
    debugMsg("CorbaController:unregister",
			 " unbinding '"
			 << NameServiceHelper::nameToEscapedString(myName) << "' from naming service");
    NameServiceHelper & helper = NameServiceHelper::getInstance();
    return helper.nameServiceUnbind(myName);
  }

  // Static utility method
  ExecState CorbaController::ExecStateFromAppState(ExecApplication::ApplicationState as) {
	switch (as) {
	case ExecApplication::APP_INITED:
	  return INITED;
	  break;

	case ExecApplication::APP_READY:
	  return READY;
	  break;

	case ExecApplication::APP_RUNNING:
	  return RUNNING;
	  break;

	case ExecApplication::APP_STOPPED:
	  return STOPPED;
	  break;

	case ExecApplication::APP_SHUTDOWN:
	  return SHUTDOWN;
	  break;

	default:
	  return STATE_UNKNOWN;
	  break;
	}
  }

  void initCorbaController() {
	REGISTER_CONTROLLER(CorbaController, "CorbaController");
  }

}
