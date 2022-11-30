/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#include "plexil-stdint.h" // uint16_t; includes plexil-config.h

#include "ClientSocket.h"
#include "Debug.hh"
#include "ExecListenerFactory.hh"
#include "Expression.hh"
#include "LuvFormat.hh"
#include "LuvListener.hh"
#include "Node.hh"
#include "SocketException.h"

#include <sstream>
#include <string>

#include <cstdlib>
#include <cstring>  // strdup()

namespace PLEXIL
{

  //
  // Local constants
  //

  // Configuration XML constants
  // Shared between LuvListenerImpl and public helper function
  static constexpr char const LUV_HOSTNAME_ATTR[] = "HostName";
  static constexpr char const LUV_PORT_ATTR[] = "Port";
  static constexpr char const LUV_BLOCKING_ATTR[] = "Blocking";

  static constexpr char const IGNORE_CONNECT_FAILURE_ATTR[] = "IgnoreConnectFailure";

  //! @class LuvListenerImpl
  //! Implements the LuvListener public API.
  class LuvListenerImpl final : public LuvListener
  {
  public:

    /**
     * @brief Constructor from configuration XML.
     */
    LuvListenerImpl(pugi::xml_node const xml)
      : LuvListener(xml), 
        m_socket(nullptr),
        m_host(LUV_DEFAULT_HOSTNAME),
        m_port(LUV_DEFAULT_PORT),
        m_block(false),
        m_ignoreConnectFailure(true)
    {
      // Parse options provided via XML
      char const *hostname = xml.attribute(LUV_HOSTNAME_ATTR).value();
      if (hostname && *hostname)
        m_host = hostname;

      m_port = xml.attribute(LUV_PORT_ATTR).as_uint(m_port);
      m_block = xml.attribute(LUV_BLOCKING_ATTR).as_bool(m_block);
      m_ignoreConnectFailure =
        xml.attribute(IGNORE_CONNECT_FAILURE_ATTR).as_bool(m_ignoreConnectFailure);

      // Report what we found
      debugMsg("LuvListener",
               "  host " << m_host
               << ", port " << m_port
               << ", " << (m_block ? "" : "don't ") << "block, "
               << (m_ignoreConnectFailure ? "" : "don't ") << " ignore connection failure");
    }

    //* Destructor.
    virtual ~LuvListenerImpl()
    {
      closeSocket();
    }

    /**
     * @brief Perform listener-specific startup.
     * @return true if successful, false otherwise.
     */
    virtual bool start() override
    { 
      return openSocket(m_port, m_host.c_str(), m_ignoreConnectFailure); 
    }

    /**
     * @brief Perform listener-specific actions to stop.
     */
    virtual void stop() override
    {
      closeSocket();
    }

    //
    // Public class member functions
    //

    /**
     * @brief Notify that a node has changed state.
     * @param prevState The old state.
     * @param transition Const reference to the transition record.
     */
    virtual void
    implementNotifyNodeTransition(NodeTransition const &trans) const override
    {
      debugMsg("LuvListener:implementNotifyNodeTransition",
               " for " << trans.node->getNodeId());
      if (m_socket) {
        std::ostringstream s;
        LuvFormat::formatTransition(s, trans);
        sendMessage(s.str());
      }
    }

    /**
     * @brief Notify that a plan has been received by the Exec.
     * @param plan The XML representation of the plan.
     */
    virtual void
    implementNotifyAddPlan(pugi::xml_node const plan) const override
    {
      debugMsg("LuvListener:implementNotifyAddPlan", " entered");
      if (m_socket) {
        sendPlanInfo();
        std::ostringstream s;
        LuvFormat::formatPlan(s, plan);
        sendMessage(s.str());
      }
    }

    /**
     * @brief Notify that a library node has been received by the Exec.
     * @param libNode The XML representation of the library node.
     */
    virtual void
    implementNotifyAddLibrary(pugi::xml_node const libNode) const override
    {
      if (m_socket) {
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
    virtual void 
    implementNotifyAssignment(Expression const *dest,
                              std::string const &destName,
                              Value const &value) const override
    {
      if (m_socket) {
        std::ostringstream s;
        LuvFormat::formatAssignment(s, dest, destName, value);
        sendMessage(s.str());
      }
    }

    //* Report whether the listener is connected to the viewer.
    virtual bool isConnected() override
    {
      return m_socket != nullptr;
    }

  private:

    //
    // Implementation details
    //

    /**
     * @brief Open the socket connection to the viewer.
     * @param port The IP port to which we are connecting.
     * @param host The hostname to which we are connecting.
     * @param ignoreFailure If true, failure is silently ignored.
     * @return False if the connection fails and ignoreFailure is false, true otherwise.
     */
    virtual bool openSocket(uint16_t port, 
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
        m_socket = nullptr;
        return ignoreFailure;
      }

      // Success!
      return true; 
    }

    //* Close the socket.
    void closeSocket()
    {
      delete m_socket;
      m_socket = nullptr;
    }

    //* Send a plan info header to the viewer.
    void sendPlanInfo() const
    {
      std::ostringstream s;
      LuvFormat::formatPlanInfo(s, m_block);
      sendMessage(s.str());
    }

    //* Send the message to the viewer.
    void sendMessage(const std::string& msg) const
    {
      debugMsg("LuvListener:sendMessage", " sending:\n" << msg);
      *m_socket << msg << LUV_END_OF_MESSAGE;
      waitForAck();
    }

    //* Wait for acknowledgement from the viewer.
    void waitForAck() const
    {
      debugMsg("LuvListener:waitForAck", " entered");
      if (m_block) {
        std::string buffer;
		do
          *m_socket >> buffer;
        while (buffer[0] != LUV_END_OF_MESSAGE);
      }
      debugMsg("LuvListener:waitForAck", " exited");
    }

    //
    // Constants
    //

	//
	// Member variables
	//
    Socket* m_socket;
    std::string m_host;
	uint16_t m_port;
    bool m_block;
    bool m_ignoreConnectFailure;
  };
  
  //! Construct a LuvListener instance with the desired settings.
  //! @param hostname The host name where the Luv instance is running.
  //! @param port The port number for the Luv instance.
  //! @param block true if the Exec should block until the user steps forward, false otherwise.
  LuvListener *makeLuvListener(const char* hostname,
                               unsigned int port,
                               bool block)
  {
    pugi::xml_document doc;
    pugi::xml_node toplevel = doc.append_child("Listener");
    toplevel.append_attribute("ListenerType").set_value("LuvListener");
    toplevel.append_attribute(LUV_HOSTNAME_ATTR).set_value(hostname);
    toplevel.append_attribute(LUV_PORT_ATTR).set_value(port);
    toplevel.append_attribute(LUV_BLOCKING_ATTR).set_value(block);
    return new LuvListenerImpl(toplevel);
  }

} // namespace PLEXIL

extern "C"
void initLuvListener()
{
  REGISTER_EXEC_LISTENER(PLEXIL::LuvListenerImpl, "LuvListener");
}
