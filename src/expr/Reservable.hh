// Copyright (c) 2006-2023, Universities Space Research Association (USRA).
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

//
// Common member functions and state for a PLEXIL variable or mutex
//

#ifndef PLEXIL_RESERVABLE_HH
#define PLEXIL_RESERVABLE_HH

#include <vector>

namespace PLEXIL
{

  // Forward declarations
  class NodeConnector;

  //! @class Reservable
  //! Mixin class implementing the state required by a variable or mutex
  //! in the PLEXIL language.
  class Reservable
  {
  public:

    using WaitQueue = std::vector<NodeConnector *>;

    //! Default constructor.
    Reservable() = default;

    //! Virtual destructor.
    virtual ~Reservable() = default;

    //! Attempt to acquire the object. On failure, add the node to the
    //! object's waiting list.
    //! @param The node wishing to acquire this object.
    //! @return true if the object was successfully acquired;
    //!         false if not.
    bool acquire(NodeConnector *node);
      
    //! If held by this node, release the object and notify other
    //! waiting nodes that the object is available.
    //! @param node The node which (we hope) previously acquired the object.
    //! @return true if the node was the holder, false if not.
    bool release(NodeConnector *node);

    //! Add a node to the list of nodes waiting on the variable.
    //! @param node Pointer to the node.
    //! @return true if the node was added, false if it was already waiting.
    bool reserve(NodeConnector *node);

    //! Remove a node from the list of nodes waiting on the variable.
    //! @param node Pointer to the node.
    //! @return true if the node was removed, false if not found.
    bool cancelReservation(NodeConnector *node);

    //! Report which node currently holds this object.
    //! @return Pointer to the node; may be null.
    NodeConnector const *getHolder() const;

    //! Get the wait queue.
    //! @return Const reference to the wait queue.
    WaitQueue const &getWaiters() const;

  private:

    //! Nodes waiting to reserve this object.
    WaitQueue m_waiters;

    //! The node currently holding this object.
    NodeConnector *m_holder;

  }; // class Reservable

} // namespace PLEXIL

#endif // PLEXIL_RESERVABLE_HH
