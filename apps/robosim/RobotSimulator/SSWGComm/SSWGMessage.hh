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

#ifndef SSWG_MESSAGE_HEADER
#define SSWG_MESSAGE_HEADER

#include <string>
#include <iostream>

#define MAX_NAME_LENGTH 32      // includes 1 char for delimiter ":"
#define MAX_MESSAGE_LENGTH 4096 // one page, includes 2 characters for CRLF and
                                // 3 chars for start tag <:>
#define MAX_READ_BUFFER_SIZE 2*MAX_NAME_LENGTH+MAX_MESSAGE_LENGTH

const char message_delimiter[2] = ":";
const char message_start_tag[4] = "<:>";
const char message_end_tag[4] = "\r\n";
const char unknown[8] = "UNKNOWN";
class SSWGMessage
{
public:

  /*!
    \brief Default constructor 
  */
  SSWGMessage(){}
  
  /*!
    \brief Default destructor 
  */
  ~SSWGMessage(){}
  
  /*!
    \brief Copies strings for the sender, receiver and message fields passed in as
    arguments to a single character array delimited by colon and terminated with a CRLF 
    "\r\n" combination. A starting tag is also used to mark the beginnin gof the consolidated
    message. This is needed to reassemble at the receiving end in case messages get split between
    packets.
    returns a boolean: True if command succeeds
  */
  bool createMessage(const std::string& sName, const std::string& rName, const std::string& msg)
  {
    memset(finalMessage, 0, MAX_READ_BUFFER_SIZE);

    strncpy(finalMessage, message_start_tag, strlen(message_start_tag));

    if (sName.length() > (MAX_NAME_LENGTH-strlen(message_delimiter)))
      {
        std::cerr << "ERROR:: Sender name (" << sName << ") is longer than the allowed limit of " 
                  << (MAX_NAME_LENGTH - strlen(message_delimiter)) << " characters." << std::endl;
        return false;
      }
    else
      {
        strncat(finalMessage, sName.c_str(), sName.length());
      }

    strncat(finalMessage, message_delimiter, strlen(message_delimiter));

    if (rName.length() > (MAX_NAME_LENGTH-strlen(message_delimiter)))
      {
        std::cerr << "ERROR:: Receiver name (" << rName << ") is longer than the allowed limit of " 
                  << (MAX_NAME_LENGTH - strlen(message_delimiter)) << " characters." << std::endl;
        return false;
      }
    else
      {
        strncat(finalMessage, rName.c_str(), rName.length());
      }

    strncat(finalMessage, message_delimiter, strlen(message_delimiter));

    unsigned int pad = strlen(message_start_tag) + strlen(message_end_tag);

    if (msg.length() > (MAX_MESSAGE_LENGTH-pad))
      {
        std::cerr << "ERROR:: Message from sender (" << sName << ") to (" << rName 
                  << ") is longer than the allowed limit of " << (MAX_MESSAGE_LENGTH - pad)
                  << " characters." << std::endl;
        return false;
      }
    else
      {
        strncat(finalMessage, msg.c_str(), msg.length());
      }
    strncat(finalMessage, message_end_tag, strlen(message_end_tag));
    //    std::cout << "The final message being written is: " << finalMessage << ", and length: " << strlen(finalMessage) << std::endl;

    return true;
  }

  /*!
    \brief Message creation specifically for forwarding. No error checking performed
  */

  void createForwardedMessage(const std::string& sName, const std::string& rName, const std::string& msg)
  {
    memset(finalMessage, 0, MAX_READ_BUFFER_SIZE);

    strncpy(finalMessage, message_start_tag, strlen(message_start_tag));
    strncat(finalMessage, sName.c_str(), sName.length());
    strncat(finalMessage, message_delimiter, strlen(message_delimiter));
    strncat(finalMessage, rName.c_str(), rName.length());
    strncat(finalMessage, message_delimiter, strlen(message_delimiter));
    strncat(finalMessage, msg.c_str(), msg.length());
    strncat(finalMessage, message_end_tag, strlen(message_end_tag));
    //    std::cout << "The final message being forwarded is: " << finalMessage << ", and length: " << strlen(finalMessage) << std::endl;
  }

  /*!
    \brief Extracts the sender from the char array received through the socket. Looks only
    at contents after the start_tag.
    Returns a false if the message does not have the necessary fields. The requested string will
    be set to the unknown string (programmable).
  */
  bool extractSender(const char* msg, std::string& sender)
  {
    std::string msgStr(msg);
    size_t ind0 = msgStr.find(message_start_tag, 0);
    size_t ind1 = msgStr.find_first_of(message_delimiter, ind0 + strlen(message_start_tag));
    if ((ind0 != std::string::npos) && (ind1 != std::string::npos))
      {
        sender = msgStr.substr(ind0+strlen(message_start_tag), ind1-strlen(message_start_tag));
        //        std::cout << "Extracted sender in extractMessage is: " << sender << std::endl;
        return true;
      }
    else
      {
        sender = std::string(unknown);
        std::cerr << "The message string does not have all the necessary fields." << std::endl;
        return false;
      }
  }

  /*!
    \brief Extracts the receiver from the char array received through the socket. Looks only
    at contents after the start_tag.
    Returns a false if the message does not have the necessary fields. The requested string will
    be set to the unknown string (programmable).
  */
  bool extractReceiver(const char* msg, std::string& receiver)
  {
    std::string msgStr(msg);
    size_t ind0 = msgStr.find(message_start_tag, 0);
    size_t ind1 = msgStr.find_first_of(message_delimiter, 
                                             ind0 + strlen(message_start_tag));
    size_t ind2 = msgStr.find_first_of(message_delimiter, ind1+1);

    if ((ind0 != std::string::npos) && (ind1 != std::string::npos) && (ind2 != std::string::npos))
      {
        receiver = msgStr.substr(ind1+1, ind2-ind1-1);
        //        std::cout << "Extracted receiver in extractMessage is: " << receiver << std::endl;
        return true;
      }
    else
      {
        receiver = std::string(unknown);
        std::cerr << "The message string does not have all the necessary fields." << std::endl;
        return false;
      }
  }

  /*!
    \brief Extracts the message from the char array received through the socket. Looks only
    at contents after the start_tag.
    Returns a false if the message does not have the necessary fields. The requested string will
    be set to the unknown string (programmable).
  */
  bool extractMessage(const char* msg, std::string& message)
  {

    std::string msgStr(msg);
    size_t ind0 = msgStr.find(message_start_tag, 0);
    size_t ind1 = msgStr.find_first_of(message_delimiter, 
                                             ind0 + strlen(message_start_tag));
    size_t ind2 = msgStr.find_first_of(message_delimiter, ind1+1);

    if ((ind0 != std::string::npos) && (ind1 != std::string::npos) && (ind2 != std::string::npos))
      {
        message = msgStr.substr(ind2+1, strlen(msg));
        //        std::cout << "Extracted message in extractMessage is: " << message << std::endl;
        return true;
      }
    else
      {
        message = std::string(unknown);
        std::cerr << "The message string does not have all the necessary fields." << std::endl;
        return false;
      }
  }
	
  char finalMessage[MAX_READ_BUFFER_SIZE];

};
#endif // SSWG_MESSAGE_HEADER
