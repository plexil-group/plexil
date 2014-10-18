/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#include "PlexilNodeType.hh"
#include "Error.hh"

#include <cstring>

namespace PLEXIL
{
  std::string const ASSIGNMENT = "Assignment";
  std::string const COMMAND = "Command";
  std::string const EMPTY = "Empty";
  std::string const LIBRARYNODECALL = "LibraryNodeCall";
  std::string const LIST = "NodeList";
  std::string const UPDATE = "Update";

  PlexilNodeType parseNodeType(char const *typeName)
  {
    switch (*typeName) {
    case 'A':
      if (ASSIGNMENT == typeName)
        return NodeType_Assignment;
      else 
        return NodeType_error;

    case 'C':
      if (COMMAND == typeName)
        return NodeType_Command;
      else 
        return NodeType_error;

    case 'E':
      if (EMPTY == typeName)
        return NodeType_Empty;
      else 
        return NodeType_error;

    case 'L':
      if (LIBRARYNODECALL == typeName)
        return NodeType_LibraryNodeCall;
      else 
        return NodeType_error;

    case 'N':
      if (LIST == typeName)
        return NodeType_NodeList;
      else 
        return NodeType_error;

    case 'U':
      if (UPDATE == typeName)
        return NodeType_Update;
      // fall thru to...

    default:
      return NodeType_error;
    }
  }

  const std::string& nodeTypeString(PlexilNodeType nodeType)
  {
    static const std::string sl_errorReturn = "Invalid Node Type";
    switch (nodeType) {
    case NodeType_NodeList:
      return LIST;
      break;
    case NodeType_Command:
      return COMMAND;
      break;
    case NodeType_Assignment:
      return ASSIGNMENT;
      break;
    case NodeType_Update:
      return UPDATE;
      break;
    case NodeType_Empty:
      return EMPTY;
      break;
    case NodeType_LibraryNodeCall:
      return LIBRARYNODECALL;
      break;

      // fall thru case
    default:
      checkError(ALWAYS_FAIL,
                 "Invalid node type " << nodeType);
      return sl_errorReturn;
      break;
    }
  }

} // namespace PLEXIL
