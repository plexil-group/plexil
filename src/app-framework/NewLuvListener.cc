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

#include "NewLuvListener.hh"
#include "Debug.hh"
#include "InterfaceManagerBase.hh"

#include "ClientSocket.h"
#include "SocketException.h"

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

#include <sstream>

namespace PLEXIL 
{

  // create a listener given configuration XML

  NewLuvListener::NewLuvListener(const TiXmlElement* xml,
								 InterfaceManagerBase& mgr)
    : EssentialLuvListener(),
      ManagedExecListener(xml, mgr),
	  m_hostname(NULL),
      m_port(0),
      m_ignoreConnectFailure(true)
  {
  }

  NewLuvListener::~NewLuvListener()
  {
	this->closeSocket();
  }

  /**
   * @brief Perform listener-specific initialization.
   * @return true if successful, false otherwise.
   */
  bool NewLuvListener::initialize()
  {
    // parse XML to find host, port, blocking flag
    const TiXmlElement* xml = this->getXml();
    m_hostname = xml->Attribute(LUV_HOSTNAME_ATTR());
    if (m_hostname == NULL)
      {
        debugMsg("LuvListener:initialize",
                 " no " << LUV_HOSTNAME_ATTR()
                 << " attribute found, using default host " << LUV_DEFAULT_HOSTNAME());
        m_hostname = LUV_DEFAULT_HOSTNAME();
      }
    int rawPort = 0;
    const char* dummy = xml->Attribute(LUV_PORT_ATTR(), &rawPort);
    if (dummy == NULL)
      {
        debugMsg("LuvListener:initialize",
                 " no " << LUV_PORT_ATTR()
                 << " attribute found, using default port " << LUV_DEFAULT_PORT());
        m_port = LUV_DEFAULT_PORT();
      }
    else
      {
        // Should range check here
        // *** NYI ***
        m_port = (uint16_t) rawPort;
      }

    dummy = xml->Attribute(LUV_BLOCKING_ATTR());
    if (dummy == NULL)
      {
        debugMsg("LuvListener:initialize",
                 " no " << LUV_BLOCKING_ATTR()
                 << " attribute found, using default \"false\"");
		this->setBlock(false);
      }
    else if (strcmp(dummy, TRUE_STR()) == 0)
      {
        this->setBlock(true);
      }
    else
      {
        this->setBlock(false);
      }

    dummy = xml->Attribute(IGNORE_CONNECT_FAILURE_ATTR());
    if (dummy == NULL)
      {
        debugMsg("LuvListener:initialize",
                 " no " << IGNORE_CONNECT_FAILURE_ATTR()
                 << " attribute found, using default \"true\"");
        m_ignoreConnectFailure = true;
      }
    else if (strcmp(dummy, FALSE_STR()) == 0)
      {
        m_ignoreConnectFailure = false;
      }
    else
      {
        m_ignoreConnectFailure = true;
      }

    return true; 
  }

  /**
   * @brief Perform listener-specific startup.
   * @return true if successful, false otherwise.
   */
  bool NewLuvListener::start() 
  { 
    return this->openSocket(m_port, m_hostname, m_ignoreConnectFailure); 
  }

  /**
   * @brief Perform listener-specific actions to stop.
   * @return true if successful, false otherwise.
   */
  bool NewLuvListener::stop() 
  {
    return true; 
  }

  /**
   * @brief Perform listener-specific actions to reset to initialized state.
   * @return true if successful, false otherwise.
   */
  bool NewLuvListener::reset() 
  {
	this->closeSocket();
    return true; 
  }

  /**
   * @brief Perform listener-specific actions to shut down.
   * @return true if successful, false otherwise.
   */
  bool NewLuvListener::shutdown() 
  { 
	this->closeSocket();
    return true; 
  }

  //
  // Public class member functions
  //

  /**
   * @brief Construct the appropriate configuration XML for the desired settings.
   * @param block true if the Exec should block until the user steps forward, false otherwise.
   * @param hostname The host name where the Luv instance is running.
   * @param port The port number for the Luv instance.
   */
  TiXmlElement* NewLuvListener::constructConfigurationXml(const bool& block,
							  const char* hostname,
							  const unsigned int port)
  {
    TiXmlElement* result = new TiXmlElement("Listener");
    result->SetAttribute("ListenerType", "LuvListener");
    result->SetAttribute(LUV_BLOCKING_ATTR(),
			block ? "true" : "false");
    result->SetAttribute(LUV_HOSTNAME_ATTR(), hostname);
    result->SetAttribute(LUV_PORT_ATTR(), port);
    return result;
  }

}
