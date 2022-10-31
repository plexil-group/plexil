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

#ifndef PLEXIL_UPDATE_HH
#define PLEXIL_UPDATE_HH

#include "SimpleMap.hh"
#include "Value.hh"

namespace PLEXIL
{

  //! \class Update
  //! \brief Stateless abstract base class representing the
  //!        information transmitted by a PLEXIL Update node.
  //! \ingroup External-Interface
  class Update
  {
  public:

    //! \typedef Shorthand for the name-value data structure type
    using PairValueMap = SimpleMap<std::string, Value>;

    //! \brief Virtual destructor.
    virtual ~Update() = default;

    //
    // API to external interfaces
    //

    //! \brief Get the map of name-value pairs.
    //! \return Const reference to the map.
    virtual const PairValueMap &getPairs() const = 0;

    //! \brief Get the node ID of the owning node.
    //! \return Const pointer to the node, as a NodeConnector.
    virtual std::string const &getNodeId() const = 0;

    //! \brief Return an acknowledgement value for the update.
    //! \param ack The value.
    virtual void acknowledge(bool ack) = 0;

    //
    // LinkedQueue item API for PlexilExec
    //

    //! \brief Get the pointer to the next Update in the queue.
    //! \return Pointer to the Update.
    virtual Update *next() const = 0;

    //! \brief Get the pointer to the pointer to the next Update in the queue.
    //! \return Pointer to the next poitner.
    virtual Update **nextPtr() = 0;
  };

}

#endif // PLEXIL_UPDATE_HH
