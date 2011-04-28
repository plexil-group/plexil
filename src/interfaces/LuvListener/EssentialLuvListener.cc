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

#include "EssentialLuvListener.hh"
#include "LuvFormat.hh"
#include "Expression.hh"
#include "Node.hh"
#include "Debug.hh"

#include "ClientSocket.h"
#include "SocketException.h"

#include <sstream>

namespace PLEXIL {

  //* Default constructor.
  EssentialLuvListener::EssentialLuvListener()
	: m_socket(NULL),
	  m_block(false)
  {
	debugMsg("LuvListener", " constructor");
  }


  //* Destructor.
  EssentialLuvListener::~EssentialLuvListener() 
  {
	if (m_socket != NULL)
	  closeSocket();
	debugMsg("LuvListener", " destructor");
  }


  /**
   * @brief Notify that a node has changed state.
   * @param prevState The old state.
   * @param node The node that has transitioned.
   * @note The current state is accessible via the node.
   */
  void 
  EssentialLuvListener::implementNotifyNodeTransition(NodeState prevState, 
                                                      const NodeId& node) const 
  {
	if (m_socket != NULL) {
	  std::ostringstream s;
	  LuvFormat::formatTransition(s, prevState, node);
	  sendMessage(s.str());
	}
  }


  /**
   * @brief Notify that a plan has been received by the Exec.
   * @param plan The intermediate representation of the plan.
   * @param parent The name of the parent node under which this plan will be inserted.
   */
  void
  EssentialLuvListener::implementNotifyAddPlan(const PlexilNodeId& plan, 
                                               const LabelStr& parent) const 
  {
	if (m_socket != NULL) {
	  sendPlanInfo();

	  std::ostringstream s;
	  LuvFormat::formatPlan(s, plan, parent);
	  sendMessage(s.str());
	}
  }


  /**
   * @brief Notify that a library node has been received by the Exec.
   * @param libNode The intermediate representation of the plan.
   * @note The default method is deprecated and will go away in a future release.
   */
  void
  EssentialLuvListener::implementNotifyAddLibrary(const PlexilNodeId& libNode) const 
  {
	if (m_socket != NULL) {
	  sendPlanInfo();

	  std::ostringstream s;
	  LuvFormat::formatLibrary(s, libNode);
	  sendMessage(s.str());
	}
  }


  /**
   * @brief Notify that a variable assignment has been performed.
   * @param dest The Expression being assigned to.
   * @param destName A string naming the destination.
   * @param value The value (in internal Exec representation) being assigned.
   */
  void 
  EssentialLuvListener::implementNotifyAssignment(const ExpressionId & dest,
                                                  const std::string& destName,
                                                  const double& value) const
  {
	if (m_socket != NULL) {
	  std::ostringstream s;
	  LuvFormat::formatAssignment(s, dest, destName, value);
	  sendMessage(s.str());
	}
  }



  /**
   * @brief Open the socket connection to the viewer.
   * @param port The IP port to which we are connecting.
   * @param host The hostname to which we are connecting.
   * @param ignoreFailure If true, failure is silently ignored.
   * @return False if the connection fails and ignoreFailure is false, true otherwise.
   */
  bool
  EssentialLuvListener::openSocket(uint16_t port, 
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


  //* Close the socket.
  void EssentialLuvListener::closeSocket()
  {
	delete m_socket;
	m_socket = NULL;
  }

  //* Report whether the listener is connected to the viewer.
  bool EssentialLuvListener::isConnected()
  {
	return m_socket != NULL;
  }

  //* Send a plan info header to the viewer.
  void EssentialLuvListener::sendPlanInfo() const
  {
	std::ostringstream s;
	LuvFormat::formatPlanInfo(s, m_block);
	sendMessage(s.str());
  }

  //* Send the message to the viewer.
  void EssentialLuvListener::sendMessage(const std::string& msg) const
  {
	debugMsg("LuvListener:sendMessage", " sending:\n" << msg);
    *m_socket << msg << LUV_END_OF_MESSAGE();
    waitForAck();
  }

  //* Wait for acknowledgement from the viewer.
  void EssentialLuvListener::waitForAck() const
  {
    if (m_block)
      {
        std::string buffer;
		do
          {
            *m_socket >> buffer;
          }
        while (buffer[0] != LUV_END_OF_MESSAGE());
      }
  }

  //* Sets whether the Exec should block until the viewer has acknowledged.
  void EssentialLuvListener::setBlock(bool newValue) 
  {
	m_block = newValue;
  }


  //* Returns the current value of the blocking flag.
  bool EssentialLuvListener::getBlock() 
  {
	return m_block;
  }

}
