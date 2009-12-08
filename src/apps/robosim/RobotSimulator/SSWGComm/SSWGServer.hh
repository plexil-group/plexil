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

#ifndef SSWG_SERVER_HEADER
#define SSWG_SERVER_HEADER

#include <string>
#include <vector>
#include <map>
#include <pthread.h>
#include "SSWGMessage.hh"

#define SERVER_MAX_CONNECTIONS  256      // max number of clients connected at a time
#define THREAD_FUNC_PTR void* (*)(void*)

class SSWGServer;

typedef struct threadArgs
{
  int socketFd;
  SSWGServer* server;
} ThreadArgs;

typedef std::map<std::string, int> ProcessNameToSocketIdMap; 
enum SSWGCommOutputLevel {SSWGCOMM_LOW=0, SSWGCOMM_MEDIUM, SSWGCOMM_HIGH};


class SSWGServer
{
public:
  
  /*!
    \brief Defualt constructor
  */
  SSWGServer(int echoMessage=0);
  
  /*!
    \brief Defualt destructor
  */	
  ~SSWGServer();
  
  /*!
    \brief The server is initially setup to accept TCP connections at the specified port number. 
    Also spawns a thread to read messages from each client. 
  */
  bool acceptConnections(int portNum);
  
  /*!
    \brief Reads client messages from a socket. If the message is meant for the server ("introduction"), 
    the name of the client is inserted into a map, otherwise, it is forwarded to the appropriate process
    specified in the receiver field. 
  */
  void readMessage(int sFd);
  
private:
  
  /*!
    \brief Forwards the message to the intended receiver whose socket file descriptor is obtained
    from a map.
  */
  void forwardMessage(const std::string& sender, const std::string& receiver, const std::string& message);
  
  /*!
    \brief Given the name of the receiver, this function returns the corresponding socket file descriptor
    stored in the map. Returns -1 if none is found. 
  */
  int determineReceiverSocketFd(const std::string& rcvr);
  int m_SocketFd;
  ThreadArgs args[SERVER_MAX_CONNECTIONS];
  int m_ConCount;
  int m_EchoMessage;
  
  ProcessNameToSocketIdMap m_NameToSockMap;
  pthread_mutex_t m_MapMutex; // make map manipulation thread safe
  int m_sFd;   // socket file descriptor
  SSWGMessage m_Msg;
  pthread_t m_ThreadId;
  std::vector<pthread_t> m_ThreadList;

};

#endif // SSWG_SERVER_HEADER
