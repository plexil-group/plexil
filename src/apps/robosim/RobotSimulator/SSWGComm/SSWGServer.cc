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

#include "SSWGServer.hh"
#include "ThreadSpawn.hh"

#ifdef UNIX
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <unistd.h>
#else
  #include "vxWorks.h"
  #include "sockLib.h"
  #include "inetLib.h"
  #include "hostLib.h"
#endif

#include <iostream>
#include <strings.h>

// C wrapper to call a C++ method. Used while spawning a thread. 
void spawnThreadForEachClient(void* args)
{	
  ThreadArgs* tArgs = static_cast<ThreadArgs*>(args);	
  int sFd = tArgs->socketFd;
  SSWGServer* server = tArgs->server; 
  server->readMessage(sFd);
}

SSWGServer::SSWGServer(int echoMessage) : m_SocketFd(-1), m_ConCount(0),
                                          m_EchoMessage(echoMessage)
{
}

SSWGServer::~SSWGServer()
{
  pthread_mutex_lock(&m_MapMutex);
  ProcessNameToSocketIdMap::const_iterator iter;
  
  for (iter = m_NameToSockMap.begin(); iter != m_NameToSockMap.end(); ++iter)
    {
      std::cout << "Closing the socket FD for the client: " << iter->first << std::endl;
      close(iter->second);
    }
  pthread_mutex_unlock(&m_MapMutex);
  
  pthread_mutex_destroy(&m_MapMutex);
  std::cout << "Closing the server socket FD: " << m_sFd << std::endl;
  close (m_sFd);

  for(std::vector<pthread_t>::const_iterator iter = m_ThreadList.begin();
      iter != m_ThreadList.end(); ++iter)
    {
      std::cout << "Cancelling thread: " << *iter << "...";
      pthread_cancel(*iter);
      pthread_join(*iter, NULL);
      std::cout << "done" << std::endl;
    }
}

void SSWGServer::readMessage(int sFd)
{
  int nRead;
  
  SSWGMessage msg;
  std::string sender;
  std::string receiver;
  std::string message;
  
  char buff[MAX_READ_BUFFER_SIZE+1]; // the extra space is for the NULL termination character
  char buffTmp[MAX_READ_BUFFER_SIZE+1]; // needed to copy over the remainder of the chunck read 
  int i=0;
  char* crlfp;
  bool readMore;
  unsigned int remainLen;
  char firstChar[2];

  // Extract the first character of the start tag. If it is present in the chunck after the 
  // message end tag, it is possible that a piece of the message start tag is present.

  firstChar[0] = message_start_tag[0];
  firstChar[1] = '\0';
        
  // read client request. Reads all received chunks until the CRLF characters are
  // found. Will assemble the buffer till that point.  The remainder will be discarded if we
  // don't find the first character of start tag. Otherwise, if we find the first character
  // of the start tag, the chunk will be preserved. This is case the start tag got cut out.

  while ((nRead = read (sFd, buff+i, MAX_READ_BUFFER_SIZE-i)) > 0)
    {
      //      std::cout << "Number of bytes read: " << nRead << std::endl;
      i += nRead;
      buff[i] = '\0';
      //std::cout << "Buffer read: " << buff << std::endl;
      readMore = false;
      do
        {
          // IF the buffer contains the end tag process it. otherwise read more
          if ((crlfp = strstr(buff, message_end_tag)))
            {
              *crlfp = '\0';
              // extract the sender, receiver and message fields from the char 
              // string just read.

              msg.extractSender(buff, sender);
              msg.extractReceiver(buff, receiver);
              msg.extractMessage(buff, message);
              
              if (m_EchoMessage >= SSWGCOMM_MEDIUM)
                std::cout << "\nServer received a message at socket FD: " << sFd << ", From: " << sender
                          << ", To: " << receiver << std::endl;
              if (m_EchoMessage >= SSWGCOMM_HIGH)
                std::cout << "Msg: " << message << std::endl;
              
              if (receiver == "SSWGServer")
                {
                  // Recursive mutex is not necessary since the same thread cannot call lock twice
                  // since they appear in "if" and "else" conditions.
                  std::cout << "\nServer is registering the client: " << sender
                            << std::endl;
                  pthread_mutex_lock(&m_MapMutex);
                  if (m_NameToSockMap.find(sender) == m_NameToSockMap.end())
                    m_NameToSockMap[sender] = sFd;
                  else
                    std::cout << "The client has already been registered. Ignoring message" << std::endl; 
                  pthread_mutex_unlock(&m_MapMutex);
                }
              else
                {
                  this->forwardMessage(sender, receiver, message);
                }

              // remember that the \r character gets replaces by \0 after substring match is
              // found
              remainLen = i - (strlen(buff) + strlen(message_end_tag));

	      //std::cout << "Total buffer read: " << i << ", size of message: " << strlen(buff)
              //          << ", remaining length: " << remainLen << std::endl;
              // Copy over remaining chunck
              // A ring buffer would have eliminated the need for buffTmp
              strncpy(buffTmp, crlfp+2, remainLen+1); // crlfp+2 will be within the bounds of buff
                                                      // since we have on extra space for \0 at the end
              strncpy(buff, buffTmp, remainLen+1);
              i = remainLen;

              // If the first character of the start tag is absent in the remaining chunck
              // discard the remainder. If the first character is present, it is possible that
              // the start tag got clipped out. So retain the chunk by not resetting i.

              if (strstr(buff, firstChar) == NULL)
                {
                  // discard the chunck since it does not contain a message start tag
                  i = 0;
                  readMore = true;
		  //std::cout << "Discard the remaining chunck" << std::endl;
                }
              else
                {
		  //std::cout << "Keeping the reminaing chunck" << std::endl;
		  //std::cout << "buff: " << buff << "\nbuff length: " << strlen(buff) << ", i: " << i << std::endl;
                }
            }
          else
            readMore = true;
          
	  //std::cout << "readMore: " << readMore << std::endl;
        } while (!readMore);
    }
  // error from read()
  std::cout << "Error while reading message from " << sender << ". Number of chars read: " << nRead << std::endl;
  pthread_mutex_lock(&m_MapMutex);
  if (m_NameToSockMap.find(sender) != m_NameToSockMap.end())
    {
      std::cout << "Removing client from map" << std::endl;
      m_NameToSockMap.erase(m_NameToSockMap.find(sender));
    }
  pthread_mutex_unlock(&m_MapMutex);
  std::cout << "Closing the file descriptor." << std::endl;
  close (sFd);
  //  std::cout << "Killing the thread" << std::endl;
  //  pthread_exit(NULL);
}

void SSWGServer::forwardMessage(const std::string& sender, const std::string& receiver, const std::string& message)
{
  pthread_mutex_lock(&m_MapMutex);
  int sFd = this->determineReceiverSocketFd(receiver);
  
  m_Msg.createForwardedMessage(sender, receiver, message);
  if (sFd != -1)
    {
      if (write (sFd, m_Msg.finalMessage, strlen(m_Msg.finalMessage)) == -1)
        {
          std::cerr << "Error occurred while writing message to: " << receiver;
          perror(". ");
          std::cerr << std::endl;
        }
    }
  pthread_mutex_unlock(&m_MapMutex);

}

int SSWGServer::determineReceiverSocketFd(const std::string& rcvr)
{
  ProcessNameToSocketIdMap::const_iterator iter;
  
  if ((iter = m_NameToSockMap.find(rcvr)) != m_NameToSockMap.end())
    {
      if (m_EchoMessage >= SSWGCOMM_HIGH) std::cout << "Found receiver's socket FD in the map: " << iter->second << std::endl;
      return iter->second;
    }
  else
    if (m_EchoMessage >= SSWGCOMM_HIGH) std::cout << "Did not find receiver's socketID in the map." << std::endl;
  
  return -1;
}

bool SSWGServer::acceptConnections(int portNum)
{
  
  if (pthread_mutex_init(&m_MapMutex, NULL) != 0)
    {
      std::cerr << "Error in initializing the mutex" << std::endl;
      return false;
    }
  
  struct sockaddr_in  serverAddr;    // server's socket address
  struct sockaddr_in  clientAddr;    // client's socket address
#ifdef UNIX
  socklen_t sockAddrSize; // size of socket address structure
#else
  int sockAddrSize;
#endif
  
  // set up the local address
  sockAddrSize = sizeof (struct sockaddr_in);
  bzero ((char *) &serverAddr, sockAddrSize);
  serverAddr.sin_family = AF_INET;
#ifdef UNIX
#else	
  serverAddr.sin_len = (u_char) sockAddrSize;
#endif
  serverAddr.sin_port = htons (portNum);
  serverAddr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  // create a TCP-based socket
  if ((m_sFd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      std::cerr << "Error creating server socket" << std::endl;
      return false;
    }

   int opt_val = 1;
#ifdef UNIX
  setsockopt(m_sFd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt_val, sizeof(int));
#else
  setsockopt(m_sFd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt_val, sizeof(int));
#endif
  
  // bind socket to local address
  if (bind (m_sFd, (struct sockaddr *) &serverAddr, sockAddrSize) == -1)
    {
      std::cerr << "Error binding server socket.";
      perror(" ");
      std::cerr << std::endl;
      close (m_sFd);
      return false;
    }
  
  // create queue for client connection requests
  if (listen (m_sFd, SERVER_MAX_CONNECTIONS) == -1)
    {
      std::cerr << "Error while listening on server socket" << std::endl;
      close (m_sFd);
      return false;
    }
  
  // accept new connect requests and spawn tasks to process them
  while (1)
    {
      if ((m_SocketFd = accept (m_sFd, (struct sockaddr *) &clientAddr,
                                &sockAddrSize)) == -1)
        {
          std::cerr << "Error while accepting connections on server socket.";
          perror(" ");
          std::cerr << std::endl;
          return false;
        }
      
      if (m_ConCount >= SERVER_MAX_CONNECTIONS)
	{
	  std::cerr << "The maximum number of allowable connections has been exceeded. Modify the SERVER_MAX_CONNECTIONS value in SSWGServer.hh if needed. New requests for connections will be ignored." << std::endl;
	}
      else
        {
          args[m_ConCount].socketFd = m_SocketFd;
          args[m_ConCount].server = this;
          
          if (threadSpawn((THREAD_FUNC_PTR)spawnThreadForEachClient, (void *)&args[m_ConCount],
                          m_ThreadId) != true)
            {
              std::cout << "Closing the new socket FD that was just accepted by the server." << std::endl;
              close(m_SocketFd);
            }
          else
            {
              m_ThreadList.push_back(m_ThreadId);
            }
        }
      
      ++m_ConCount;
    }
}
