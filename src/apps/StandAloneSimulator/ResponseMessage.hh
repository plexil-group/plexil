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
#ifndef RESPONSE_MESSAGE_HH
#define RESPONSE_MESSAGE_HH

#include <string>
#include <vector>
#include "ResponseBase.hh"
#include "simdefs.hh"

/**
 * @brief ResponseMessage represents an outgoing message that has been scheduled for output.
 */
class ResponseMessage
{
public:
  ResponseMessage(const ResponseBase* _base,
		  void* _id = NULL, 
		  int _type=MSG_COMMAND)
    : base(_base),
      id(_id), 
      messageType(_type) 
  {}

  virtual ~ResponseMessage()
  {
  }

  const ResponseBase* getResponseBase() const
  {
    return base;
  }

  void* getId() const
  {
    return id;
  }

  int getMessageType() const
  {
    return messageType;
  }

  const std::string& getName() const
  {
    static const std::string emptyString("");
    if (base == NULL)
      return emptyString;
    return base->getName();
  }

private:  
  // deliberately not implemented
  ResponseMessage();

  const ResponseBase* base;
  void* id;
  int messageType;
};

#endif // RESPONSE_MESSAGE_HH
