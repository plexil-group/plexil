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

#ifndef Essential_Luv_Listener_hh
#define Essential_Luv_Listener_hh

#include "ConstantMacros.hh"
#include "Id.hh"
#include "ExecDefs.hh" // for NodeState

#include <string>

// forward references outside of namespace

class Socket;

namespace PLEXIL {

  // forward references in namespace PLEXIL
  class LabelStr;

  class Expression;
  typedef Id<Expression> ExpressionId;

  class Node;
  typedef Id<Node> NodeId;

  class PlexilNode;
  typedef Id<PlexilNode> PlexilNodeId;

  /**
   * @brief Base class for an Exec listener which supports the Plexil Viewer (nee' LUV).
   * @note Does NOT derive from ExecListener due to a conflict with ManagedExecListener in the app framework
   */
  class EssentialLuvListener
  {
  public:
    //
    // Public class constants
    //

    // Defaults
    DECLARE_STATIC_CLASS_CONST(char*, LUV_DEFAULT_HOSTNAME, "localhost");
    DECLARE_STATIC_CLASS_CONST(unsigned int, LUV_DEFAULT_PORT, 65400);

    // End-of-message marker
    DECLARE_STATIC_CLASS_CONST(char, LUV_END_OF_MESSAGE, (char)4);

	//* Default constructor.
	EssentialLuvListener();

	//* Destructor.
	virtual ~EssentialLuvListener();

    /**
     * @brief Notify that a node has changed state.
     * @param prevState The old state.
     * @param node The node that has transitioned.
     * @note The current state is accessible via the node.
     */
    void implementNotifyNodeTransition(NodeState prevState, 
									   const NodeId& node) const;

    /**
     * @brief Notify that a plan has been received by the Exec.
     * @param plan The intermediate representation of the plan.
     * @param parent The name of the parent node under which this plan will be inserted.
     */
    void implementNotifyAddPlan(const PlexilNodeId& plan, 
								const LabelStr& parent) const;

    /**
     * @brief Notify that a library node has been received by the Exec.
     * @param libNode The intermediate representation of the plan.
     * @note The default method is deprecated and will go away in a future release.
     */
    void implementNotifyAddLibrary(const PlexilNodeId& libNode) const;

    /**
     * @brief Notify that a variable assignment has been performed.
     * @param dest The Expression being assigned to.
     * @param destName A string naming the destination.
     * @param value The value (in internal Exec representation) being assigned.
     */
    void implementNotifyAssignment(const ExpressionId & dest,
								   const std::string& destName,
								   const double& value) const;


	/**
	 * @brief Open the socket connection to the viewer.
	 * @param port The IP port to which we are connecting.
	 * @param host The hostname to which we are connecting.
	 * @param ignoreFailure If true, failure is silently ignored.
	 * @return False if the connection fails and ignoreFailure is false, true otherwise.
	 */
	bool openSocket(uint16_t port, 
					const char* host, 
					bool ignoreFailure);

	//* Close the socket.
	void closeSocket();

	//* Report whether the listener is connected to the viewer.
	bool isConnected();

	//* Sets whether the Exec should block until the viewer has acknowledged.
	void setBlock(bool newValue);

	//* Returns the current value of the blocking flag.
	bool getBlock();

  protected:

	//* Send a plan info header to the viewer.
	void sendPlanInfo() const;

	//* Send the message to the viewer.
	void sendMessage(const std::string& msg) const;

	//* Wait for acknowledgement from the viewer.
	void waitForAck() const;

	//
	// Member variables
	//
    Socket* m_socket;
    bool m_block;

  private:

	// deliberately unimplemented
	EssentialLuvListener(const EssentialLuvListener&);
	EssentialLuvListener& operator=(const EssentialLuvListener&);

  };

}

#endif // Essential_Luv_Listener_hh
