// Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef NODE_FACTORY_HH
#define NODE_FACTORY_HH

#include "NodeConstants.hh"
#include "PlexilNodeType.hh"

namespace PLEXIL
{

  // Forward declaration
  class NodeImpl;

  //! \class NodeFactory
  //! \brief Stateless abstract base class for an abstract factory for NodeImpl.
  //! \ingroup Exec-Core
  class NodeFactory
  {
  public:

    //! \brief Virtual destructor.
    virtual ~NodeFactory() = default;

    //! \brief Primary factory method.
    //! \param name The NodeId of the node to construct.
    //! \param type The node type to construct.
    //! \param parent The parent node of the node to be constructed.
    //! \return Pointer to the new NodeImpl.
    static NodeImpl *createNode(char const *name,
                                PlexilNodeType type,
                                NodeImpl *parent = nullptr);

    //! \brief Alternate factory method.  Used only by Exec test module.
    //! \param type The name of the node type to construct.
    //! \param name The NodeId of the node to construct.
    //! \param state The initial NodeState.
    //! \param parent The parent node of the node to be constructed.
    //! \return Pointer to the new NodeImpl.
    static NodeImpl *createNode(const std::string& type, 
                                const std::string& name, 
                                NodeState state,
                                NodeImpl *parent = nullptr);

  protected:

    //! \brief Primary factory method delegated to derived classes.
    //! \param name The NodeId of the node to construct.
    //! \param parent The parent node of the node to be constructed.
    //! \return Pointer to the new NodeImpl.
    virtual NodeImpl *create(char const *name, 
                             NodeImpl *parent = nullptr) const = 0;

    //! \brief Alternate factory method.  Used only by Exec test module.
    //! \param type The name of the node type to construct.
    //! \param name The NodeId of the node to construct.
    //! \param state The initial NodeState.
    //! \param parent The parent node of the node to be constructed.
    //! \return Pointer to the new NodeImpl.
    virtual NodeImpl *create(const std::string& type,
                             const std::string& name,
                             NodeState state,
                             NodeImpl *parent = nullptr) const = 0;
  };

}

#endif // NODE_FACTORY_HH
