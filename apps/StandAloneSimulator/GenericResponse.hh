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
#ifndef GENERIC_RESPONSE_HH
#define GENERIC_RESPONSE_HH

#include <string>
#include <iostream>
#include <vector>
#include "ResponseBase.hh"
#include "ResponseMessage.hh"

class GenericResponse : public ResponseBase
{
public:
  GenericResponse(timeval delay, const std::vector<double>& value) 
    : ResponseBase(delay), m_ReturnValue(value)
  {

  }

  ~GenericResponse(){}

  virtual ResponseMessage* createResponseMessage()
  {
    std::string vectToString;
    for (unsigned int i = 0; i < m_ReturnValue.size(); ++i)
      {
        std::ostringstream str;
        str << m_ReturnValue[i];
        vectToString += str.str();
        if (i != (m_ReturnValue.size()-1))
          vectToString += " ";
      }
    vectToString += '\n';
    std::cout << "Creating a generic response with a return value of: " 
              << vectToString;
    return new ResponseMessage(-1, vectToString);
  }

private:
  std::vector<double> m_ReturnValue;
};

#endif //GENERIC_RESPONSE_HH
