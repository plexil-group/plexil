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

#ifndef LIBRARY_CALL_NODE_HH
#define LIBRARY_CALL_NODE_HH

#include "ListNode.hh"

namespace PLEXIL
{

  //! \class LibraryCallNode
  //! \brief Implementation of a LibraryNodeCall node in PLEXIL.
  //!
  //! LibraryCallNode is implemented as a ListNode with a single child
  //! node, with certain behaviors overridden.  The library node being
  //! called cannot access any of the caller's variables by name; they
  //! are only accessible if explicitly passed via aliases.
  //!
  //! \ingroup Exec-Core
  class LibraryCallNode : public ListNode
  {
  public:

    //! \brief Primary constructor.
    //! \param nodeId The name of this node, as a pointer to const character string.
    //! \param parent (Optional) Pointer to the parent of this node.
    LibraryCallNode(char const *nodeId, NodeImpl *parent = nullptr);

    //! \brief Alternate constructor.  Used only by Exec test module.
    //! \param type Const reference to a node type name.
    //! \param name Const reference to the name to give this instance.
    //! \param state The state to assign this node.
    //! \param parent (Optional) The parent of this node; may be null.
    LibraryCallNode(const std::string& type,
                    const std::string& name,
                    NodeState state,
                    NodeImpl *parent = nullptr);

    //! \brief Virtual destructor.
    //! \note ListNode destructor performs all the necessary calls.
    virtual ~LibraryCallNode() = default;

    //! \brief Get the type of this node.
    //! \return The type of this node.
    virtual PlexilNodeType getType() const override
    {
      return NodeType_LibraryNodeCall;
    }

    //! \brief Get the name -> variable mapping that children of this node should reference.
    //! \return Const pointer to a variable map; may be null.
    //! \note Overrides ListNode method.
    virtual NodeVariableMap const *getChildVariableMap() const override;

    //! \brief Add an alias to the library call.
    //! \param name The name of the formal parameter in the called node.
    //! \param exp The expression to alias the name to.
    //! \param isGarbage If true, delete the expression when the alias is deleted.
    //! \note Only used by plan parser.
    bool addAlias(char const *name, Expression *exp, bool isGarbage);

    //! \brief Reserve space for some number of entries in the alias map.
    //! \param n The number of entries to reserve.
    //! \note Only used by plan parser.
    void allocateAliasMap(size_t n);

  protected:

    //! \brief Delete any additional objects as applicable for the node type.
    //! \note Wraps ListNode method.
    virtual void cleanUpNodeBody() override;

  private:

    // Copy, move constructors, assignment operators not implemented.
    LibraryCallNode(LibraryCallNode const &) = delete;
    LibraryCallNode(LibraryCallNode &&) = delete;
    LibraryCallNode &operator=(LibraryCallNode const &) = delete;
    LibraryCallNode &operator=(LibraryCallNode &&) = delete;

    //! \brief The map of called node names to actual expressions.
    NodeVariableMapPtr m_aliasMap;
  };

}

#endif // LIBRARY_CALL_NODE_HH
