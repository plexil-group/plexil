// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_NODE_TYPE_HH
#define PLEXIL_NODE_TYPE_HH

#include <string>

// Take care of annoying VxWorks macro
#undef UPDATE

namespace PLEXIL
{
  ///@{

  //! \brief String constant used as a node type name.
  //! \ingroup Exec-Core
  extern std::string const ASSIGNMENT; // = "Assignment"
  extern std::string const COMMAND; // = "Command"
  extern std::string const EMPTY; // = "Empty"
  extern std::string const LIBRARYNODECALL; // = "LibraryNodeCall"
  extern std::string const LIST; // = "NodeList"
  extern std::string const UPDATE; // = "Update"
  ///@}

  //! \enum PlexilNodeType
  //! \brief Represents the type of a PLEXIL Node.
  //! \ingroup Exec-Core
  enum PlexilNodeType {
    NodeType_uninitialized = 0,
    NodeType_NodeList,
    NodeType_Command,
    NodeType_Assignment,
    NodeType_Update,
    NodeType_Empty,
    NodeType_LibraryNodeCall,
    NodeType_error
  };

  //! \brief Get a string representation of a node type value.
  //! \param nodeType The node type.
  //! \return Const reference to the string.
  //! \ingroup Exec-Core
  std::string const &nodeTypeString(PlexilNodeType nodeType);

  //! \brief Get the PlexilNodeType value named by a string.
  //! \param typeName Pointer to const null-terminated string.
  //! \return The node type. Returns NodeType_error if not parseable.
  //! \ingroup Exec-Core
  PlexilNodeType parseNodeType(char const *typeName);
  
} // namespace PLEXIL

#endif // PLEXIL_NODE_TYPE_HH
