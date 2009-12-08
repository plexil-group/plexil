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

#ifndef SSWG_CLIENT_HEADER
#define SSWG_CLIENT_HEADER

#include "SSWGMessage.hh"
#include <string>

class SSWGCallbackHandler;

class SSWGClient
{
public:

	/*!
	 \brief Constructor takes in as argument an abstract base class for callback purposes
	*/
	SSWGClient();
	
	/*!
	 \brief Destructor. Closes a socket if one is still open.
	 */
	~SSWGClient();
	
	/*!
	  \brief Connects the client to the server at the given IP address and port number, The argument \a name
	  is the name of the client. As soon as the connection is establised successfully, a thread is 
	  spawned to listen to any asynchronous messages. 
	  
	  returns a boolean; True if command succeeded  
	*/
	// Not making the callback pointer a const since it would force the virtual function to be const as well.
	bool connectToServer(const std::string& name, const std::string serverIPAdr, int portNum, SSWGCallbackHandler* const callBack);
	
	/*!
	 \brief Sends a message to the intended receiver via the server.
	 
	 returns a boolean. True if command succeeded
	 */
	bool sendMessage(const std::string& msg, const std::string& recvName);
	
	/*!
	 \brief Reads messages if anything has arrived from the appropriate socket.
	 */
	void readMessage();

private:

	/*!
	  \brief Close the socket if still open
	 */
	void closeSocket();
	int m_SocketFd;
  bool m_Connected;
	SSWGCallbackHandler* m_ClientCallbackHandler;
	std::string m_SenderName;
  pthread_t m_ThreadId;
};

#endif // SSWG_CLIENT_HEADER
