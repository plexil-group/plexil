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
#include "PlexilSimResponseFactory.hh"
#include "PlexilSimResponse.hh"
#include "GenericResponse.hh"

PlexilSimResponseFactory::PlexilSimResponseFactory()
{
}

PlexilSimResponseFactory::~PlexilSimResponseFactory()
{
}

ResponseBase* PlexilSimResponseFactory::parse(const std::string& cmdName, timeval tDelay,
                                            std::istringstream& inStr)
{
  if (cmdName == "move")
    {
      int returnValue;
      if (parseType<int>(inStr, returnValue))
        return new MoveResponse(tDelay, returnValue);
    }
  else if (cmdName == "foo")
    {
      int returnValue;
      if (parseType<int>(inStr, returnValue))
        return new foo(tDelay, returnValue);
    }
  else
    {
      // No customization present. See if the default version can be used.
      double returnValue;
      if (parseType<double>(inStr, returnValue))
        {
          return new GenericResponse(tDelay, returnValue);
        }
      else
        {
          std::cout << "Error: The return value structure neither matches "
                    << " nor the customization nor the generic structure"
                    << std::endl;
          return NULL;
        }
    }
}
