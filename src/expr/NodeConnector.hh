// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
// All rights reserved.
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

#ifndef NODE_CONNECTOR_HH
#define NODE_CONNECTOR_HH

#include <string>

namespace PLEXIL
{
  // Forward reference
  class Expression;

  //! \class NodeConnector
  //! \brief Stateless abstract base class represnting a minimal
  //!        subset of the Node API needed by the plan parser.
  //! \ingroup Exec-Core
  class NodeConnector 
  {
  public:

    //! \brief Virtual destructor.
    virtual ~NodeConnector() = default;

    //! \brief Get the node's name.
    //! \return Const reference to the name string.
    virtual std::string const &getNodeId() const = 0;

    //! \brief Look up a declared variable by name.
    //! \param name Name of the variable, as a const pointer to character string.
    //! \return Pointer to the variable, as an expression.  Will be null if not found.
    //! \note Used only by XML parser and its unit tests.
    virtual Expression *findVariable(char const *name) = 0;

    //! \brief Notify the node that a resource on which it is pending has become available.
    //! \note Used by Reservable as part of the resource contention resolution logic.
    virtual void notifyResourceAvailable() = 0;
  };

} // namespace PLEXIL

#endif // NODE_CONNECTOR_HH
