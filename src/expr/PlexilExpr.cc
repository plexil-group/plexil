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

#include "PlexilExpr.hh"

namespace PLEXIL
{
  //
  // PlexilExpr
  //
  PlexilExpr::PlexilExpr() 
    : m_id(this),
      m_lineNo(0),
      m_colNo(0)
  {
  }

  PlexilExpr::~PlexilExpr() 
  {
    m_id.remove();
  }

  const PlexilExprId& PlexilExpr::getId() const
  {
    return m_id;
  }
  
  const std::string& PlexilExpr::name() const 
  {
    return m_name;
  }
  
  int PlexilExpr::lineNo() const 
  {
    return m_lineNo;
  }
  
  int PlexilExpr::colNo() const 
  {
    return m_colNo;
  }

  // FIXME: move this to common location, IF anything else needs it
  static char const *WHITESPACE_CHARS = " \t\n";

  void PlexilExpr::setName(const std::string& name)
  {
    m_name = name;
    size_t nonBlank = m_name.find_first_not_of(WHITESPACE_CHARS);
    if (nonBlank != 0)
      m_name.erase(0, nonBlank);
    nonBlank = m_name.find_last_not_of(WHITESPACE_CHARS);
    if (nonBlank + 1 < m_name.length())
      m_name.erase(nonBlank + 1);
  }
  
  void PlexilExpr::setLineNo(int n)
  {
    m_lineNo = n;
  }
  
  void PlexilExpr::setColNo(int n) 
  {
    m_colNo = n;
  }

  //
  // PlexilVarRef
  //
  PlexilVarRef::PlexilVarRef()
    : PlexilExpr(),
      m_defaultValue(PlexilExprId::noId()),
      m_type(UNKNOWN_TYPE),
      m_typed(false)
  {
  }

  PlexilVarRef::~PlexilVarRef()
  {
    if (m_defaultValue.isId())
      delete (PlexilExpr*) m_defaultValue;
  }

  bool PlexilVarRef::typed() const 
  {
    return m_typed;
  }
  
  ValueType PlexilVarRef::type() const 
  {
    return m_type;
  }
  
  const PlexilExprId& PlexilVarRef::defaultValue() const 
  {
    return m_defaultValue;
  }
  
  const PlexilVarId& PlexilVarRef::variable() const 
  {
    return m_variable;
  }
         
  void PlexilVarRef::setDefaultValue(const PlexilExprId& defaultValue)
  {
    m_defaultValue = defaultValue;
  }

  void PlexilVarRef::setType(ValueType type)
  {
    m_type = type; 
    m_typed = true;
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

  //
  // PlexilOp
  //

  PlexilOp::PlexilOp()
    : PlexilExpr()
  {
  }
  
  PlexilOp::~PlexilOp()
  {
    for (std::vector<PlexilExprId>::iterator it = m_subExprs.begin();
         it != m_subExprs.end();
         ++it)
      delete (PlexilExpr*) *it;
    m_subExprs.clear();
  }

  const std::string& PlexilOp::getOp() const
  {
    return m_op;
  }
  
  void PlexilOp::setOp(const std::string& op)
  {
    m_op = op;
    setName(op);
  }

  const std::vector<PlexilExprId>& PlexilOp::subExprs() const 
  {
    return m_subExprs;
  }
  
  void PlexilOp::addSubExpr(PlexilExprId expr)
  {
    m_subExprs.push_back(expr);
  }

  //
  // PlexilArrayElement
  //

  PlexilArrayElement::PlexilArrayElement()
    : PlexilExpr()
  {
    setName("ArrayElement");
  }
  
  PlexilArrayElement::~PlexilArrayElement()
  {
    for (std::vector<PlexilExprId>::iterator it = m_subExprs.begin();
         it != m_subExprs.end();
         ++it)
      delete (PlexilExpr*) *it;
    m_subExprs.clear();
  }

  const std::string& PlexilArrayElement::getArrayName() const
  {
    return m_arrayName;
  }
  
  void PlexilArrayElement::setArrayName(const std::string& name)
  {
    m_arrayName = name;
  }

  const std::vector<PlexilExprId>& PlexilArrayElement::subExprs() const
  {
    return m_subExprs;
  }
  
  void PlexilArrayElement::addSubExpr(PlexilExprId expr)
  {
    m_subExprs.push_back(expr);
  }

  //
  // PlexilValue
  //

  PlexilValue::PlexilValue(ValueType type, const std::string& value)
    : PlexilExpr(),
      m_value(value),
      m_type(type)
  {
    // FIXME: this computes a string that could be a constant
    setName(plexilTypeName(m_type) + "Value");
  }

  PlexilValue::~PlexilValue()
  {
  }

  ValueType PlexilValue::type() const
  {
    return m_type;
  }

  const std::string& PlexilValue::value() const
  {
    return m_value;
  }

  //
  // PlexilArrayValue
  //   
         
  PlexilArrayValue::PlexilArrayValue(ValueType type,
                                     unsigned maxSize,
                                     const std::vector<std::string>& values)
    : PlexilValue(type),
      m_maxSize(maxSize),
      m_values(values)
  {
    // FIXME: no longer needed
    // Handle special case of string array
    setName(type == STRING_TYPE ? "StringArrayValue" : "ArrayValue");
  }

  PlexilArrayValue::~PlexilArrayValue()
  {
  } 

  const std::vector<std::string>& PlexilArrayValue::values() const
  {
    return m_values;
  }

  unsigned PlexilArrayValue::maxSize() const
  {
    return m_maxSize;
  }

  //
  // PlexilVar
  //
   
  PlexilVar::PlexilVar(const std::string& name,
                       ValueType type)
    : PlexilExpr(),
      m_type(type),
      m_varId(this, PlexilExpr::getId()), 
      m_value(NULL)
  {
    setName(name);
  }
   
  PlexilVar::PlexilVar(const std::string& name,
                       ValueType type, 
                       const std::string& value)
    : PlexilExpr(),
      m_type(type),
      m_varId(this, PlexilExpr::getId()), 
      m_value(new PlexilValue(type, value))
  {
    setName(name);
  }
   
  PlexilVar::PlexilVar(const std::string& name, 
                       ValueType type, 
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

  bool PlexilVar::isArray() const
  {
    return false;
  }

  const PlexilVarId& PlexilVar::getId() const 
  {
    return m_varId;
  }

  ValueType PlexilVar::type() const
  {
    return m_type;
  }

  const std::string& PlexilVar::factoryTypeString() const
  {
    return plexilTypeName(m_type);
  }

  const PlexilValue* PlexilVar::value() const
  {
    return m_value;
  }

  //
  // PlexilArrayVar
  //

  PlexilArrayVar::PlexilArrayVar(const std::string& name, 
                                 ValueType type, 
                                 const unsigned maxSize)
    : PlexilVar(name, type, NULL),
      m_maxSize(maxSize)
  {
  }

  PlexilArrayVar::PlexilArrayVar(const std::string& name, 
                                 ValueType type, 
                                 const unsigned maxSize, 
                                 std::vector<std::string>& values)
    : PlexilVar(name, type, new PlexilArrayValue(type, maxSize, values)),
      m_maxSize(maxSize)
  {
  }
   
  PlexilArrayVar::~PlexilArrayVar() 
  {
  }

  ValueType PlexilArrayVar::type() const
  {
    return arrayType(m_type);
  }

  const std::string& PlexilArrayVar::factoryTypeString() const 
  {
    return plexilTypeName(arrayType(m_type));
  }
  bool PlexilArrayVar::isArray() const
  {
    return true;
  }
  ValueType PlexilArrayVar::elementType() const 
  {
    return m_type;
  }
  
  unsigned PlexilArrayVar::maxSize() const 
  {
    return m_maxSize;
  }

} // namespace PLEXIL
