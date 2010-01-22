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
#ifndef PLEXILSIM_RESPONSE_HH
#define PLEXILSIM_RESPONSE_HH

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "ResponseBase.hh"
#include "ResponseMessage.hh"

class MoveResponse : public ResponseBase
{
public:
  MoveResponse(const std::string& name, timeval delay, const int returnValue)
    : ResponseBase(name, delay), m_ReturnValue(returnValue) {}

  ~MoveResponse(){}

  virtual ResponseMessage* createResponseMessage()
  {

    std::ostringstream str;
    str << m_ReturnValue;
    std::cout << "Creating a Move response: " << str.str() << std::endl;
    return new ResponseMessage(-1, str.str()+'\n');
  }

private:
  const int m_ReturnValue;
};

class foo : public ResponseBase
{
public:
  foo(const std::string& name, timeval delay, const int returnValue)
    : ResponseBase(name, delay), m_ReturnValue(returnValue) {}

  ~foo(){}

  virtual ResponseMessage* createResponseMessage()
  {

    std::ostringstream str;
    str << m_ReturnValue;
    std::cout << "Creating a foo message: " << str.str() << std::endl;
    return new ResponseMessage(-1, str.str()+'\n');
  }

private:
  const int m_ReturnValue;
};

#endif //ROBOSIM_RESPONSE_HH
