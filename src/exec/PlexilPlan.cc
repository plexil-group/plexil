/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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
#include "ExecDefs.hh"
#include "Debug.hh"
#include "Node.hh"
#include <sstream>

namespace PLEXIL {

  PlexilNodeType PlexilParser::parseNodeType(const std::string & typeName)
  {
    if (typeName == LIST())
      return NodeType_NodeList;
    else if (typeName == COMMAND())
      return NodeType_Command;
    else if (typeName == ASSIGNMENT())
      return NodeType_Assignment;
    else if (typeName == UPDATE())
      return NodeType_Update;
    else if (typeName == EMPTY())
      return NodeType_Empty;
    else if (typeName == LIBRARYNODECALL())
      return NodeType_LibraryNodeCall;
        else
          return NodeType_error;
  }

  const std::string& PlexilParser::nodeTypeString(PlexilNodeType nodeType)
  {
    static const std::string errorReturn = "Invalid Node Type";
    switch (nodeType)
      {
      case NodeType_NodeList:
        return PlexilParser::LIST();
        break;
      case NodeType_Command:
        return PlexilParser::COMMAND();
        break;
      case NodeType_Assignment:
        return PlexilParser::ASSIGNMENT();
        break;
      case NodeType_Update:
        return PlexilParser::UPDATE();
        break;
      case NodeType_Empty:
        return PlexilParser::EMPTY();
        break;
      case NodeType_LibraryNodeCall:
        return PlexilParser::LIBRARYNODECALL();
        break;

        // fall thru case
      default:
        checkError(ALWAYS_FAIL,
                   "Invalid node type " << nodeType);
        return errorReturn;
        break;
      }
  }

  ValueType
  PlexilParser::parseValueTypePrefix(const std::string & str, 
                                                                         std::string::size_type prefixLen)
  {
        switch (prefixLen) {
        case 4: 
          if (0 == str.compare(0, prefixLen, REAL_STR()))
                return PLEXIL::REAL_TYPE;
          else if (0 == str.compare(0, prefixLen, DATE_STR()))
                return PLEXIL::DATE_TYPE;
          else 
                return PLEXIL::UNKNOWN_TYPE;

        case 5:
          if (0 == str.compare(0, prefixLen, ARRAY_STR()))
                return PLEXIL::ARRAY_TYPE;
          else
                return PLEXIL::UNKNOWN_TYPE;

        case 6:
          if (0 == str.compare(0, prefixLen, STRING_STR()))
                return PLEXIL::STRING_TYPE;
          else
                return PLEXIL::UNKNOWN_TYPE;

        case 7:
          if (0 == str.compare(0, prefixLen, INTEGER_STR()))
                return PLEXIL::INTEGER_TYPE;
          else if (0 == str.compare(0, prefixLen, BOOL_STR()))
                return PLEXIL::BOOLEAN_TYPE;
          else
                return PLEXIL::UNKNOWN_TYPE;

        case 8:
          if (0 == str.compare(0, prefixLen, DURATION_STR()))
            return PLEXIL::DURATION_TYPE;
          else
            return PLEXIL::UNKNOWN_TYPE;


        case 9:
          if (0 == str.compare(0, prefixLen, NODE_STATE_STR()))
                return PLEXIL::NODE_STATE_TYPE;
          else
                return PLEXIL::UNKNOWN_TYPE;

        case 11:
          if (0 == str.compare(0, prefixLen, NODE_OUTCOME_STR()))
                return PLEXIL::OUTCOME_TYPE;
          else if (0 == str.compare(0, prefixLen, NODE_FAILURE_STR()))
                return PLEXIL::FAILURE_TYPE;
          else
                return PLEXIL::UNKNOWN_TYPE;

        case 17:
          if (0 == str.compare(0, prefixLen, NODE_COMMAND_HANDLE_STR()))
                return PLEXIL::COMMAND_HANDLE_TYPE;
          else
                return PLEXIL::UNKNOWN_TYPE;
      
          // default case
        default:
          return PLEXIL::UNKNOWN_TYPE;
        }
  }

  const std::string& PlexilParser::valueTypeString(const ValueType& typ)
  {
    switch (typ)
      {
      case PLEXIL::INTEGER_TYPE:
        return INTEGER_STR();
      case PLEXIL::REAL_TYPE:
        return REAL_STR();
      case PLEXIL::BOOLEAN_TYPE:
        return BOOL_STR();
      case PLEXIL::STRING_TYPE:
        return STRING_STR();
      case PLEXIL::DATE_TYPE:
        return DATE_STR();
      case PLEXIL::DURATION_TYPE:
        return DURATION_STR();
      case PLEXIL::ARRAY:
        return ARRAY_STR();
      case PLEXIL::NODE_STATE_TYPE:
        return NODE_STATE_STR();
      case PLEXIL::OUTCOME_TYPE:
        return NODE_OUTCOME_STR();
      case PLEXIL::FAILURE_TYPE:
        return NODE_FAILURE_STR();
      case PLEXIL::COMMAND_HANDLE_TYPE:
        return NODE_COMMAND_HANDLE_STR();

      default:
        return UNKNOWN_STR();
      }
  }


  // Trim whitespace when setting expression names
  void PlexilExpr::setName(const std::string& name)
  {
    m_name = name;
    size_t nonBlank = m_name.find_first_not_of(PlexilParser::WHITESPACE_CHARS());
    if (nonBlank != 0)
      m_name.erase(0, nonBlank);
    nonBlank = m_name.find_last_not_of(PlexilParser::WHITESPACE_CHARS());
    if (nonBlank + 1 < m_name.length())
      m_name.erase(nonBlank + 1);
  }


  void PlexilState::setName(const std::string& name)
  {
    PlexilValue* pv = new PlexilValue(PLEXIL::STRING, name);
    setNameExpr(pv->getId());
  }
   
  const std::string& PlexilState::name() const
  {
    if (Id<PlexilValue>::convertable(m_nameExpr))
      return ((PlexilValue*)&(*m_nameExpr))->value();
    return m_nameExpr->name();
  }
   
  PlexilNode::PlexilNode()
    : m_id(this),
      m_priority(WORST_PRIORITY),
      m_lineNo(0),
      m_colNo(0),
      m_nodeType(NodeType_uninitialized)
  {}

  PlexilNode::~PlexilNode() {
    //delete everything here
    if (m_intf.isId())
      delete (PlexilInterface*) m_intf;
    m_intf = PlexilInterfaceId::noId();
    if (m_nodeBody.isId())
      delete (PlexilNodeBody*) m_nodeBody;
    m_nodeBody = PlexilNodeBodyId::noId();
    for (std::vector<std::pair<PlexilExprId, std::string> >::iterator it = m_conditions.begin();
         it != m_conditions.end();
         ++it) {
      delete (PlexilExpr*) it->first;
    }
    m_conditions.clear();
    for (std::vector<PlexilVarId>::iterator it = m_declarations.begin();
         it != m_declarations.end();
         ++it) {
      delete (PlexilVar*) *it;
    }
    m_declarations.clear();
    m_id.remove();
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
      const Id<PlexilLibNodeCallBody> callBody = (const Id<PlexilLibNodeCallBody>) m_nodeBody;
          // FIXME: move check up into XML parser
      checkError(callBody.isId(),
                 "PlexilNode::getLibraryReferences: node is not a library call node");
      insertUnique(refs, callBody->libNodeName());
      break;
    }

    case NodeType_NodeList: {
      const Id<PlexilListBody> listBody = (const Id<PlexilListBody>) m_nodeBody;
          // FIXME: move check up into XML parser
      checkError(listBody.isId(),
                 "PlexilNode::getLibraryReferences: node is not a list node");
      const std::vector<PlexilNodeId>& kids = listBody->children();
      for (std::vector<PlexilNodeId>::const_iterator it = kids.begin();
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
    m_id.remove();
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
         var != m_in.end(); ++var)
      {
        if (target == (*var)->name())
          return *var;
      }
    return NULL;
  }
  // find a variable in the set of InOut variables

  const PlexilVarRef* PlexilInterface::findInOutVar(const std::string& target)
  {
    for (std::vector<PlexilVarRef*>::const_iterator var = m_inOut.begin();
         var != m_inOut.end(); ++var)
      {
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
         

  PlexilValue::PlexilValue(const ValueType& type, const std::string& value)
    : PlexilExpr(), m_value(value), m_type(type)
  {
    // FIXME: this computes a string that could be a constant
    setName(PlexilParser::valueTypeString(m_type) + "Value");
  }

  PlexilArrayValue::PlexilArrayValue(
                                     const ValueType& type,
                                     unsigned maxSize,
                                     const std::vector<std::string>& values)
    : PlexilValue(type), m_maxSize(maxSize), m_values(values)
  {
    // Handle special case of string array
    setName(type == STRING ? "StringArrayValue" : "ArrayValue");
  }

  void PlexilVarRef::setVariable(const PlexilVarId& var)
  {
        m_variable = var;
        this->setName(var->name());
        setType(var->type());
        if (var->value() != NULL) {
          setDefaultValue(var->value()->getId());
        }
  }
   
  PlexilVar::PlexilVar(const std::string& name, const ValueType& type)
    : PlexilExpr(),
          m_type(type),
      m_varId(this, PlexilExpr::getId()), 
      m_value(NULL)
  {
        setName(name);
  }
   
  PlexilVar::PlexilVar(const std::string& name, const ValueType& type, 
                                           const std::string& value)
    : PlexilExpr(),
          m_type(type),
      m_varId(this, PlexilExpr::getId()), 
      m_value(new PlexilValue(type, value))
  {
        setName(name);
  }
   
  PlexilVar::PlexilVar(const std::string& name, const ValueType& type, 
                       PlexilValue* value)
    : PlexilExpr(),
          m_type(type),
      m_varId(this, PlexilExpr::getId()),
      m_value(value)
  {
        setName(name);
  }
   
  PlexilVar::~PlexilVar()
  {
    if (m_value)
      delete m_value;
    m_varId.removeDerived(PlexilExpr::getId());
  }
   

  PlexilArrayVar::PlexilArrayVar(const std::string& name, 
                                 const ValueType& type, 
                                 const unsigned maxSize)
    : PlexilVar(name, type, NULL),
      m_maxSize(maxSize)
  {
  }

  PlexilArrayVar::PlexilArrayVar(const std::string& name, 
                                 const ValueType& type, 
                                 const unsigned maxSize, 
                                 std::vector<std::string>& values)
    : PlexilVar(name, type, new PlexilArrayValue(type, maxSize, values)),
      m_maxSize(maxSize)
  {
  }
   
  PlexilArrayVar::~PlexilArrayVar() 
  {
  }


  PlexilArrayElement::PlexilArrayElement()
    : PlexilExpr()
  {
    setName("ArrayElement");
  }

  void PlexilArrayElement::setArrayName(const std::string& name)
  {
    m_arrayName = name;
  }

  PlexilVarRef::~PlexilVarRef()
  {
    if (m_defaultValue.isId())
      delete (PlexilExpr*) m_defaultValue;
  }

  // wrapper call for link which creates the seen library nodes
  // data structure before calling the recursive linker

  bool PlexilNode::link(const std::map<std::string, PlexilNodeId>& libraries)
  {
    PlexilNodeSet seen;
    bool result = link(libraries, seen);
    return result;
  }

  // Resolve links between the plan and a library node.
  // Returns false if there is a circular library reference
  // or if there are unresolved library node calls present after linking is completed,
  // true otherwise.
   
  bool PlexilNode::link(const std::map<std::string, PlexilNodeId>& libraries, PlexilNodeSet& seen)
  {
    if (nodeType() == NodeType_LibraryNodeCall) {
      Id<PlexilLibNodeCallBody> & body = (Id<PlexilLibNodeCallBody> &)m_nodeBody;

      // find the referenced library
      std::map<std::string, PlexilNodeId>::const_iterator libraryIt = 
        libraries.find(body->libNodeName());
      if (libraryIt == libraries.end()) {
        // Report unresolved library call error
        debugMsg("PlexilPlan:link", "Unresolved library call: " << body->libNodeName());
        return false;
      }

      // found it -- test for a circular library reference
      PlexilNodeId library = libraryIt->second;
      for (PlexilNodeSet::iterator seenLib = seen.begin(); 
           seenLib != seen.end();
           ++seenLib)
        {
          if (*seenLib == library.operator->()) {
            // TODO: show entire chain of references
            debugMsg("PlexilPlan:link",
                     " Circular library reference: "
                     << body->libNodeName());
            return false;
          }
        }

      // link the the two nodes
      body->setLibNode(library);

      // add this to the seen library nodes
      seen.push_back(library.operator->());

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
      Id<PlexilListBody> & body = (Id<PlexilListBody> &)m_nodeBody;
      const std::vector<PlexilNodeId>& children = body->children();
      for(std::vector<PlexilNodeId>::const_iterator child = children.begin();
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
}
