/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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
#include "Utils.hh"
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
    else if (typeName == FUNCTION())
      return NodeType_FunctionCall;
    else if (typeName == UPDATE())
      return NodeType_Update;
    else if (typeName == REQUEST())
      return NodeType_Request;
    else if (typeName == EMPTY())
      return NodeType_Empty;
    else if (typeName == LIBRARYNODECALL())
      return NodeType_LibraryNodeCall;
    
    checkError(ALWAYS_FAIL,
               "No node type named \"" << typeName << '\"');
    return NodeType_error;
  }

  const std::string& PlexilParser::nodeTypeString(PlexilNodeType nodeType)
  {
    static const std::string* errorReturn = NULL;
    if (errorReturn == NULL)
      errorReturn = new std::string("Unknown Node Type");

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
      case NodeType_FunctionCall:
        return PlexilParser::FUNCTION();
        break;
      case NodeType_Update:
        return PlexilParser::UPDATE();
        break;
      case NodeType_Request:
        return PlexilParser::REQUEST();
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
        break;
      }
    return *errorReturn;
  }

  PlexilType PlexilParser::parseValueType(const std::string & str)
  {
    if (str == INTEGER_STR())
      return PLEXIL::INTEGER;
    if (str == REAL_STR())
      return PLEXIL::REAL;
    if (str == BOOL_STR())
      return PLEXIL::BOOLEAN;
    if (str == BLOB_STR())
      return PLEXIL::BLOB;
    if (str == ARRAY_STR())
      return PLEXIL::ARRAY;
    if (str == STRING_STR())
      return PLEXIL::STRING;
    if (str == TIME_STR())
      return PLEXIL::TIME;
    if (str == NODE_STATE_STR())
      return PLEXIL::NODE_STATE;
    if (str == NODE_OUTCOME_STR())
      return PLEXIL::NODE_OUTCOME;
    if (str == NODE_FAILURE_STR())
      return PLEXIL::FAILURE_TYPE;
    if (str == NODE_COMMAND_HANDLE_STR())
      return PLEXIL::COMMAND_HANDLE;
      
    // default case
    checkError(ALWAYS_FAIL, "Invalid value type name '" << str << "'");
    return PLEXIL::UNKNOWN_TYPE;
  }

  const std::string& PlexilParser::valueTypeString(const PlexilType& typ)
  {
    switch (typ)
      {
      case PLEXIL::INTEGER:
	return INTEGER_STR();
      case PLEXIL::REAL:
	return REAL_STR();
      case PLEXIL::BOOLEAN:
	return BOOL_STR();
      case PLEXIL::BLOB:
	return BLOB_STR();
      case PLEXIL::STRING:
	return STRING_STR();
      case PLEXIL::TIME:
	return TIME_STR();
      case PLEXIL::ARRAY:
	return ARRAY_STR();
      case PLEXIL::NODE_STATE:
	return NODE_STATE_STR();
      case PLEXIL::NODE_OUTCOME:
	return NODE_OUTCOME_STR();
      case PLEXIL::FAILURE_TYPE:
	return NODE_FAILURE_STR();
      case PLEXIL::COMMAND_HANDLE:
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
    : m_lineNo(0),
      m_colNo(0),
      m_priority(WORST_PRIORITY),
      m_id(this)
  {}

  PlexilNode::~PlexilNode() {
    m_id.remove();
    //delete everything here
    //     cleanup(m_declarations);
    //     cleanup(m_conditions);
    //     if(m_nodeBody.isValid())
    //       delete m_nodeBody;
    //     if(m_intf.isValid())
    //       delete m_intf;
  }

  /**
   * @brief Get the names of all library nodes referenced by this node and its descendants.
   * @return A vector of library node names.
   */
  std::set<std::string> PlexilNode::getLibraryReferences() const {
    std::set<std::string> result;
    this->getLibraryReferences(result);
    debugMsg("PlexilNode:getLibraryReferences", " found " << result.size() << " unique library references");
    return result;
  }

  /**
   * @brief Get the names of all library nodes referenced by this node and its descendants.
   * @param The vector of referenced library node names to be returned.
   */
  void PlexilNode::getLibraryReferences(std::set<std::string>& refs) const {
    switch (m_nodeType) {

    case NodeType_LibraryNodeCall: {
      const Id<PlexilLibNodeCallBody> callBody = (const Id<PlexilLibNodeCallBody>) m_nodeBody;
      checkError(callBody.isId(),
		 "PlexilNode::getLibraryReferences: node is not a library call node");
      refs.insert(callBody->libNodeName());
      break;
    }

    case NodeType_NodeList: {
      const Id<PlexilListBody> listBody = (const Id<PlexilListBody>) m_nodeBody;
      checkError(listBody.isId(),
		 "PlexilNode::getLibraryReferences: node is not a list node");
      const std::vector<PlexilNodeId>& kids = listBody->children();
      for (std::vector<PlexilNodeId>::const_iterator it = kids.begin();
	   it != kids.end();
	   it++) {
	(*it)->getLibraryReferences(refs);
      }
      break;
    }

    default:
      break;
    }
  }

  PlexilInterface::~PlexilInterface() {
    m_id.remove();
    //     cleanup(m_in);
    //     cleanup(m_inOut);
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
         

  PlexilValue::PlexilValue(const PlexilType& type, const std::string& value)
    : PlexilExpr(), m_type(type), m_value(value)
  {
    checkError(!value.empty() || type == STRING,
	       "PlexilValue constructor: attempt to construct empty "
	       << PlexilParser::valueTypeString(type)
	       << " value");
    setName(PlexilParser::valueTypeString(m_type) + "Value");
  }

  PlexilArrayValue::PlexilArrayValue(
				     const PlexilType& type,
				     unsigned maxSize,
				     const std::vector<std::string>& values)
    : PlexilValue(type), m_maxSize(maxSize), m_values(values)
  {
    setName("ArrayValue");
  }
   
  PlexilVar::PlexilVar(const std::string& name, const PlexilType& type, 
		       const std::string& value)
    : m_lineNo(0), 
      m_colNo(0), 
      m_type(type),
      m_id(this), 
      m_name(name), 
      m_value(new PlexilValue(type, value))
  {
  }
   
  PlexilVar::PlexilVar(const std::string& name, const PlexilType& type, 
		       PlexilValue* value)
    : m_lineNo(0),
      m_colNo(0),
      m_type(type),
      m_id(this),
      m_name(name),
      m_value(value)
  {
  }
   
  PlexilVar::~PlexilVar()
  {
    m_id.remove();
  }
   

  PlexilArrayVar::PlexilArrayVar(const std::string& name, 
				 const PlexilType& type, 
				 const unsigned maxSize, 
				 std::vector<std::string>& values)
    : PlexilVar(name, type, new PlexilArrayValue(type, maxSize, values)),
      m_maxSize(maxSize)
  {
    checkError(values.size() <= m_maxSize,
	       "Number of initial values of " << type << 
	       " array variable \'" << name << 
	       "\' exceeds maximun of " << m_maxSize);
  }
   
  PlexilArrayVar::~PlexilArrayVar() 
  {
    //m_id.remove();
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


  // wrapper call for link which creates the seen library nodes
  // data structure before calling the recursive linker

  bool PlexilNode::link(const std::vector<PlexilNodeId>& libraries)
  {
    PlexilNodeSet seen;
    bool result = link(libraries, seen);
	return result;
  }

  // Resolve links between the plan and a library node.
  // Returns false if there is a circular library reference
  // or if there are unresolved library node calls present after linking is completed,
  // true otherwise.
   
  bool PlexilNode::link(const std::vector<PlexilNodeId>& libraries, PlexilNodeSet& seen)
  {
    if (nodeType() == NodeType_LibraryNodeCall) {
      Id<PlexilLibNodeCallBody> & body = (Id<PlexilLibNodeCallBody> &)m_nodeBody;

      // find the referenced library
      PlexilNodeId library;
      for (std::vector<PlexilNodeId>::const_iterator libraryIt = libraries.begin();
           libraryIt != libraries.end();
           ++libraryIt) {
        if (body->libNodeName() == (*libraryIt)->nodeId()) {
          library = *libraryIt;
          break;
        }
      }
      if (library.isNoId()) {
		// Report unresolved library call error
		debugMsg("PlexilPlan:link", "Unresolved library call: " << body->libNodeName());
		return false;
	  }

	  // found it -- test for a circular library reference
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
