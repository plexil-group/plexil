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

#ifndef PLEXIL_NODE_VARIABLE_MAP_HH
#define PLEXIL_NODE_VARIABLE_MAP_HH

#include "SimpleMap.hh"
#include "map-utils.hh"

namespace PLEXIL
{

  // Forward declarations
  class Expression;

  //! \class NodeVariableMap
  //! \brief A name-to-variable mapping representing the variables accessible
  //!        within a node.  Has a link to the parent node for recursive lookup.
  //! \ingroup Exec-Core
  class NodeVariableMap final:
    public SimpleMap<char const *, Expression *, CStringComparator>
  {
  public:

    //! \brief Primary constructor.
    //! \param parentMap (Optional) Const pointer to the variable map in an ancestor node.
    NodeVariableMap(NodeVariableMap const *parentMap = nullptr);

    //! \brief Virtual destructor.
    virtual ~NodeVariableMap();
    
    //! \brief Erase the map.
    virtual void clear() override;

    //! \brief Find the named variable in this map or its ancestors.
    //! \param name Pointer to const null-terminated string.
    //! \return Pointer to the variable, as an Expression.
    Expression *findVariable(char const *name) const;

  protected:

    //! \brief Insert a new variable into the map.
    //! \param it Location at which to insert the new entry.
    //! \param k The name, used as the map's key.
    //! \param v Pointer to the Expression.
    //! \return Iterator pointing to the new entry.
    virtual iterator 
    insertEntry(iterator it, char const * const &k, Expression * const &v) override;

  private:

    // Copy, move constructors, assignment operators not implemented.
    NodeVariableMap(NodeVariableMap const &) = delete;
    NodeVariableMap(NodeVariableMap &&) = delete;
    NodeVariableMap &operator=(NodeVariableMap const &) = delete;
    NodeVariableMap &operator=(NodeVariableMap &&) = delete;

    NodeVariableMap const *m_parentMap; //!< Pointer to the map in an ancestor Node.  May be null.
  };

} // namespace PLEXIL

#endif // PLEXIL_NODE_VARIABLE_MAP_HH
