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

#include "PlexilPlan.hh"
#include "Debug.hh"
#include "Error.hh"
#include "Node.hh"
#include <sstream>

namespace PLEXIL {
   
  PlexilNode::PlexilNode()
    : m_priority(WORST_PRIORITY),
      m_intf(NULL),
      m_nodeBody(NULL),
      m_lineNo(0),
      m_colNo(0),
      m_nodeType(NodeType_uninitialized)
  {}

  PlexilNode::~PlexilNode() {
    //delete everything here
    delete m_intf;
    m_intf = NULL;
    delete m_nodeBody;
    m_nodeBody = NULL;
    for (std::vector<std::pair<PlexilExpr *, std::string> >::iterator it = m_conditions.begin();
         it != m_conditions.end();
         ++it) {
      delete it->first;
    }
    m_conditions.clear();
    for (std::vector<PlexilVar *>::iterator it = m_declarations.begin();
         it != m_declarations.end();
         ++it) {
      delete *it;
    }
    m_declarations.clear();
  }

  /**
   * @brief Get the names of all library nodes referenced by this node and its descendants.
   * @return A vector of library node names.
   */
  std::vector<std::string> PlexilNode::getLibraryReferences() const {
    std::vector<std::string> result;
    this->getLibraryReferences(result);
    debugMsg("PlexilNode:getLibraryReferences", " found " << result.size() << " unique library references");
    return result;
  }

  // Stupid utility made necessary because STL sorts its sets.
  template <typename T>
  void insertUnique(std::vector<T>& vec, const T& thing)
  {
    for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
      if (*it == thing)
        return;
    }
    vec.push_back(thing);
  }

  /**
   * @brief Get the names of all library nodes referenced by this node and its descendants.
   * @param refs The vector of referenced library node names to be returned.
   */
  void PlexilNode::getLibraryReferences(std::vector<std::string>& refs) const {
    switch (m_nodeType) {

    case NodeType_LibraryNodeCall: {
      PlexilLibNodeCallBody const *callBody =
        dynamic_cast<PlexilLibNodeCallBody const *>(m_nodeBody);
      // FIXME: move check up into XML parser
      checkError(callBody,
                 "PlexilNode::getLibraryReferences: node is not a library call node");
      insertUnique(refs, callBody->libNodeName());
      break;
    }

    case NodeType_NodeList: {
      PlexilListBody const *listBody = dynamic_cast<PlexilListBody const *>(m_nodeBody);
      // FIXME: move check up into XML parser
      checkError(listBody,
                 "PlexilNode::getLibraryReferences: node is not a list node");
      const std::vector<PlexilNode *>& kids = listBody->children();
      for (std::vector<PlexilNode *>::const_iterator it = kids.begin();
           it != kids.end();
           ++it) {
        (*it)->getLibraryReferences(refs);
      }
      break;
    }

    default:
      break;
    }
  }

  PlexilInterface::~PlexilInterface() {
    for (std::vector<PlexilVarRef*>::iterator it = m_in.begin();
         it != m_in.end();
         ++it)
      delete *it;
    m_in.clear();
    for (std::vector<PlexilVarRef*>::iterator it = m_inOut.begin();
         it != m_inOut.end();
         ++it)
      delete *it;
    m_inOut.clear();
  }

  // find a variable in the set of In variables

  const PlexilVarRef* PlexilInterface::findInVar(const PlexilVarRef* target)
  {
    return findInVar(target->name());
  }
  // find a variable in the set of InOut variables

  const PlexilVarRef* PlexilInterface::findInOutVar(const PlexilVarRef* target)
  {
    return findInOutVar(target->name());
  }
  // find a var in the interface

  const PlexilVarRef* PlexilInterface::findVar(const PlexilVarRef* target)
  {
    return findVar(target->name());
  }
  // find a variable in the set of In variables

  const PlexilVarRef* PlexilInterface::findInVar(const std::string& target)
  {
    for (std::vector<PlexilVarRef*>::const_iterator var = m_in.begin();
         var != m_in.end(); ++var) {
      if (target == (*var)->name())
        return *var;
    }
    return NULL;
  }
  // find a variable in the set of InOut variables

  const PlexilVarRef* PlexilInterface::findInOutVar(const std::string& target)
  {
    for (std::vector<PlexilVarRef*>::const_iterator var = m_inOut.begin();
         var != m_inOut.end(); ++var) {
      if (target == (*var)->name())
        return *var;
    }
    return NULL;
  }
  // find a var in the interface

  const PlexilVarRef* PlexilInterface::findVar(const std::string& target)
  {
    const PlexilVarRef* var = findInVar(target);
    return var == NULL ? findInOutVar(target) : var;
  }

  // wrapper call for link which creates the seen library nodes
  // data structure before calling the recursive linker

  bool PlexilNode::link(const std::map<std::string, PlexilNode *>& libraries)
  {
    PlexilNodeSet seen;
    bool result = link(libraries, seen);
    return result;
  }

  // Resolve links between the plan and a library node.
  // Returns false if there is a circular library reference
  // or if there are unresolved library node calls present after linking is completed,
  // true otherwise.
   
  bool PlexilNode::link(const std::map<std::string, PlexilNode *>& libraries, PlexilNodeSet& seen)
  {
    if (nodeType() == NodeType_LibraryNodeCall) {
      PlexilLibNodeCallBody *body = dynamic_cast<PlexilLibNodeCallBody *>(m_nodeBody);

      // find the referenced library
      std::map<std::string, PlexilNode *>::const_iterator libraryIt = 
        libraries.find(body->libNodeName());
      if (libraryIt == libraries.end()) {
        // Report unresolved library call error
        debugMsg("PlexilPlan:link", "Unresolved library call: " << body->libNodeName());
        return false;
      }

      // found it -- test for a circular library reference
      PlexilNode *library = libraryIt->second;
      for (PlexilNodeSet::iterator seenLib = seen.begin(); 
           seenLib != seen.end();
           ++seenLib) {
        if (*seenLib == library) {
          // TODO: show entire chain of references
          debugMsg("PlexilPlan:link",
                   " Circular library reference: "
                   << body->libNodeName());
          return false;
        }
      }

      // link the the two nodes
      debugMsg("PlexilPlan:link",
               " linking " << m_nodeId <<  " to " << body->libNodeName());
      body->setLibNode(library);

      // add this to the seen library nodes
      seen.push_back(library);

      // resolve any library calls in the library,
      if (!library->link(libraries, seen)) 
        return false;

      // now remove said item from the seen set (pop the stack)
      seen.pop_back();

      // return success
      return true;
    }

    // if this is a list node, recurse into its children
    else if (nodeType() == NodeType_NodeList) {
      // iterate through the list nodes children and check for library calls
      PlexilListBody const *body = dynamic_cast<PlexilListBody const *>(m_nodeBody);
      assertTrue_1(body);
      const std::vector<PlexilNode *>& children = body->children();
      for (std::vector<PlexilNode *>::const_iterator child = children.begin();
           child != children.end();
           ++child) {
        if (!(*child)->link(libraries, seen))
          return false;
      }
      return true;
    }
    else 
      // Nothing to do, return true
      return true;
  }

  void PlexilNodeRef::setGeneration(int gen)
  {
    m_generation = gen;
    if (gen == 0)
      return;

    switch (m_dir) {
    case SELF:
      if (gen == 1)
        m_dir = PARENT;
      else
        m_dir = GRANDPARENT;
      break;

    case PARENT: m_dir = GRANDPARENT; break;

    case CHILD:
      if (gen == 1)
        m_dir = SIBLING;
      else 
        m_dir = UNCLE;
      break;

    case SIBLING: m_dir = UNCLE; break;

    default: // includes NO_DIR, GRANDPARENT, UNCLE
      assertTrueMsg(ALWAYS_FAIL, "PlexilNodeRef::setGeneration(): invalid direction");
    }
  }

  PlexilLibNodeCallBody::PlexilLibNodeCallBody(const std::string& libNodeName)
    : PlexilNodeBody(),
      m_libNodeName(libNodeName),
      m_libNode(NULL)
  {
  }

  PlexilLibNodeCallBody::~PlexilLibNodeCallBody()
  {
    for (PlexilAliasMap::iterator it = m_aliases.begin();
         it != m_aliases.end();
         it = m_aliases.begin()) {
      delete it->second;
      m_aliases.erase(it);
    }
  }

  void PlexilLibNodeCallBody::addAlias(const std::string& param, PlexilExpr *value)
  {
    PlexilExpr *&alias = m_aliases[param];
    checkError(!alias, "Alias '" << param
               << "' apears more then once in call to "
               << m_libNodeName);
    alias = value;
  }

}
