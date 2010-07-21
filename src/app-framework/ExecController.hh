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

#ifndef EXEC_CONTROLLER_H
#define EXEC_CONTROLLER_H

#include "Id.hh"

// Forward references in global namespace
class TiXmlElement;

namespace PLEXIL {

  // forward references
  class ExecApplication;

  /**
   * @brief An abstract base class for external interfaces for controlling execution.
   */

  class ExecController {
  public:
	ExecController(ExecApplication & app, const TiXmlElement * configData)
	  : m_application(app), m_xml(configData)
	{}

	virtual ~ExecController() {}

	/**
     * @brief Get the configuration XML for this instance.
     */
    const TiXmlElement* getXml()
    {
      return m_xml;
    }

	/**
	 * @brief Get the application for this instance.
	 */
	ExecApplication& getApplication() {
	  return m_application;
	}

	/**
     * @brief Get the ID for this instance.
     */
    Id<ExecController>& getId()
    {
      return m_id;
    }

	/**
     * @brief Prepare the controller for use.
     */
	virtual bool initialize() = 0;

	/**
     * @brief Terminate operation
     */
	virtual void controllerShutdown() = 0;

  private:

	// deliberately unimplemented
	ExecController();
	ExecController(const ExecController&);
	ExecController& operator=(const ExecController&);
	
	ExecApplication & m_application;
	const TiXmlElement * m_xml;
	Id<ExecController> m_id;
  };

  typedef Id<ExecController> ExecControllerId;

}

#endif // EXEC_CONTROLLER_H
