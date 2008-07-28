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

#include "SSWGClient.hh"
#include "ThreadSpawn.hh"
#include "SSWGCallbackHandler.hh"

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

#include <pthread.h>
#include <strings.h>

//C wrapper to call a C++ method. Used while spawning a thread. 
void spawnThreadForServerMessages(void* args)
{	
  SSWGClient* client = static_cast<SSWGClient *>(args);
  client->readMessage();
}

SSWGClient::SSWGClient() : m_SocketFd(-1), m_Connected(false)
{
}

SSWGClient::~SSWGClient()
{
  closeSocket();
  if (m_Connected)
    {
      pthread_cancel(m_ThreadId);
      pthread_join(m_ThreadId, NULL);
    }
}

bool SSWGClient::connectToServer(const std::string& name, const std::string serverIPAdr, int portNum,
                                 SSWGCallbackHandler* const callBack)
{
  struct sockaddr_in  serverAddr;    // server's socket address
  int                 sockAddrSize;  // size of socket address structure
  
  m_SenderName = name;
  m_ClientCallbackHandler = callBack;
  if ((m_SocketFd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      std::cerr << "Error during socket creation" << std::endl;
      return false;
    }
  // bind not required - port number is dynamic
  // build server socket address
  sockAddrSize = sizeof (struct sockaddr_in);
  bzero ((char *) &serverAddr, sockAddrSize);
  serverAddr.sin_family = AF_INET;
#ifdef UNIX
#else	
  serverAddr.sin_len = (u_char) sockAddrSize;
#endif
  serverAddr.sin_port = htons (portNum);
  
  if (((serverAddr.sin_addr.s_addr =
        inet_addr (const_cast<char *>(serverIPAdr.c_str()))) ==
       static_cast<in_addr_t>(-1)))
    {
      std::cerr << "ERROR:: Invalid server IP address" << std::endl;
      closeSocket();
      return false;
    }
  // connect to server
  
  if (connect (m_SocketFd, (struct sockaddr *) &serverAddr, sockAddrSize) == -1)
    {
      std::cerr << "Error during connect.";
      perror(" ");
      std::cerr << std::endl;
      closeSocket();
      return false;
    }
  
  // Set up a thread to listen to replies
  
  if (threadSpawn((THREAD_FUNC_PTR)spawnThreadForServerMessages, (void *)this,
                  m_ThreadId) == false)
    {
      std::cout << "Closing the client socket FD that was just connected" << std::endl;
      this->closeSocket();
      return false;
    }
  
  m_Connected = true;
  return this->sendMessage("Identification", "SSWGServer");
}

bool SSWGClient::sendMessage(const std::string& msgT, const std::string& recvName)
{
  SSWGMessage msg;
  
  if (msg.createMessage(m_SenderName, recvName, msgT) == false) return false;
  
  if (write (m_SocketFd, msg.finalMessage, strlen(msg.finalMessage)) == -1)
    {
      std::cerr << "Error occurred while sending a message.";
      perror(" ");
      std::cerr << std::endl;
      closeSocket();
      return false;
    }
  
  return true;
}

void SSWGClient::readMessage()
{
  int nRead;          // number of bytes read
  
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
  

  while ((nRead = read (m_SocketFd, buff+i, MAX_READ_BUFFER_SIZE-i)) > 0)
    {
      //      std::cout << "Number of bytes read: " << nRead << std::endl;
      i += nRead;
      buff[i] = '\0';
      
      //      std::cout << "Buffer read: " << buff << std::endl;
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
              
              //                std::cout << "Extracted sender in read is: " << sender << std::endl;
              //                std::cout << "Extracted receiver in read is: " << receiver << std::endl;
              //                std::cout << "Extracted message in read is: " << message << std::endl;
              //          std::cout << "Client (" << m_SenderName << ") received a message. From: " << sender
              //                    << ", To: " << receiver << ", Msg: " << message << std::endl;
              m_ClientCallbackHandler->receivedMessage(sender, message);

              // remember that the \r character gets replaces by \0 after substring match is
              // found
              remainLen = i - (strlen(buff) + strlen(message_end_tag));

              //              std::cout << "Total buffer read: " << i << ", size of message: " << strlen(buff)
              //                        << ", remaining length: " << remainLen << std::endl;
              
              // Copy over remaining chunck
              // A ring buffer would have eliminated the need for buffTmp
              strncpy(buffTmp, crlfp+2, remainLen+1); // crlfp+2 will be within the bounds of buff
                                                      // since we have on extra space for \0 at the end
              strncpy(buff, buffTmp, remainLen+1);
              i = remainLen;

              //              std::cout << "before testing the first char" << std::endl;
              // If the first character of the start tag is absent in the remaining chunck
              // discard the remainder. If the first character is present, it is possible that
              // the start tag got clipped out. So retain the chunk by not resetting i.

              if (strstr(buff, firstChar) == NULL)
                {
                  // discard the chunck since it does not contain a message start tag
                  i = 0;
                  readMore = true;
                  //                  std::cout << "Discard the remaining chunck" << std::endl;
                }
              else
                {
                  //                  std::cout << "Keeping the reminaing chunck" << std::endl;
                  //                  std::cout << "buff: " << buff << "\nbuff length: " << strlen(buff) << ", i: " << i << std::endl;
                }
              //              std::cout << "after testing the first char" << std::endl;
            }
          else
            readMore = true;

          //          std::cout << "readMore: " << readMore << std::endl;
          
        } while (!readMore);
    }

  // error from read()
  std::cout << "Error while reading message. Number of chars read: " << nRead << std::endl;
  closeSocket();
  std::cout << "Exitting the readMessage thread." << std::endl;
  pthread_exit(NULL);
}

void SSWGClient::closeSocket()
{
  if (m_SocketFd > 0)
    {
      std::cout << "Closing the client socket." << std::endl;
      close(m_SocketFd);
      m_SocketFd=-1;
    }
}

