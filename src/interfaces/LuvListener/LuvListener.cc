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

#include "LuvListener.hh"
#include "LuvFormat.hh"

#include "Debug.hh"
#include "ExecListenerFactory.hh"
#include "Node.hh"

#include "ClientSocket.h"
#include "SocketException.h"

#include <sstream>

#include "plexil-stdlib.h"
#include "plexil-string.h" // strdup()

namespace PLEXIL
{

  LuvListener::LuvListener(pugi::xml_node const xml)
	: ExecListener(xml),
	  m_socket(NULL),
	  m_host(NULL),
	  m_port(0),
	  m_block(false),
	  m_ignoreConnectFailure(true)
  {
  }

  LuvListener::LuvListener(const std::string& host, 
						   const uint16_t port, 
						   const bool block,
						   const bool ignoreConnectionFailure)
	: ExecListener(),
	  m_socket(NULL),
	  m_host(strdup(host.c_str())),
	  m_port(port),
	  m_block(block),
	  m_ignoreConnectFailure(ignoreConnectionFailure)
  {
	// open the socket
	openSocket(m_port, m_host, m_ignoreConnectFailure);
  }

  LuvListener::~LuvListener()
  {
	closeSocket();
    free((void *) m_host);
  }

  bool LuvListener::initialize()
  {
    // parse XML to find host, port, blocking flag
    pugi::xml_node const xml = this->getXml();
	if (xml.empty()) {
	  // Default host and port values if not set
      if (!*m_host) {
        free((void *) m_host);
        m_host = strdup(LUV_DEFAULT_HOSTNAME());
      }
      if (!m_port) {
        m_port = LUV_DEFAULT_PORT();
      }
	  return true;
    }

	pugi::xml_attribute hostAttr = xml.attribute(LUV_HOSTNAME_ATTR());
    if (hostAttr.empty()) {
	  debugMsg("LuvListener:initialize",
			   " no " << LUV_HOSTNAME_ATTR()
			   << " attribute found, using default host " << LUV_DEFAULT_HOSTNAME());
      m_host = strdup(LUV_DEFAULT_HOSTNAME());
	}
	else {
	  // FIXME: add sanity check?
	  m_host = strdup(hostAttr.value());
	}

	pugi::xml_attribute portAttr = xml.attribute(LUV_PORT_ATTR());
    if (portAttr.empty()) {
	  debugMsg("LuvListener:initialize",
			   " no " << LUV_PORT_ATTR()
			   << " attribute found, using default port " << LUV_DEFAULT_PORT());
	  m_port = LUV_DEFAULT_PORT();
	}
    else {
	  // Should range check here
	  // *** NYI ***
	  m_port = (uint16_t) portAttr.as_uint();
	}

	pugi::xml_attribute blockAttr = xml.attribute(LUV_BLOCKING_ATTR());
    if (blockAttr.empty()) {
	  debugMsg("LuvListener:initialize",
			   " no " << LUV_BLOCKING_ATTR()
			   << " attribute found, using default \"false\"");
	  m_block = false;
	}
    else {
	  m_block = blockAttr.as_bool();
	}

	pugi::xml_attribute ignoreFailAttr = xml.attribute(IGNORE_CONNECT_FAILURE_ATTR());
    if (ignoreFailAttr.empty()) {
	  debugMsg("LuvListener:initialize",
			   " no " << IGNORE_CONNECT_FAILURE_ATTR()
			   << " attribute found, using default \"true\"");
	  m_ignoreConnectFailure = true;
	}
    else {
	  m_ignoreConnectFailure = ignoreFailAttr.as_bool();
	}

    return true; 
  }

  bool LuvListener::start() 
  { 
    return openSocket(m_port, m_host, m_ignoreConnectFailure); 
  }

  bool LuvListener::stop() 
  {
    return true; 
  }

  bool LuvListener::reset() 
  {
	this->closeSocket();
    return true; 
  }

  bool LuvListener::shutdown() 
  { 
	this->closeSocket();
    return true; 
  }

  //
  // Public class member functions
  //

  pugi::xml_document* LuvListener::constructConfigurationXml(const bool& block,
															 const char* hostname,
															 const unsigned int port)
  {
	pugi::xml_document* result = new pugi::xml_document();
	pugi::xml_node toplevel = result->append_child("Listener");
    toplevel.append_attribute("ListenerType").set_value("LuvListener");
    toplevel.append_attribute(LUV_BLOCKING_ATTR()).set_value(block);
    toplevel.append_attribute(LUV_HOSTNAME_ATTR()).set_value(hostname);
    toplevel.append_attribute(LUV_PORT_ATTR()).set_value(port);
    return result;
  }

  void 
  LuvListener::implementNotifyNodeTransition(NodeState prevState, 
											 NodeState newState, 
											 Node *node) const 
  {
    debugMsg("LuvListener:implementNotifyNodeTransition", " for " << node->getNodeId());
	if (m_socket != NULL) {
	  std::ostringstream s;
	  LuvFormat::formatTransition(s, prevState, newState, node);
	  sendMessage(s.str());
	}
  }

  void
  LuvListener::implementNotifyAddPlan(pugi::xml_node const plan) const 
  {
    debugMsg("LuvListener:implementNotifyAddPlan", " entered");
	if (m_socket != NULL) {
      sendPlanInfo();
      std::ostringstream s;
      LuvFormat::formatPlan(s, plan);
      sendMessage(s.str());
	}
  }

  void
  LuvListener::implementNotifyAddLibrary(pugi::xml_node const libNode) const 
  {
	if (m_socket != NULL) {
	  sendPlanInfo();
      std::ostringstream s;
      LuvFormat::formatLibrary(s, libNode);
      sendMessage(s.str());
	}
  }

  void 
  LuvListener::implementNotifyAssignment(Expression const *dest,
										 std::string const &destName,
										 Value const &value) const
  {
	if (m_socket != NULL) {
	  std::ostringstream s;
	  LuvFormat::formatAssignment(s, dest, destName, value);
	  sendMessage(s.str());
	}
  }

  bool
  LuvListener::openSocket(uint16_t port, 
						  const char* host, 
						  bool ignoreFailure) 
  {
	try {
	  debugMsg("LuvListener:start",
			   " opening client socket to host " << host << ", port " << port);
	  m_socket = new ClientSocket(std::string(host), port);
	}    
	catch (const SocketException &e) {
	  debugMsg("LuvListener:start",
			   " socket error: " << e.description());
	  delete m_socket;
	  m_socket = NULL;
	  return ignoreFailure;
	}

	// Success!
    return true; 
  }

  void LuvListener::closeSocket()
  {
	delete m_socket;
	m_socket = NULL;
  }

  bool LuvListener::isConnected()
  {
	return m_socket != NULL;
  }

  void LuvListener::sendPlanInfo() const
  {
	std::ostringstream s;
	LuvFormat::formatPlanInfo(s, m_block);
	sendMessage(s.str());
  }

  void LuvListener::sendMessage(const std::string& msg) const
  {
	debugMsg("LuvListener:sendMessage", " sending:\n" << msg);
    *m_socket << msg << LUV_END_OF_MESSAGE();
    waitForAck();
  }

  void LuvListener::waitForAck() const
  {
    debugMsg("LuvListener:waitForAck", " entered");
    if (m_block) {
        std::string buffer;
		do
          *m_socket >> buffer;
        while (buffer[0] != LUV_END_OF_MESSAGE());
      }
    debugMsg("LuvListener:waitForAck", " exited");
  }
  
  extern "C"
  void initLuvListener()
  {
    REGISTER_EXEC_LISTENER(LuvListener, "LuvListener");
  }

}
